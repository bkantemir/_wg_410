#pragma once
#include "Vertex01.h"
#include <vector>

class CoordOnLine
{
public:
	std::vector<Vertex01*>* pVerts;
	int startVertN = 0;
	float segmentLength;
	float offsetPercent = 0;
	float pos[4];
	float eulerDg[3];
public:
	int applyShift(float shift, int anglesToo) { return applyShift(this, shift, anglesToo); };
	static int applyShift(CoordOnLine* pCoL, float shift, int anglesToo);
};