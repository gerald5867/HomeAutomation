#pragma once
#include <wiringPi.h>
#include "GPIOPorts.h"

class Relai final {
public:
	explicit Relai(GPIOPorts gpioPort) : m_port(gpioPort) { 
		pinMode(static_cast<int>(gpioPort), OUTPUT); 
		TurnOff();
	}
	void TurnOn() { digitalWrite(static_cast<int>(m_port), LOW); }
	void TurnOff() { digitalWrite(static_cast<int>(m_port), HIGH); }
private:
	GPIOPorts m_port;
};