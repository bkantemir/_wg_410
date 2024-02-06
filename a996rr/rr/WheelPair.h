#pragma once
#include "OnRails.h"


class WheelPair : public OnRails
{
public:
	bool wheelRoot;
	int d2wheel2follow = 0;
	float followAngleShift = 0;
	int alignedWithLeader = 1;
	float wheelRadius = 1;
	float wheelCircumference = 1;
	float spinAngle = 0;
	int spokesN = 1; //0-static, 1-normal rotation,>1-spokes
	float angleBetweenSpokes = 5;
public:
	WheelPair() {};
	WheelPair(WheelPair* pSS0) { memcpy((void*)this, (void*)pSS0, sizeof(WheelPair)); };
	virtual WheelPair* clone() {
		if (strcmp(this->className, "WheelPair") != 0)
			return NULL;
		return new WheelPair(this);
	};
	virtual int moveSubj() { return moveWheelPair(this); };
	static int moveWheelPair(WheelPair* pGS);
	virtual void buildModelMatrix() { SceneSubj::buildModelMatrixStandard(this); };
	virtual int onLoad(std::string tagStr) { return onLoadWheelPair(this, tagStr); };
	static int onLoadWheelPair(WheelPair* pSS, std::string tagStr);
	virtual int onDeploy(std::string tagStr) { return onDeployWheelPair(this, tagStr); };
	static int onDeployWheelPair(WheelPair* pSS, std::string tagStr);
	virtual int readClassProps(std::string tagStr) { return readClassPropsWP(this, tagStr); };
	static int readClassPropsWP(WheelPair* pSS, std::string tagStr);
	virtual int scaleMe(float k) { wheelRadius *= k; wheelCircumference *= k; return scaleStandard(this, k); };

};