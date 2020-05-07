#pragma once
enum class GPIOPorts {
	HVAC_ON = 7,
	AC_IN_ON = 8,
	RESERVED_RELAI = 3,
	VENTILATION_ON = 16,
	ELEKTROLYT_PUMP_ON = 1,
	HVAC_COOLING_ON = 9,
	HVAC_HEATING_ON = 10, // NOTE FOR Heating also HVAC_COOLING_ON must be turned on!!
};