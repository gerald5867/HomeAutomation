#pragma once
#include <string>
#include <cstring>
#include <memory>
#include "IDevice.h"

class serialib;

struct QPIGS_Result {
	float gridVoltage;
	float gridFrequency;
	float outputVoltage;
	float outpotFreuency;
	int outputApparentPower;
	int outputActivePower;
	int outputLoadPercent;
	int BUSVoltage;
	int batteryVoltage;
	int batteryChargingCurrent;
	int batteryCapacity;
	int inverterHeatSinkTemeperature;
	int PVInputCurrent;
	float PVInputVoltage;
	float batteryVoltageFromSCC;
	int batteryDischargeCurrent;
	struct {
		bool addSBUPriority;
		bool configurationStatus;
		bool SCCFirmwareVersion;
		bool loadStatus;
		bool batteryVoltageToSteadyWhileCharging;
		bool ChargingStatus;
		bool ChargingStatusSCC1;
		bool ChargingStatusAC;
		bool chargingStatusAllInOne[3]; //000: Do nothing 110: Charging on with SCC1 charge on 101 : Charging on with AC charge on 111 : Charging on with SCC1
	} deviceStatus;
	int batteryVoltageOffsetForFansOn;
	int EEPROMversion;
	int PVChargingPower;

	operator bool() const { 
		static const QPIGS_Result EmptyRes = {0};
		int res = std::memcmp(this, &EmptyRes, sizeof(QPIGS_Result));
		return res != 0;
	}
};

class Inverter final : public IDevice
{
public:
	explicit Inverter(const std::string& usbPort);
	~Inverter();
	bool IsLoadingBattery() const;

protected:
	virtual void Update() override;

private:
	void UpdateQPIGS();

	void WriteCommand(const std::string& command);
	void ReadCommand();

	void TryParseQPIGSResult();

	std::string m_usbPort;
	std::string m_cmdBuf;
	std::unique_ptr<serialib> m_pSerialConnection;
	QPIGS_Result m_lastQipgsResult = {};
};

