#pragma once
#include <string>
#include <vector>
#include "linmath.h"
#include "platform.h"

int checkGLerrors(std::string ref);
void mat4x4_mul_vec4plus(vec4 vOut, mat4x4 M, vec4 vIn, float v3, bool adjustByW=false);

void v3setAll(float* vOut, float x);
void v3set(float* vOut, float x, float y, float z);
void v4copy(float* vOut, float* vIn);
void v3copy(float* vOut, float* vIn);
void v2copy(float* vOut, float* vIn);
void m16copy(float* vOut, float* vIn);
float v3pitchRd(float* vIn);
float v3yawRd(float* vIn);
float v3pitchDg(float* vIn);
float v3yawDg(float* vIn);
float v3yawDgFromTo(float* v1, float* v2);
int signOf(float n);

float v4dotProduct(float* a0, float* b0); //for quaternions
float v4dotProductNorm(float* a, float* b); //for quaternions
float v3dotProduct(float* a0, float* b0);
float v3dotProductNorm(float* a, float* b);
float v2dotProductNorm(float* a, float* b);
void v3inverse(float inV[]);
void v3inverse(float outV[], float inV[]);
float v3length(float* v);
float v3lengthXZ(float v[]);
float v3lengthXY(float v[]);
bool v3equals(float v[], float x);
bool vNmatch(int n, float v0[], float v1[]);
bool v4match(float v0[], float v1[]);
bool v3match(float v0[], float v1[]);
bool v2match(float v0[], float v1[]);
void v3fromTo(float* v, float* v0, float* v1);
float v3lengthFromTo(float* v0, float* v1);
float v3lengthFromToXY(float* v0, float* v1);
float v3lengthFromToXZ(float* v0, float* v1);
void v3dirFromTo(float* v, float* v0, float* v1);
void v3norm(float* v);
void v3normXY(float* vOut, float* vIn);

uint64_t getSystemMillis();
uint64_t getSystemNanos();

int getRandom(int fromN, int toN);
float getRandom(float fromN, float toN);
std::vector<std::string>* splitString(std::string inString, std::string delimiter);
std::string trimString(std::string inString);
bool fileExists(const char* filePath);
std::string getFullPath(std::string filePath);
std::string getInAppPath(std::string filePath);
int makeDirs(std::string filePath);

float angleDgFromTo(float a0, float a2);
float angleDgNorm360(float a); //returns angle in 0:360 range
float angleDgNorm180(float a); //returns angle in -180:+180 range

void mylog_v3(std::string vTitle, float* v);

char* v3toStr(float* v);
char* v3toStrXZ(float* v);

float v2angleRd(float* v); //as in school
float v2angleDg(float* v); //as in school
float v2angleFromToDg(float* v0, float* v1); //as in school
float circumference(float r);

