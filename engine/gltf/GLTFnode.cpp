#include "GLTFnode.h"
#include "GLTFanimation.h"
#include "TheApp.h"
#include "Shadows.h"
#include "GLTFskinShader.h"
#include "GLTFanim2nodesMap.h"


extern TheApp theApp;
extern float degrees2radians;

GLTFnode::~GLTFnode() {
}
int GLTFnode::applyAnimation(GLTFnode* pNode) {
	if (pNode->gltfAnimN < 0)
		return 0;

	std::vector<GLTFanimation*>* pAnims = &GLTFanimation::GLTFanimations_default;
	GLTFanimation* pAnim = pAnims->at(pNode->gltfAnimN);
	std::vector<GLTFanim2nodesMap*>* pMaps = &GLTFanim2nodesMap::anim2nodesMaps_default;
	GLTFanim2nodesMap* pMap = pMaps->at(pNode->anim2skinMapN);

	//clear pSS->anim2nodeMapScale(s)
	for (int nN = 1; nN < pNode->totalNativeElements; nN++) {
		int nodeN = nN + pNode->nInSubjsSet;
		GLTFnode* pSS = (GLTFnode*)pNode->pSubjsSet->at(nodeN);
		if (pSS == NULL)
			continue;
		pSS->anim2nodeMapScale = pMap->midScale;
	}

	float gltfAnimTime = 0;
	pNode->gltfAnimTimePass = pNode->gltfAnimTimePass + (float)theApp.millisPerFrame / 1000.0f;
	if (pNode->animCycle == 1) { //straight cycle
		if (pNode->gltfAnimTimePass > pAnim->timeTotal)//anim ended - start over
			pNode->gltfAnimTimePass -= pAnim->timeTotal;
		gltfAnimTime = pNode->gltfAnimTimePass;
	}
	else if (pNode->animCycle == 2) { //back-and-forth:forth
		if (pNode->gltfAnimTimePass > pAnim->timeTotal*2) //anim ended - start over
			pNode->gltfAnimTimePass -= (pAnim->timeTotal * 2);
		if (pNode->gltfAnimTimePass <= pAnim->timeTotal)
			gltfAnimTime = pNode->gltfAnimTimePass;
		else //2nd pass - moving back
			gltfAnimTime = pAnim->timeTotal*2 - pNode->gltfAnimTimePass;
	}
	
	//for each input find prev and next indices
	for (int iN = pAnim->inputs.size() - 1; iN >= 0; iN--) {
		GLTFsamplerInput* pI = pAnim->inputs.at(iN);
		if (gltfAnimTime < pI->timePrev || gltfAnimTime > pI->timeNext) {
			//find new boundaries
			pI->iPrev = 0;
			pI->iNext = pI->inputsN - 1;
			for (int i = 0; i < pI->inputsN; i++) {
				float inputValue = pI->input[i];
				if (inputValue == gltfAnimTime) {
					pI->iPrev = i;
					pI->iNext = i;
					break;
				}
				else if (inputValue < gltfAnimTime)
					pI->iPrev = i;
				else if (inputValue > gltfAnimTime) {
					pI->iNext = i;
					break;
				}
			}
		}
		pI->timePrev = pI->input[pI->iPrev];
		pI->timeNext = pI->input[pI->iNext];

		if (pI->iPrev == pI->iNext)
			pI->timeRate = 1;
		else
			pI->timeRate = (gltfAnimTime - pI->timePrev) / (pI->timeNext - pI->timePrev);
	}
	//scan channels
	std::vector<GLTFchannel*>* pChannels = &pAnim->channels;
	int channelsN = pChannels->size();
	for (int cN = 0; cN < channelsN; cN++) {
		GLTFchannel* pChannel = pChannels->at(cN);

		GLTFanim2nodesBone* pMapBone = pMap->anim2nodesBones.at(pChannel->target_node);
		if (pMapBone->nodesBoneN < 0)
			continue;
		int targetN = pMapBone->nodesBoneN + pNode->nInSubjsSet;

		GLTFsampler* pSampler = pAnim->samplers.at(pChannel->samplerN);
		GLTFsamplerInput* pInput = pAnim->inputs.at(pSampler->inputN);
		GLTFsamplerOutput* pOutput = pAnim->outputs.at(pSampler->outputN);

		float* outputPrev = &pOutput->output[pInput->iPrev * pOutput->strideInFloats];
		float* outputNext = &pOutput->output[pInput->iNext * pOutput->strideInFloats];

		float currentOutput[4];
		if (pOutput->strideInFloats > 4)
			mylog("ERROR in GLTFnode::applyAnimation: pOutput->strideInFloats=%d\n", pOutput->strideInFloats);
		if (pInput->timeRate == 0.0f)
			memcpy(currentOutput, outputPrev, pOutput->strideInFloats * sizeof(float));
		else if (pInput->timeRate == 1.0f)
			memcpy(currentOutput, outputNext, pOutput->strideInFloats * sizeof(float));
		else {
			if (pChannel->target_path[0] == 'r') {
				//rotation
				//slerpQuat(currentOutput, outputPrev, outputNext, pInput->timeRate);

				memcpy(currentOutput, outputNext, pOutput->strideInFloats * sizeof(float));
			}
			else {
				for (int i = 0; i < pOutput->strideInFloats; i++)
					currentOutput[i] = outputPrev[i] * (1.0f - pInput->timeRate) + outputNext[i] * pInput->timeRate;
			}
		}

		GLTFnode* pTarget = (GLTFnode*)pNode->pSubjsSet->at(targetN);

		if (pChannel->target_path[0] == 't') //translation
			memcpy(pTarget->ownCoords.pos, currentOutput, 3 * sizeof(float));
		else if (pChannel->target_path[0] == 'r') //rotation
			pTarget->ownCoords.setQuaternion(currentOutput);
		else if (pChannel->target_path[0] == 's') //scale
			memcpy(pTarget->scale, currentOutput, 3 * sizeof(float));
		else
			mylog("ERROR in GLTFnode::applyAnimation: pChannel->target_path=%s\n", pChannel->target_path);
		/*
		//correct scale
		for (int i = 0; i < 3; i++)
			pTarget->scale[i] *= pMapBone->scale;
			*/
			/*
		if (pChannel->target_path[0] == 'r') //rotation
			pTarget->ownCoords.setQuaternion(currentOutput);
			*/
	}
	//update matrices
	for (int nN = 1; nN < pNode->totalNativeElements; nN++) {
		int nodeN = nN + pNode->nInSubjsSet;
		GLTFnode* pSS = (GLTFnode*)pNode->pSubjsSet->at(nodeN);
		if (pSS == NULL)
			continue;
		
		if (pSS->anim2nodeMapScale != 1.0)
			for (int i = 0; i < 3; i++) {
				pSS->ownCoords.pos[i] *= pSS->anim2nodeMapScale;
				pSS->scale[i] *= pSS->anim2nodeMapScale;
			}
			
		pSS->buildModelMatrix();
	}
	return 1;
}

