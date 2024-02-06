#include "Coupler.h"
#include "RollingStock.h"
#include "TheApp.h"
#include "Shadows.h"

extern TheApp theApp;
extern float degrees2radians;


float Coupler::couplingDistances[3] = { 5,7,10 };

int Coupler::onLoadCoupler(Coupler* pCp, std::string tagStr) {
	//tag instructions
	if (tagStr.length() == 0)
		return 0;

	if (XMLparser::varExists("full", tagStr))
		pCp->hook = 1;
	return 1;
}

int Coupler::checkCoupler(Coupler* pCoupler) {
	SceneSubj* pRoot = pCoupler->pSubjsSet->at(pCoupler->rootN);
	if (strstr(pRoot->className, "RollingStock") != pRoot->className)
		return 0;
	RollingStock* pWagon = (RollingStock*)pRoot;
	memcpy(&pCoupler->railCoordOld, &pCoupler->railCoord, sizeof(RailCoord));
	theApp.gameTable.shiftRailCoord(&pCoupler->railCoord, &pWagon->railCoord, &theApp.gameTable, pCoupler->zOffsetFromRoot);
	pCoupler->railCoord.alignedWithRail *= pCoupler->alignedWithRoot;
	/*
	{
		//hook y pos
		SceneSubj* pLock= pCoupler->pSubjsSet->at(pCoupler->nInSubjsSet+1);
		
		pLock->ownCoords.pos[1] = 30;// 25;
	}
	*/
	if (pCoupler->connected == 1)
		return 0; //hard connected
	if (pCoupler->connected == 2) {
		//conditionally connected
		return 0;
	}
	//if here - not coupled, check couplers around,
	float far = theApp.gameTable.tileSize;
	Coupler* p2skip = NULL;
	//check coupler from prev frame firs
	if (pCoupler->pCounterCoupler != NULL) {
		Coupler* pCp2 = pCoupler->pCounterCoupler;
		if (pCp2->connected > 0) {
			//coupler intercepted
			pCoupler->pCounterCoupler = NULL;
		}
		else {
			//mylog("%d: checking coupler %d vs %d\n", (int)theApp.frameN, pCoupler->nInSubjsSet, pCoupler->pCounterCoupler->nInSubjsSet);
			float d = theApp.gameTable.offset2railCoord(&pCoupler->railCoord, &pCoupler->pCounterCoupler->railCoord, &theApp.gameTable, far);
			if (d < far) {//still actual
				//update distance
				pCoupler->distanceBefore = pCoupler->distance;
				pCoupler->distance = d;
			}
			else { //too far
				//release corresponding coupler too
				p2skip = pCp2;
				if (pCp2->pCounterCoupler == pCoupler) {
					pCp2->pCounterCoupler = NULL;
					pCp2->connected = -1;
				}
				pCoupler->pCounterCoupler = NULL;
				pCoupler->connected = -1;
			}
		}
	}
	if (pCoupler->pCounterCoupler == NULL) {
		//check around for candidates
		Coupler* pClosest = NULL;
		float bestDistance = 1000000;
		for (int sN = 0; sN < pCoupler->nInSubjsSet; sN++) {
			SceneSubj* pSS = pCoupler->pSubjsSet->at(sN);
			if (pSS == NULL)
				continue;
			if (strcmp(pSS->className, "coupler") != 0)
				continue;
			Coupler* pCp2 = (Coupler*)pSS;

			if (pCp2 == p2skip) {
				continue;
			}
			if (pCp2->connected > 0) {
				continue;
			}
			if (pCp2->rootN == pCoupler->rootN) {
				continue;
			}

			float d = abs(pCoupler->absCoords.pos[0] - pCp2->absCoords.pos[0]);
			if (d >= far) {
				continue; //too far
			}
			d = abs(pCoupler->absCoords.pos[2] - pCp2->absCoords.pos[2]);
			if (d >= far) {
				continue; //too far
			}
			d = theApp.gameTable.offset2railCoord(&pCoupler->railCoord, &pCp2->railCoord, &theApp.gameTable, far);

			if (d >= far) {
				continue; //wrong direction
			}
			if (bestDistance > d) {
				bestDistance = d;
				pClosest = pCp2;
			}
		}
		if (pClosest != NULL) {	//have new candidate
			pCoupler->connected = 0;//coupler ahead
			pCoupler->pCounterCoupler = pClosest;
			pCoupler->distance = bestDistance;
			pCoupler->distanceBefore = far;
		}
	}
	if (pCoupler->pCounterCoupler == NULL)
		return 0;
	//if here - have a counter coupler
	int couplersInvolved = pCoupler->hook + pCoupler->pCounterCoupler->hook;
	float couplingDistance = couplingDistances[couplersInvolved];
	if (pCoupler->distance <= couplingDistance) {
		//if here - close enough
		RollingStock* pWagon2 = (RollingStock*)pWagon->pSubjsSet->at(pCoupler->pCounterCoupler->rootN);
		if (pWagon->trainRootN == pWagon2->trainRootN)
			return 0;//ignore - own tail
		if (couplingDistance - pCoupler->distance > 3) {
			//TOO close - keep divorce
			RollingStock::divorceTrains(pWagon,pWagon2);
			//mylog("d=%d - divorce\n", (int)pCoupler->distance);
		}
		else {
			//if here - coupling?
			if(pCoupler->distance < pCoupler->distanceBefore)
			{
				/*
				Coupler* pCp2 = pCoupler->pCounterCoupler;
				mylog("\n>>>%d: d=%d - ", (int)theApp.frameN,(int)pCoupler->distance);
				mylog("%d.", pCoupler->rootN);
				if (pCoupler->zOffsetFromRoot > 0)
					mylog("head");
				else
					mylog("back");
				mylog("-to-");
				mylog("%d.", pCp2->rootN);
				if (pCp2->zOffsetFromRoot > 0)
					mylog("head");
				else
					mylog("back");
				mylog(" coupling\n", (int)pCoupler->distance);
				RollingStock::trainToLog(pCoupler);
				RollingStock::trainToLog(pCp2);
				*/
				pCoupler->connected = 1;
				pCoupler->couplersInvolved = couplersInvolved;
			}
		}
	}
	//duplicate in corresponding coupler
	Coupler* pCp2 = pCoupler->pCounterCoupler;
	if (pCoupler->connected>0 || pCp2->pCounterCoupler == pCoupler || pCp2->pCounterCoupler == NULL ) {
		pCp2->pCounterCoupler = pCoupler;
		pCp2->distance = pCoupler->distance;
		pCp2->connected = pCoupler->connected;
		pCp2->couplersInvolved = pCoupler->couplersInvolved;
	}
	
	if (pCoupler->connected > 0) {
		/*
		{
		RollingStock* pW = (RollingStock*)pCoupler->pSubjsSet->at(pCoupler->rootN);
		pW = (RollingStock*)pW->pSubjsSet->at(pW->trainRootN);
		int expectedWagonsN = pW->tr.carsTotal;
		pW = (RollingStock*)pCp2->pSubjsSet->at(pCp2->rootN);
		pW = (RollingStock*)pW->pSubjsSet->at(pW->trainRootN);
		expectedWagonsN += pW->tr.carsTotal;
		}
		*/
		int wagonsN=RollingStock::reinspectTrain(pCp2);
		/*
		mylog("Result:\n");
		RollingStock::trainToLog(pCp2);

		if (expectedWagonsN != wagonsN) {
			mylog("Wrong coupling!!\n");
			int a = 0;
		}
		*/
	}
		
	return 1;
}
int Coupler::renderCoupler(Coupler* pCp, Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter) {
	if (pCp->djTotalN < 1)
		return 0;
	//render coupler box
	renderStandard(pCp, pCam, dirToMainLight, dirToTranslucent, renderFilter);
	if (pCp->hook == 0)
		return 1;
	//have coupler hook
	if (pCp->connected < 1 || pCp->couplersInvolved < 2) {
		//render simple single coupler
		SceneSubj* pHook = Rail::railModels.at(Rail::couplerModelN);
		m16copy((float*)pHook->absModelMatrix, (float*)pCp->absModelMatrix);
		m16copy((float*)pHook->absModelMatrixUnscaled, (float*)pCp->absModelMatrix);

		renderStandard(pHook, pCam, dirToMainLight, dirToTranslucent, renderFilter);
		return 1;
	}
	//if here - have connected couplers pair
	if (pCp->nInSubjsSet < pCp->pCounterCoupler->nInSubjsSet)
		return 0;//will render later, with counterCoupler
	//here - render couplers pair
	Coupler* pCp2 = pCp->pCounterCoupler;
	float pos[4];
	for (int i = 0; i < 3; i++)
		pos[i] = (pCp->absCoords.pos[i] + pCp2->absCoords.pos[i]) * 0.5f;

	float yaw = v3yawDgFromTo(pCp->absCoords.pos, pCp2->absCoords.pos);
	mat4x4 posModelMatrix;
	mat4x4_translate(posModelMatrix, pos[0], pos[1], pos[2]);
	mat4x4 absModelMatrix;
	mat4x4_rotate_Y(absModelMatrix, posModelMatrix, yaw * degrees2radians);

	SceneSubj* pPair = Rail::railModels.at(Rail::couplersCoupleModelN);
	m16copy((float*)pPair->absModelMatrix, (float*)absModelMatrix);
	m16copy((float*)pPair->absModelMatrixUnscaled, (float*)absModelMatrix);

	renderStandard(pPair, pCam, dirToMainLight, dirToTranslucent, renderFilter);

	//shaft
	float dist = v3lengthFromToXZ(pCp->absCoords.pos, pCp2->absCoords.pos);
	mat4x4_scale_aniso(absModelMatrix, absModelMatrix, 1, 1, dist + 2);

	SceneSubj* pShaft = Rail::railModels.at(Rail::couplersCoupleModelN + 1);
	m16copy((float*)pShaft->absModelMatrix, (float*)absModelMatrix);
	m16copy((float*)pShaft->absModelMatrixUnscaled, (float*)absModelMatrix);

	renderStandard(pShaft, pCam, dirToMainLight, dirToTranslucent, renderFilter);
	return 1;
}
int Coupler::renderDepthMapCoupler(Coupler* pCp, Camera* pCam) {
	if (pCp->djTotalN < 1)
		return 0;
	//render coupler box
	renderDepthMapStandard(pCp, pCam);
	if (pCp->hook == 0)
		return 1;
	//have coupler hook
	if (pCp->connected < 1 || pCp->couplersInvolved < 2) {
		//render simple single coupler
		SceneSubj* pHook = Rail::railModels.at(Rail::couplerModelN);
		m16copy((float*)pHook->absModelMatrix, (float*)pCp->absModelMatrix);
		m16copy((float*)pHook->absModelMatrixUnscaled, (float*)pCp->absModelMatrix);

		renderDepthMapStandard(pHook, pCam);
		return 1;
	}
	//if here - have connected couplers pair
	if (pCp->nInSubjsSet < pCp->pCounterCoupler->nInSubjsSet)
		return 0;//will render later, with counterCoupler
	//here - render couplers pair
	Coupler* pCp2 = pCp->pCounterCoupler;
	float pos[4];
	for (int i = 0; i < 3; i++)
		pos[i] = (pCp->absCoords.pos[i] + pCp2->absCoords.pos[i]) * 0.5f;

	float yaw = v3yawDgFromTo(pCp->absCoords.pos, pCp2->absCoords.pos);
	mat4x4 posModelMatrix;
	mat4x4_translate(posModelMatrix, pos[0], pos[1], pos[2]);
	mat4x4 absModelMatrix;
	mat4x4_rotate_Y(absModelMatrix, posModelMatrix, yaw * degrees2radians);

	SceneSubj* pPair = Rail::railModels.at(Rail::couplersCoupleModelN);
	m16copy((float*)pPair->absModelMatrix, (float*)absModelMatrix);
	m16copy((float*)pPair->absModelMatrixUnscaled, (float*)absModelMatrix);
	renderDepthMapStandard(pPair, pCam);

	//shaft
	float dist = v3lengthFromToXZ(pCp->absCoords.pos, pCp2->absCoords.pos);
	mat4x4_scale_aniso(absModelMatrix, absModelMatrix, 1, 1, dist + 2);

	SceneSubj* pShaft = Rail::railModels.at(Rail::couplersCoupleModelN + 1);
	m16copy((float*)pShaft->absModelMatrix, (float*)absModelMatrix);
	m16copy((float*)pShaft->absModelMatrixUnscaled, (float*)absModelMatrix);
	renderDepthMapStandard(pShaft, pCam);
	return 1;
}

