#pragma once
#include "XMLparser.h"
#include "ModelBuilder.h"
#include "GroupTransform.h"
#include "MaterialAdjust.h"
#include "ProgressBar.h"

class ModelLoader : public XMLparser
{
public:
	ModelBuilder* pRootModelBuilder = NULL;
	ModelBuilder* pModelBuilder = NULL;
	bool ownModelBuilder = false;
	std::vector<SceneSubj*>* pSubjsVector = NULL;
	std::vector<DrawJob*>* pDrawJobs = NULL;
	std::vector<unsigned int>* pBuffersIds = NULL;

	MaterialAdjust* pMaterialAdjust = NULL;
	int lineStartsAt = -1;

	ProgressBar* pPBar=NULL;
public:
	ModelLoader(std::vector<SceneSubj*>* pSubjsVector0, std::vector<DrawJob*>* pDrawJobs0, std::vector<unsigned int>* pBuffersIds0,
		int subjN=-1, ModelBuilder* pMB00=NULL, ModelBuilder* pMB = NULL, std::string filePath="", ProgressBar* pPBar0 = NULL);
	virtual ~ModelLoader() {
		if (!ownModelBuilder)
			return;
		pModelBuilder->buildDrawJobs(pModelBuilder, pSubjsVector, pDrawJobs, pBuffersIds);
		delete pModelBuilder;
		return;
	};
	virtual int loadModel(std::vector<SceneSubj*>* pSubjsVector0, std::vector<DrawJob*>* pDrawJobs0, std::vector<unsigned int>* pBuffersIds0,
		std::string sourceFile, std::string sourceFileRoot, std::string subjClass = "", std::string tagStr = "", ProgressBar* pPBar = NULL, ModelBuilder* pMB00 = NULL, bool runOnLoad = true) {
		return loadModelStandard(pSubjsVector0, pDrawJobs0, pBuffersIds0, sourceFile, sourceFileRoot, subjClass, tagStr, pPBar, pMB00, runOnLoad);
	};
	static int loadModelStandard(std::vector<SceneSubj*>* pSubjsVector0, std::vector<DrawJob*>* pDrawJobs0, std::vector<unsigned int>* pBuffersIds0,
		std::string sourceFile, std::string sourceFileRoot="", std::string subjClass = "", std::string tagStr = "", ProgressBar* pPBar = NULL, ModelBuilder* pMB00 = NULL, bool runOnLoad = true);
	virtual int processTag() { return processTag(this); };
	static int processTag(ModelLoader* pML);
	static int processTag_a(ModelLoader* pML); //apply
	static int setIntValueFromHashMap(int* pInt, std::map<std::string, float> floatsHashMap, std::string varName, std::string tagStr);
	static int setFloatValueFromHashMap(float* pFloat, std::map<std::string, float> floatsHashMap, std::string varName, std::string tagStr);
	static int setTexture(ModelLoader* pML, int* pInt, std::string txName, std::string tagStr);
	static int setMaterialTextures(ModelLoader* pML, Material* pMT, std::string tagStr);
	static int fillProps_vs(VirtualShape* pVS, ModelLoader* pML, std::string tagStr); //virtual shape
	static int fillProps_mt(Material* pMT, std::string tagStr, ModelLoader* pML); //Material
	static int fillProps_gt(GroupTransform* pGS, ModelBuilder* pMB, std::string tagStr);
	static int processTag_clone(ModelLoader* pML);
	static int addMark(char* marks, std::string newMark);
	static int processTag_do(ModelLoader* pML);
	static int processTag_a2mesh(ModelLoader* pML);
	static int processTag_line2mesh(ModelLoader* pML);
	static int processTag_points2mesh(ModelLoader* pML);
	static int processTag_element(ModelLoader* pML);
	static int processTag_include(ModelLoader* pML);
	static int processTag_short(ModelLoader* pML);
	static int processTag_lineStart(ModelLoader* pML);
	static int processTag_lineEnd(ModelLoader* pML);
	static int processTag_a2group(ModelLoader* pML);
	static int processTag_color_as(ModelLoader* pML);
	static int processTag_lastLineTexure(ModelLoader* pML);
	static int processTag_group2line(ModelLoader* pML);
	static int processTag_clone2line(ModelLoader* pML);
	static int processTag_lineTip(ModelLoader* pML);
	static int processTag_model(ModelLoader* pML);
	static int buildGabaritesFromDrawJobs(Gabarites* pGB, std::vector<DrawJob*>* pDrawJobs, int djStartN, int djTotalN);
	int addSubj(SceneSubj* pSS) { return addSubj(this, pSS); };
	static int addSubj(ModelLoader* pML, SceneSubj* pSS);
	static int loadModel_ifExists(std::vector<SceneSubj*>* pSubjsVector0,
		std::string sourceFile, std::string sourceFileRoot, std::string subjClass, std::string tagStr, ProgressBar* pPBar, bool runOnLoad);
	virtual ModelLoader* newModelLoader(std::vector<SceneSubj*>* pSubjsVector0, std::vector<DrawJob*>* pDrawJobs0, std::vector<unsigned int>* pBuffersIds0,
		int subjN = -1, ModelBuilder* pMB00 = NULL, ModelBuilder* pMB = NULL, std::string filePath = "", ProgressBar* pPBar0 = NULL) {
		return new ModelLoader(pSubjsVector0, pDrawJobs0, pBuffersIds0,	subjN, pMB00, pMB, filePath, pPBar0);};
	static int setMaterialLineWidth(ModelBuilder* pMB,float lineWidth);
};
