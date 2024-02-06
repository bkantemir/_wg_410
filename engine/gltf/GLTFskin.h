#pragma once
#include <vector>
#include "platform.h"

class GLTFskin
{
public:
	char skinName[32] = "";
	int sceletonRootN = -1;
	int jointsN = -1;
	int* joints = NULL;
	float* inverseBindMatrices = NULL;

	//following calculated when render animation
	float* jointsMatrices = NULL;
	uint32_t updateFrameN = -1;
	int forRootNodeN = -1;

	static std::vector<GLTFskin*> GLTFskins_default;

public:
};

