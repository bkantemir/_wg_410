#include "Gabarites.h"
#include "utils.h"

void Gabarites::adjustMidRad(Gabarites* pGB) {
	for (int i = 0; i < 3; i++) {
		pGB->bbRad[i] = (pGB->bbMax[i] - pGB->bbMin[i]) * 0.5f;
		pGB->bbMid[i] = (pGB->bbMax[i] + pGB->bbMin[i]) * 0.5f;
	}
	pGB->boxRad = v3lengthFromTo(pGB->bbMin, pGB->bbMax) / 2;
}
void Gabarites::adjustMinMaxByPoint(Gabarites* pGB, float* newPoint) {
	for (int i = 0; i < 3; i++) {
		if (pGB->bbMin[i] > newPoint[i])
			pGB->bbMin[i] = newPoint[i];
		if (pGB->bbMax[i] < newPoint[i])
			pGB->bbMax[i] = newPoint[i];
	}
}
void Gabarites::toLog(std::string title, Gabarites* pGB) {
	mylog("%s:\n",title.c_str());
	mylog("bbMin: %s\n", v3toStr(pGB->bbMin));
	mylog("bbMax: %s\n", v3toStr(pGB->bbMax));
	mylog("bbMid: %s\n", v3toStr(pGB->bbMid));
	mylog("bbRad: %s\n", v3toStr(pGB->bbRad));
}

void Gabarites::fillBBox(Gabarites* pGB, Gabarites* pGB00, mat4x4 mMVP, float* targetRads) {
    //calculate mid point position
    mat4x4_mul_vec4plus(pGB->bbMid, mMVP, pGB00->bbMid, 1, true);
    //build size vectors
    float sizeVector[3][4];
    for (int axisN = 0; axisN < 3; axisN++) {
        if (pGB00->bbRad[axisN] == 0) {
            v3setAll(sizeVector[axisN], 0);
            v3setAll(pGB->sizeDirXY[axisN], 0);
            pGB->sizeXY[axisN] = 0;
            continue;
        }
        float vIn[4];
        v3copy(vIn, pGB00->bbMid);
        vIn[axisN] += pGB00->bbRad[axisN];
        float vOut[4];
        mat4x4_mul_vec4plus(vOut, mMVP, vIn, 1, true);

        v3fromTo(sizeVector[axisN], pGB->bbMid, vOut);
        v3normXY(pGB->sizeDirXY[axisN], sizeVector[axisN]);
        pGB->sizeXY[axisN] = v3lengthXY(sizeVector[axisN]);
    }
    //calculate visible bounding box
    v3setAll(pGB->bbMin, 1000000);
    v3setAll(pGB->bbMax, -1000000);
    for (int z = -1; z <= 1; z += 2)
        for (int y = -1; y <= 1; y += 2)
            for (int x = -1; x <= 1; x += 2)
                for (int i = 0; i < 3; i++) {
                    float xx = pGB->bbMid[i] + sizeVector[2][i] * z + sizeVector[1][i] * y + sizeVector[0][i] * x;
                    if (pGB->bbMin[i] > xx)
                        pGB->bbMin[i] = xx;
                    if (pGB->bbMax[i] < xx)
                        pGB->bbMax[i] = xx;
                }
    adjustMidRad(pGB);

    if (targetRads == NULL)
        pGB->isInViewRange = 0; //not a GL range
    else{ //dealing with GL range, 
        //check if within view range
        int score = 0;
        for (int i = 0; i < 3; i++) {
            if (pGB->bbMin[i] > 1) {
                score = -1;
                break;
            }
            if (pGB->bbMax[i] < -1) {
                score = -1;
                break;
            }
            if (pGB->bbMin[i] > -1)
                score++;
            if (pGB->bbMax[i] < 1)
                score++;
        }
        if (score < 0)
            pGB->isInViewRange = -1; //off-screen
        else if (score == 6)
            pGB->isInViewRange = 1; //on-screen
        else
            pGB->isInViewRange = 0; //partially

        //convert xy coords from GL to pixels
        for (int i = 0; i < 2; i++) {
            pGB->bbMin[i] *= targetRads[i];
            pGB->bbMax[i] *= targetRads[i];
        }
        pGB->adjustMidRad(pGB);
    }
}

