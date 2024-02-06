#include "Camera.h"
#include "TheApp.h"
#include "Line3D.h"
#include "Shadows.h"
#include "utils.h"

//extern TheGame theGame;
extern float degrees2radians;

void Camera::setCameraPosition(Camera* pCam) {
	v3set(pCam->ownCoords.pos, 0, 0, -pCam->focusDistance);
	mat4x4_mul_vec4plus(pCam->ownCoords.pos, pCam->ownCoords.rotationMatrix, pCam->ownCoords.pos, 1);
	for (int i = 0; i < 3; i++)
		pCam->ownCoords.pos[i] += pCam->lookAtPoint[i];
}
void Camera::buildLookAtMatrix(Camera* pCam) {
	float cameraUp[4] = { 0,1,0,0 }; //y - up
	mat4x4_mul_vec4plus(cameraUp, pCam->ownCoords.rotationMatrix, cameraUp, 0);
	mat4x4_look_at(pCam->lookAtMatrix, pCam->ownCoords.pos, pCam->lookAtPoint, cameraUp);
}
void Camera::onTargetResize(Camera* pCam, int width, int height, Gabarites* pWorldBox) {
	pCam->targetDims[0] = (float)width;
	pCam->targetDims[1] = (float)height;
	pCam->targetRads[0] = pCam->targetDims[0] / 2;
	pCam->targetRads[1] = pCam->targetDims[1] / 2;
	pCam->targetAspectRatio = (float)width / (float)height;
	//glViewport(0, 0, width, height);
	pCam->reset(pCam, pWorldBox);
}
void Camera::reset(Camera* pCam, Gabarites* pWorldBox) {
	pickDistance(pCam);
	setCameraPosition(pCam);
	buildLookAtMatrix(pCam);
	setNearAndFarClips(pCam, pWorldBox);
	buildViewProjectionWithClips(pCam);
			/*
			mylog("\nRESET Camera for %dx%d ratio=%f\n", (int)pCam->targetSize[0], (int)pCam->targetSize[1], pCam->targetAspectRatio);
			mylog("stageSize = %d x %d, pitch=%d yaw=%d\n", (int)pCam->stageSize[0], (int)pCam->stageSize[1],(int)pCam->ownCoords.getEulerDg(0), (int)pCam->ownCoords.getEulerDg(1));
			mylog("viewRangeDg=%d, focusDistance=%d\n", (int)pCam->viewRangeDg,(int)pCam->focusDistance);
			mylog("LookAt %s, ", v3toStr(pCam->lookAtPoint));
			mylog("Camera pos %s, ", v3toStr(pCam->ownCoords.pos));
			mylog("dist=%d\n", (int)v3lengthFromTo(pCam->lookAtPoint, pCam->ownCoords.pos));
			mylog("Near: %d, far: %d, dist=%d\n", (int)pCam->nearClip, (int)pCam->farClip, (int)(pCam->farClip- pCam->nearClip));
			*/

	Shadows::resetShadowsFor(pCam,pWorldBox);
}
float Camera::pickDistance(Camera* pCam) {
	float viewRangeDg = pCam->viewRangeDg;
	if (viewRangeDg == 0)
		viewRangeDg = 10;
	float cotangentA = 1.0f / tanf(degrees2radians * viewRangeDg / 2);
	float cameraDistanceV = pCam->stageSize[1] / 2 * cotangentA;
	float cameraDistanceH = pCam->stageSize[0] / 2 * cotangentA / pCam->targetAspectRatio;
	pCam->focusDistance = (float)fmax(cameraDistanceV, cameraDistanceH);
	return pCam->focusDistance;
}
void Camera::setNearAndFarClips(Camera* pCam, Gabarites* pWorldBox) {
	//set temporary mProjection matrix with unreasonable near and far clips
	float worldRadTmp = pWorldBox->boxRad*2;
	pCam->nearClip = pCam->focusDistance - worldRadTmp;
	if (pCam->nearClip < 1) pCam->nearClip = 1;
	pCam->farClip = pCam->focusDistance + worldRadTmp;
	pCam->buildViewProjectionWithClips(pCam);
	//reset clips
	pCam->nearClip = 1000000;
	pCam->farClip = -1000000;
	v3setAll(pCam->visibleBox.bbMin, 1000000);
	v3setAll(pCam->visibleBox.bbMax, -1000000);
	pCam->visibleRays.clear();

	mat4x4 mInvertedCamViewProjection;
	mat4x4_invert(mInvertedCamViewProjection, pCam->mViewProjection);
	float vIn[4] = { 0,0,0,1 };
	float vOut[4];
	for (float y = -1.0; y <= 1.0; y += 0.1) {
		for (float x = -1.0; x <= 1.0; x += 0.1) {
			Line3D ray;
			//set nearClip
			//v3set(vIn, x, y / pCam->targetAspectRatio, -1);
			v3set(vIn, x, y, -1);
			mat4x4_mul_vec4plus(vOut, mInvertedCamViewProjection, vIn, 1, true);
			v3copy(ray.p0, vOut);
			//set farClip
			//v3set(vIn, x, y / pCam->targetAspectRatio, 1);
			v3set(vIn, x, y, 1);
			mat4x4_mul_vec4plus(vOut, mInvertedCamViewProjection, vIn, 1, true);
			v3copy(ray.p1, vOut);
			if (Line3D::fitBox(&ray, pWorldBox) == 0)
				continue; //ray completely out of box

			//update near and far clips and visible box sizes
			//near clip
			float dist = v3lengthFromTo(pCam->ownCoords.pos, ray.p0);
			if (pCam->nearClip > dist)
				pCam->nearClip = dist;
			//far clip
			dist = v3lengthFromTo(pCam->ownCoords.pos, ray.p1);
			if (pCam->farClip < dist)
				pCam->farClip = dist;
			//update visibleBox
			Gabarites::adjustMinMaxByPoint(&pCam->visibleBox, ray.p0);
			Gabarites::adjustMinMaxByPoint(&pCam->visibleBox, ray.p1);
			pCam->visibleRays.push_back(new Line3D(ray));
		}
	}
	Gabarites::adjustMidRad(&pCam->visibleBox);
}
void Camera::buildViewProjectionNoClips(Camera* pCam) {
	float r = pCam->stageSize[0] / 2 + 5.0f;
	pCam->nearClip = pCam->focusDistance - r;
	pCam->farClip = pCam->focusDistance + r;
	if (pCam->nearClip < 0) pCam->nearClip = 0;
	buildViewProjectionWithClips(pCam);
}
void Camera::buildViewProjectionWithClips(Camera* pCam) {
	mat4x4 mProjection;
	if (pCam->viewRangeDg == 0) {
		float w = pCam->stageSize[0] / 2 + 10.0f;
		float h = w / pCam->targetAspectRatio;
		mat4x4_ortho(mProjection, -w, w, -h, h, pCam->nearClip, pCam->farClip);
		mat4x4_mul(pCam->mViewProjection, mProjection, pCam->lookAtMatrix);
	}
	else {
		mat4x4_perspective(mProjection, pCam->viewRangeDg * degrees2radians, pCam->targetAspectRatio, pCam->nearClip, pCam->farClip);
		mat4x4_mul(pCam->mViewProjection, mProjection, pCam->lookAtMatrix);
		pCam->mViewProjection[1][3] = 0; //keystone effect
	}
}
void Camera::setCollisionCamera(Camera* pCam, Gabarites* pViewBox) {
	pCam->ownCoords.setEulerDg(90, 0, 0); //set camera angles/orientation
	mat4x4_from_quat(pCam->ownCoords.rotationMatrix, pCam->ownCoords.getRotationQuat());
	pCam->viewRangeDg = 0;
	pCam->stageSize[0] = pViewBox->bbRad[0] * 2;
	pCam->stageSize[1] = pViewBox->bbRad[2] * 2;
	pCam->reset(pCam, pViewBox);
}
