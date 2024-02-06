#include "DirLight.h"
#include "rr/RollingStock.h"
#include "utils.h"

int DirLight::moveDirLight(DirLight* pSS) {
	SceneSubj* pRoot = pSS->pSubjsSet->at(pSS->rootN);
	if (strstr(pRoot->className, "RollingStock") != pRoot->className)
		return -1;
	RollingStock* pRS = (RollingStock*)pRoot;
	if (pRS->desirableZdir == 0)
		return 0;
	int childShouldBe = pRoot->desirableZdir * pSS->dir * pSS->alignedWithRoot;
	if (childShouldBe < 0)
		childShouldBe = 0;
	if (pSS->activeChildN != childShouldBe)
		pSS->activateChildN(childShouldBe);
	return 1;
}