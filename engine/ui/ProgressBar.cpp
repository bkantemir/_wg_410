#include "ProgressBar.h"
#include "utils.h"

ProgressBar::ProgressBar(int stepsN) {
	std::vector<DrawJob*>* pDrawJobs = &DrawJob::drawJobs_default;
	ProgressBar* pPB = this;
	pPB->stepsN = stepsN;
	pPB->stepN = 0;
	pPB->djStartN = djNclr;
	memcpy(&pPB->mt0, &pDrawJobs->at(pPB->djStartN)->mt, sizeof(Material));
	showProgressBar(pPB);
	lastStepMillis = getSystemMillis();
}
ProgressBar::ProgressBar(std::string fullPath) {
	std::vector<DrawJob*>* pDrawJobs = &DrawJob::drawJobs_default;
	ProgressBar* pPB = this;
	pPB->stepsN = 100;

	strcpy_s(pPB->fullPath, 256, fullPath.c_str());
	if (fileExists(fullPath.c_str())) {
		FILE* inFile = NULL;
		fopen_s(&inFile, pPB->fullPath, "r");
		if (inFile != NULL) {
			fread(&pPB->stepsN, 1, sizeof(int), inFile);
			fclose(inFile);
		}
	}
	pPB->stepN = 0;
	pPB->djStartN = djNclr;
	memcpy(&pPB->mt0, &pDrawJobs->at(pPB->djStartN)->mt, sizeof(Material));
	showProgressBar(pPB);
	lastStepMillis = getSystemMillis();
}
ProgressBar::~ProgressBar() {
	ProgressBar* pPB = this;
	mylog("~ProgressBar(): %d steps of %d\n", pPB->stepN, pPB->stepsN);
	//save info?
	if (pPB->stepN != pPB->stepsN)
		if (strlen(pPB->fullPath) > 10) {
			makeDirs(std::string(pPB->fullPath));
			FILE* outFile = NULL;
			fopen_s(&outFile, pPB->fullPath, "w+");
			if (outFile != NULL)
			{
				int forRecord = (pPB->stepN + pPB->stepsN) / 2;
				fwrite(&forRecord, 1, sizeof(int), outFile);
				fflush(outFile);
				fclose(outFile);
			}
			else
				mylog("ERROR creating %s\n", pPB->fullPath);
		}
	pPB->stepN = pPB->stepsN - 1;
	showProgressBar(pPB);
}
int ProgressBar::showProgressBar(ProgressBar* pPB) {
	//return 0;
	std::vector<DrawJob*>* pDrawJobs = &DrawJob::drawJobs_default;
	//clear screen	
	glClearColor(0.8, 0.8, 0.8, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float rate = (float)pPB->stepN / (pPB->stepsN - 1);
	if (rate > 1)
		rate = 1;

	float w = screenSize[0] * 0.9;
	float h = 2;
	float y = screenSize[1] * 0.25;
	float x = 0;

	//black frame
	Material* pMt = &pPB->mt0;
	pMt->uColor.setRGBA(0, 0, 0, 255);
	setCoords(pPB, x, y, w + 4, h + 4, "");
	pPB->render();

	//bar
	pMt->uColor.setRGBA(0, 255, 0, 255);
	x = (screenSize[0] - w) / 2;
	w = w * rate;
	setCoords(pPB, x, y, w, h, "left");
	pPB->render();

	//bar tip
	pMt->uColor.setRGBA(200, 255, 200, 255);
	h += 2;
	x = x + w - h;
	w = h;
	setCoords(pPB, x, y, w, h, "left");
	pPB->render();

	mySwapBuffers();
	return 1;
}
int ProgressBar::nextStep(ProgressBar* pPB, std::string stepTitle) {
	if (pPB == NULL)
		return 0;
	pPB->stepN++;
	showProgressBar(pPB);

	uint64_t currentMillis = getSystemMillis();
	uint64_t currentStepMillis = currentMillis - pPB->lastStepMillis;
	pPB->lastStepMillis = currentMillis;
	//mylog(" step %d - %d millis. %s\n", pPB->stepN, (int)currentStepMillis, stepTitle.c_str());
	return 1;
}
