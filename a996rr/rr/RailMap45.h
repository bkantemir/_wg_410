#pragma once
#include "TheTable.h"
#include "Rail.h"
#include "ModelLoader.h"
#include "RailCoord.h"

class RailMap45 : public TheTable
{
public:
	std::vector<SceneSubj*> railsMap;

public:
	virtual ~RailMap45();
	virtual void cleanUp();
	void initVirtualNet();
	void initVirtualNet00(float dx, float dz, unsigned int color32);
	void basicOval();
	int getRailN(float idxX, float idxZ, float yawIn, float yawOut);
	Rail*  getRail(float idxX, float idxZ, float yawIn, float yawOut);
	int setRail(float idxX, float idxZ, float yawIn, float yawOut, int railStatus);
	int addNewRail(float idxX, float idxZ, float yawIn, float yawOut, int railStatus);
	void initRailModelsVirtual();
	void initRailModels();
	void sleeperGroup(ModelLoader* pML, int sizeMode); //sizeMode 0-normal, -1-small, -2-tiny
	int buildModelRailStraight(ModelLoader* pML, float railLenght, int sizeMode);
	int buildModelRailCurved(ModelLoader* pML, int sizeMode);
	void buildModelRailPairSection(ModelLoader* pML, float railLenght, int sizeMode);
	int findClosestRail(RailCoord* pRC, Coords* pCoords) { return findClosestRail(pRC, this, pCoords); };
	static int findClosestRail(RailCoord* pRC, RailMap45* pMap, Coords* pCoords);
	int addDiagonalShortTo(float idxX, float idxZ, float yaw2extend, int railStatus);
	int fillRailEnd(Rail* pR, int whichEnd);
	static int shiftRailCoord(RailCoord* pRCout, RailCoord* pRCin, RailMap45* pMap, float shiftZ);
	static int shiftRailCoord(RailCoord* pRC, RailMap45* pMap, float shiftZ);
	static void fillRailCoordsFromPercent(RailCoord* pRC, Rail* pR);
	int checkRailCrossing(Rail* pR);
	static float offset2railCoord(RailCoord* pRC1, RailCoord* pRC2, RailMap45* pMap, float distLimit=1000000);
};


