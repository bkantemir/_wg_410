#include "Switcher.h"

int Switcher::onLoadSwitcher(Switcher* pSS0, std::string tagStr) {
	//inspect childs
	for (int sN = pSS0->pSubjsSet->size() - 1; sN > pSS0->nInSubjsSet; sN--) {
		SceneSubj* pSS = pSS0->pSubjsSet->at(sN);
		if (pSS->nInSubjsSet - pSS->d2parent != pSS0->nInSubjsSet)
			continue;
		//if here - switcher's child
		if (pSS0->d2lastChild == 0)
			pSS0->d2lastChild = pSS->d2parent;
		pSS0->childsN++;
	}
	activateChildN(pSS0,0);
	return pSS0->childsN;
}
int Switcher::activateChildN(Switcher* pSS0, int activateChildN) {
	pSS0->activeChildN = activateChildN;
	int childN = 0;
	for (int sN = pSS0->nInSubjsSet + 1; sN <= pSS0->nInSubjsSet + pSS0->d2lastChild; sN++) {
		SceneSubj* pSS = pSS0->pSubjsSet->at(sN);
		if (pSS == NULL)
			continue;
		if (pSS->nInSubjsSet - pSS->d2parent != pSS0->nInSubjsSet)
			continue;
		//if here - switcher's child
		if (childN == activateChildN)
			pSS->setHide(0);
		else
			pSS->setHide(1);
		childN++;
	}
	return 1;
}

