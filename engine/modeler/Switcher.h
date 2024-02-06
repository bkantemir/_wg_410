#pragma once
#include "SceneSubj.h"

class Switcher : public SceneSubj
{
public:
	int childsN = 0;
	int d2lastChild = 0;
	int activeChildN=0; //child #, not nInSubjsSet
public:
	virtual int onLoad(std::string tagStr) { return onLoadSwitcher(this, tagStr); };
	static int onLoadSwitcher(Switcher* pSS, std::string tagStr);
	int activateChildN(int childN) {return activateChildN(this, childN);};
	static int activateChildN(Switcher* pSS, int childN);
};