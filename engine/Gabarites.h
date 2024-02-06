#pragma once
#include <vector>
#include "geom2d/LineXY.h"
#include "utils.h"

class Gabarites
{
public:
	//bounding box
	float bbMin[3] = { 1000000,1000000,1000000 };
	float bbMax[3] = { -1000000,-1000000,-1000000 };

	float bbMid[4];
	float bbRad[3];

	float sizeDirXY[3][4];
	float sizeXY[3];
	//chord
	int chordType = -1; //-1-don't need, 0-round, 2-square
	float chordR;
	LineXY chord;

	float boxRad = 0;

	int isInViewRange = 0; //1-completely on-screen, 0 - partially, -1 - off-screen

public:
	void clear() { v3setAll(this->bbMin, 1000000); v3setAll(this->bbMax, -1000000);	};
	static void adjustMidRad(Gabarites* pGB);
	static void adjustMinMaxByPoint(Gabarites* pGB, float* newPoint);

	static void fillBBox(Gabarites* pGB, Gabarites* pGabarites00, mat4x4 mMVP, float* targetRads);
	static int fillGabarites(Gabarites* pGB, mat4x4 absModelMatrix, Gabarites* pGB00, mat4x4 mViewProjection, float* targetRads);
	static float checkCollision(float* collisionPoint, Gabarites* pGB1, Gabarites* pGB2);
	static void toLog(std::string title,Gabarites* pGB);
};

