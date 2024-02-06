#pragma once
#include "LineXY.h"

class ArcXY //circular arc
{
public:
	float centerPos[4] = { 0,0,0,0 };
	//float p0[4] = { 0,0,0,0 };
	//float p1[4] = { 0,0,0,0 };
	float a0 = 0; //start angle
	float a1 = 0; //end angle
	int circleDir = 1; //1-CCW, -1-CW
	float radius = 0;
	bool fullCircle = false;
public:
	//ArcXY(void) {};
	//ArcXY(float centerX, float centerY, float rad, float a0, float a1, int circleDir=1) { initArc(this, centerX, centerY, rad, a0, a1, circleDir); };
	static void initArc(ArcXY* pA, float centerX, float centerY, float rad, float a0, float a1, int circleDir);
	/*
	static void calculateLine(LineXY* pLn);
	static void initDirXY(LineXY* pLn, float* p01);
	static bool matchingLines(LineXY* pLine0, LineXY* pLine1);
	static bool parallelLines(LineXY* pLine0, LineXY* pLine1);
	static int lineSegmentsIntersectionXY(float* vOut, LineXY* pL1, LineXY* pL2);
	static int linesIntersectionXY(float* vOut, LineXY* pL1, LineXY* pL2);
	static bool isPointIn(float* vOut, LineXY* pL);
	static bool isPointBetween(float* v, float* v1, float* v2);
	*/
	static int crossLine(float* vOut2d, ArcXY* pA, LineXY* pL, int* solutionPreference);
};

