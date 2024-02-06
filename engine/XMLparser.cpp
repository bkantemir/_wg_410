#include "XMLparser.h"
#include "platform.h"
#include "utils.h"
#include "MyColor.h"
#include <vector>

extern std::string filesRoot;


int XMLparser::removeComments(XMLparser* pXP) {
	char* tmpBuf = new char[pXP->dataSize + 1];
//mylog("======Source:\n%s----------\n", pXP->pData);
	//find all occurances of "/*"
	std::vector<char*> commentsStarts;
	// /* comments */
	char* scanFrom = pXP->pData;
	while (1) {
		char* commentStarts = strstr(scanFrom, "/*");
		if (commentStarts == NULL)
			break;
		commentsStarts.push_back(commentStarts);
		scanFrom = commentStarts + 2;
	}
	//here we have a list of /* comments */
	while(commentsStarts.size() > 0){
		//get last comment
		char* commentStarts = commentsStarts.back();
		commentsStarts.pop_back();
		char* commentEnds = strstr(commentStarts, "*/") + 2;
		int commentLength = (int)(commentEnds - commentStarts);
		//shift text left
		char* textEnd = pXP->pData + pXP->dataSize;
		int remainingLength = (int)(textEnd - commentEnds);
		memcpy(tmpBuf, commentEnds, remainingLength);
		memcpy(commentStarts, tmpBuf, remainingLength);
		pXP->dataSize -= commentLength;
		scanFrom = commentStarts;
		pXP->pData[pXP->dataSize] = 0;
		//mylog("======\n%s----------\n", pXP->pData);
	}
//mylog("===1st pass===\n%s----------\n", pXP->pData);
	//line comments
	scanFrom = pXP->pData;
	while (1) {
		char* commentStarts = strstr(scanFrom, "//");
		if (commentStarts == NULL)
			break;
		char* commentEnds = strstr(commentStarts, "\n");
		if (commentEnds == NULL) {
			//end of file reached
			commentEnds = pXP->pData + pXP->dataSize;
		}
		int commentLength = (int)(commentEnds - commentStarts);
		//shift text left
		char* textEnd = pXP->pData + pXP->dataSize;
		int remainingLength = (int)(textEnd - commentEnds);
		//mylog("============\ncommentLength=%d remainingLength=%d\noffset start=%d end=%d\n%s\n", commentLength, remainingLength, 
		//	(int)(commentStarts- pXP->pData), (int)(commentEnds- pXP->pData), pXP->pData);
		memcpy(tmpBuf, commentEnds, remainingLength);
		memcpy(commentStarts, tmpBuf, remainingLength);
		pXP->dataSize -= commentLength;
		scanFrom = commentStarts;
		pXP->pData[pXP->dataSize] = 0;
	}
//mylog("===2nd pass===\n%s----------\n", pXP->pData);
	// <!-- comments -->
	scanFrom = pXP->pData;
	while (1) {
		char* commentStarts = strstr(scanFrom, "<!--");
		if (commentStarts == NULL)
			break;
		commentsStarts.push_back(commentStarts);
		scanFrom = commentStarts + 4;
	}
	//here we have a list of <!-- comments -->
	while (commentsStarts.size() > 0) {
		//get last comment
		char* commentStarts = commentsStarts.back();
		commentsStarts.pop_back();
		char* commentEnds = strstr(commentStarts, "-->");
		int commentLength = (int)(commentEnds - commentStarts) + 3;
		//shift text left
		char* textEnd = pXP->pData + pXP->dataSize;
		int remainingLength = (int)(textEnd - commentEnds);
		memcpy(tmpBuf, commentEnds, remainingLength);
		memcpy(commentStarts, tmpBuf, remainingLength);
		pXP->dataSize -= commentLength;
		scanFrom = commentStarts;
		pXP->pData[pXP->dataSize] = 0;
		//mylog("======\n%s----------\n", pXP->pData);
	}
	pXP->pData[pXP->dataSize] = 0;
//mylog("======%s===Final cut:\n%s----------\n",pXP->fullPath.c_str(), pXP->pData);
	delete[] tmpBuf;
	return 1;
}
int XMLparser::nameEndsAt(std::string varName, std::string tag) {
	int scanFrom = 0;
	int nameLength = varName.length();
	std::string optsBefore = "< ";
	std::string optsAfter = " =/>\n";
	while (1) {
		int varStartsAt = tag.find(varName, scanFrom);
		if (varStartsAt == std::string::npos)
			return -1;
		scanFrom = varStartsAt + nameLength;
		if (varStartsAt > 0) {
			char charBefore = tag.at(varStartsAt - 1);
			if (optsBefore.find(charBefore) == std::string::npos)
				continue;
		}
		char charAfter = tag.at(scanFrom);
		if (optsAfter.find(charAfter) == std::string::npos)
			continue;
		return scanFrom;
	}
}

