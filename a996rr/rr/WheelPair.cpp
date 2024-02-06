#include "WheelPair.h"
#include "TheApp.h"
#include "XMLparser.h"
#include "RollingStock.h"

extern TheApp theApp;
extern float PI;


int WheelPair::moveWheelPair(WheelPair* pWP) {
    if (pWP->wheelRoot) {
        if (pWP->d2wheel2follow != 0) {
            WheelPair* pLeader = (WheelPair*)pWP->pSubjsSet->at(pWP->nInSubjsSet - pWP->d2wheel2follow);
            pWP->spinAngle = pLeader->spinAngle;
            if (pWP->alignedWithLeader < 0)
                pWP->spinAngle = -pWP->spinAngle;
            if (pWP->followAngleShift != 0)
                pWP->spinAngle = angleDgNorm180(pWP->spinAngle + pWP->followAngleShift);
        }
        else { //main wheel itself
            //get root speed
            RollingStock* pWagon = (RollingStock*)pWP->pSubjsSet->at(pWP->rootN);
            RollingStock* pRoot = (RollingStock*)pWagon->pSubjsSet->at(pWagon->trainRootN);
            float rootMovedBy = pRoot->ownZspeed;
            if (rootMovedBy != 0) {                
                float spinD = 360.0 / pWP->wheelCircumference * rootMovedBy;
                if (pWP->alignedWithRoot < 0)
                    spinD = -spinD;
                if (pWagon->alignedWithTrainHead != pRoot->alignedWithTrainHead)
                    spinD = -spinD;
                pWP->spinAngle= angleDgNorm180(pWP->spinAngle + spinD);
            }
        }
    }
    else { //not a wheelRoot
        SceneSubj* pParent = pWP->pSubjsSet->at(pWP->nInSubjsSet - pWP->d2parent);
        WheelPair* pParentWP = (WheelPair*)pParent;
        pWP->spinAngle = pParentWP->spinAngle;
    }
    if(pWP->spokesN>0)
        pWP->ownCoords.setPitch(pWP->spinAngle);
    return 1;
}


int WheelPair::onDeployWheelPair(WheelPair* pWP, std::string tagStr) {
    //set random angle
    if (pWP->wheelRoot)
        pWP->spinAngle = getRandom(0, 360);
    return 1;
}

int WheelPair::onLoadWheelPair(WheelPair* pWP, std::string tagStr) {
    SceneSubj* pParent = pWP->pSubjsSet->at(pWP->nInSubjsSet - pWP->d2parent);
    pWP->wheelRoot = (strstr(pParent->className, "WheelPair") == NULL);

    //tag instructions
    readClassPropsWP(pWP, tagStr);
    return 1;
}

int WheelPair::readClassPropsWP(WheelPair* pWP, std::string tagStr) {
    if (tagStr.length() == 0)
        return 0;


    XMLparser::setFloatValue(&pWP->followAngleShift, "followAngleShift", tagStr);

    XMLparser::setFloatValue(&pWP->wheelRadius, "wheelRadius", tagStr);
    pWP->wheelCircumference = PI * 2.0 * pWP->wheelRadius;

    XMLparser::setIntValue(&pWP->spokesN, "spokesN", tagStr);
    if (XMLparser::varExists("static", tagStr))
        pWP->spokesN = 0;
    //check leader
    pWP->d2wheel2follow = 0;
    std::string followWhom = XMLparser::getStringValue("follow", tagStr);
    if (!followWhom.empty()) {
        //find leader by name
        for (int sN = pWP->nInSubjsSet - 1; sN >= pWP->rootN; sN--) {
            SceneSubj* pSS = pWP->pSubjsSet->at(sN);
            if (strcmp(pSS->name64, followWhom.c_str()) == 0) {
                pWP->d2wheel2follow = pWP->nInSubjsSet - sN;
                break;
            }
        }
    }
    if(pWP->spokesN>0)
        pWP->angleBetweenSpokes = 360.0 / pWP->spokesN;

    return 1;
}