int Gabarites::fillGabarites(Gabarites* pGB, mat4x4 absModelMatrix, Gabarites* pGB00, mat4x4 mViewProjection, float* targetRads) {
    //build MVP matrix for given subject
    mat4x4 mMVP;
    mat4x4_mul(mMVP, mViewProjection, absModelMatrix);

    fillBBox(pGB, pGB00, mMVP, targetRads);
    if (pGB->isInViewRange < 0)
        return 0;
    if(pGB->chordType<0)
        return 0; //don't need chord

     //CHORDE: select biggest axis
     int biggestAxisN = 0;
     float biggestSize = 0;
     for (int axisN = 0; axisN < 3; axisN++) {
         if (biggestSize >= pGB->sizeXY[axisN])
             continue;
         biggestSize = pGB->sizeXY[axisN];
         biggestAxisN = axisN;
     }
     float* pDirChord = pGB->sizeDirXY[biggestAxisN];
     //perpendicular
     float pDirPerp[2];
     pDirPerp[0] = pDirChord[1];
     pDirPerp[1] = -pDirChord[0];
     float chordH = 0; //half-length
     float chordR = 0; //rad
     for (int i = 0; i < 3; i++) {
         if (i == biggestAxisN) {
             chordH += pGB->sizeXY[i];
             continue;
         }
         if (pGB->sizeXY[i] == 0)
             continue;
         chordH = chordH + abs(v2dotProductNorm(pDirChord, pGB->sizeDirXY[i])) * pGB->sizeXY[i];
         chordR = chordR + abs(v2dotProductNorm(pDirPerp, pGB->sizeDirXY[i])) * pGB->sizeXY[i];
     }
     pGB->chordR = chordR;
     chordH -= chordR;
     
     if (chordH < 1.0f) //dot
         pGB->chord.initLineXY(&pGB->chord, pGB->bbMid, pGB->bbMid);
     else {
         float dot0[2];
         float dot1[2];
         for (int i = 0; i < 2; i++) {
             float d = pDirChord[i] * chordH;
             dot0[i] = pGB->bbMid[i] + d;
             dot1[i] = pGB->bbMid[i] - d;
         }
         pGB->chord.initLineXY(&pGB->chord, dot0, dot1);
     }
     
     return 1;
}
float Gabarites::checkCollision(float* collisionPoint, Gabarites* pGB1, Gabarites* pGB2) {
    //returns penetration depth
    //check bounding box first
    for (int i = 0; i < 3; i++){
        if (pGB1->bbMin[i] >= pGB2->bbMax[i])
            return 0;
        if (pGB1->bbMax[i] <= pGB2->bbMin[i])
            return 0;
    }
    //if here - bounding boxes overlap
    float chordsRR = pGB1->chordR + pGB2->chordR;
    float dist = LineXY::dist_l2l(&pGB1->chord, &pGB2->chord);
    float overlap = chordsRR - dist;
    if (overlap <= 0)//have collision
        return 0;
    //find collision point
    float k1 = pGB2->chordR / chordsRR;
    float k2 = 1.0f - k1;
    for (int i = 0; i < 2; i++)
        collisionPoint[i] = pGB1->chord.closestPoint[i] * k1 + pGB2->chord.closestPoint[i] * k2;
    //z component
    float zOverlapMin = std::max(pGB1->bbMin[2], pGB2->bbMin[2]);
    float zOverlapMax = std::min(pGB1->bbMax[2], pGB2->bbMax[2]);
    collisionPoint[2] = (zOverlapMin + zOverlapMax) / 2;
    return overlap;
}
