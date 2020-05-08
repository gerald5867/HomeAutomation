#include "Inverter.h"
#include <limits.h>
#include <stdint.h>
#include <thread>
#include <vector>
#include <string>
#include "serialib.h"
#include "Logger.h"
#include "nlohmann/json.hpp"
#include "GPIOPorts.h"
#include "Relai.h"

struct Inverter::Impl {
	nlohmann::json m_json;
};

std::vector<std::string> SplitString(const char* str, int len, char DELIM = ' ') {
	std::vector<std::string> result;
	int i = 0, lastSplitPos = 0;
	for (; i < len; ++i) {
		if (str[i] == DELIM) {
			result.emplace_back(std::string{ str + lastSplitPos, str + i });
			lastSplitPos = i + 1;
		}
	}
	result.emplace_back(std::string{ str + lastSplitPos, str + i });
	return result;
}

const std::vector<std::string> SplittedBatteryType = SplitString(BatteryTypeString, BatteryTypeLength, ',');
const std::vector<std::string> SplittedInputVoltageRange = SplitString(InputVoltageRangeString, InputVoltageRangeLength, ',');
const std::vector<std::string> SplittedOutputSourcePriority = SplitString(OutputSourcePriorityString, OutputSourcePriorityLength, ',');
const std::vector<std::string> SplittedChargerSourcePriority = SplitString(ChargerSourcePriorityString, ChargerSourcePriorityLength, ',');
const std::vector<std::string> SplittedMachineType = SplitString(MachineTypeString, MachineTypeLength, ',');
const std::vector<std::string> SplittedTopology = SplitString(TopologyString, TopologyLength, ',');
const std::vector<std::string> SplittedOutputMode = SplitString(OutputModeString, OutputModeLength, ',');
const std::vector<std::string> SplittedPvConditionParallelOK = SplitString(PvConditionParallelOKString, PvConditionParallelOKLength, ',');
const std::vector<std::string> SplittedPvPowerBalance = SplitString(PvPowerBalanceString, PvPowerBalanceLength, ',');


using INT16U = std::uint16_t;
using INT8U = std::uint8_t;

INT16U cal_crc_half(const INT8U *pin, INT8U len) {
	INT16U crc;
	INT8U da;
	const INT8U *ptr;
	INT8U bCRCHign;
	INT8U bCRCLow;
	INT16U crc_ta[16] =
	{
	0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
	0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef
	};
	ptr = pin;
	crc = 0;
	while (len-- != 0) {
		da = ((INT8U)(crc >> 8)) >> 4;
		crc <<= 4;
		crc ^= crc_ta[da ^ (*ptr >> 4)];
		da = ((INT8U)(crc >> 8)) >> 4;
		crc <<= 4;
		crc ^= crc_ta[da ^ (*ptr & 0x0f)];
		ptr++;
	}
	bCRCLow = crc;
	bCRCHign = (INT8U)(crc >> 8);
	if (bCRCLow == 0x28 || bCRCLow == 0x0d || bCRCLow == 0x0a) {
		bCRCLow++;
	}
	if (bCRCHign == 0x28 || bCRCHign == 0x0d || bCRCHign == 0x0a) {
		bCRCHign++;
	}
	crc = ((INT16U)bCRCHign) << 8;
	crc += bCRCLow;
	return(crc);
}


Inverter::Inverter(const std::string& usbPort) 
: IDevice(1)
, m_usbPort(usbPort)
, m_pSerialConnection(new serialib)
, m_updateJsonTimeOut(std::chrono::seconds{5})
, m_logJsonToFileTimeOut(std::chrono::minutes{10})
, m_qpigsTimeout(std::chrono::seconds{5})
, m_qpiriTimeout(std::chrono::seconds{60}) {
	m_pImpl.reset(new Impl());
	m_pGridACInRelai = std::make_unique<Relai>(GPIOPorts::AC_IN_ON);
	char errorOpening = m_pSerialConnection->openDevice(usbPort.c_str(), 2400);
	if (errorOpening != 1) {
		LOG_ERROR("Connecting to inverter was not success! errorcode : {0}", errorOpening);
	}
}


Inverter::~Inverter() {
	m_pSerialConnection->closeDevice();
}

void Inverter::Update() {
	UpdateQPIGS();
	UpdateQPIRI();

	UpdateJSON();
	LogJSON();
	m_isFirstUpdate = false;
}

void Inverter::UpdateQPIGS() {
	if (!m_qpigsTimeout.IsExpired() && !m_isFirstUpdate) {
		return;
	}
	m_qpigsTimeout.Reset();
	int watchdog = 5;
	m_lastQipgsResult = {};
	while (!m_lastQipgsResult && --watchdog >= 0) {
		WriteCommand("QPIGS");
		ReadCommand();
		TryParseQPIGSResult();
	}
}

