#pragma once
#include "Switcher.h"

class DirLight : public Switcher
{
public:
	int dir = 1; //if-1 - reverse
public:
	DirLight() {};
	DirLight(DirLight* pSS0) { memcpy((void*)this, (void*)pSS0, sizeof(DirLight)); };
	virtual DirLight* clone() {
		if (strcmp(this->className, "DirLight") != 0)
			return NULL;
		return new DirLight(this);
	};
	virtual int moveSubj() { return moveDirLight(this); };
	static int moveDirLight(DirLight* pGS);

};
