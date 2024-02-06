#pragma once
#include <vector>
#include "Coords2D.h"
#include "linmath.h"
#include "DrawJob.h"

class UISubj
{
public:
	int d2parent = 0; //shift to parent object
	char className[32] = "";
	char name[32] = "";

	Coords2D ownCoords;
	char countFrom[32] = ""; //top left...
	Coords2D absCoords;
	Coords2D ownSpeed;
	float scale[3] = { 1,1,1 };
	mat4x4 transformMatrix = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
	bool transformMatrixIsReady = false;

	int djStartN = 0; //first DJ N in DJs array (DrawJob::drawJobs)
	int djTotalN = 1; //number of DJs
	Material mt0;
	std::vector<UISubj*>* pSubjs = NULL;
	int nInSubjs = -1;

	static int djNtex;
	static int djNclr;
	static int djNdepthmap;
	static mat4x4 mOrthoViewProjection;
	static std::vector<UISubj*> uiSubjs_default;

	static float screenSize[2];
	static float screenAspectRatio;

public:
	UISubj(std::vector<UISubj*>* pSubjs = &uiSubjs_default);
	virtual ~UISubj();
	static int init();
	static int cleanUp();
	static int renderAll();
	static int onScreenResize(int w, int h);
	virtual int render() { return renderStandard(this); };
	static int renderStandard(UISubj* pUI);
	static int addZBufferSubj(std::string uiName, float x, float y, float w, float h, std::string alignTo, int uTex0);
	static int addTexSubj(std::string uiName, float x, float y, float w, float h, std::string alignTo, int uTex0);
	static int addClrSubj(std::string uiName, float x, float y, float w, float h, std::string alignTo, unsigned int rgba);
	static int setCoords(UISubj* pUI, float x, float y, float w, float h, std::string countFrom);
	static void buildModelMatrix(UISubj* pUI);
	static int executeDJbasic(DrawJob* pDJ, float* uMVP, Material* pMt);

};