//42 30
void Inverter::WriteCommand(const std::string& command) {
	m_cmdBuf.clear();
	m_cmdBuf.reserve(command.size() + 3);
	m_cmdBuf = command;
	auto crc = cal_crc_half(reinterpret_cast<const INT8U*>(m_cmdBuf.c_str()), m_cmdBuf.length());
	m_cmdBuf.append(1, (crc >> (char)8) & (char)0xff);
	m_cmdBuf.append(1, crc & (char)0xff);
	m_cmdBuf.append(1, (char)0xD);
	int sendbufres = m_pSerialConnection->writeBytes(m_cmdBuf.c_str(), m_cmdBuf.length());
	if (sendbufres <= 0) {
		LOG_ERROR("could not send any bytes to inverter sending command {0} failed!", command);
	}
}


void Inverter::ReadCommand() {
	m_cmdBuf.clear();
	m_cmdBuf.resize(4048);
	int n = m_pSerialConnection->readBytes(&m_cmdBuf[0], m_cmdBuf.length(), 1000);
	m_cmdBuf = m_cmdBuf.substr(1); // skip first byte "("
} 


void Inverter::TryParseQPIGSResult() {
	const std::vector<std::string>& splittedVec = SplitString(m_cmdBuf.data(), m_cmdBuf.length());
	if (splittedVec.size() < 20) {
		LOG_ERROR("Invalid result from inverter [{0}] for QPIGS request!!", m_usbPort);
		return;
	}
	try {
		m_lastQipgsResult.gridVoltage = std::atof(splittedVec[0].c_str());
		m_lastQipgsResult.gridFrequency = std::atof(splittedVec[1].c_str());
		m_lastQipgsResult.outputVoltage = std::atof(splittedVec[2].c_str());
		m_lastQipgsResult.outpotFreuency = std::atof(splittedVec[3].c_str());
		m_lastQipgsResult.outputApparentPower = std::stoi(splittedVec[4]);
		m_lastQipgsResult.outputActivePower = std::stoi(splittedVec[5]);
		m_lastQipgsResult.outputLoadPercent = std::stoi(splittedVec[6]);
		m_lastQipgsResult.BUSVoltage = std::stoi(splittedVec[7]);
		m_lastQipgsResult.batteryVoltage = std::stoi(splittedVec[8]);
		m_lastQipgsResult.batteryChargingCurrent = std::stoi(splittedVec[9]);
		m_lastQipgsResult.batteryCapacity = std::stoi(splittedVec[10]);
		m_lastQipgsResult.inverterHeatSinkTemeperature = std::stoi(splittedVec[11]);
		m_lastQipgsResult.PVInputCurrent = std::stoi(splittedVec[12]);
		m_lastQipgsResult.PVInputVoltage = std::atof(splittedVec[13].c_str());
		m_lastQipgsResult.batteryVoltageFromSCC = std::atof(splittedVec[14].c_str());
		m_lastQipgsResult.batteryDischargeCurrent = std::stoi(splittedVec[15]);

		m_lastQipgsResult.deviceStatus.addSBUPriority = splittedVec[16][0] - '0';
		m_lastQipgsResult.deviceStatus.configurationStatus = splittedVec[16][1] - '0';
		m_lastQipgsResult.deviceStatus.SCCFirmwareVersion = splittedVec[16][2] - '0';
		m_lastQipgsResult.deviceStatus.loadStatus = splittedVec[16][3] - '0';
		m_lastQipgsResult.deviceStatus.batteryVoltageToSteadyWhileCharging = splittedVec[16][4] - '0';
		m_lastQipgsResult.deviceStatus.ChargingStatus = splittedVec[16][5] - '0';
		m_lastQipgsResult.deviceStatus.ChargingStatusSCC1 = splittedVec[16][6] - '0';
		m_lastQipgsResult.deviceStatus.ChargingStatusAC = splittedVec[16][7] - '0';

		m_lastQipgsResult.batteryVoltageOffsetForFansOn = std::stoi(splittedVec[17]);
		m_lastQipgsResult.EEPROMversion = std::stoi(splittedVec[18]);
		m_lastQipgsResult.PVChargingPower = std::stoi(splittedVec[19]);
	}
	catch (const std::exception& ex) {
		LOG_ERROR("could not convert some data from inverter : {0}", ex.what());
	}
}


bool Inverter::IsLoadingBattery() const {
	return m_lastQipgsResult.batteryChargingCurrent > 5;
}


