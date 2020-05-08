#include "Ventilation.h"
#include "IInverterInfo.h"
#include "GPIOPorts.h"
#include "Relai.h"
#include "Logger.h"
#include <thread>

const std::chrono::minutes VentilationTimeAfterBatteryWasLoading{ 30 };
const std::chrono::hours VentilationIntervalAlways{ 2 };


Ventilation::Ventilation(IInverterInfo& inf)
: IDevice(30)
, m_inverterInfo(inf)
, m_lastTimeBatteryWasLoading(ClockT::now() - VentilationTimeAfterBatteryWasLoading)
, m_lastTimeVentilationWasRunning(ClockT::now()) {
	m_pVentilationRelai = std::make_unique<Relai>(GPIOPorts::VENTILATION_ON);
}
Ventilation::~Ventilation() = default;


void Ventilation::Update() {
	const auto now =  ClockT::now();
	const bool batteryGetsLoaded = m_inverterInfo.AnyInverterLoadsTheBattery();
	const bool shouldStartEventWithoutBatteryIsLoading 
		= std::chrono::duration_cast<std::chrono::hours>(now - m_lastTimeVentilationWasRunning) >= VentilationIntervalAlways;
	if (batteryGetsLoaded) {
		m_lastTimeBatteryWasLoading = now;
		m_lastTimeVentilationWasRunning = now;
		m_pVentilationRelai->TurnOn();
		m_isRunning = true;
		LOG_INFO("Turning ventilation on because battery gets loaded!");
	}
	if (shouldStartEventWithoutBatteryIsLoading) {
		m_lastTimeVentilationWasRunning = now;
		m_pVentilationRelai->TurnOn();
		m_isRunning = true;
		LOG_INFO("Turning ventilation on because last time is too long ago!");
	}
	const bool VentilationTimeAfterBatteryWasLoadingIsOver =
		std::chrono::duration_cast<std::chrono::minutes>(now - m_lastTimeBatteryWasLoading) >= VentilationTimeAfterBatteryWasLoading;
	if (VentilationTimeAfterBatteryWasLoadingIsOver && !shouldStartEventWithoutBatteryIsLoading && m_isRunning) {
		m_pVentilationRelai->TurnOff();
		LOG_INFO("Turning ventilation off!");
		m_isRunning = false;
	}
}


void Ventilation::TestFunctionality() {
	m_pVentilationRelai->Test();
}
