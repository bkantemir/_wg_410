#pragma once
#include "ModelLoader.h"

class ModelLoaderRR : public ModelLoader
{
public:
	float wheelRadius = 10;
	float wheelRadialSections = 10;
	float rimWallH = 1;
	float axisD = 2;
	float centerDiameter = 0;
	float distanceBetweenSpokes = 0;
	float spokeWidth=1;
	float dishOffset2rimWall = 0;
public:
	ModelLoaderRR(std::vector<SceneSubj*>* pSubjsVector0, std::vector<DrawJob*>* pDrawJobs0, std::vector<unsigned int>* pBuffersIds0,
		int subjN = -1, ModelBuilder* pMB00 = NULL, ModelBuilder* pMB = NULL, std::string filePath = "", ProgressBar* pPBar0 = NULL);
	virtual ModelLoaderRR* newModelLoader(std::vector<SceneSubj*>* pSubjsVector0, std::vector<DrawJob*>* pDrawJobs0, std::vector<unsigned int>* pBuffersIds0,
		int subjN = -1, ModelBuilder* pMB00 = NULL, ModelBuilder* pMB = NULL, std::string filePath = "", ProgressBar* pPBar0 = NULL) {
		return new ModelLoaderRR(pSubjsVector0, pDrawJobs0, pBuffersIds0, subjN, pMB00, pMB, filePath, pPBar0);
	};
	virtual int loadModel(std::vector<SceneSubj*>* pSubjsVector0, std::vector<DrawJob*>* pDrawJobs0, std::vector<unsigned int>* pBuffersIds0,
		std::string sourceFile, std::string sourceFileRoot, std::string subjClass = "", std::string tagStr = "", ProgressBar* pPBar = NULL, ModelBuilder* pMB00 = NULL, bool runOnLoad = true) {
		return loadModelRR(pSubjsVector0, pDrawJobs0, pBuffersIds0, sourceFile, sourceFileRoot, subjClass, tagStr, pPBar, pMB00, runOnLoad);
	};
	static int loadModelRR(std::vector<SceneSubj*>* pSubjsVector0, std::vector<DrawJob*>* pDrawJobs0, std::vector<unsigned int>* pBuffersIds0,
		std::string sourceFile, std::string sourceFileRoot, std::string subjClass = "", std::string tagStr = "", ProgressBar* pPBar = NULL, ModelBuilder* pMB00 = NULL, bool runOnLoad = true);
	virtual int processTag() { return processTagRR(this); };
	static int processTagRR(ModelLoaderRR* pML);
	static int processTag_buildWheelRair(ModelLoaderRR* pML);
};

