#pragma once
#include <vector>
#include "Coords.h"

class GLTFanim2nodesBone
{
public:
	char name[64] = "";
	int animBoneN = -1; //should match with position in GLTFanim2skinMap.anim2skinBones vector
	Coords ownCoords;
	float ownScale[3];
	float boneLever = 0;

	int nodesBoneN = -1;
	float scale = 1;
public:
	GLTFanim2nodesBone() {};
	GLTFanim2nodesBone(GLTFanim2nodesBone* pB0) { memcpy((void*)this, (void*)pB0, sizeof(GLTFanim2nodesBone)); };

};
class GLTFanim2nodesMap
{
public:
	int animSkeletonMapN = -1;
	int rootNodeN = -1;
	std::vector<GLTFanim2nodesBone*> anim2nodesBones;
	float midScale = 1;

	static std::vector<GLTFanim2nodesMap*> anim2nodesMaps_default;
};

