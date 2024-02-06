#pragma once
#include "SceneSubj.h"
#include "RailCoord.h"

class Coupler : public SceneSubj
{
public:
	RailCoord railCoord;
	RailCoord railCoordOld;
	Coupler* pCounterCoupler = NULL;
	int connected = -1; //-1-not connected, 0-coupler ahead, 1-connected
	int couplersInvolved = 0;
	float distance = 1000000;//dist to CounterCoupler
	float distanceBefore = 1000000;//dist to CounterCoupler in prev frame
	int hook = 0; //1-has hook

	static float couplingDistances[3];


public:
	Coupler() {};
	Coupler(Coupler* pSS0) { memcpy((void*)this, (void*)pSS0, sizeof(Coupler)); };
	virtual Coupler* clone() {
		if (strcmp(this->className, "coupler") != 0)
			return NULL;
		return new Coupler(this);
	};
	virtual int render(Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter) {
		return renderCoupler(this, pCam, dirToMainLight, dirToTranslucent, renderFilter);
	};
	static int renderCoupler(Coupler* pCp, Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter);

	virtual int renderDepthMap(Camera* pCam) { return renderDepthMapCoupler(this, pCam); };
	static int renderDepthMapCoupler(Coupler* pCp, Camera* pCam);

	virtual int moveSubj() { return checkCoupler(this);};
	static int checkCoupler(Coupler* pGS);
	virtual int onLoad(std::string tagStr) { return onLoadCoupler(this, tagStr); };
	static int onLoadCoupler(Coupler* pCp, std::string tagStr);
};
