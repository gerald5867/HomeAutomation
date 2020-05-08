#pragma once
#include <chrono>
#include <memory>
#include "IDevice.h"

class IInverterInfo;
class Relai;

class ElektrolytPump final : public IDevice
{
public:
	explicit ElektrolytPump(IInverterInfo& info);
	~ElektrolytPump();

	virtual void Update() override;
	virtual void TestFunctionality() override;
private:

	using ClockT = std::chrono::high_resolution_clock;
	bool m_isRunning = false;
	ClockT::time_point m_lastRunTime = ClockT::now() - std::chrono::hours(1);
	std::unique_ptr<Relai> m_pPumpRelai;

	const IInverterInfo& m_inverterInfo;
};

