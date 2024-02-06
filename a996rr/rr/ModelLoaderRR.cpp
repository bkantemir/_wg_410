#include "ModelLoaderRR.h"
#include "TheApp.h"
#include "WheelPair.h"
#include "GroupTransform.h"

extern TheApp theApp;
extern float degrees2radians;

int ModelLoaderRR::processTagRR(ModelLoaderRR* pML) {
	ModelBuilder* pMB = pML->pModelBuilder;
	std::string tagStr = pML->currentTag;

	if (pML->tagName.compare("buildWheelPair") == 0) {
		return processTag_buildWheelRair(pML);
	}
	return ModelLoader::processTag(pML);
}
ModelLoaderRR::ModelLoaderRR(std::vector<SceneSubj*>* pSubjsVector0, std::vector<DrawJob*>* pDrawJobs0, std::vector<unsigned int>* pBuffersIds0,
	int subjN, ModelBuilder* pMB00, ModelBuilder* pMB, std::string filePath, ProgressBar* pPBar0) : ModelLoader( pSubjsVector0, 
		pDrawJobs0, pBuffersIds0, subjN, pMB00, pMB, filePath, pPBar0) {
};

int ModelLoaderRR::loadModelRR(std::vector<SceneSubj*>* pSubjsVector0, std::vector<DrawJob*>* pDrawJobs0, std::vector<unsigned int>* pBuffersIds0,
	std::string sourceFile, std::string sourceFileRoot, std::string subjClass, std::string tagStr, ProgressBar* pPBar, ModelBuilder* pMB00, bool runOnLoad) {
	//returns element's (Subj) number or -1
	int subjN = loadModel_ifExists(pSubjsVector0, sourceFile, sourceFileRoot, subjClass, tagStr, pPBar, runOnLoad);
	if (subjN >= 0)
		return subjN;
	//if here - model wasn't loaded before - load
	subjN = pSubjsVector0->size();
	SceneSubj* pSS = theApp.newSceneSubj(subjClass, sourceFile, pSubjsVector0, pDrawJobs0);
	if(sourceFileRoot.empty())
		strcpy_s(pSS->source256root, 256, sourceFile.c_str());
	else
		strcpy_s(pSS->source256root, 256, sourceFileRoot.c_str());
	ModelLoaderRR* pML = new ModelLoaderRR(pSubjsVector0, pDrawJobs0, pBuffersIds0, subjN, pMB00, NULL, sourceFile, pPBar);
	processSource(pML);
	delete pML;
	pSS->totalNativeElements = pSubjsVector0->size() - subjN;
	pSS->totalElements = pSS->totalNativeElements;
	pSS->resetRoot();
	buildGabaritesFromDrawJobs(&pSS->gabaritesOnLoad, pDrawJobs0, pSS->djStartN, pSS->djTotalN);

	if (runOnLoad) {
		pSS->resetRoot();
		pSS->onLoad(tagStr);
	}
	ProgressBar::nextStep(pPBar, "loadModel");
	return subjN;
}

