#include <iostream>
#include "TechnicRoom.h"
#include "Logger.h"

int main() {
	TechnicRoom room;
	LOG_INFO("starting application...");
	room.Run();
	return 0;
}