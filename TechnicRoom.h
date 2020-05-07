#pragma once
#include <memory>

class Inverter;
class ElektrolytPump;

class TechnicRoom
{
public:
	void Run();
	TechnicRoom();
	~TechnicRoom();
private:
	std::shared_ptr<Inverter> m_pInv1;
	std::shared_ptr<Inverter> m_pInv2;
	std::shared_ptr<Inverter> m_pInv3;
	std::shared_ptr<Inverter> m_pInv4;
	std::unique_ptr<ElektrolytPump> m_pPump;
};

