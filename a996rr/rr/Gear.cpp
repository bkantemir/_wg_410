#include "Gear.h"
#include "XMLparser.h"
#include "WheelPair.h"
#include "Rail.h"
#include "geom2d/ArcXY.h"

extern float degrees2radians;

float Gear::dxMainRod = 7;
float Gear::dxRimWall = 4.5 + Rail::wheelWidth / 2;
float Gear::dxSideRod = (dxRimWall + dxMainRod) / 2;
float Gear::dxBetweenRods = dxSideRod - dxRimWall;


int Gear::onLoadGear(Gear* pGr, std::string tagStr) {
    SceneSubj* pParent = pGr->pSubjsSet->at(pGr->nInSubjsSet - pGr->d2parent);
    pGr->gearRoot = (strstr(pParent->className, "Gear") == NULL);
        
	//tag instructions
	readClassPropsGear(pGr, tagStr);

    //cylinder line
    if (pGr->gearRoot) {
        float p0[3] = { 0,0,1 };
        float p1[3] = { 0,0,0 };
        if (pGr->d2wheel2follow != 0) {
            WheelPair* pLeaderWheel = (WheelPair*)pGr->pSubjsSet->at(pGr->nInSubjsSet - pGr->d2wheel2follow);
            p0[1] = pLeaderWheel->ownCoords.pos[1];
            p1[1] = pLeaderWheel->ownCoords.pos[1];
        }
        LineXY::initLineZY(&pGr->cylinderShaft, p0, p1);
    }
    //dx correction
    if (strcmp(pGr->name64, "side_rod") == 0)
        pGr->ownCoords.pos[0] = dxSideRod;
    else if (strcmp(pGr->name64, "main_rod") == 0)
        pGr->ownCoords.pos[0] = dxMainRod;
    else if (strcmp(pGr->name64, "crosshead") == 0)
        pGr->ownCoords.pos[0] = dxMainRod;
    return 1;
}
int Gear::readClassPropsGear(Gear* pGr, std::string tagStr) {
    if (tagStr.length() == 0)
        return 0;

    //check leader wheel
    pGr->d2wheel2follow = 0;
    std::string followWhom = XMLparser::getStringValue("follow", tagStr);
    if (!followWhom.empty()) {
        //find leader by name
        for (int sN = pGr->nInSubjsSet - 1; sN >= pGr->rootN; sN--) {
            SceneSubj* pSS = pGr->pSubjsSet->at(sN);
            if (strcmp(pSS->name64, followWhom.c_str()) == 0) {
                pGr->d2wheel2follow = pGr->nInSubjsSet - sN;
                break;
            }
        }
    }

    XMLparser::setFloatValue(&pGr->followAngleShift, "followAngleShift", tagStr);

    XMLparser::setFloatValue(&pGr->dz, "dz", tagStr);

    if (strcmp(pGr->name64, "crosshead") == 0) {
        //find d2mainrod
        for (int sN = pGr->nInSubjsSet - 1; sN >= pGr->rootN; sN--) {
            SceneSubj* pSS = pGr->pSubjsSet->at(sN);
            if (strcmp(pSS->name64, "main_rod") == 0) {
                pGr->d2mainrod = pGr->nInSubjsSet - sN;
                break;
            }
        }
    }

    return 1;
}
int Gear::moveGear(Gear* pGr) {
    std::vector<SceneSubj*>* pSubjsSet = pGr->pSubjsSet;
    WheelPair* pLeaderWheel = NULL;
    if (pGr->d2wheel2follow != 0)
        pLeaderWheel = (WheelPair*)pSubjsSet->at(pGr->nInSubjsSet - pGr->d2wheel2follow);
    if (pGr->gearRoot) {
        pGr->spinAngle = pLeaderWheel->spinAngle;
        if (pGr->followAngleShift != 0)
            pGr->spinAngle = angleDgNorm180(pGr->spinAngle + pGr->followAngleShift);
        float angleRd = (pGr->spinAngle) * degrees2radians;
        pGr->spinDz = -sin(angleRd) * pGr->lever;
        pGr->spinDy = -cos(angleRd) * pGr->lever;
        return 1;
    }
    //gear elements
    Gear* pGrRoot = (Gear*)pSubjsSet->at(pGr->nInSubjsSet - pGr->d2parent);
    pGr->cullFace= pGrRoot->cullFace;

    if (pLeaderWheel == NULL) //get from gear root
        pLeaderWheel = (WheelPair*)pSubjsSet->at(pGrRoot->nInSubjsSet - pGrRoot->d2wheel2follow);
    if (strcmp(pGr->name64, "side_rod") == 0) {
        pGr->ownCoords.pos[1] = pGrRoot->spinDy + pLeaderWheel->ownCoords.pos[1];
        pGr->ownCoords.pos[2] = pGrRoot->spinDz + pLeaderWheel->ownCoords.pos[2] + pGr->dz;
        return 1;
    }
    if (strcmp(pGr->name64, "main_rod") == 0) {
        pGr->ownCoords.pos[1] = pGrRoot->spinDy + pLeaderWheel->ownCoords.pos[1];
        pGr->ownCoords.pos[2] = pGrRoot->spinDz + pLeaderWheel->ownCoords.pos[2] + pGr->dz;
        ArcXY arc;
        ArcXY::initArc(&arc, pGr->ownCoords.pos[2], pGr->ownCoords.pos[1], pGr->lever, 0, 180, 1);
        int preferance[2] = { 1,0 };//pick higher X
        ArcXY::crossLine(pGr->crossPoint, &arc, &pGrRoot->cylinderShaft, preferance);
        float angle = v2angleFromToDg(arc.centerPos, pGr->crossPoint);
        pGr->ownCoords.set1Dg(0, angle);
        return 1;
    }
    if (strcmp(pGr->name64, "crosshead") == 0) {
        Gear* pMainRod = (Gear*)pSubjsSet->at(pGr->nInSubjsSet - pGr->d2mainrod);
        pGr->ownCoords.pos[1] = pMainRod->crossPoint[1];
        pGr->ownCoords.pos[2] = pMainRod->crossPoint[0];
        return 1;
    }
    return 1;
}


