#pragma once
#include <string>

class RailCoord
{
public:
	int railN = -1;
	int alignedWithRail = 1; //-1-not aligned
	float percentOnRail = 0.5;
	float currentYaw = 0;
	float xyz[4];
public:
	static void toLog(std::string title, RailCoord* pRC);
};