int XMLparser::processSource(XMLparser* pXP) {
	while (pXP->nextTag(pXP)) {
		pXP->processTag();
	}
	return 1;
}

std::string XMLparser::getStringValue(std::string varName, std::string tag) {
	//returns std::string
	int valueStartsAt = nameEndsAt(varName, tag);
	if (valueStartsAt < 0)
		return ""; //var not found
	valueStartsAt = tag.find_first_not_of(" ", valueStartsAt);
	char c = tag.at(valueStartsAt);
	if (c != '=')
		return ""; //var exists, but value not set
	valueStartsAt = tag.find_first_not_of(" ", valueStartsAt + 1);
	c = tag.at(valueStartsAt);
	std::string optsQuote = "\"'";
	int valueEndsAt = 0;
	if (optsQuote.find(c) != std::string::npos) {
		//the value is in quotes
		valueStartsAt++;
		valueEndsAt = tag.find(c, valueStartsAt);
	}
	else { //value not quoted
		valueEndsAt = tag.find_first_of(" />\n", valueStartsAt);
	}
	return tag.substr(valueStartsAt, valueEndsAt - valueStartsAt);
}

bool XMLparser::varExists(std::string varName, std::string tag) {
	int valueStartsAt = nameEndsAt(varName, tag);
	if (valueStartsAt < 0)
		return false; //var not found
	return true;
}
int XMLparser::setCharsValue(char* pChars, int charsLength, std::string varName, std::string tag) {
	if (!varExists(varName, tag))
		return 0; //var not found
	std::string val = getStringValue(varName, tag);
	strcpy_s(pChars, charsLength, (char*)val.c_str());
	return 1;
}
int XMLparser::setIntValue(int* pInt, std::string varName, std::string tag) {
	if (!varExists(varName, tag))
		return 0; //var not found
	std::string val = getStringValue(varName, tag);
	if(val.length()!=0)
		*pInt = stoi(val);
	return 1;
}
int XMLparser::getIntValue(std::string varName, std::string tag) {
	if (!varExists(varName, tag))
		return 0; //var not found
	std::string val = getStringValue(varName, tag);
	return stoi(val);
}
unsigned int XMLparser::getUintValue(std::string varName, std::string tag) {
	if (!varExists(varName, tag))
		return 0; //var not found
	std::string val = getStringValue(varName, tag);
	return stoul(val);
}
int XMLparser::setFloatValue(float* pFloat, std::string varName, std::string tag) {
	if (!varExists(varName, tag))
		return 0; //var not found
	std::string val = getStringValue(varName, tag);
	*pFloat = stof(val);
	return 1;
}
float XMLparser::getFloatValue(std::string varName, std::string tag) {
	if (!varExists(varName, tag))
		return 0; //var not found
	std::string val = getStringValue(varName, tag);
	return stof(val);
}
int XMLparser::setFloatArray(float* pFloats, int arrayLength, std::string varName, std::string tag) {
	if (!varExists(varName, tag))
		return 0; //var not found

	//mylog("%s in %s\n", varName.c_str(), tag.c_str());

	std::string valuesString = getStringValue(varName, tag);
	std::vector<std::string>* pValuesVector = splitString(valuesString, ",");
	int valsN = pValuesVector->size();
	if (valsN == 1) {
		float val = stof(pValuesVector->at(0));
		for (int i = 0; i < arrayLength; i++)
			pFloats[i] = val;
		pValuesVector->clear();
		delete pValuesVector;
		return 1;
	}
	if (valsN != arrayLength) {
		pValuesVector->clear();
		delete pValuesVector;
		mylog("ERROR in XMLparser::getFloatArray, %s, %s\n", varName.c_str(), tag.c_str());
		return -1;
	}
	for (int i = 0; i < valsN; i++) {
		if (pValuesVector->at(i).compare("x") != 0)
			pFloats[i] = stof(pValuesVector->at(i));
	}
	pValuesVector->clear();
	delete pValuesVector;
	return 1;
}
int XMLparser::setIntArray(int* pInts, int arrayLength, std::string varName, std::string tag) {
	if (!varExists(varName, tag))
		return 0; //var not found
	std::string valuesString = getStringValue(varName, tag);
	std::vector<std::string>* pValuesVector = splitString(valuesString, ",");
	int valsN = pValuesVector->size();
	if (valsN == 1) {
		int val = stoi(pValuesVector->at(0));
		for (int i = 0; i < arrayLength; i++)
			pInts[i] = val;
		pValuesVector->clear();
		delete pValuesVector;
		return 1;
	}
	if (valsN != arrayLength) {
		pValuesVector->clear();
		delete pValuesVector;
		mylog("ERROR in XMLparser::getIntArray, %s, %s\n", varName.c_str(), tag.c_str());
		return -1;
	}
	for (int i = 0; i < valsN; i++) {
		if(pValuesVector->at(i).compare("x") != 0)
			pInts[i] = stoi(pValuesVector->at(i));
	}
	pValuesVector->clear();
	delete pValuesVector;
	return 1;
}
int XMLparser::setUintColorValue(unsigned int* pInt, std::string varName, std::string tag) {
	if (!varExists(varName, tag))
		return 0; //var not found
	MyColor clr;
	std::string val = getStringValue(varName, tag);
	if (val.at(0) == '#') {
		//the value is in HTML HEX format (like #ff0000)
		int r = std::stoi(val.substr(1, 2), nullptr, 16);
		int g = std::stoi(val.substr(3, 2), nullptr, 16);
		int b = std::stoi(val.substr(5, 2), nullptr, 16);
		int a = 255;
		if (val.size() > 7)
			a = std::stoi(val.substr(7, 2), nullptr, 16);
		clr.setRGBA(r, g, b, a);
	}
	else if (val.find(",") != std::string::npos) {
		//the value is an array of ints (?)
		std::vector<std::string>* pValuesVector = splitString(val, ",");
		int r = std::stoi(pValuesVector->at(0));
		int g = std::stoi(pValuesVector->at(1));
		int b = std::stoi(pValuesVector->at(2));
		int a = 255;
		if (pValuesVector->size() > 3)
			a = std::stoi(pValuesVector->at(3));
		clr.setRGBA(r, g, b, a);
		pValuesVector->clear();
		delete pValuesVector;
	}
	else {
		mylog("ERROR in XMLparser::setUintColorValue: unhandled Color format %s\n",tag.c_str());
		return -1;
	}
	*pInt = clr.getUint32();
	return 1;
}
int XMLparser::getIntBoolValue(std::string varName, std::string tag) {
	if (!varExists(varName, tag))
		return 0; //var not found
	std::string val = getStringValue(varName, tag);
	if (val.compare("") == 0) return 1;
	else if (val.compare("1") == 0) return 1;
	else if (val.compare("0") == 0) return 0;
	else if (val.compare("yes") == 0) return 1;
	else if (val.compare("no") == 0) return 0;
	else if (val.compare("true") == 0) return 1;
	else if (val.compare("false") == 0) return 0;
	else {
		mylog("ERROR in XMLparser::getIntBoolValue, %s=%s in %s\n", varName.c_str(), val.c_str(), tag.c_str());
		return -1;
	}
	return 1;
}

