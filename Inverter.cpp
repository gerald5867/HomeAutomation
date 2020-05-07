#include "Inverter.h"
#include <iostream>
#include <limits.h>
#include <stdint.h>
#include <thread>
#include <vector>
#include <string>
#include "serialib.h"

std::vector<std::string> SplitString(const char* str, int len, char DELIM = ' ') {
	std::vector<std::string> result;
	for (int i = 0, lastSplitPos = 0; i < len; ++i) {
		if (str[i] == DELIM) {
			result.emplace_back(std::string{ str + lastSplitPos, str + i });
			lastSplitPos = i + 1;
		}
	}
	return result;
}


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
, m_pSerialConnection(new serialib) {
	char errorOpening = m_pSerialConnection->openDevice(usbPort.c_str(), 2400);
	if (errorOpening != 1) {
		std::cout << "ERROR Connecting to inverter was not success! errorcode : " << errorOpening;
	}
}


Inverter::~Inverter() {
	m_pSerialConnection->closeDevice();
}

void Inverter::Update() {
	UpdateQPIGS();
}

void Inverter::UpdateQPIGS() {
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
		std::cout << "ERROR sending bytes via rs232!!!" << std::endl;
	}
}


void Inverter::ReadCommand() {
	m_cmdBuf.clear();
	m_cmdBuf.resize(4048);
	int n = m_pSerialConnection->readBytes(&m_cmdBuf[0], m_cmdBuf.length(), 1000);
} 


void Inverter::TryParseQPIGSResult() {
	const std::vector<std::string>& splittedVec = SplitString(m_cmdBuf.data(), m_cmdBuf.length());
	if (splittedVec.size() != 20) {
		return;
	}
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


bool Inverter::IsLoadingBattery() const {
	return m_lastQipgsResult.batteryChargingCurrent > 10;
}
