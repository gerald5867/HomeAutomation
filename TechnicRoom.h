#pragma once
#include <memory>
#include <chrono>
#include "IInverterInfo.h"

class Inverter;
class ElektrolytPump;
class Ventilation;

class TechnicRoom : public IInverterInfo
{
public:
	void Run();
	TechnicRoom();
	~TechnicRoom();

	virtual bool AnyInverterLoadsTheBattery() const override;
private:
	using ClockT = std::chrono::high_resolution_clock;
	ClockT::time_point m_lastLoggerFlush = ClockT::now();

	std::shared_ptr<Inverter> m_pInv1;
	std::shared_ptr<Inverter> m_pInv2;
	std::shared_ptr<Inverter> m_pInv3;
	std::shared_ptr<Inverter> m_pInv4;

	std::unique_ptr<ElektrolytPump> m_pPump;
	std::unique_ptr<Ventilation> m_pVentilation;
};

