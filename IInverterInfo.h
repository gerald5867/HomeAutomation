#pragma once

class IInverterInfo {
public:
	virtual ~IInverterInfo() = default;
	virtual bool AnyInverterLoadsTheBattery() const = 0;
};