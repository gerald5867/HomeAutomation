#pragma once

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
		static const QPIGS_Result EmptyRes = { 0 };
		int res = std::memcmp(this, &EmptyRes, sizeof(QPIGS_Result));
		return res != 0;
	}
};

#define ENUMCLASS_WITHSTRINGARR(name, ...) \
	enum class name { __VA_ARGS__ }; \
	constexpr const char name##String[] = #__VA_ARGS__; \
	constexpr std::size_t name##Length = sizeof(name##String) - 1;

ENUMCLASS_WITHSTRINGARR(BatteryType, AGM, Flooded, User);
ENUMCLASS_WITHSTRINGARR(InputVoltageRange, Appliance, UPS);
ENUMCLASS_WITHSTRINGARR(OutputSourcePriority , UtilityFirst, SolarFirst, SBUFirst );
ENUMCLASS_WITHSTRINGARR(ChargerSourcePriority , UtilityFirst, SolarFirst, SolarAndUtility, OnlySolarChargingPermitted );
ENUMCLASS_WITHSTRINGARR(MachineType , GridTie, OffGrid, Hybrid, OffGridWith2Trackers, OffGridWith3Trackers );
ENUMCLASS_WITHSTRINGARR(Topology , Transformerless, Transfomer );
ENUMCLASS_WITHSTRINGARR(OutputMode , SingleMachineOutput, ParallelOutput, Phase1Of3Phase, Phase2Of3Phase, Phase3Of3Phase );
ENUMCLASS_WITHSTRINGARR(PvConditionParallelOK , OneUnitHasPvConnectedOK, OnlyAllInvertersHaveConnectedPV );
ENUMCLASS_WITHSTRINGARR(PvPowerBalance , PvInputMaxCurrentWillBeMaxChargedCurrent, PvInputMaxPowerIsSumOfMaxChargedPowerAndLoadsPower );

struct QPIRI_Result {
	float gridRatingVoltage; 
	float gridRatingCurrent;
	float ACOutputRatingVoltage;
	float ACOutputRatingFrequency;
	float ACOutputRatingCurrent;
	int ACOutputRatingApparentPower;
	int ACOutputRatingActivePower;
	float batteryRatingVoltage;
	float batteryReChargeVoltage;
	float batteryUnderVoltage;
	float batteryBulkVoltage;
	float batteryFloatVoltage;
	BatteryType batteryType;
	int currentMaxACChargingCurrent;
	int currentMaxChargingCurrent;
	InputVoltageRange inputVoltageRange;
	OutputSourcePriority outputSourcePrio;
	ChargerSourcePriority chargerSourcePrio;
	int parallelMaxNum;
	MachineType machineType;
	Topology topology;
	OutputMode outputMode;
	float batteryRedischargeVoltage;
	PvConditionParallelOK pvParallelCondition;
	PvPowerBalance pvPowerBalance;

	operator bool() const {
		static const QPIRI_Result EmptyRes = { 0 };
		int res = std::memcmp(this, &EmptyRes, sizeof(QPIRI_Result));
		return res != 0;
	}
};

enum QMODResult {
	QMODResult_PowerOnMode = 'P',
	QMODResult_StandbyMode = 'S',
	QMODResult_LineMode = 'L',
	QMODResult_BatteryMode = 'B',
	QMODResult_FaultMode = 'F',
	QMODResult_PowerSavingMode = 'H'
};



struct QPIGS2Result {
	int pvInputCurrent2;
	float pvInputVoltage2;
	float batteryVoltageFromSCC2;
	int pvChargingPower2;
	struct DeviceStatus {
		bool chargingStatusSCC2;
		bool chargingStatusSCC3;
		bool reserved[6];
	} status;
	int ACChargingCurrent;
	int ACChargingPower;
	int pvInputCurrent3;
	float pvInputVoltage3;
	float batteryVoltageFromSCC3;
	int pvChargingPower3;
	int pvTotalChargingPower;
};