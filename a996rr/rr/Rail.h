#pragma once
#include "SceneSubj.h"
#include "geom2d/LineXY.h"
#include "geom2d/ArcXY.h"

class RailEnd
{
public:
	float toTileIndexXZ[2];
	float endYaw = 0;
	int toRailN = -1;
	int toRailEndN = -1; //0 or 1 (start or end)
	bool isSwitch = false;
	bool isOn = false;
	int altRailN = -1;
	int altRailEndN = -1;
	int ownRailN = -1;
public:
	static void toLog(std::string title, RailEnd* pRE);
};

class Rail : public SceneSubj
{
public:
	float tileIndexXZ[2];
	float yawInOut[2]; //orientation degrees: 0-down, CCW
	float railLength;
	int railType = 0;//0-straight, 1-curve
	int railStatus = 0; //0-normal, 1-virtual, -1-snowFlake
	LineXY line2d;
	ArcXY arc2d;
	RailEnd railEnd[2];
	float p0[4] = { 0,0,0,0 }; //pointIn
	float p1[4] = { 0,0,0,0 }; //pointOut
	int crossingRailN = -1;
	float crossingPercent = 0.5;
	float crossingPoint[4];
	//curve-specific
	float curveCenter[4] = { 0,0,0,0 };
	int CCW = 1;//-1-CW
	float radiusAngle[2]; //orientation degrees: 0-down, CCW

	int sizeMode = 0; //-1-small, -2-tiny

	//models sizes
	static float railroadGauge;
	static float railWidth;
	static float railHeight;
	static float railSleeperLength;
	static float railSleeperWidth;
	static float railSleeperHeight;
	static float fastenerWidth;
	static float railSleepersInterval;
	static float couplerY;
	//map-related
	static float railsLevel;
	static float curveRadius;
	static float tileCenter2curveCenter;
	static float railLenghtLong;	//minimum 27
	static float railLenghtShort;
	static float railLenghtCurved;
	//wheel related
	static float wheelWidth;
	static float rimBorderLineWidth;

	//models
	static int railModelSnowFlakeN;
	static int railModelSnowFlakeSmallN;

	static int railModelStraightVirtualN;
	static int railModelShortVirtualN;
	static int railModelCurvedVirtualN;

	static int railModelStraightN;
	static int railModelShortN;
	static int railModelCurvedN;

	static int railModelStraightNsmall;
	static int railModelShortNsmall;
	static int railModelCurvedNsmall;

	static int railModelStraightNtiny;
	static int railModelShortNtiny;
	static int railModelCurvedNtiny;

	static int couplerModelN;
	static int couplersCoupleModelN;

	static std::vector<SceneSubj*> railModels;

public:
	Rail() {};
	Rail(Rail* pR0) { memcpy(this, pR0, sizeof(Rail)); };

	virtual int render(Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter) {
		return renderRail(this, pCam, dirToMainLight, dirToTranslucent, renderFilter);
	};
	static int renderRail(Rail* pGS, Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter);
	float dist2point(float* pos,float maxDist=1000000) { return dist2point(this,pos,maxDist); };
	static float dist2point(Rail* pR,float* pos, float maxDist = 1000000);
	static void toLog(std::string title, Rail* pR);
	static std::string ang2dir(float ang);
};