int GLTFnode::slerpQuat(float* qOut, float* q1, float* q2, float interpolation) {
	float dotProduct = v4dotProduct(q1, q2);
	float theta, st, sut, sout, coeff1, coeff2;

	// algorithm adapted from Shoemake's paper
	float lambda = interpolation / 2.0;

	theta = (float)acos(dotProduct);
	if (theta < 0.0)
		theta = -theta;

	st = (float)sin(theta);
	sut = (float)sin(lambda * theta);
	sout = (float)sin((1 - lambda) * theta);
	coeff1 = sout / st;
	coeff2 = sut / st;

	qOut[0] = coeff1 * q1[0] + coeff2 * q2[0];
	qOut[1] = coeff1 * q1[1] + coeff2 * q2[1];
	qOut[2] = coeff1 * q1[2] + coeff2 * q2[2];
	qOut[3] = coeff1 * q1[3] + coeff2 * q2[3];

	vec4_norm(qOut, qOut);
	return 1;
}
int GLTFnode::processGLTFnode(GLTFnode* pNode) {
	//check root
	if (pNode->d2parent == 0)
		pNode->rootN = pNode->nInSubjsSet;
	else {
		int parentN = pNode->nInSubjsSet - pNode->d2parent;
		SceneSubj* pParent = pNode->pSubjsSet->at(parentN);
		//inherit parent
		pNode->rootN = pParent->rootN;
		//pNode->hide = pParent->hide;
	}

	//pNode->moveSubj();
	if (pNode->gltfAnimN >= 0)
		applyAnimation(pNode);
	if (pNode->absCoordsUpdateFrameN != theApp.frameN)
		pNode->buildModelMatrix();
	return 1;
}
int GLTFnode::render(Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter) {
	GLTFnode* pNode = this;
	//pNode->skinN = -1;
	if (pNode->skinN < 0)
		return renderStandard(pNode, pCam, dirToMainLight, dirToTranslucent, renderFilter);
	//animated
	return renderGLTFskinned(pNode, pCam, dirToMainLight, dirToTranslucent, renderFilter);
};
int GLTFnode::renderGLTFskinned(GLTFnode* pSS, Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter) {
	std::vector<GLTFskin*>* pGLTFskins = &GLTFskin::GLTFskins_default;
	std::vector<unsigned int>* pBuffersIds = &DrawJob::buffersIds_default;

	GLTFskin* pSkin = pGLTFskins->at(pSS->skinN);
	GLTFnode* pAnimRoot = (GLTFnode*)pSS->pSubjsSet->at(pSS->rootN + 1);

	if (pAnimRoot->uboBonesId < 0)
		pAnimRoot->uboBonesId = DrawJob::newBufferId(pBuffersIds);
	if (pAnimRoot->uboBonesUpdateFrameN != theApp.frameN) {
		buildJointsMatrices(pSkin, pAnimRoot);
		//update UBO
		glBindBuffer(GL_UNIFORM_BUFFER, pAnimRoot->uboBonesId);
		glBufferData(GL_UNIFORM_BUFFER, pSkin->jointsN * 16 * sizeof(float), pSkin->jointsMatrices, GL_DYNAMIC_DRAW);
		pAnimRoot->uboBonesUpdateFrameN = theApp.frameN;
	}

	if (renderFilter == pSS->renderOrder)
		renderFilter = -1; //render all

	//subj's distance from camera
	float cameraSpacePos[4];
	mat4x4_mul_vec4plus(cameraSpacePos, pCam->lookAtMatrix, pSS->absCoords.pos, 1);
	float zDistance = abs(cameraSpacePos[2]);
	float viewRangeDg = pCam->viewRangeDg;
	if (viewRangeDg == 0)
		viewRangeDg = 30;
	float cotangentA = 1.0f / tanf(degrees2radians * viewRangeDg / 2.0f);
	float halfScreenVertSizeInUnits = zDistance / cotangentA;
	float sizeUnitPixelsSize = (float)theApp.mainCamera.targetRads[1] / halfScreenVertSizeInUnits;
	sizeUnitPixelsSize *= pSS->scale[0];
	//render subject
	if (pSS->pDrawJobs == NULL)
		pSS->pDrawJobs = &DrawJob::drawJobs_default;

	//if(theApp.frameN==0) mylog("frame=%d subjN=%d djTotalN=%d\n", (int)theApp.frameN, pSS->nInSubjsSet, pSS->djTotalN);

	for (int i0 = 0; i0 < pSS->djTotalN; i0++) {
		int i = i0;
		if (renderFilter > 0) //render transparent only, most likely - GLTF reversed order
			i = pSS->djTotalN - 1 - i0;

		DrawJob* pDJ = pSS->pDrawJobs->at(pSS->djStartN + i);

		Material* pMt = &pDJ->mt;

		if (renderFilter >= 0)
			if (renderFilter != pMt->uAlphaBlending > 0)
				continue;

		//if (theApp.frameN == 0) mylog("djN=%d shaderN=%d\n", i,pMt->shaderN);

		Material* pMt2 = &pDJ->mtLayer2;
		if (i == 0) {
			Material mt;
			memcpy((void*)&mt, (void*)&pDJ->mt, sizeof(Material));
			pMt = &mt;
			//2-nd layer
			Material mt2;
			memcpy((void*)&mt2, (void*)&pDJ->mtLayer2, sizeof(Material));
			pMt2 = &mt2;
		}
		Material* pAltMt = NULL;
		Material* pAltMt2 = NULL;
		if (i == 0 && pSS->mt0isSet > 0) { //already customized
			pMt = &pSS->mt0;
			pMt2 = &pSS->mt0Layer2;
		}
		else { //customize?
			customizeMaterial(&pMt, &pMt2, &pAltMt, &pAltMt2, pSS);
			if (i == 0) {
				memcpy((void*)&pSS->mt0, (void*)pMt, sizeof(Material));
				memcpy((void*)&pSS->mt0Layer2, (void*)pMt2, sizeof(Material));
				pSS->mt0isSet = 1;
			}
		}

		executeDJskinned(pDJ,
			pAnimRoot->uboBonesId, pCam->mViewProjection, pCam->lookAtMatrix, Shadows::shadowCamera.mViewProjection,
			dirToMainLight, dirToTranslucent,
			pCam->ownCoords.pos, sizeUnitPixelsSize, pMt);

		//have 2-nd layer ?
		if (pMt2 != NULL)
			if (pMt2->shaderN >= 0) {
				executeDJskinned(pDJ,
					pAnimRoot->uboBonesId, pCam->mViewProjection, pCam->lookAtMatrix, Shadows::shadowCamera.mViewProjection,
					dirToMainLight, dirToTranslucent,
					pCam->ownCoords.pos, sizeUnitPixelsSize, pMt2);
			}

		if (pAltMt != NULL) {
			delete pAltMt;
			pAltMt = NULL;
		}
		if (pAltMt2 != NULL) {
			delete pAltMt2;
			pAltMt2 = NULL;
		}
	}
	return 1;
}

