#pragma once
#include "OnRails.h"
#include "Train.h"

class RollingStock : public OnRails
{
public:
	Train tr;
	int trainRootN = -1;
	int alignedWithTrainHead = 1;
	float zOffsetFromHead = 0;
	int nInTrain = 0;
	Coupler* pCouplerFront = NULL;
	Coupler* pCouplerBack = NULL;
	float unitLength = 0;
	bool powered = false; //1-locomotive
	bool activeLoco = false;

	static float accelerationLinPassive;
	static float accelerationLinActive;
	static float divorceSpeed;
	static float maxTrainSpeed;
public:
	RollingStock() {};
	RollingStock(RollingStock* pSS0) { memcpy((void*)this, (void*)pSS0, sizeof(RollingStock)); };
	virtual RollingStock* clone() {
		if (strcmp(this->className, "RollingStock") != 0)
			return NULL;
		return new RollingStock(this);
	};
	virtual int onDeploy(std::string tagStr) { return onDeployRS(this, tagStr); };
	static int onDeployRS(RollingStock* pSS, std::string tagStr);
	virtual int onLoad(std::string tagStr) { return onLoadRS(this, tagStr); };
	static int onLoadRS(RollingStock* pSS, std::string tagStr);
	virtual int moveSubj() { return moveRS(this); };
	static int moveRS(RollingStock* pRS);
	virtual bool ignoreCollision(SceneSubj* pS2) { return ignoreCollisionRollingStock(this, pS2); };
	static bool ignoreCollisionRollingStock(RollingStock* pRS1, SceneSubj* pS2);
	virtual int processCollision(float penetrationDepth,SceneSubj* pS2, float* collisionPointWorld, float* hitPointNormal, float* hitSpeed) {
		return processCollisionRollingStock(penetrationDepth,this, pS2, collisionPointWorld, hitPointNormal, hitSpeed);
	};
	static int processCollisionRollingStock(float penetrationDepth, RollingStock* pS1, SceneSubj* pS2, float* collisionPointWorld, float* hitPointNormal, float* hitSpeed);
	virtual int readClassProps(std::string tagStr) { return readClassPropsRS(this,  tagStr); };
	static int readClassPropsRS(RollingStock* pRS,std::string tagStr);

	static int checkTrains(std::vector<SceneSubj*>* pSceneSubjs);
	static float couplerClearance(Coupler* pCoupler);
	static int reinspectTrain(Coupler* pCoupler);
	static int emergencyTrainShift(RollingStock* pRoot1, float dist, RollingStock* pRoot2);
	static int divorceTrains(RollingStock* pW1, RollingStock* pW2);
	static int trainToLog(RollingStock* pW);
	static int trainToLog(Coupler* pCp);
	static std::string path2title(std::string path);
	static bool trainRootIsWrong(RollingStock* pW0);
	static Coupler* findHeadCoupler(Coupler* pCp);
	static int assignNewTrainHead(Coupler* pCp);
	static int decouple(Coupler* pCp);
};


