#pragma once
#include "UISubj.h"

class ProgressBar : UISubj
{
public:
	int stepN = 0;
	int stepsN = 1;
	uint64_t lastStepMillis = 0;
	char fullPath[256] = "";
public:
	ProgressBar(int stepsN);
	ProgressBar(std::string fullPath);
	virtual ~ProgressBar();
	static int showProgressBar(ProgressBar* pPB);
	static int nextStep(ProgressBar* pPB, std::string stepTitle = "");
};