int GLTFnode::executeDJskinned(DrawJob* pDJ,
	int uboBonesId, mat4x4 uVPcamera, mat4x4 uLookAt, mat4x4 uVPshadow,
	float* uVectorToLight, float* uVectorToTranslucent, float* uCameraPosition, float line1pixSize, Material* pMt) {

	if (pMt == NULL)
		pMt = &(pDJ->mt);

	int shaderN = pMt->shaderN;
	if (shaderN < 0)
		return 0;

	if (lineWidthIsImportant(pMt->primitiveType)) {
		float lw = line1pixSize * pMt->lineWidth;
		if (lw < 0.5)
			return 0;
		glLineWidth(lw);
	}

	GLTFskinShader* pShader = (GLTFskinShader*)Shader::shaders.at(shaderN);
	glUseProgram(pShader->GLid);

	if (pShader->l_uboBones >= 0)
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboBonesId); // 0 - layout (binding = 0) uniform uboBones

	//input uniform matrices (ViewProjection)
	if (pShader->l_uVPcamera >= 0)
		glUniformMatrix4fv(pShader->l_uVPcamera, 1, GL_FALSE, (const GLfloat*)uVPcamera);
	if (pShader->l_uLookAt >= 0)
		glUniformMatrix4fv(pShader->l_uLookAt, 1, GL_FALSE, (const GLfloat*)uLookAt);
	//shadow uniform matrix
	if (pShader->l_uVPshadow >= 0)
		glUniformMatrix4fv(pShader->l_uVPshadow, 1, GL_FALSE, (const GLfloat*)uVPshadow);

	executeDJcommon(pDJ, pShader, uVectorToLight, uVectorToTranslucent, uCameraPosition, pMt);
	return 1;
}

