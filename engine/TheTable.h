#pragma once
#include "DrawJob.h"
#include "SceneSubj.h"

class TheTable
{
public:
	int tableTiles[2] = { 1,1 };
	float tileSize = 100;
	float groundLevel = 0;
	Gabarites worldBox;
	float suggestedStageSize = 100;

	std::vector<DrawJob*> table_drawJobs;
	std::vector<unsigned int> table_buffersIds;
	std::vector<SceneSubj*> tableParts;

public:
	virtual ~TheTable();
	int initTable(float tileSizeXZ, float tileSizeUp, float tileSizeDown, int tilesNx, int tilesNz);
	void cleanUp();
	void placeAt(float* pos, float x, float y,float z);
};


