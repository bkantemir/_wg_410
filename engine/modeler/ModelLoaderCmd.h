#pragma once
#include "XMLparser.h"
#include "SceneSubj.h"
#include <map>

class ModelLoaderCmd : public XMLparser
{
public:
	SceneSubj* pSS00 = NULL;
	std::vector<SceneSubj*>* pSubjsVector = NULL;

	std::vector<MaterialAdjust*>* pTempMaterials = NULL;
	std::vector<MyColor*>* pTempColors = NULL;
	std::map<std::string, std::string> tempVarsHashMap;
public:
	ModelLoaderCmd(SceneSubj* pSS0, std::string filePath) : XMLparser(filePath) {
		pSS00 = pSS0;
		pSubjsVector = pSS00->pSubjsSet;
	};
	virtual ~ModelLoaderCmd();
	virtual int processTag() { return processTag(this); };
	static int processTag(ModelLoaderCmd* pML);
	static std::string pickOneOf(std::string optsString);
	static MyColor* findColor(ModelLoaderCmd* pML, const char* colorLabel);
	static MaterialAdjust* findMaterial(ModelLoaderCmd* pML, const char* materialLabel);
};