int GLTFnode::buildJointsMatrices(GLTFskin* pSkin, GLTFnode* pRoot) {
	if (pSkin->forRootNodeN == pRoot->nInSubjsSet)
		if (pSkin->updateFrameN == theApp.frameN)
			return 0;
	pSkin->forRootNodeN = pRoot->nInSubjsSet;
	pSkin->updateFrameN = theApp.frameN;

	for (int jN = 0; jN < pSkin->jointsN; jN++) {
		int nodeN = pSkin->joints[jN] + pRoot->nInSubjsSet;
		GLTFnode* pNode = (GLTFnode*)pRoot->pSubjsSet->at(nodeN);
		if (pNode == NULL)
			continue;

		float* jointMatrix = &pSkin->jointsMatrices[jN * 16];
		float* inverseBindMatrix = &pSkin->inverseBindMatrices[jN * 16];
		mat4x4_mul((vec4*)jointMatrix, pNode->absModelMatrix, (vec4*)inverseBindMatrix);
		//mat4x4_dup((vec4*)jointMatrix, (vec4*)inverseBindMatrix);
	}
	return 1;
}
int GLTFnode::renderDepthMap(Camera* pCam) {
	GLTFnode* pNode = this;
	//pNode->skinN = -1;
	if (pNode->skinN < 0)
		return renderDepthMapStandard(pNode, pCam);
	//animated
	return renderDepthMapSkinned(pNode, pCam);
};

