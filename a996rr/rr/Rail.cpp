#include "Rail.h"
#include "Shadows.h"
#include "TheApp.h"

extern float degrees2radians;
extern TheApp theApp;

float Rail::railroadGauge = 9;
float Rail::railWidth = 1;
float Rail::railHeight = 2;
float Rail::railSleeperLength = 16;
float Rail::railSleeperWidth = 2;
float Rail::railSleeperHeight = 1;
float Rail::fastenerWidth = 1.5;
float Rail::railSleepersInterval = 5;
float Rail::couplerY = 5;
//map-related
float Rail::railsLevel = 1;
float Rail::curveRadius = 1;
float Rail::tileCenter2curveCenter = 1;
float Rail::railLenghtLong = 1;
float Rail::railLenghtShort = 1;
float Rail::railLenghtCurved = 1;
//wheel-related
float Rail::wheelWidth = 1.6;
float Rail::rimBorderLineWidth = 0.5;
//models
int Rail::railModelSnowFlakeN = -1;
int Rail::railModelSnowFlakeSmallN = -1;

int Rail::railModelStraightVirtualN = -1;
int Rail::railModelShortVirtualN = -1;
int Rail::railModelCurvedVirtualN = -1;

int Rail::railModelStraightN = -1;
int Rail::railModelShortN = -1;
int Rail::railModelCurvedN = -1;

int Rail::railModelStraightNsmall = -1;
int Rail::railModelShortNsmall = -1;
int Rail::railModelCurvedNsmall = -1;

int Rail::railModelStraightNtiny = -1;
int Rail::railModelShortNtiny = -1;
int Rail::railModelCurvedNtiny = -1;


int Rail::couplerModelN = -1;
int Rail::couplersCoupleModelN = -1;

std::vector<SceneSubj*> Rail::railModels;

int Rail::renderRail(Rail* pSS, Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter) {
    mat4x4 mMVP;
    mat4x4 mMVP4dm;
    float sizeUnitPixelsSize;
    renderStandard_prepare(pSS, pCam, dirToMainLight, dirToTranslucent, mMVP, mMVP4dm, &sizeUnitPixelsSize);

    if (pSS->railStatus == 0) {//real rails only, not virtual
        int needSizeMode = 0;
        if (sizeUnitPixelsSize * Rail::railWidth < 1)
            needSizeMode = -2;
        else if (sizeUnitPixelsSize * Rail::railWidth < 4)
            needSizeMode = -1;

        if (pSS->sizeMode != needSizeMode) {
            pSS->sizeMode = needSizeMode;
            int modelN = -1;
            if (pSS->railType == 0) {//0-straight, 1-curve)
                if (pSS->railLength == railLenghtLong) {
                    if (needSizeMode == -2)
                        modelN = railModelStraightNtiny;
                    else if (needSizeMode == -1)
                        modelN = railModelStraightNsmall;
                    else
                        modelN = railModelStraightN;
                }
                else { //short
                    if (needSizeMode == -2)
                        modelN = railModelShortNtiny;
                    else if (needSizeMode == -1)
                        modelN = railModelShortNsmall;
                    else
                        modelN = railModelShortN;
                }
            }
            else { //curve
                if (needSizeMode == -2)
                    modelN = railModelCurvedNtiny;
                else if (needSizeMode == -1)
                    modelN = railModelCurvedNsmall;
                else
                    modelN = railModelCurvedN;
            }
            Rail* pR0 = (Rail*)railModels.at(modelN);
            pSS->djStartN = pR0->djStartN;
            pSS->djTotalN = pR0->djTotalN;
            pSS->mt0isSet = 0;
        }
    }
    renderStandard_execute(pSS, mMVP, mMVP4dm, pCam, dirToMainLight, dirToTranslucent, sizeUnitPixelsSize, renderFilter);

    return 1;
}
float Rail::dist2point(Rail* pR, float* pos3d, float maxDist) {
    for (int i = 0; i < 3; i += 2)
        if (abs(pR->ownCoords.pos[i] - pos3d[i]) - pR->gabaritesOnLoad.bbRad[i] >= maxDist)
            return maxDist;
    //get precise distance
    float p2d[4] = { 0,0,0,0 };
    p2d[0] = pos3d[0]; //x
    p2d[1] = pos3d[2]; //z
    float dist = pR->line2d.dist_l2p(p2d);

    return dist;
}

void RailEnd::toLog(std::string title, RailEnd* pRE) {
    mylog("%s:rail %d.%d @tile %.1fx%.1f\n", title.c_str(), pRE->toRailN,pRE->toRailEndN, pRE->toTileIndexXZ[0], pRE->toTileIndexXZ[1]);
}

void Rail::toLog(std::string title, Rail* pR) {
    mylog("%s:%d tile=%.1fx%.1f %s->%s l=%d\n", title.c_str(), pR->nInSubjsSet, pR->tileIndexXZ[0], pR->tileIndexXZ[1], ang2dir(pR->yawInOut[0]).c_str(), ang2dir(pR->yawInOut[1]).c_str(), (int)pR->railLength);
    RailEnd::toLog("from", &pR->railEnd[0]);
    RailEnd::toLog("  to", &pR->railEnd[1]);
}
std::string Rail::ang2dir(float ang) {
    int a = (int)angleDgNorm360(ang);
    switch (a) {
    case 0: return "S";
    case 45: return "SE";
    case 90: return "E";
    case 135: return "NE";
    case 180: return "N";
    case 225: return "NW";
    case 270: return "W";
    case 315: return "SW";
    default: return "HZ";
    }
}