void Inverter::UpdateJSON() {
	if (!m_updateJsonTimeOut.IsExpired() && !m_isFirstUpdate) {
		return;
	}
	m_updateJsonTimeOut.Reset();
	auto& json = m_pImpl->m_json;
	json["outputApparentPower"] = m_lastQipgsResult.outputApparentPower;
	json["outputActivePower"] = m_lastQipgsResult.outputActivePower;
	json["outputLoadPercent"] = m_lastQipgsResult.outputLoadPercent;
	json["BUSVoltage"] = m_lastQipgsResult.BUSVoltage;
	json["batteryVoltage"] = m_lastQipgsResult.batteryVoltage;
	json["batteryChargingCurrent"] = m_lastQipgsResult.batteryChargingCurrent;
	json["inverterHeatSinkTemeperature"] = m_lastQipgsResult.inverterHeatSinkTemeperature;
	json["PVInputCurrent"] = m_lastQipgsResult.PVInputCurrent;
	json["PVInputVoltage"] = m_lastQipgsResult.PVInputVoltage;
	json["batteryVoltageFromSCC"] = m_lastQipgsResult.batteryVoltageFromSCC;
	json["batteryDischargeCurrent"] = m_lastQipgsResult.batteryDischargeCurrent;
	json["batteryDischargeCurrent"] = m_lastQipgsResult.batteryDischargeCurrent;

	json["deviceStatus.addSBUPriority"] = m_lastQipgsResult.deviceStatus.addSBUPriority;
	json["deviceStatus.configurationStatus"] = m_lastQipgsResult.deviceStatus.configurationStatus;
	json["deviceStatus.SCCFirmwareVersion"] = m_lastQipgsResult.deviceStatus.SCCFirmwareVersion;
	json["deviceStatus.loadStatus"] = m_lastQipgsResult.deviceStatus.loadStatus;
	json["deviceStatus.batteryVoltageToSteadyWhileCharging"] = m_lastQipgsResult.deviceStatus.batteryVoltageToSteadyWhileCharging;
	json["deviceStatus.ChargingStatus"] = m_lastQipgsResult.deviceStatus.ChargingStatus;
	json["deviceStatus.ChargingStatusSCC1"] = m_lastQipgsResult.deviceStatus.ChargingStatusSCC1;
	json["deviceStatus.ChargingStatusAC"] = m_lastQipgsResult.deviceStatus.ChargingStatusAC;

	json["PVChargingPower"] = m_lastQipgsResult.PVChargingPower;

	json["ACOutputRatingVoltage"] = m_lastQpiriResult.ACOutputRatingVoltage;
	json["ACOutputRatingFrequency"] = m_lastQpiriResult.ACOutputRatingFrequency;
	json["ACOutputRatingCurrent"] = m_lastQpiriResult.ACOutputRatingCurrent;
	json["ACOutputRatingApparentPower"] = m_lastQpiriResult.ACOutputRatingApparentPower;
	json["ACOutputRatingActivePower"] = m_lastQpiriResult.ACOutputRatingActivePower;
	json["batteryRatingVoltage"] = m_lastQpiriResult.batteryRatingVoltage;
	json["batteryReChargeVoltage"] = m_lastQpiriResult.batteryReChargeVoltage;
	json["batteryUnderVoltage"] = m_lastQpiriResult.batteryUnderVoltage;
	json["batteryBulkVoltage"] = m_lastQpiriResult.batteryBulkVoltage;
	json["batteryFloatVoltage"] = m_lastQpiriResult.batteryFloatVoltage;
	json["batteryType"] = SplittedBatteryType[(std::size_t)m_lastQpiriResult.batteryType];
	json["currentMaxACChargingCurrent"] = m_lastQpiriResult.currentMaxACChargingCurrent;
	json["currentMaxChargingCurrent"] = m_lastQpiriResult.currentMaxChargingCurrent;
	json["inputVoltageRange"] = m_lastQpiriResult.inputVoltageRange;
	json["outputSourcePrio"] = SplittedOutputSourcePriority[(std::size_t)m_lastQpiriResult.outputSourcePrio];
	json["chargerSourcePrio"] = SplittedChargerSourcePriority[(std::size_t)m_lastQpiriResult.chargerSourcePrio];
	json["parallelMaxNum"] = m_lastQpiriResult.parallelMaxNum;
	json["machineType"] = SplittedMachineType[(std::size_t)m_lastQpiriResult.machineType];
	json["topology"] = SplittedTopology[(std::size_t)m_lastQpiriResult.topology];
	json["outputMode"] = SplittedOutputMode[(std::size_t)m_lastQpiriResult.outputMode];
	json["batteryRedischargeVoltage"] = m_lastQpiriResult.batteryRedischargeVoltage;
	json["pvParallelCondition"] = SplittedPvConditionParallelOK[(std::size_t)m_lastQpiriResult.pvParallelCondition];
	json["pvPowerBalance"] = SplittedPvPowerBalance[(std::size_t)m_lastQpiriResult.pvPowerBalance];
}


