#include "Coords.h"
#include "platform.h"
#include "utils.h"
#include <string>

extern float PI;
extern float degrees2radians;
extern float radians2degrees;

void Coords::getPositionFromMatrix(float* pos, mat4x4 m) {
	//extracts position from matrix
	for (int i = 0; i < 3; i++)
		pos[i] = m[3][i];
}

void Coords::eulerDg2quaternion(quat q1, float* eulerDg) {
	/*
	quat_identity(q1);
	rotateQuaternion(q1, eulerDg[1], 1);
	rotateQuaternion(q1, eulerDg[0], 0);
	rotateQuaternion(q1, eulerDg[2], 2);
	vec4_norm(q1, q1);
	*/
	float pitch = degrees2radians * eulerDg[0];
	float yaw = degrees2radians * eulerDg[1];
	float roll = degrees2radians * eulerDg[2];
	float cr = cos(roll * 0.5);
	float sr = sin(roll * 0.5);
	float cp = cos(pitch * 0.5);
	float sp = sin(pitch * 0.5);
	float cy = cos(yaw * 0.5);
	float sy = sin(yaw * 0.5);

	q1[3] = cr * cp * cy + sr * sp * sy;
	q1[2] = sr * cp * cy - cr * sp * sy;
	q1[0] = cr * sp * cy + sr * cp * sy;
	q1[1] = cr * cp * sy - sr * sp * cy;

}
/*
void Coords::rotateQuaternion(quat q1, float eulerDg, int axisN) {
	if (eulerDg == 0)
		return;
	vec3 axis = { 0.0,0.0,0.0 };
	axis[axisN] = 1.0;
	quat qT;
	quat_rotate(qT, degrees2radians * eulerDg, axis);
	quat q2;
	quat_mul(q2, q1, qT);
	memcpy(q1, q2, sizeof(quat));
}
*/
void Coords::quaternion2eulerDg(float* eulerDg, quat q1) {
	// assumes q1 is a normalised quaternion
	float qx = q1[0];
	float qy = q1[1];
	float qz = q1[2];
	float qw = q1[3];
	float sqx = qx * qx;
	float sqy = qy * qy;
	float sqz = qz * qz;
	float sqw = qw * qw;
	float yaw, pitch, roll;


	// if the input quaternion is normalized, this is exactly one. Otherwise, this acts as a correction factor for the quaternion's not-normalizedness
	float unit = sqx + sqy + sqz + sqw;

	// this will have a magnitude of 0.5 or greater if and only if this is a singularity case
	float test = qx * qw - qy * qz;

	if (test > 0.4995f * unit) // singularity at north pole
	{
		pitch = PI / 2.0;
		yaw = 2.0f * atan2(qy, qx);
		roll = 0;
	}
	else if (test < -0.4995f * unit) // singularity at south pole
	{
		pitch = -PI / 2.0;
		yaw = -2.0f * atan2(qy, qx);
		roll = 0;
	}
	else // no singularity - this is the majority of cases
	{
		pitch = asin(2.0f * (qw * qx - qy * qz));
		yaw = atan2(2.0f * qw * qy + 2.0f * qz * qx, 1.0 - 2.0f * (sqx + sqy));
		roll = atan2(2.0f * qw * qz + 2.0f * qx * qy, 1.0 - 2.0f * (sqz + sqx));
	}
	// all the math so far has been done in radians. 
	//convert result to degrees and ensure the degree values are between -180 and 180
	eulerDg[0] = angleDgNorm180(radians2degrees * pitch);
	eulerDg[1] = angleDgNorm180(radians2degrees * yaw);
	eulerDg[2] = angleDgNorm180(radians2degrees * roll);
}
int Coords::setEulerDg(Coords* pCoord, float pitch, float yaw, float roll) {
	if (pCoord->eulerDg[0] == pitch)
		if (pCoord->eulerDg[1] == yaw)
			if (pCoord->eulerDg[2] == roll)
				return 0;
	pCoord->eulerDg[0] = pitch;
	pCoord->eulerDg[1] = yaw;
	pCoord->eulerDg[2] = roll;
	eulerDg2quaternion(pCoord->rotationQuat, pCoord->eulerDg);
	mat4x4_from_quat(pCoord->rotationMatrix, pCoord->rotationQuat);
	return 1;
}
int Coords::set1Dg(Coords* pCoord, int aN, float angle) {
	if (pCoord->eulerDg[aN] == angle)
		return 0;
	pCoord->eulerDg[aN] = angle;
	eulerDg2quaternion(pCoord->rotationQuat, pCoord->eulerDg);
	mat4x4_from_quat(pCoord->rotationMatrix, pCoord->rotationQuat);
	return 1;
}
int Coords::setQuaternion(Coords* pCoord, quat q1) {
	if (v4match(pCoord->rotationQuat, q1))
		return 0;
	memcpy(pCoord->rotationQuat, q1, sizeof(quat));
	quaternion2eulerDg(pCoord->eulerDg, q1);
	mat4x4_from_quat(pCoord->rotationMatrix, pCoord->rotationQuat);
	return 1;
}
