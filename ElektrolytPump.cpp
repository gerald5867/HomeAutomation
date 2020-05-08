#include "ElektrolytPump.h"
#include "GPIOPorts.h"
#include "Relai.h"
#include "Inverter.h"
#include "Logger.h"
#include "IInverterInfo.h"


ElektrolytPump::ElektrolytPump(IInverterInfo& inverterInfo)
: IDevice(60)
, m_inverterInfo(inverterInfo) {
	m_pPumpRelai.reset(new Relai(GPIOPorts::ELEKTROLYT_PUMP_ON));
}


ElektrolytPump::~ElektrolytPump() = default;


void ElektrolytPump::Update() {
	auto now = ClockT::now();
	bool oneHourIsOver = std::chrono::duration_cast<std::chrono::hours>(now - m_lastRunTime).count() >= 1;
	bool batteryIsLoading = m_inverterInfo.AnyInverterLoadsTheBattery();
	if (!m_isRunning && batteryIsLoading && oneHourIsOver) {
		LOG_INFO("Starting ElectrolytPump IsRunning : {0} BatteryIsLoading : {1} OneHourIsOver : {2}", m_isRunning, batteryIsLoading, oneHourIsOver);
		m_lastRunTime = now;
		m_pPumpRelai->TurnOn();
		m_isRunning = true;
	}
	static const std::chrono::minutes PumpRunTime(5);
	if (m_isRunning && std::chrono::duration_cast<std::chrono::minutes>(now - m_lastRunTime) >= PumpRunTime) {
		LOG_INFO("Stopping ElectrolytPump IsRunning : {0} BatteryIsLoading : {1} OneHourIsOver : {2}", m_isRunning, batteryIsLoading, oneHourIsOver);
		m_pPumpRelai->TurnOff();
		m_isRunning = false;
	}
}


void ElektrolytPump::TestFunctionality() {
	m_pPumpRelai->Test();
}

