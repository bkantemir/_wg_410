#pragma once
#include "SceneSubj.h"
#include "geom2d/LineXY.h"

class Gear : public SceneSubj
{
public:
	bool gearRoot = false;
	int d2wheel2follow = 0; //root only
	float followAngleShift = 0;
	float spinAngle = 0;
	float spinDz = 0;
	float spinDy = 0;
	LineXY cylinderShaft;//root only
	float crossPoint[2]; //for main_rod and knees
	int d2mainrod = 0; //for crosshead
	float dz = 0;
	//models gear related
	static float dxMainRod;
	static float dxRimWall;
	static float dxSideRod;
	static float dxBetweenRods;

public:
	Gear() {};
	Gear(Gear* pSS0) { memcpy((void*)this, (void*)pSS0, sizeof(Gear)); };
	virtual Gear* clone() {
		if (strcmp(this->className, "Gear") != 0)
			return NULL;
		return new Gear(this);
	};
	virtual int onLoad(std::string tagStr) { return onLoadGear(this, tagStr); };
	static int onLoadGear(Gear* pSS, std::string tagStr);
	virtual int moveSubj() { return moveGear(this); };
	static int moveGear(Gear* pGS);
	virtual int readClassProps(std::string tagStr) { return readClassPropsGear(this, tagStr); };
	static int readClassPropsGear(Gear* pSS, std::string tagStr);
	virtual int scaleMe(float k) { 
		dz *= k; 
		for (int i = 0; i < 3; i++) {
			cylinderShaft.p0[i] *= k;
			cylinderShaft.p1[i] *= k;
		}
		LineXY::calculateLine(&cylinderShaft);
		return scaleStandard(this, k); 
	};

};