int ModelLoaderRR::processTag_buildWheelRair(ModelLoaderRR* pML) {
	ModelBuilder* pMB = pML->pModelBuilder;
	std::string tagStr = pML->currentTag;

	//<buildWheelRair = rims radius = 4.5 rimWall = 1 noAxis / > //noBackSide
	std::string wheelPart = getStringValue("buildWheelPair", tagStr);
	if (wheelPart.find("rims") == 0) {
		setFloatValue(&pML->wheelRadius, "radius", tagStr);
		setFloatValue(&pML->rimWallH, "rimWall", tagStr);
		bool doBack = !varExists("noBack", tagStr);
		bool doAxis = !varExists("noAxis", tagStr);
		bool doLip = !varExists("noLip", tagStr);
		pMB->lockGroup(pMB); { //1 wheel
			//rim
			pML->doTag("<mt_use='rim' />");
			pML->doTag("<vs=cylinder />");
			VirtualShape* pVS = pMB->pCurrentVShape;
			pVS->whl[0] = pML->wheelRadius * 2;
			pVS->whl[1] = pVS->whl[0];
			pVS->whl[2] = Rail::wheelWidth;
			pVS->sectionsR = round(pML->wheelRadius * 3);
			if (pVS->sectionsR < 6)
				pVS->sectionsR = 6;
			pML->wheelRadialSections = pVS->sectionsR;
			pMB->lockGroup(pMB); {
				pML->doTag("<a=wrap />");
			}
			pMB->releaseGroup(pMB);
			//border inner rim
			pMB->lockGroup(pMB); {
				pML->doTag("<mt_use=rim-wire />");
				setMaterialLineWidth(pMB, Rail::rimBorderLineWidth);
				pML->processTag_lineStart(pML);
				//line ring
				VirtualShape vs;
				vs.whl[0] = pML->wheelRadius * 2 + Rail::rimBorderLineWidth;
				vs.whl[1] = vs.whl[0];
				vs.whl[2] = 0;
				vs.sectionsR = pML->wheelRadialSections;
				float stepDg = (vs.angleFromTo[1] - vs.angleFromTo[0]) / vs.sectionsR;
				for (int rpn = 0; rpn <= vs.sectionsR; rpn++) {
					// rpn - radial point number
					float angleRd = (vs.angleFromTo[0] + stepDg * rpn) * degrees2radians;
					float kx = cosf(angleRd);
					float ky = sinf(angleRd);
					pMB->addVertex(pMB, kx, ky, 1);
				}
				pML->processTag_lineEnd(pML);
				pMB->scaleGroup(pMB, vs.whl[0] / 2, vs.whl[1] / 2, 1);
				pMB->shiftGroup(pMB, 0, 0, -Rail::wheelWidth);// +Rail::rimBorderLineWidth / 2);
			}
			pMB->releaseGroup(pMB);
			if (pML->rimWallH >= pML->wheelRadius) {
				//full dish
				//dish outer side				
				pMB->lockGroup(pMB); {
					pML->doTag("<mt_use=rim_wall />");
					Material* pMT = pMB->materialsList.at(pMB->usingMaterialN);
					float xywh[4] = { 0,0,1,1 };
					TexCoords* pTC2nm = NULL;
					if (varExists("xywh2nm", tagStr)) {
						setFloatArray(xywh, 4, "xywh2nm", tagStr);
						std::string flipStr = getStringValue("flip2nm", tagStr);
						TexCoords tc2nm;
						tc2nm.set(pMT->uTex2nm, xywh[0], xywh[1], xywh[2], xywh[3], flipStr);
						pTC2nm = &tc2nm;
					}
					else if (varExists("xywh2nm_GL", tagStr)) {
						setFloatArray(xywh, 4, "xywh2nm_GL", tagStr);
						std::string flipStr = getStringValue("flip2nm", tagStr);
						TexCoords tc2nm;
						tc2nm.set_GL(&tc2nm, xywh[0], xywh[1], xywh[2], xywh[3], flipStr);
						pTC2nm = &tc2nm;
					}
					TexCoords* pTC = NULL;
					if (varExists("xywh", tagStr)) {
						setFloatArray(xywh, 4, "xywh", tagStr);
						std::string flipStr = getStringValue("flip", tagStr);
						TexCoords tc;
						tc.set(pMT->uTex0, xywh[0], xywh[1], xywh[2], xywh[3], flipStr);
						pTC = &tc;
					}
					else if (varExists("xywh_GL", tagStr)) {
						setFloatArray(xywh, 4, "xywh_GL", tagStr);
						std::string flipStr = getStringValue("flip", tagStr);
						TexCoords tc;
						tc.set_GL(&tc, xywh[0], xywh[1], xywh[2], xywh[3], flipStr);
						pTC = &tc;
					}
					pMB->buildFace(pMB, "front", pVS, pTC, pTC2nm);
					//pMB->shiftGroup(pMB, 0, 0, Rail::wheelWidth / 2);
				}
				pMB->releaseGroup(pMB);
				//dish inner black side
				if (doBack) {
					pMB->lockGroup(pMB); {
						pML->doTag("<mt_use=gunmetal />");
						pML->doTag("<a=back />");
					}
					pMB->releaseGroup(pMB);
				}
			}
			else if (pML->rimWallH > 0) {
				//rim wall outer
				pMB->lockGroup(pMB); {
					pML->doTag("<mt_use=rim_wall />");
					pML->doTag("<a=wrap />");
					float scale = (pML->wheelRadius - pML->rimWallH) / pML->wheelRadius;
					GroupTransform gt;
					GroupTransform* pGT = &gt;
					pGT->pGroup = pMB->pCurrentGroup;
					pGT->scale[0] = (pML->wheelRadius - pML->rimWallH) / pML->wheelRadius;
					pGT->scale[1] = pGT->scale[0];
					pGT->scale[2] = 1;
					strcpy_s(pGT->applyTo, 32, "front");
					gt.executeGroupTransform(pMB);
					pML->doTag("<do scale='1,1,0' />");
					pML->doTag("<do='set normals' xyz='0,0,1' />");
					pMB->shiftGroup(pMB, 0, 0, Rail::wheelWidth / 2);
				}
				pMB->releaseGroup(pMB);
				//rim inner blackwall
				if (doBack) {
					pMB->lockGroup(pMB); {
						pML->doTag("<mt_use=gunmetal />");
						pML->doTag("<a=wrap />");
						float scale = (pML->wheelRadius - pML->rimWallH) / pML->wheelRadius;
						GroupTransform gt;
						GroupTransform* pGT = &gt;
						pGT->pGroup = pMB->pCurrentGroup;
						pGT->scale[0] = (pML->wheelRadius - pML->rimWallH) / pML->wheelRadius;
						pGT->scale[1] = pGT->scale[0];
						pGT->scale[2] = 1;
						strcpy_s(pGT->applyTo, 32, "back");
						gt.executeGroupTransform(pMB);
						pML->doTag("<do scale='1,1,0' />");
						pMB->shiftGroup(pMB, 0, 0, -Rail::wheelWidth / 2);
					}
					pMB->releaseGroup(pMB);
				}
			}
			//inner lip
			if (doLip && pML->rimWallH < pML->wheelRadius) {
				pMB->lockGroup(pMB); {
					pML->doTag("<mt_use=wheel_main />");
					pVS->whl[0] = pML->wheelRadius * 2 - pML->rimWallH * 2;
					pVS->whl[1] = pVS->whl[0];
					pML->doTag("<a=wrap side=in />");
				}
				pMB->releaseGroup(pMB);
			}
		}
		pML->doTag("</group ay= 90 px=4.5 >");
		pML->doTag("<clone=reflect xyz='1,0,0' />");
		//axis
		if (doAxis) {
			pMB->lockGroup(pMB); {
				pML->doTag("<mt_use=gunmetal />");
				pML->doTag("<vs=cylinder />");
				VirtualShape* pVS = pMB->pCurrentVShape;
				pVS->whl[0] = pML->axisD;
				pVS->whl[1] = pVS->whl[0];
				pVS->whl[2] = Rail::railroadGauge;
				pVS->sectionsR = 6;
				pML->doTag("<a=wrap headZto=x/>");
			}
			pMB->releaseGroup(pMB);
		}
		return 1;
	}

	if (wheelPart.find("wheelCenter") == 0) {
		setFloatValue(&pML->centerDiameter, "diameter", tagStr);
		pMB->lockGroup(pMB); { //1 wheel
			pML->doTag("<mt_use='wheel_main' />");
			Material mt;
			memcpy((void*)&mt, (void*)pMB->materialsList.at(pMB->usingMaterialN), sizeof(Material));
			fillProps_mt(&mt, tagStr, pML);
			pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
			pML->doTag("<vs=box />");
			VirtualShape* pVS = pMB->pCurrentVShape;
			pVS->whl[0] = pML->centerDiameter;
			pVS->whl[1] = pML->centerDiameter;
			pVS->whl[2] = Rail::wheelWidth;
			pML->doTag("<a=front xywh='0,0,1,1' xywh2nm_GL='0,0,1,1' />");
		}
		pML->doTag("</group ay= 90 px=4.5 >");
		pML->doTag("<clone=reflect xyz='1,0,0' />");
		return 1;
	}

	if (wheelPart.find("spokes") == 0) {
		setFloatValue(&pML->spokeWidth, "spokeWidth", tagStr);
		WheelPair* pWP = (WheelPair*)pML->pSubjsVector->at(pMB->usingSubjN);
		if (pWP->spokesN > 1) {
			pML->distanceBetweenSpokes = circumference(pML->wheelRadius) / pWP->spokesN;
		}
		else if (varExists("spokesN", tagStr)) {
			pWP->spokesN = getIntValue("spokesN", tagStr);
			pML->distanceBetweenSpokes = circumference(pML->wheelRadius) / pWP->spokesN;
		}
		else if (varExists("distanceBetweenSpokes", tagStr)) {
			pML->distanceBetweenSpokes = getIntValue("distanceBetweenSpokes", tagStr);
			pWP->spokesN = round(circumference(pML->wheelRadius) / pML->distanceBetweenSpokes);
		}
		else if (pML->distanceBetweenSpokes > 0) {
			pWP->spokesN = round(circumference(pML->wheelRadius) / pML->distanceBetweenSpokes);
		}
		else
			;//use pWP->spokesN
		pWP->angleBetweenSpokes = 360.0 / pWP->spokesN;

		pMB->lockGroup(pMB); { //1 wheel
			pML->doTag("<mt_use='wheel_wire' />");
			Material mt;
			memcpy((void*)&mt, (void*)pMB->materialsList.at(pMB->usingMaterialN), sizeof(Material));
			mt.lineWidth = pML->spokeWidth;
			mt.primitiveType = GL_LINES;
			pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
			pMB->lockGroup(pMB); { //1 spoke
				pMB->addVertex(pMB, 0, -1, 0);// (pML->centerDiameter / 2 - 0.5), 0);
				pMB->addVertex(pMB, 0, -(pML->wheelRadius - pML->spokeWidth), 0);
			}
			pMB->releaseGroup(pMB);
			for (int sN = 1; sN < pWP->spokesN; sN++) {
				pML->doTag("<clone >");
				pMB->rotateGroupDg(pMB, 0, 0, pWP->angleBetweenSpokes);
				pML->doTag("</clone >");
			}
			pMB->rotateGroupDg(pMB, 0, 0, 180);
		}
		pMB->releaseGroup(pMB);

		pML->dishOffset2rimWall = -pML->spokeWidth / 2 - 0.1;
		wheelPart.assign("group2dish");
	}
	if (wheelPart.find("group2dish") == 0) {
		setFloatValue(&pML->dishOffset2rimWall,"offset2wall", tagStr);

		Group01* pGr = pMB->pLastClosedGroup;
		pMB->shiftGroup(pMB, 0, 0, Rail::wheelWidth / 2 + pML->dishOffset2rimWall, pGr);
		pMB->rotateGroupDg(pMB, 0, 90, 0, pGr);
		pMB->shiftGroup(pMB, 4.5, 0, 0, pGr);
		pML->doTag("<clone=reflect xyz='1,0,0' >");
		pMB->rotateGroupDg(pMB, 90, 0,0);
		pML->doTag("</clone >");
		return 1;
	}

	if (wheelPart.find("wheelWeight") == 0) {
		float angleRange = getIntValue("angleRange", tagStr);
		std::string centerShift = getStringValue("centerShift", tagStr);
		bool doBackSide = !varExists("noBackSide", tagStr);
		float weightW = Rail::wheelWidth / 3;
		pMB->lockGroup(pMB); { //1 wheel
			pML->doTag("<mt_use='wheel_main' />");
			pML->doTag("<vs=cylinder />");
			VirtualShape* pVS = pMB->pCurrentVShape;
			float r = pML->wheelRadius - pML->rimWallH;
			pVS->whl[0] = r*2;
			pVS->whl[1] = pVS->whl[0];				
			pVS->whl[2] = weightW;
			pVS->sectionsR = round((pML->wheelRadius * 4 * angleRange)/360);
			if (pVS->sectionsR < 1)
				pVS->sectionsR = 1;
			pVS->angleFromTo[0] = -angleRange / 2;
			pVS->angleFromTo[1] =  angleRange / 2;
			pML->doTag("<a=front />");
			//pML->doTag("<do='set normals' xyz='0,0,1'/>");
			if(doBackSide)
				pML->doTag("<a=back mt_use=black />");
			pML->doTag("<a=cut />");
			if (centerShift.compare("") != 0) {
				float dy = cos(angleRange * degrees2radians / 2) * r;
				if (centerShift.compare("concave") == 0)
					dy += (sin(angleRange * degrees2radians / 2) * r * 0.15);
				GroupTransform gt;
				GroupTransform* pGT = &gt;
				pGT->pGroup = pMB->pCurrentGroup;
				pGT->rMax[2] = 0.1;
				pGT->shift[0] = dy;
				pGT->executeGroupTransform(pMB);
			}
			pML->doTag("<do='calc merge normals' />");
			pMB->rotateGroupDg(pMB, 0, 0, 90);
			pMB->shiftGroup(pMB, 0, 0, Rail::wheelWidth/2- weightW/2);
		}
		pML->doTag("</group ay= 90 px=4.5 >");
		pML->doTag("<clone=reflect xyz='1,0,0' >");
		pMB->rotateGroupDg(pMB, 90, 0, 0);
		pML->doTag("</clone >");
		return 1;
	}

	if (wheelPart.find("rodAxis") == 0) {
		float diam = getFloatValue("diameter", tagStr);
		float shift = getFloatValue("lever", tagStr);
		pMB->lockGroup(pMB); { //1 wheel
			pML->doTag("<mt_use='wheel_main' />");
			Material mt;
			memcpy((void*)&mt, (void*)pMB->materialsList.at(pMB->usingMaterialN), sizeof(Material));
			fillProps_mt(&mt, tagStr, pML);
			pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
			pML->doTag("<vs=box />");
			VirtualShape* pVS = pMB->pCurrentVShape;
			pVS->whl[0] = diam;
			pVS->whl[1] = diam;
			pVS->whl[2] = Rail::wheelWidth;
			pML->doTag("<a=front xywh='0,0,1,1' xywh2nm_GL='0,0,1,1' />");
			pMB->shiftGroup(pMB, 0, -shift, 0);
		}
		pML->doTag("</group ay= 90 px=4.5 >");
		pML->doTag("<clone=reflect xyz='1,0,0' >");
		pMB->rotateGroupDg(pMB, 90, 0, 0);
		pML->doTag("</clone >");
		return 1;
	}
	mylog("ERROR in ModelLoaderRR::processTag_buildWheelRair: unhandled wheelPart %s\n %s\n", wheelPart.c_str(), tagStr.c_str());
	mylog("file %s\n", pML->fullPath.c_str());
	return -1;
}

