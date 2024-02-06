#include "ModelLoaderCmd.h"
#include "utils.h"
//#include "MyColor.h"

ModelLoaderCmd::~ModelLoaderCmd() {
	if (pTempColors != NULL) {
		int totalN = pTempColors->size();
		for (int i = 0; i < totalN; i++)
			delete pTempColors->at(i);
		pTempColors->clear();
		delete pTempColors;
		pTempColors = NULL;
	}
	if (pTempMaterials != NULL) {
		int totalN = pTempMaterials->size();
		for (int i = 0; i < totalN; i++)
			delete pTempMaterials->at(i);
		pTempMaterials->clear();
		delete pTempMaterials;
		pTempMaterials = NULL;
	}
}
int ModelLoaderCmd::processTag(ModelLoaderCmd* pML) {
	std::string tagStr = pML->currentTag;

//mylog("%s\n", tagStr.c_str());

	if (pML->tagName.compare("setMaterial") == 0) {
		std::string materialLabel = getStringValue("setMaterial", tagStr);
		std::string scope = getStringValue("scope", tagStr);
		std::string value = "";
		if (varExists("pickOneOf", tagStr))
			value = pickOneOf(getStringValue("pickOneOf", tagStr));
		else if (varExists("valueOf", tagStr))
			value = getStringValue("valueOf", tagStr);
		if (value.empty()) {
			mylog("ERROR in ModelLoaderCmd::processTag: can't set material %s\n", tagStr.c_str());
			return -1;
		}
		MaterialAdjust* pMA0 = findMaterial(pML, value.c_str());
		if (pMA0 == NULL) {
			mylog("ERROR in ModelLoaderCmd::processTag: can't find material '%s' %s\n", value.c_str(), tagStr.c_str());
			return -1;
		}
		SceneSubj* pSS = pML->pSS00;

		if (pSS->pCustomMaterials == NULL)
			pSS->pCustomMaterials = new std::vector<MaterialAdjust*>;
		if (pML->pTempMaterials == NULL)
			pML->pTempMaterials = new std::vector<MaterialAdjust*>;
		std::vector<MaterialAdjust*>* pMAs = pSS->pCustomMaterials;
		if (scope.compare("temp") == 0)
			pMAs = pML->pTempMaterials;
		int materialsN = pMAs->size();
		for (int mN = 0; mN < materialsN; mN++) {
			MaterialAdjust* pMA = pMAs->at(mN);
			if (strcmp(pMA->materialName32, materialLabel.c_str()) == 0) {
				pMAs->erase(pSS->pCustomMaterials->begin() + mN);
				break;
			}
		}
		MaterialAdjust* pMA = new MaterialAdjust(pMA0);
		strcpy_s(pMA->materialName32, 32, materialLabel.c_str());
		pMAs->push_back(pMA);
		return 1;
	}
	if (pML->tagName.compare("element") == 0) {
		std::string look4label = getStringValue("element", tagStr);
		std::string action = getStringValue("action", tagStr);
		for (int i = 0; i < pML->pSS00->totalElements; i++) {
			int eN = pML->pSS00->nInSubjsSet + i;
			SceneSubj* pSS = pML->pSS00->pSubjsSet->at(eN);
			if (pSS == NULL)
				continue;
			if (pSS->rootN != pML->pSS00->rootN)
				continue;
			if (strcmp(pSS->name64, look4label.c_str()) != 0)
				continue;
			if (action.compare("hide") == 0)
				pSS->setHide(1);
			else if (action.compare("unhide") == 0)
				pSS->setHide(0);
		}
		return 1;
	}
	if (pML->tagName.compare("proceedTo") == 0) {
		std::string look4label = getStringValue("proceedTo", tagStr);
		//keep scanning until meet the label
		while (nextTag(pML) != 0) {
			if (pML->tagName.compare("label") != 0)
				continue;
			if (pML->getStringValue("label", pML->currentTag).compare(look4label) == 0)
				return 1;
		}
		return 1;
	}
	if (pML->tagName.compare("label") == 0)
		return 1; //ignore
	if (pML->tagName.compare("/if") == 0)
		return 1; //ignore
	if (pML->tagName.compare("if") == 0) {
		std::string ifCondition = getStringValue("if", tagStr);
		std::string var1 = "";
		std::string val1 = "";
		if (varExists("val1", tagStr))
			val1 = getStringValue("val1", tagStr);
		else if (varExists("var1", tagStr)) {
			var1 = getStringValue("var1", tagStr);
			val1 = pML->tempVarsHashMap[var1];
		}
		std::string var2 = "";
		std::string val2 = "";
		if (varExists("val2", tagStr))
			val2 = getStringValue("val2", tagStr);
		else if (varExists("var2", tagStr)) {
			var2 = getStringValue("var2", tagStr);
			val2 = pML->tempVarsHashMap[var2];
		}
		bool conditionMet = true;
		if (ifCondition.compare("==") == 0)
			conditionMet = (val1.compare(val2) == 0);
		else if (ifCondition.compare("!=") == 0)
			conditionMet = (val1.compare(val2) != 0);
		else if (ifCondition.compare("sameColor") == 0) {
			MyColor* p1 = findColor(pML, var1.c_str());
			MyColor* p2 = findColor(pML, var2.c_str());
			uint32_t clr1 = p1->getUint32();
			uint32_t clr2 = p2->getUint32();
			conditionMet = (clr1 == clr2);
		}
		if (!conditionMet) {
			//keep scanning for corresponding </if>
			int levelN = 0;
			while (nextTag(pML) != 0) {
				if (pML->tagName.compare("if") == 0) {
					levelN++;
					continue;
				}
				if (pML->tagName.compare("/if") == 0) {
					levelN--;
					if(levelN<0)
						return 1;
				}
			}
		}
		return 1;
	}
	if (pML->tagName.compare("setVar") == 0) {
		std::string varName = getStringValue("setVar", tagStr);
		std::string value = "";
		if (varExists("pickOneOf", tagStr))
			value = pickOneOf(getStringValue("pickOneOf", tagStr));
		else if (varExists("value", tagStr))
			value = getStringValue("value", tagStr);
		pML->tempVarsHashMap[varName] = value;
		return 1;
	}
	if (pML->tagName.compare("setColor") == 0) {
		std::string colorLabel = getStringValue("setColor", tagStr);
		std::string scope = getStringValue("scope", tagStr);
		std::string value = "";
		if (varExists("pickOneOf", tagStr))
			value = pickOneOf(getStringValue("pickOneOf", tagStr));
		else if (varExists("valueOf", tagStr))
			value = getStringValue("valueOf", tagStr);
		if (value.empty()) {
			mylog("ERROR in ModelLoaderCmd::processTag: can't set color %s\n", tagStr.c_str());
			return -1;
		}
		MyColor* pCL0 = findColor(pML, value.c_str());
		if (pCL0 == NULL) {
			mylog("ERROR in ModelLoaderCmd::processTag: can't find color '%s' %s\n", value.c_str(), tagStr.c_str());
			return -1;
		}
		SceneSubj* pSS = pML->pSS00;
		if (pSS->pCustomColors == NULL)
			pSS->pCustomColors = new std::vector<MyColor*>;
		int colorsN = pSS->pCustomColors->size();
		for (int cN=0; cN < colorsN; cN++) {
			MyColor* pCL = pSS->pCustomColors->at(cN);
			if (strcmp(pCL->colorName, colorLabel.c_str()) == 0) {
				delete pSS->pCustomColors->at(cN);
				pSS->pCustomColors->erase(pSS->pCustomColors->begin() + cN);
				break;
			}
		}
		MyColor* pCL = new MyColor(pCL0);
		strcpy_s(pCL->colorName, 32, colorLabel.c_str());
		pSS->pCustomColors->push_back(pCL);
		return 1;
	}
	mylog("ERROR in ModelLoaderCmd::processTag, unhandled tag %s, file %s\n", pML->currentTag.c_str(), pML->fullPath.c_str());
	return -1;
}
std::string ModelLoaderCmd::pickOneOf(std::string optsString) {
	std::vector<std::string>* pOptsVector = splitString(optsString, ",");
	int optsN = pOptsVector->size();
	int optN = getRandom(0, optsN - 1);
	std::string outStr = pOptsVector->at(optN);
	delete pOptsVector;
	return outStr;
}
MyColor* ModelLoaderCmd::findColor(ModelLoaderCmd* pML, const char* colorLabel) {
	if (colorLabel[0] == '#') {
		//the value is in HTML HEX format (like #ff0000)
		std::string val = std::string(colorLabel);
		int r = std::stoi(val.substr(1, 2), nullptr, 16);
		int g = std::stoi(val.substr(3, 2), nullptr, 16);
		int b = std::stoi(val.substr(5, 2), nullptr, 16);
		int a = 255;
		if (val.size() > 7)
			a = std::stoi(val.substr(7, 2), nullptr, 16);
		MyColor* pCL = new MyColor();
		pCL->setRGBA(r, g, b, a);
		//myStrcpy_s(pCL->colorName, 32, colorLabel);
		if (pML->pTempColors == NULL)
			pML->pTempColors = new std::vector<MyColor*>;
		pML->pTempColors->push_back(pCL);
		return pCL;
	}
	if (strcmp(colorLabel,"random")==0) {
		int r = getRandom(0, 255);
		int g = getRandom(0, 255);
		int b = getRandom(0, 255);
		int a = 255;
		MyColor* pCL = new MyColor();
		pCL->setRGBA(r, g, b, a);
		//myStrcpy_s(pCL->colorName, 32, colorLabel);
		if (pML->pTempColors == NULL)
			pML->pTempColors = new std::vector<MyColor*>;
		pML->pTempColors->push_back(pCL);
		return pCL;
	}
	//normal case
	std::vector<std::vector<MyColor*>*> listsList;
	listsList.push_back(pML->pTempColors);
	listsList.push_back(pML->pSS00->pCustomColors);
	listsList.push_back(&MyColor::colorsList);
	int listsN = listsList.size();
	for (int lN = 0; lN < listsN; lN++) {
		std::vector<MyColor*>* pColorsList = listsList.at(lN);
		if (pColorsList == NULL)
			continue;
		int optsN = pColorsList->size();
		for (int oN = 0; oN < optsN; oN++) {
			MyColor* pCL = pColorsList->at(oN);
			if (strcmp(pCL->colorName, colorLabel) == 0)
				return pCL;
		}
	}
	//if here - not found
	return NULL;
}
MaterialAdjust* ModelLoaderCmd::findMaterial(ModelLoaderCmd* pML, const char* materialLabel) {
	std::vector<std::vector<MaterialAdjust*>*> listsList;
	listsList.push_back(pML->pSS00->pCustomMaterials);
	listsList.push_back(pML->pTempMaterials);
	listsList.push_back(&MaterialAdjust::materialAdjustsList);
	int listsN = listsList.size();
	for (int lN = 0; lN < listsN; lN++) {
		std::vector<MaterialAdjust*>* pMaterialsList = listsList.at(lN);
		if (pMaterialsList == NULL)
			continue;
		int optsN = pMaterialsList->size();
		for (int oN = 0; oN < optsN; oN++) {
			MaterialAdjust* pMA = pMaterialsList->at(oN);
			if (strcmp(pMA->materialName32, materialLabel) == 0)
				return pMA;
		}
	}
	//if here - not found
	return NULL;
}


