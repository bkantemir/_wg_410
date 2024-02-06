#include "DrawJob.h"
#include "platform.h"
#include "utils.h"
#include "Shader.h"
#include "Shadows.h"
#include "Texture.h"

//static arrays (vectors) of all loaded DrawJobs, VBO ids
std::vector<DrawJob*> DrawJob::drawJobs_default;
std::vector<unsigned int> DrawJob::buffersIds_default;

DrawJob::DrawJob(std::vector<DrawJob*>* pDrawJobs) {
	if (pDrawJobs == NULL)
		pDrawJobs = &drawJobs_default;
	pDrawJobs->push_back(this);
}
DrawJob::~DrawJob() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	if (glVAOid > 0) {
		glDeleteVertexArrays(1, &glVAOid);
		glVAOid = 0;
	}
}
int DrawJob::cleanUp() {
	int itemsN = drawJobs_default.size();
	//delete all drawJobs
	for (int i = 0; i < itemsN; i++) {
		DrawJob* pDJ = drawJobs_default.at(i);
		delete pDJ;
	}
	drawJobs_default.clear();
	//delete Buffers
	itemsN = buffersIds_default.size();
	//delete all buffers
	for (int i = 0; i < itemsN; i++) {
		unsigned int id = buffersIds_default.at(i);
		glDeleteBuffers(1, &id);
	}
	buffersIds_default.clear();
	return 1;
}

int DrawJob::newBufferId(std::vector<unsigned int>* pBuffersIds) {
	unsigned int bufferId;
	glGenBuffers(1, &bufferId);
	pBuffersIds->push_back(bufferId);
	return (int)bufferId;
}

bool DrawJob::lineWidthIsImportant(int primitiveType) {
	if (primitiveType == GL_TRIANGLES) return false;
	if (primitiveType == GL_TRIANGLE_STRIP) return false;
	if (primitiveType == GL_TRIANGLE_FAN) return false;
	return true;
}
int DrawJob::setAttribRef(AttribRef* pAR, unsigned int glVBOid, int offset, int stride) {
	pAR->glVBOid = glVBOid;
	pAR->offset = offset;
	pAR->stride = stride;
	return 1;
}
int DrawJob::setDesirableOffsetsForSingleVBO(DrawJob* pDJ, int* pStride, int shaderN, int VBOid) {
	//sets desirable offsets and stride according to given shader needs
	//assuming that we have 1 single VBO
	Shader* pSh = Shader::shaders.at(shaderN);
	int stride = 0;
	pDJ->aPos.offset = 0; //attribute o_aPos, always 0
	stride += sizeof(float) * 3; //aPos size - 3 floats (x,y,z)
	if (pSh->l_aNormal >= 0) { //attribute normal
		pDJ->aNormal.offset = stride;
		stride += sizeof(float) * 3;
	}
	if (pSh->l_aTuv >= 0) { //attribute TUV (texture coordinates)
		pDJ->aTuv.offset = stride; //attribute TUV (texture coordinates)
		stride += sizeof(float) * 2;
	}
	if (pSh->l_aTuv2 >= 0) { //for normal map
		pDJ->aTuv2.offset = stride;
		stride += sizeof(float) * 2;
	}
	if (pSh->l_aTangent >= 0) { //for normal map
		pDJ->aTangent.offset = stride;
		stride += sizeof(float) * 3;
	}
	if (pSh->l_aBinormal >= 0) { //for normal map
		pDJ->uHaveBinormal = 1;
		pDJ->aBinormal.offset = stride;
		stride += sizeof(float) * 3;
	}
	if (pSh->l_aJoints >= 0) { //for animations
		pDJ->aJoints.offset = stride;
		stride += 4; //4 unsigned bytes
		//}
		//if (pSh->l_aWeights >= 0) { //for animations
		pDJ->aWeights.offset = stride;
		stride += sizeof(float) * 4;
	}
	*pStride = stride;
	//add stride and VBOid to all attributes
	AttribRef* pAR = NULL;
	pAR = &pDJ->aPos; pAR->glVBOid = VBOid; pAR->stride = stride;
	pAR = &pDJ->aNormal; pAR->glVBOid = VBOid; pAR->stride = stride;
	pAR = &pDJ->aTuv; pAR->glVBOid = VBOid; pAR->stride = stride;
	pAR = &pDJ->aTuv2; pAR->glVBOid = VBOid; pAR->stride = stride;
	pAR = &pDJ->aTangent; pAR->glVBOid = VBOid; pAR->stride = stride;
	pAR = &pDJ->aBinormal; pAR->glVBOid = VBOid; pAR->stride = stride;
	pAR = &pDJ->aJoints; pAR->glVBOid = VBOid; pAR->stride = stride;
	pAR = &pDJ->aWeights; pAR->glVBOid = VBOid; pAR->stride = stride;

	return 1;
}
unsigned int activeVBOid;
int DrawJob::buildVAOforShader(DrawJob* pDJ, int shaderN) {
	if (shaderN < 0)
		return 0;
	unsigned int* pVAOid = &pDJ->glVAOid;
	//delete VAO if exists already
	if (*pVAOid > 0) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteVertexArrays(1, pVAOid);
	}
	glGenVertexArrays(1, pVAOid);
	glBindVertexArray(*pVAOid);

	//open shader descriptor to access variables locations
	Shader* pShader = Shader::shaders.at(shaderN);

	activeVBOid = 0;
	attachAttribute(pShader->l_aPos, 3, GL_FLOAT, &pDJ->aPos);
	attachAttribute(pShader->l_aNormal, 3, GL_FLOAT, &pDJ->aNormal);
	attachAttribute(pShader->l_aTuv, 2, GL_FLOAT, &pDJ->aTuv);
	attachAttribute(pShader->l_aTuv2, 2, GL_FLOAT, &pDJ->aTuv2); //for normal map
	attachAttribute(pShader->l_aTangent, 3, GL_FLOAT, &pDJ->aTangent); //for normal map
	attachAttribute(pShader->l_aBinormal, 3, GL_FLOAT, &pDJ->aBinormal); //for normal map

	attachAttribute(pShader->l_aJoints, 4, GL_UNSIGNED_BYTE, &pDJ->aJoints); //for animations
	attachAttribute(pShader->l_aWeights, 4, GL_FLOAT, &pDJ->aWeights); //for animations

	if (pDJ->glEBOid > 0)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pDJ->glEBOid);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return 1;
}


