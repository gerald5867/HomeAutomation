#include "TechnicRoom.h"
#include <chrono>
#include <thread>
#include <wiringPi.h>
#include "IDevice.h"
#include "ElektrolytPump.h"
#include "Inverter.h"


TechnicRoom::TechnicRoom() {
	wiringPiSetup();
	m_pInv1 = std::make_shared<Inverter>("/dev/ttyUSB0");
	m_pInv2 = std::make_shared<Inverter>("/dev/ttyUSB1");
	m_pInv3 = std::make_shared<Inverter>("/dev/ttyUSB2");
	m_pInv4 = std::make_shared<Inverter>("/dev/ttyUSB3");
	m_pPump = std::make_unique<ElektrolytPump>(std::vector{m_pInv1, m_pInv2, m_pInv3, m_pInv4});
}


TechnicRoom::~TechnicRoom() {

}


void TechnicRoom::Run() {
	while (true) {
		auto now = std::chrono::high_resolution_clock::now();
		for (auto pDevice : IDevice::s_devices) {
			if (std::chrono::duration_cast<std::chrono::seconds>(now - pDevice->m_lastUpdate).count() > pDevice->m_updateSeconds) {
				pDevice->Update();
				pDevice->m_lastUpdate = now;
			}
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}
