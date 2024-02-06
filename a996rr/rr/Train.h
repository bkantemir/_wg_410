#pragma once
#include "Coupler.h"

class Train
{
public:
	Coupler* pCouplerFront;
	Coupler* pCouplerBack;
	int trainId = -1;
	float trainLength = 0;
	float poweredLength = 0;
	int carsTotal = 1;
	int primeLocoN = -1;
	bool divorcing = false;
	//int divorceWithTrainId = -1;
	//float divorceDir = 0;
public:
};
