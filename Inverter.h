#pragma once
#include <string>
#include <cstring>
#include <memory>
#include "IDevice.h"
#include "InverterCmdResults.h"
#include "TimeoutClock.h"

class serialib;
class Relai;

class Inverter final : public IDevice
{
public:
	explicit Inverter(const std::string& usbPort);
	~Inverter();
	bool IsLoadingBattery() const;

protected:
	virtual void Update() override;
	virtual void TestFunctionality() override;

private:
	void UpdateQPIGS();
	void UpdateQPIRI();
	void UpdateJSON();
	void LogJSON();

	void WriteCommand(const std::string& command);
	void ReadCommand();

	void TryParseQPIGSResult();
	void TryParseQPIRIResult();

	bool m_isFirstUpdate = true;

	std::string m_usbPort;
	std::string m_cmdBuf;

	std::unique_ptr<serialib> m_pSerialConnection;
	std::unique_ptr<Relai> m_pGridACInRelai;

	utils::TimeOutClockSeconds m_updateJsonTimeOut;
	utils::TimeOutClockMinutes m_logJsonToFileTimeOut;

	utils::TimeOutClockSeconds m_qpigsTimeout;
	utils::TimeOutClockSeconds m_qpiriTimeout;

	QPIGS_Result m_lastQipgsResult = {};
	QPIRI_Result m_lastQpiriResult = {};

	struct Impl;
	std::unique_ptr<Impl> m_pImpl;
};