void Inverter::LogJSON() {
	if (!m_logJsonToFileTimeOut.IsExpired() && !m_isFirstUpdate) {
		return;
	}
	m_logJsonToFileTimeOut.Reset();
	LOG_DEBUG("\nInverter [{0}][{1}]:", m_usbPort, SplittedOutputMode[(std::size_t)m_lastQpiriResult.outputMode]);
	LOG_DEBUG(m_pImpl->m_json.dump(2));
	utils::Logger::Instance().Flush();
}


void Inverter::UpdateQPIRI() {
	if (!m_qpiriTimeout.IsExpired() && !m_isFirstUpdate) {
		return;
	}
	m_qpiriTimeout.Reset();
	int watchdog = 5;
	m_lastQpiriResult = {};
	while (!m_lastQpiriResult && --watchdog >= 0) {
		WriteCommand("QPIRI");
		ReadCommand();
		TryParseQPIRIResult();
	}
}


void Inverter::TryParseQPIRIResult() {
	const std::vector<std::string>& splittedVec = SplitString(m_cmdBuf.data(), m_cmdBuf.length());
	if (splittedVec.size() < 25) {
		LOG_ERROR("Invalid result from inverter [{0}] for QPIRI request!!", m_usbPort);
		return;
	}
	try {
		m_lastQpiriResult.gridRatingVoltage 			= std::atof(splittedVec[0].c_str());
		m_lastQpiriResult.gridRatingCurrent 			= std::atof(splittedVec[1].c_str());
		m_lastQpiriResult.ACOutputRatingVoltage 		= std::atof(splittedVec[2].c_str());
		m_lastQpiriResult.ACOutputRatingFrequency 		= std::atof(splittedVec[3].c_str());
		m_lastQpiriResult.ACOutputRatingCurrent 		= std::atof(splittedVec[4].c_str());
		m_lastQpiriResult.ACOutputRatingApparentPower   = std::stoi(splittedVec[5]);
		m_lastQpiriResult.ACOutputRatingActivePower		= std::stoi(splittedVec[6]);
		m_lastQpiriResult.batteryRatingVoltage          = std::atof(splittedVec[7].c_str());
		m_lastQpiriResult.batteryReChargeVoltage		= std::atof(splittedVec[8].c_str());
		m_lastQpiriResult.batteryUnderVoltage			= std::atof(splittedVec[9].c_str());
		m_lastQpiriResult.batteryBulkVoltage			= std::atof(splittedVec[10].c_str());
		m_lastQpiriResult.batteryFloatVoltage			= std::atof(splittedVec[11].c_str());
		m_lastQpiriResult.batteryType					= static_cast<BatteryType>(std::stoi(splittedVec[12]));
		m_lastQpiriResult.currentMaxACChargingCurrent   = std::stoi(splittedVec[13]);
		m_lastQpiriResult.currentMaxChargingCurrent     = std::stoi(splittedVec[14]);
		m_lastQpiriResult.inputVoltageRange				= static_cast<InputVoltageRange>(std::stoi(splittedVec[15]));
		m_lastQpiriResult.outputSourcePrio				= static_cast<OutputSourcePriority>(std::stoi(splittedVec[16]));
		m_lastQpiriResult.chargerSourcePrio				= static_cast<ChargerSourcePriority>(std::stoi(splittedVec[17]));
		m_lastQpiriResult.parallelMaxNum				= std::stoi(splittedVec[18]);
		m_lastQpiriResult.machineType					= static_cast<MachineType>(std::stoi(splittedVec[19]));
		m_lastQpiriResult.topology					    = static_cast<Topology>(std::stoi(splittedVec[20]));	
		m_lastQpiriResult.outputMode					= static_cast<OutputMode>(std::stoi(splittedVec[21]));	
		m_lastQpiriResult.batteryRedischargeVoltage		= std::atof(splittedVec[22].c_str());
		m_lastQpiriResult.pvParallelCondition			= static_cast<PvConditionParallelOK>(std::stoi(splittedVec[23]));
		m_lastQpiriResult.pvPowerBalance				= static_cast<PvPowerBalance>(std::stoi(splittedVec[24]));
	}
	catch (const std::exception& ex) {
	LOG_ERROR("could not convert some data from inverter : {0}", ex.what());
	}
}


void Inverter::TestFunctionality() {
	m_pGridACInRelai->Test(std::chrono::seconds{3});
}
