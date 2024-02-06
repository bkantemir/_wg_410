#pragma once

class Triangle01
{
public:
	char marks128[128] = "";
	int subjN = -1; //game subject number
	int materialN = -1; //material number
	int flag = 0;
	int idx[3] = { 0,0,0 }; //3 vertex indices
	//for normals calculation
	float normal[3] = { 0,0,0 };
	int matchingNormalsN = 0;
	float triangleArea = 0;
public:
	Triangle01() {};
	Triangle01(Triangle01* pTR0) { memcpy((void*)this, (void*)pTR0, sizeof(Triangle01)); };
};