int DrawJob::attachAttribute(int varLocationInShader, int attributeSizeInUnits, int unitType, AttribRef* pAR) {
	if (varLocationInShader < 0)
		return 0; //not used in this shader
	if (pAR->glVBOid == 0) {
		mylog("ERROR in DrawJob::attachAttribute, nk such attribute/VBO\n");
		return -1;
	}
	glEnableVertexAttribArray(varLocationInShader);
	if (activeVBOid != pAR->glVBOid) {
		activeVBOid = pAR->glVBOid;
		//attach input stream data
		glBindBuffer(GL_ARRAY_BUFFER, activeVBOid);
	}
	glVertexAttribPointer(varLocationInShader, attributeSizeInUnits, unitType, GL_FALSE, pAR->stride, (void*)(long)pAR->offset);
	return 1;
}


int DrawJob::buildGabaritesFromDrawJobs(Gabarites* pGB, std::vector<DrawJob*>* pDrawJobs, int djStartN, int djTotalN) {
	if (djTotalN < 1)
		return 0;
	DrawJob* pDJ = pDrawJobs->at(djStartN);
	memcpy(pGB, &pDJ->gabarites, sizeof(Gabarites));
	for (int djN = 1; djN < djTotalN; djN++) {
		pDJ = pDrawJobs->at(djStartN + djN);
		for (int i = 0; i < 3; i++) {
			if (pGB->bbMin[i] > pDJ->gabarites.bbMin[i])
				pGB->bbMin[i] = pDJ->gabarites.bbMin[i];
			if (pGB->bbMax[i] < pDJ->gabarites.bbMax[i])
				pGB->bbMax[i] = pDJ->gabarites.bbMax[i];
		}
	}
	for (int i = 0; i < 3; i++) {
		pGB->bbMid[i] = (pGB->bbMax[i] + pGB->bbMin[i]) / 2;
		pGB->bbRad[i] = (pGB->bbMax[i] - pGB->bbMin[i]) / 2;
	}
	return 1;
}
