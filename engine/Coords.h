#pragma once
#include "linmath.h"
#include "utils.h"

class Coords
{
private:
	float eulerDg[3] = { 0,0,0 }; //Euler angles (pitch, yaw, roll) in degrees
	quat rotationQuat = { 0,0,0,1 }; //quaternion x,y,z,w
public:
	float pos[4] = { 0,0,0,0 }; //x,y,z position + 4-th element for compatibility with 3D 4x4 matrices math
	mat4x4 rotationMatrix = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
public:
	static void getPositionFromMatrix(float* pos, mat4x4 m);

	static void eulerDg2quaternion(quat q1, float* eulerDg);
	//static void rotateQuaternion(quat q1, float eulerDg, int axisN);
	static void quaternion2eulerDg(float* eulerDg, quat q1);
	int setEulerDg(float pitch, float yaw, float roll) { return setEulerDg(this, pitch, yaw, roll); };
	static int setEulerDg(Coords* pCoord, float pitch, float yaw, float roll);
	int setYaw(float angle) { return set1Dg(this, 1, angle); };
	int setPitch(float angle) { return set1Dg(this, 0, angle); };
	int set1Dg(int aN, float angle) {return set1Dg(this, aN, angle);};
	static int set1Dg(Coords* pCoord, int aN, float angle);
	bool noRotation() { if (v3equals(this->eulerDg, 0)) return true; else return false; }
	float* getRotationQuat() { return this->rotationQuat; };
	int setQuaternion(quat q1) { return setQuaternion(this, q1); };
	static int setQuaternion(Coords* pCoord, quat q1);
	float getEulerDg(int nAxis) { return eulerDg[nAxis]; };
};