int XMLparser::setIntBoolValue(int* pInt, std::string varName, std::string tag) {
	if (!varExists(varName, tag))
		return 0; //var not found
	std::string val = getStringValue(varName, tag);
	if (val.compare("") == 0) *pInt = 1;
	else if (val.compare("1") == 0) *pInt = 1;
	else if (val.compare("0") == 0) *pInt = 0;
	else if (val.compare("yes") == 0) *pInt = 1;
	else if (val.compare("no") == 0) *pInt = 0;
	else if (val.compare("true") == 0) *pInt = 1;
	else if (val.compare("false") == 0) *pInt = 0;
	else {
		mylog("ERROR in XMLparser::setIntBoolValue, %s=%s in %s\n",varName.c_str(),val.c_str(),tag.c_str());
		return -1;
	}
	return 1;
}
int XMLparser::setGlRepeatValue(int* pInt, std::string varName, std::string tag) {
	//glRepeat options: GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
	if (!varExists(varName, tag))
		return 0; //var not found
	std::string val = getStringValue(varName, tag);
	if (val.compare("GL_REPEAT") == 0) *pInt = GL_REPEAT;
	else if (val.compare("GL_CLAMP_TO_EDGE") == 0) *pInt = GL_CLAMP_TO_EDGE;
	else if (val.compare("GL_MIRRORED_REPEAT") == 0) *pInt = GL_MIRRORED_REPEAT;
	return 1;
}
std::string XMLparser::buildFullPath(XMLparser* pXP, std::string filePath) {
	if (filePath.at(0) == '/')
		return (filesRoot + filePath);
	std::string inAppFolder = pXP->inAppFolder;
	while (filePath.find("..") == 0) {
		filePath = filePath.substr(3);
		int slashPos = inAppFolder.find_last_of("/", inAppFolder.size() - 2);
		inAppFolder = inAppFolder.substr(0, slashPos+1);
	}
	filePath = inAppFolder + filePath;
	return (filesRoot + filePath);
}
bool XMLparser::nextTag(XMLparser* pXP) {
	//returns 0 if no more tags, 1 - tag extracted
	char* tagStarts = strstr(pXP->readFrom, "<");
	if (tagStarts == NULL)
		return false;
	pXP->readFrom = tagStarts + 1;

	char* tagEnds = strstr(pXP->readFrom, ">");
	if (tagEnds == NULL)
		return false;
	pXP->readFrom = tagEnds + 1;
	int tagLength = (int)(tagEnds - tagStarts) + 1;
	pXP->currentTag.assign(tagStarts, tagLength);

	inspectLastTag(pXP);
	return true;
}
void XMLparser::inspectLastTag(XMLparser* pXP) {
	pXP->tagLength = pXP->currentTag.length();
	//extract tagName
	int nameStart = pXP->currentTag.find_first_not_of(" <");
	int nameEnd = pXP->currentTag.find_first_of(" =/>\n", nameStart + 1);
	pXP->tagName = pXP->currentTag.substr(nameStart, (nameEnd - nameStart));
	//open/closed tag
	char lastChar = pXP->currentTag.at(pXP->tagLength - 2);
	pXP->closedTag = (lastChar == '/');
}
void XMLparser::doTag(XMLparser* pXP, std::string tagStr) {
	pXP->currentTag.assign(tagStr);
	pXP->inspectLastTag(pXP);
	pXP->processTag();
	pXP->currentTag.assign("");
}

