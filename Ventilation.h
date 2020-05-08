#pragma once
#include "IDevice.h"
#include <chrono>
#include <memory>

class IInverterInfo;
class Relai;

//Abluft bzw Zuluft immer wenn batterie geladen wird und 30 min danach und alle 2 stunden
class Ventilation final : public IDevice
{
public:
	explicit Ventilation(IInverterInfo& info);
	~Ventilation();

	virtual void Update() override;
	virtual void TestFunctionality() override;
private:
	const IInverterInfo& m_inverterInfo;
	
	using ClockT = std::chrono::high_resolution_clock;
	ClockT::time_point m_lastTimeBatteryWasLoading;
	ClockT::time_point m_lastTimeVentilationWasRunning;
	std::unique_ptr<Relai> m_pVentilationRelai;
	bool m_isRunning = false;
};

