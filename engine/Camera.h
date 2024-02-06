#pragma once
#include "Coords.h"
#include "Gabarites.h"
#include "linmath.h"
#include "Line3D.h"

class Camera
{
public:
	Coords ownCoords;
	mat4x4 lookAtMatrix; //view?
	mat4x4 mViewProjection;
	float lookAtPoint[4] = { 0,0,0, 0 };
	float focusDistance = 100;
	float viewRangeDg = 30;
	float stageSize[2] = { 500, 375 };
	float nearClip = 0;
	float farClip = 100;
	float targetDims[2] = { 10,10 };
	float targetRads[2] = { 5,5 };
	float targetAspectRatio = 1;
	Gabarites visibleBox;
	std::vector<Line3D*> visibleRays;
public:
	static float pickDistance(Camera* pCam);
	static void setCameraPosition(Camera* pCam);
	static void buildLookAtMatrix(Camera* pCam);
	static void onTargetResize(Camera* pCam, int width, int height, Gabarites* pWorldBox);
	static void buildViewProjectionNoClips(Camera* pCam);
	static void buildViewProjectionWithClips(Camera* pCam);
	static void reset(Camera* pCam, Gabarites* pWorldBox);
	static void setNearAndFarClips(Camera* pCam, Gabarites* pWorldBox);
	static void setCollisionCamera(Camera* pCam, Gabarites* viewBox);

};

