#include "ElektrolytPump.h"
#include "GPIOPorts.h"
#include "Relai.h"
#include "Inverter.h"


ElektrolytPump::ElektrolytPump(const std::vector<std::shared_ptr<Inverter>>& inverter)
: IDevice(60)
, m_pInverter(inverter) {
	m_pPumpRelai.reset(new Relai(GPIOPorts::ELEKTROLYT_PUMP_ON));
}


ElektrolytPump::~ElektrolytPump() = default;


void ElektrolytPump::Update() {
	auto now = ClockT::now();
	bool oneHourIsOver = std::chrono::duration_cast<std::chrono::hours>(now - m_lastRunTime).count() >= 1;
	if (!m_isRunning && BatteryIsLoading() && oneHourIsOver) {
		m_lastRunTime = now;
		m_pPumpRelai->TurnOn();
		m_isRunning = true;
	}
	static const std::chrono::minutes PumpRunTime(5);
	if (m_isRunning && std::chrono::duration_cast<std::chrono::minutes>(now - m_lastRunTime) >= PumpRunTime) {
		m_pPumpRelai->TurnOff();
		m_isRunning = false;
	}
}


bool ElektrolytPump::BatteryIsLoading() const {
	for (const auto& pInverter : m_pInverter) {
		if (pInverter->IsLoadingBattery()) {
			return true;
		}
	}
	return false;
}
