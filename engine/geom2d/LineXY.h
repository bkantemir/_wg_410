#pragma once
#include "stdio.h"

class LineXY
{
	//y=a_slope*x+b_intercept
public:
	float p0[4] = { 0,0,0,0 };
	float p1[4] = { 0,0,0,0 };
	//line equation
	float a_slope = 0; //a
	float b_intercept = 0; //b. if 0 - horizontal line
	bool isVertical = false;
	float x_vertical = 0;
	bool isHorizontal = false;
	bool isDot = false;
	float closestPoint[4] = { 0,0,0,0 };
public:
	LineXY(void) {};
	LineXY(float* p00, float* p01) { initLineXY(this, p00, p01); };
	static void calculateLine(LineXY* pLn);
	static void initLineXY(LineXY* pLn, float* p00, float* p01);
	static void initLineXZ(LineXY* pLn, float* p00, float* p01);
	static void initLineZY(LineXY* pLn, float* p00, float* p01);
	static void initDirXY(LineXY* pLn, float* p01);
	static bool matchingLines(LineXY* pLine0, LineXY* pLine1);
	static bool parallelLines(LineXY* pLine0, LineXY* pLine1);
	static int lineSegmentsIntersectionXY(float* vOut, LineXY* pL1, LineXY* pL2);
	static int linesIntersectionXY(float* vOut, LineXY* pL1, LineXY* pL2);
	static bool isPointOnLine(float* p2d, LineXY* pL);
	static bool isPointIn(float* p2d, LineXY* pL);
	static bool isPointBetween(float* p2d, float* v1, float* v2);
	float dist_l2p(float* p2d) {return dist_l2p(this, p2d);};
	static float dist_l2p(LineXY* pLn,float* p2d);
	static void buildPerpendicular(LineXY* pOut, LineXY* pLn, float* p2d);
	static float dist_l2l(LineXY* pL1, LineXY* pL2);
	static float dist_p2l(float* p1, LineXY* pL2) {return dist_l2p(pL2, p1);};
	static float dist_p2p(float* p1, float* p2);
};