int GLTFnode::renderDepthMapSkinned(GLTFnode* pSS, Camera* pCam) {
	if (pSS->pDrawJobs == NULL)
		pSS->pDrawJobs = &DrawJob::drawJobs_default;

	if (pSS->dropsShadow < 1)
		return 0;

	std::vector<GLTFskin*>* pGLTFskins = &GLTFskin::GLTFskins_default;
	std::vector<unsigned int>* pBuffersIds = &DrawJob::buffersIds_default;

	GLTFskin* pSkin = pGLTFskins->at(pSS->skinN);
	GLTFnode* pAnimRoot = (GLTFnode*)pSS->pSubjsSet->at(pSS->rootN + 1);

	if (pAnimRoot->uboBonesId < 0)
		pAnimRoot->uboBonesId = DrawJob::newBufferId(pBuffersIds);
	if (pAnimRoot->uboBonesUpdateFrameN != theApp.frameN) {
		buildJointsMatrices(pSkin, pAnimRoot);
		//update UBO
		glBindBuffer(GL_UNIFORM_BUFFER, pAnimRoot->uboBonesId);
		glBufferData(GL_UNIFORM_BUFFER, pSkin->jointsN * 16 * sizeof(float), pSkin->jointsMatrices, GL_DYNAMIC_DRAW);
		pAnimRoot->uboBonesUpdateFrameN = theApp.frameN;
	}

	//render subject
	for (int i = 0; i < pSS->djTotalN; i++) {
		DrawJob* pDJ = pSS->pDrawJobs->at(pSS->djStartN + i);
		if (pDJ->mt.dropsShadow < 1)
			continue;
		Material* pMt = &pDJ->mtShadow;

		executeDJskinned(pDJ, pAnimRoot->uboBonesId,
			pCam->mViewProjection, pCam->lookAtMatrix, NULL, NULL, NULL,
			pCam->ownCoords.pos, Shadows::sizeUnitPixelsSize * pSS->scale[0], pMt);
	}
	return 1;
}
int GLTFnode::startAnimation(GLTFnode* pNode, int animN, int cycleType, int anim2skinMapN) {
	pNode->gltfAnimN = animN;
	pNode->gltfAnimTimePass = 0;
	pNode->animCycle = cycleType;

	std::vector<GLTFanim2nodesMap*>* pMaps = &GLTFanim2nodesMap::anim2nodesMaps_default;
	std::vector<GLTFanimation*>* pAnims = &GLTFanimation::GLTFanimations_default;
	GLTFanimation* pAnim = pAnims->at(animN);
	if (anim2skinMapN < 0) {
		if (pNode->anim2skinMapN >= 0) {
			GLTFanim2nodesMap* pMap = pMaps->at(pNode->anim2skinMapN);
			if (pMap->rootNodeN == pNode->nInSubjsSet && pMap->animSkeletonMapN == pAnim->animSkeletonMapN)
				anim2skinMapN = pNode->anim2skinMapN;
		}
	}
	if (anim2skinMapN < 0) {
		//find map
		int mapsTotalN = pMaps->size();
		for (int mN = pAnim->animSkeletonMapN; mN < mapsTotalN; mN++) {
			GLTFanim2nodesMap* pMap = pMaps->at(mN);
			if (pMap->animSkeletonMapN != pAnim->animSkeletonMapN)
				continue;
			if (pMap->rootNodeN < 0)
				break;
			if (pMap->rootNodeN != pNode->nInSubjsSet)
				continue;
			anim2skinMapN = mN;
			break;
		}
		if (anim2skinMapN < 0) {
			//map not found - get default map
			anim2skinMapN = pAnim->animSkeletonMapN;
			GLTFanim2nodesMap* pMap00 = pMaps->at(anim2skinMapN);
			GLTFanim2nodesMap* pMap = pMap00;
			int bonesN = pMap00->anim2nodesBones.size();
			if (pMap->rootNodeN >= 0) { //already assigned, add new
				anim2skinMapN = pMaps->size();
				pMap = new GLTFanim2nodesMap();
				pMaps->push_back(pMap);
				pMap->animSkeletonMapN = pAnim->animSkeletonMapN;
				//copy bones
				for (int i = 0; i < bonesN; i++) {
					GLTFanim2nodesBone* pBone = new GLTFanim2nodesBone(pMap00->anim2nodesBones.at(i));
					pMap->anim2nodesBones.push_back(pBone);
				}
			}
			mylog("\nanim2skinMapN %d\n", anim2skinMapN);

			pMap->rootNodeN = pNode->nInSubjsSet;
			//re-map bones
			int boneNfirst = pNode->nInSubjsSet;
			int boneNlast = pNode->totalNativeElements + boneNfirst;
			float scalesSum = 0;
			int scaledBones = 0;
			for (int bN = 0; bN < bonesN; bN++) {
				GLTFanim2nodesBone* pBone = pMap->anim2nodesBones.at(bN);
				//find corresponding nodesBoneN
				pBone->nodesBoneN = -1;
				for (int nN = boneNfirst; nN < boneNlast; nN++) {
					SceneSubj* pSS = pNode->pSubjsSet->at(nN);
					if (strcmp(pBone->name, pSS->name64) == 0) {//mathing name
						pBone->nodesBoneN = nN - boneNfirst;
						float nodeLever = v3length(pSS->ownCoords.pos);
						if (pBone->boneLever == 0)
							pBone->scale = 0;
						else {
							pBone->scale = nodeLever / pBone->boneLever;
							scalesSum += pBone->scale;
							scaledBones++;
						}
						break;
					}
				}
				mylog("boneN %d '%s' pBone->nodesBoneN=%d pBone->scale=%f\n", bN, pBone->name, pBone->nodesBoneN, pBone->scale);
			}
			pMap->midScale = scalesSum / scaledBones;
		}
	}
	pNode->anim2skinMapN = anim2skinMapN;
	return 1;
}


