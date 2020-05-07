#pragma once
#include <vector>
#include <memory>
#include <chrono>
#include "IDevice.h"

class Inverter;
class Relai;
class ElektrolytPump final : public IDevice
{
public:
	explicit ElektrolytPump(const std::vector<std::shared_ptr<Inverter>>& inverter);
	~ElektrolytPump();
	virtual void Update() override;
private:
	bool BatteryIsLoading() const;

	using ClockT = std::chrono::high_resolution_clock;
	bool m_isRunning = false;
	ClockT::time_point m_lastRunTime = ClockT::now() - std::chrono::hours(1);
	std::unique_ptr<Relai> m_pPumpRelai;
	std::vector<std::shared_ptr<Inverter>> m_pInverter;
};

