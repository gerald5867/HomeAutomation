#pragma once
#include <wiringPi.h>
#include <chrono>
#include <thread>
#include "GPIOPorts.h"
#include "Logger.h"

class Relai final {
public:
	explicit Relai(GPIOPorts gpioPort) : m_port(gpioPort) { 
		pinMode(static_cast<int>(gpioPort), OUTPUT); 
		TurnOff();
	}
	void TurnOn() { digitalWrite(static_cast<int>(m_port), LOW); }
	void TurnOff() { digitalWrite(static_cast<int>(m_port), HIGH); }
	void Test(std::chrono::seconds testPeriod = std::chrono::seconds{5}) {
		TurnOn();
		LOG_INFO("Testing relai on GPIO : [{0}]", static_cast<int>(m_port));
		std::this_thread::sleep_for(testPeriod);
		TurnOff();
	}
private:
	GPIOPorts m_port;
};