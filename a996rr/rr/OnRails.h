#pragma once
#include "SceneSubj.h"
#include "RailCoord.h"

class OnRails : public SceneSubj
{
public:
	RailCoord railCoord;
	RailCoord railCoordOld;
	float wheelBase = 0;
	float wheelBaseZ = 0;
public:
	OnRails() {};
	OnRails(OnRails* pSS0) { memcpy((void*)this, (void*)pSS0, sizeof(OnRails)); };
	virtual OnRails* clone() {
		if (strcmp(this->className, "OnRails") != 0)
			return NULL;
		return new OnRails(this);
	};
	virtual int applySpeed() { return applySpeedOnRails(this); };
	static int applySpeedOnRails(OnRails* pGS);
	virtual int moveSubj() { return moveOnRails(this); };
	static int moveOnRails(OnRails* pGS);
	virtual void buildModelMatrix() { buildModelMatrixOnRails(this); };
	static void buildModelMatrixOnRails(OnRails* pSS);
	virtual int onLoad(std::string tagStr) { return onLoadOR(this, tagStr); };
	static int onLoadOR(OnRails* pSS, std::string tagStr);
};


