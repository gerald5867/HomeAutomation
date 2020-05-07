#pragma once
#include <set>
#include <algorithm>
#include <chrono>

class IDevice {
protected:
	explicit IDevice(long long updateSeconds) : m_updateSeconds(updateSeconds) { s_devices.emplace(this); }
	virtual ~IDevice() { s_devices.erase(this); }
	virtual void Update() = 0;
private:
	friend class TechnicRoom;
	long long m_updateSeconds;
	inline static std::set<IDevice*> s_devices;
	using ClockT = std::chrono::high_resolution_clock;
	ClockT::time_point m_lastUpdate = ClockT::now();
};