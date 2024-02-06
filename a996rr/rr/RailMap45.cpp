#include "RailMap45.h"
#include "ModelLoader.h"
#include "TheApp.h"

extern float PI;
extern float degrees2radians;
extern TheApp theApp;

RailMap45::~RailMap45() {

}

void RailMap45::cleanUp() {
	for (int i = railsMap.size() - 1; i >= 0; i--) {
		SceneSubj* pR = railsMap.at(i);
		if(pR != NULL)
			delete pR;
	}
	railsMap.clear();

	TheTable::cleanUp();
}
void RailMap45::initVirtualNet() {
	initVirtualNet00(0, 0, MyColor::getUint32(1.0f, 1.0f, 1.0f));
}

void RailMap45::initVirtualNet00(float dx, float dz, unsigned int color32) {
	float y0 = theApp.gameTable.groundLevel + 0.5;
	for (int zN = 0; zN < tableTiles[1]; zN++) {
		float z0 = tileSize * zN - worldBox.bbRad[2] + tileSize / 2 + dz;
		if (abs(z0) > worldBox.bbRad[2] - tileSize / 2)
			continue;
		for (int xN = 0; xN < tableTiles[0]; xN++) {
			float x0 = tileSize * xN - worldBox.bbRad[0] + tileSize / 2 +dx;
			if (abs(x0) > worldBox.bbRad[0] - tileSize / 2)
				continue;
			Rail* pSS = new Rail((Rail*)Rail::railModels.at(Rail::railModelSnowFlakeN));
			railsMap.push_back(pSS);
			pSS->nInSubjsSet = railsMap.size() - 1;
			pSS->pSubjsSet = &railsMap; //which vector/set this subj belongs to
			v3set(pSS->ownCoords.pos, x0, y0, z0);
			pSS->buildModelMatrix();

			pSS->uHighLightLevel = 1.0;
			pSS->uHighLightColor.setUint32(color32);
		}
	}
	//crossings
	for (int zN = 0; zN < tableTiles[1]; zN++) {
		float z0 = tileSize * zN - worldBox.bbRad[2];
		if (abs(z0) >= worldBox.bbRad[2] - tileSize / 2)
			continue;
		for (int xN = 0; xN < tableTiles[0]; xN++) {
			float x0 = tileSize * xN - worldBox.bbRad[0];
			if (abs(x0) >= worldBox.bbRad[0] - tileSize / 2)
				continue;
			//initVirtualTile(x0, z0);
			Rail* pSS = new Rail((Rail*)Rail::railModels.at(Rail::railModelSnowFlakeSmallN));
			railsMap.push_back(pSS);
			pSS->nInSubjsSet = railsMap.size() - 1;
			pSS->pSubjsSet = &railsMap; //which vector/set this subj belongs to
			v3set(pSS->ownCoords.pos, x0, y0, z0);
			pSS->buildModelMatrix();
		}
	}
}
void RailMap45::basicOval() {

	//setRail(3.5, 3.5, 135, -45, 0);

	setRail(1.5, 0.5, 315, 90, 0);
	setRail(2.5, 0.5, 270, 45, 0);
	setRail(3.5, 1.5, 225, 45, 0);
	setRail(4.5, 2.5, 225, 0, 0);
	setRail(4.5, 3.5, 180, 315, 0);
	setRail(3.5, 4.5, 135, 270, 0);
	setRail(2.5, 4.5, 90, 225, 0);
	setRail(1.5, 3.5, 45, 225, 0);
	setRail(0.5, 2.5, 45, 180, 0);
	setRail(0.5, 1.5, 0, 135, 0);
	/*
	for (int rN = 0; rN < railsMap.size(); rN++) {
		Rail* pR = (Rail*)railsMap.at(rN);
		Rail::toLog("", pR);
		for (int eN = 0; eN < 2; eN++) {
			RailEnd* pRE = &pR->railEnd[eN];
			if (pRE->toRailN < 0)
				int a = 0;
		}
	}
	*/
}



void RailMap45::initRailModelsVirtual() {

	ModelBuilder* pMB = new ModelBuilder();
	Material mt;
	strcpy_s(mt.shaderType32, 32, "flat");
	strcpy_s(mt.materialName32, 32, "virtual rail");
	mt.uColor.setRGBA(255, 255, 255);
	mt.lineWidthFixed = true;
	mt.dropsShadow = false;
	mt.lineWidth = 1;

	//railModelStraightVirtualN
	mt.primitiveType = GL_LINES;
	pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
	Rail::railModelStraightVirtualN = Rail::railModels.size();
	Rail* pSS = new Rail();
	Rail::railModels.push_back(pSS);
	pSS->railStatus = 1; //virtual
	//pSS->ownCoords.pos[1] = Rail::railsLevel;
	pSS->pDrawJobs = &table_drawJobs;
	pSS->pSubjsSet = &Rail::railModels;
	pSS->nInSubjsSet = Rail::railModels.size() - 1;
	pSS->buildModelMatrix();
	pMB->useSubjN(pMB, pSS->nInSubjsSet);
	pMB->lockGroup(pMB);
	pMB->addVertex(pMB, -tileSize / 2, 0, 0);
	pMB->addVertex(pMB, tileSize / 2, 0, 0);
	pMB->releaseGroup(pMB);

	//pMB->buildDrawJobs(pMB, &Rail::railModels, &table_drawJobs, &table_buffersIds);

	//railModelShortVirtualN
	mt.primitiveType = GL_LINES;
	pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
	Rail::railModelShortVirtualN = Rail::railModels.size();
	pSS = new Rail();
	Rail::railModels.push_back(pSS);
	pSS->railStatus = 1; //virtual
	//pSS->ownCoords.pos[1] = Rail::railsLevel;
	pSS->pDrawJobs = &table_drawJobs;
	pSS->pSubjsSet = &Rail::railModels;
	pSS->nInSubjsSet = Rail::railModels.size() - 1;
	pSS->buildModelMatrix();
	pMB->useSubjN(pMB, pSS->nInSubjsSet);
	pMB->lockGroup(pMB);
	pMB->addVertex(pMB, -Rail::railLenghtShort / 2, 0, 0);
	pMB->addVertex(pMB, Rail::railLenghtShort / 2, 0, 0);
	pMB->releaseGroup(pMB);

	//pMB->buildDrawJobs(pMB, &Rail::railModels, &table_drawJobs, &table_buffersIds);

	//railModelCurvedVirtualN
	mt.primitiveType = GL_LINE_STRIP;
	pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
	Rail::railModelCurvedVirtualN = Rail::railModels.size();
	pSS = new Rail();
	Rail::railModels.push_back(pSS);
	pSS->railStatus = 1; //virtual
	//pSS->ownCoords.pos[1] = Rail::railsLevel;
	pSS->pDrawJobs = &table_drawJobs;
	pSS->pSubjsSet = &Rail::railModels;
	pSS->nInSubjsSet = Rail::railModels.size() - 1;
	pSS->buildModelMatrix();
	pSS->gabaritesWorld.chordType = -1;
	pSS->gabaritesOnScreen.chordType = -1;

	pMB->useSubjN(pMB, pSS->nInSubjsSet);
	int lineStartsAt = pMB->vertices.size();
	pMB->lockGroup(pMB);
	for (float aY = -15; aY <= 15; aY += 5) {
		float angleRd = (aY - 90.0) * degrees2radians;
		float kx = cosf(angleRd) * Rail::curveRadius;
		float kz = sinf(angleRd) * Rail::curveRadius;
		pMB->addVertex(pMB, kx, 0, kz);
	}
	//mark first and last verts
	Vertex01* pV = pMB->vertices.back();
	pV->endOfSequence = 1;
	pV = pMB->vertices.at(lineStartsAt);
	pV->endOfSequence = -1;
	pMB->moveGroupDg(pMB, 0, 0, 0, 0, 0, Rail::tileCenter2curveCenter);
	pMB->releaseGroup(pMB);

	//pMB->buildDrawJobs(pMB, &Rail::railModels, &table_drawJobs, &table_buffersIds);
	
	
	mt.uAlphaBlending = true;
	mt.uAlphaFactor = 0.2;
	mt.lineWidth = 1;

	//railModelSbowFlakeN
	mt.primitiveType = GL_LINES;
	pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
	pMB->lockGroup(pMB);
	Rail::railModelSnowFlakeN = Rail::railModels.size();
	pSS = new Rail();
	Rail::railModels.push_back(pSS);
	pSS->railStatus = -1; //virtual
	//pSS->ownCoords.pos[1] = Rail::railsLevel-2.5;
	pSS->pDrawJobs = &table_drawJobs;
	pSS->pSubjsSet = &Rail::railModels;
	pSS->nInSubjsSet = Rail::railModels.size() - 1;
	pSS->buildModelMatrix();
	pMB->useSubjN(pMB, pSS->nInSubjsSet);
	for (float yaw = 0; yaw < 180; yaw += 45) {
		pMB->lockGroup(pMB);
		pMB->addVertex(pMB, 0, 0, -tileSize / 2);
		pMB->addVertex(pMB, 0, 0, tileSize / 2);
		pMB->rotateGroupDg(pMB, 0, yaw, 0);
		pMB->releaseGroup(pMB);
	}
	pMB->releaseGroup(pMB);
	//curved rails
	mt.primitiveType = GL_LINE_STRIP;
	pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
	for (float yaw = 0; yaw <= 360; yaw += 45) {
		int lineStartsAt = pMB->vertices.size();
		pMB->lockGroup(pMB);
		for (float aY = -22.5; aY <= 22.5; aY += 5) {
			float angleRd = (aY - 90.0) * degrees2radians;
			float kx = cosf(angleRd) * Rail::curveRadius;
			float kz = sinf(angleRd) * Rail::curveRadius;
			pMB->addVertex(pMB, kx, 0, kz);
		}
		//mark first and last verts
		Vertex01* pV = pMB->vertices.back();
		pV->endOfSequence = 1;
		pV = pMB->vertices.at(lineStartsAt);
		pV->endOfSequence = -1;
		pMB->shiftGroup(pMB, 0, 0, Rail::tileCenter2curveCenter);
		pMB->rotateGroupDg(pMB, 0, yaw - 22.5, 0);
		pMB->releaseGroup(pMB);
	}
	//pMB->buildDrawJobs(pMB, &Rail::railModels, &table_drawJobs, &table_buffersIds);

	//railModelCrossN
	mt.primitiveType = GL_LINES;
	mt.lineWidth = 3;
	pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
	pMB->lockGroup(pMB);
	Rail::railModelSnowFlakeSmallN = Rail::railModels.size();
	pSS = new Rail();
	Rail::railModels.push_back(pSS);
	pSS->railStatus = -1; //virtual
	//pSS->ownCoords.pos[1] = Rail::railsLevel-2.5;
	pSS->pDrawJobs = &table_drawJobs;
	pSS->pSubjsSet = &Rail::railModels;
	pSS->nInSubjsSet = Rail::railModels.size() - 1;
	pSS->buildModelMatrix();
	pMB->useSubjN(pMB, pSS->nInSubjsSet);
	for (float yaw = 45; yaw < 180; yaw += 90) {
		pMB->lockGroup(pMB);
		pMB->addVertex(pMB, 0, 0, -Rail::railLenghtShort / 2);
		pMB->addVertex(pMB, 0, 0, Rail::railLenghtShort / 2);
		pMB->moveGroupDg(pMB, 0, yaw, 0, 0, 0, 0);
		pMB->releaseGroup(pMB);
	}
	pMB->releaseGroup(pMB);
	pMB->buildDrawJobs(pMB, &Rail::railModels, &table_drawJobs, &table_buffersIds);
	delete pMB;
}

void RailMap45::initRailModels() {
	Rail::railsLevel = groundLevel + 3;
	Rail::curveRadius = tileSize * 1.207;
	Rail::tileCenter2curveCenter = tileSize * 1.306;
	Rail::railLenghtLong = tileSize;
	Rail::railLenghtShort = tileSize * 0.414;// 0.207;
	Rail::railLenghtCurved = Rail::curveRadius * PI * 2 / 8;

	suggestedStageSize = Rail::railLenghtLong * 6;

	ModelLoader* pML = new ModelLoader(&Rail::railModels, &table_drawJobs, &table_buffersIds,-1,NULL);
	ModelBuilder* pMB = pML->pModelBuilder;
	pML->doTag("<include='/dt/mt/materials.txt' />");
	pML->doTag("<texture_as=tx1 src='/dt/md/rr/rail01.bmp'/>");


	int year = 1910; //getRandom(1860, 2023); //
	pML->doTag("<mt_save_as=rail_wire mt_use=metal-wire uSpecularIntencity=0.2 uColor=#cccccc lineWidth=0.8 lineWidthFixed=2 / >");
	pML->doTag("<mt_save_as=rail_top mt_use=metal uColor=#cccccc / >");
	if (year < 1920) { //old time

		pML->doTag("<mt_save_as=sleeper_wall mt_use=teflon uColor=#665544 / >");
		pML->doTag("<mt_save_as=sleeper_top mt_use=sleeper_wall />");
		pML->doTag("<mt_save_as=fastener mt_use=metal uColor=#442211 uTex2nm_use=tx1 />");

		pML->doTag("<mt_save_as=rail_wall mt_use=sleeper_wall uTex2nm_use=tx1 / >");// 
	}
	else if (year > 1980) { //modern/concrete
		//pML->doTag("<mt_save_as=rail_wall mt_use=metal uColor=#555555 uTex2nm_use=tx1 / >");// 
		pML->doTag("<mt_save_as=rail_wall mt_use=metal uColor=#333333 uTex2nm_use=tx1 / >");// 

		pML->doTag("<mt_save_as=sleeper_wall mt_use=satin uColor=#9f999f / >");
		pML->doTag("<mt_save_as=sleeper_top mt_use=sleeper_wall uTex2nm_use=tx1 />");
		//pML->doTag("<mt_save_as=fastener mt_use=rail_wall />");// uColor = #888888 / > "); //
		pML->doTag("<mt_save_as=fastener mt_use=rail_wall uColor = #888898 / > "); //
	}
	else { //default
		pML->doTag("<mt_save_as=rail_wall mt_use=rail_top uColor=#dddddd uTex2nm_use=tx1 / >");// 

		pML->doTag("<mt_save_as=sleeper_wall mt_use=satin uColor=#222222 / >");
		pML->doTag("<mt_save_as=sleeper_top mt_use=sleeper_wall />");
		pML->doTag("<mt_save_as=fastener mt_use=sleeper_wall uColor=#555555 uTex2nm_use=tx1 />");
	}
	Rail::railModelStraightN = buildModelRailStraight(pML, Rail::railLenghtLong, 0);
	Rail::railModelShortN = buildModelRailStraight(pML, Rail::railLenghtShort, 0);
	Rail::railModelCurvedN = buildModelRailCurved(pML, 0);

	Rail::railModelStraightNsmall = buildModelRailStraight(pML, Rail::railLenghtLong, -1);
	Rail::railModelShortNsmall = buildModelRailStraight(pML, Rail::railLenghtShort, -1);
	Rail::railModelCurvedNsmall = buildModelRailCurved(pML, -1);

	Rail::railModelStraightNtiny = buildModelRailStraight(pML, Rail::railLenghtLong, -2);
	Rail::railModelShortNtiny = buildModelRailStraight(pML, Rail::railLenghtShort, -2);
	Rail::railModelCurvedNtiny = buildModelRailCurved(pML, -2);

	delete pML;

	Rail::couplerModelN = ModelLoader::loadModelStandard(&Rail::railModels, &table_drawJobs, &table_buffersIds, "/dt/md/rr/coupler/couplerlock01.txt");
	Rail::couplersCoupleModelN = ModelLoader::loadModelStandard(&Rail::railModels, &table_drawJobs, &table_buffersIds, "/dt/md/rr/coupler/couplerspair01.txt");
}
int RailMap45::buildModelRailStraight(ModelLoader* pML, float railLenght, int sizeMode) {
	ModelBuilder* pMB = pML->pModelBuilder;

	Rail* pR = new Rail();
	pR->ownCoords.pos[1] = Rail::railsLevel;
	pR->railType = 0;
	pR->railLength = railLenght;
	pR->gabaritesWorld.chordType = -1;
	pR->gabaritesOnScreen.chordType = -1;

	int railModelN = pML->addSubj(pR);

	pML->doTag("<group >"); {//entire section with sleepers
		buildModelRailPairSection(pML, railLenght,sizeMode);
		
		if (sizeMode<0) {
			//rail top wire
			pML->doTag("<mt_use=rail_wire />");
			pML->doTag("<group >"); {
				pML->doTag("<short p0='0,0,-0.5' dz=1 />");
			}
			pMB->scaleGroup(pMB, 1, 1, railLenght);
			pMB->shiftGroup(pMB, Rail::railroadGauge / 2, 0, 0);
			pML->doTag("</group >");
			pML->doTag("<clone=reflect xyz='1,0,0'/>");
		}
		
		
		//sleepers
		int intervalsN = round(railLenght / Rail::railSleepersInterval);
		float sleepersInterval = railLenght / intervalsN;
		float sleeperZ = -railLenght / 2 + sleepersInterval / 2;
		pML->doTag("<group >"); {
			sleeperGroup(pML, sizeMode);
			pMB->shiftGroup(pMB, 0, 0, sleeperZ);
		}
		pML->doTag("</group >");
		while (1) {
			sleeperZ += sleepersInterval;
			if (sleeperZ >= railLenght / 2)
				break;
			pML->doTag("<clone>");
			pMB->shiftGroup(pMB, 0, 0, sleepersInterval);
			pML->doTag("</clone>");
		}
		
	}
	//whole rail section with sleepers
	pML->doTag("</group ay=90 >");
	return railModelN;
}
void RailMap45::buildModelRailPairSection(ModelLoader* pML, float railLenght, int sizeMode) {
	ModelBuilder* pMB = pML->pModelBuilder;
	pML->doTag("<vs=box-tank whl='20,6,2' />");
	float ext = Rail::railWidth * 0.3f;
	VirtualShape* pVS = pMB->pCurrentVShape;
	v3set(pVS->whl, Rail::railWidth-ext*2, Rail::railHeight-ext, railLenght);
	pVS->extU = ext;
	pVS->extL = ext;
	pVS->extR = ext;

	pML->doTag("<group >"); { //1 rails
		if (sizeMode >-1) {
			pML->doTag("<mt_use='rail_top' />");
			pML->doTag("<a='top all' />");
		}
		if (sizeMode > -2) {
			pML->doTag("<mt_use='rail_wall' />");
			pML->doTag("<a='left,right' xywh2nm='0.5,0,0,8' />");
			pMB->shiftGroup(pMB, 0, -(Rail::railHeight-ext)/2-ext, 0);
			pML->doTag("<do='calculate normals' />");
			pML->doTag("<do='merge normals' />");
		}
	}
	pMB->shiftGroup(pMB, Rail::railroadGauge/2, 0, 0);
	pML->doTag("</group >");
	pML->doTag("<clone >");
	pMB->shiftGroup(pMB, -Rail::railroadGauge, 0, 0);
	pML->doTag("</clone >");
}
int RailMap45::buildModelRailCurved(ModelLoader* pML, int sizeMode) {
	ModelBuilder* pMB = pML->pModelBuilder;

	Rail* pR = new Rail();
	strcpy_s(pR->name64, 64, "rail");
	pR->ownCoords.pos[1] = Rail::railsLevel;
	pR->railType = 1;
	pR->railLength = Rail::railLenghtCurved;
	pR->gabaritesWorld.chordType = -1;
	pR->gabaritesOnScreen.chordType = -1;
	int railModelN = pML->addSubj(pR);

	pMB->lockGroup(pMB); { //entire rail section
		float stepDg = 5;
		if (sizeMode<0) {
			//wire rails
			pML->doTag("<mt_use=rail_wire />");
			//outer rail
			float r = Rail::curveRadius + Rail::railroadGauge / 2;
			pML->doTag("<line>"); //
			for (float aY = -22.5; aY <= 22.5; aY += stepDg) {
				float angleRd = (aY - 90.0) * degrees2radians;
				float kx = cosf(angleRd) * r;
				float kz = sinf(angleRd) * r;
				pMB->addVertex(pMB, kx, 0, kz);
			}
			pML->doTag("</line>");
			//inner rail
			r = Rail::curveRadius - Rail::railroadGauge / 2;
			pML->doTag("<line>"); //
			for (float aY = -22.5; aY <= 22.5; aY += stepDg) {
				float angleRd = (aY - 90.0) * degrees2radians;
				float kx = cosf(angleRd) * r;
				float kz = sinf(angleRd) * r;
				pMB->addVertex(pMB, kx, 0, kz);
			}
			pML->doTag("</line>");
		}
		if (sizeMode > -2) {
			//guideline
			pML->doTag("<mt_type=flat uColor=#00ff00 lineWidth=1 />");
			pML->doTag("<line mark='guideline1' dontRender >"); //
			for (float aY = -22.5; aY <= 22.5; aY += stepDg) {
				float angleRd = (aY - 90.0) * degrees2radians;
				float kx = cosf(angleRd) * Rail::curveRadius;
				float kz = sinf(angleRd) * Rail::curveRadius;
				pMB->addVertex(pMB, kx, 0, kz);
			}
			pML->doTag("</line>");
			//rails to clone
			pML->doTag("<group >"); {//rails pair section
				buildModelRailPairSection(pML, 1,sizeMode);
			}
			pML->doTag("<group2line='guideline1' />");
			pML->doTag("<do='calculate normals'>");
			pML->doTag("<do='merge normals'>");
			pML->doTag("</group >");
		}
		//sleepers
		int intervalsN = round(Rail::railLenghtCurved / Rail::railSleepersInterval);
		stepDg = 45.0f / intervalsN;
		pMB->lockGroup(pMB);
		sleeperGroup(pML,sizeMode);
		pMB->moveGroupDg(pMB, 0, 90, 0, 0, 0, -Rail::curveRadius);
		float nextDg = -22.5 + stepDg / 2;
		pMB->moveGroupDg(pMB, 0, nextDg, 0, 0, 0, 0);
		pMB->releaseGroup(pMB);

		while (1) {
			nextDg += stepDg;
			if ((nextDg >= 22.5))
				break;
			pML->doTag("<clone >");
			pMB->moveGroupDg(pMB, 0, stepDg, 0, 0, 0, 0);
			pML->doTag("</clone >");
		}

	}

	// move entire rail section
	pMB->moveGroupDg(pMB, 0, 0, 0, 0, 0, Rail::tileCenter2curveCenter);
	//pMB->moveGroupDg(pMB, 0, 90, 0, 0, 10, 0);
	//pML->doTag("<do scale='4,4,4'/>");
	pMB->releaseGroup(pMB);

	return railModelN;
}


void RailMap45::sleeperGroup(ModelLoader* pML,int sizeMode) {

	ModelBuilder* pMB = pML->pModelBuilder;
	pML->doTag("<vs=box whl='20,6,2' />");
	VirtualShape* pVS = pMB->pCurrentVShape;
	pML->doTag("<group >"); {
		v3set(pVS->whl, Rail::railSleeperLength, Rail::railSleeperHeight, Rail::railSleeperWidth);
		if (sizeMode > -2) {
			pML->doTag("<mt_use=sleeper_wall />");
			pML->doTag("<a='left,right,front,back' />");
		}
		pML->doTag("<mt_use=sleeper_top />");
		pML->doTag("<a=top xywh2nm='1,0.5,15,0' />");
		if (sizeMode > -2) {
			//fasteners
			pML->doTag("<group >"); {
				pML->doTag("<mt_use=fastener />");
				v3set(pVS->whl, Rail::fastenerWidth, 0, Rail::fastenerWidth);
				//fastener
				pML->doTag("<group >"); {
					pML->doTag("<a='top' xywh2nm='1,1,7,7' flip2nm=-90 />");
					pML->doTag("<do py=0.5 applyTo=left>");
					pML->doTag("<do='calculate normals'>");
					pMB->moveGroupDg(pMB, 0, 0, 0, Rail::fastenerWidth / 2 + Rail::railWidth / 2, 0.6, 0);
				}
				pML->doTag("</group>");
				pML->doTag("<clone=reflect xyz='1, 0, 0' />");
				pMB->moveGroupDg(pMB, 0, 0, 0, Rail::railroadGauge / 2, 0, 0);
			}
			pML->doTag("</group >");
			pML->doTag("<clone=reflect xyz='1, 0, 0' />");
		}
	}
	pMB->shiftGroup(pMB, 0, -Rail::railHeight - Rail::railSleeperHeight / 2, 0);
	pML->doTag("</group >");
}
int RailMap45::findClosestRail(RailCoord* pRC, RailMap45* pTable, Coords* pCoords) {
	std::vector<SceneSubj*>* pMap = &pTable->railsMap;
	float bestDistance = 1000000;
	int bestRailN = -1;
	Rail* pR = NULL;
	int railsN = pMap->size();
	for (int rN = 0; rN < railsN; rN++) {
		pR = (Rail*)pMap->at(rN);
		if (pR == NULL)
			continue;
		if (pR->railStatus != 0)
			continue;

		float dist=pR->dist2point(pCoords->pos, bestDistance);
		if (bestDistance > dist) {
			bestDistance = dist;
			bestRailN = rN;
			if (bestDistance == 0)
				break;
		}
	}
	if (bestRailN < 0)
		return -1;
	if (pRC == NULL)
		return bestRailN;
	pRC->railN = bestRailN;
	pR = (Rail*)pMap->at(bestRailN);
	pRC->percentOnRail = 0.5;
	//aligned?
	if(abs(angleDgFromTo(pCoords->getEulerDg(1),pR->yawInOut[1])) <= 90)
		pRC->alignedWithRail = 1;
	else
		pRC->alignedWithRail = -1;
	fillRailCoordsFromPercent(pRC, pR);
	/*
	//dismiss previously selected rails
	for (int rN = 0; rN < railsN; rN++) {
		Rail* pR0 = (Rail*)pMap->at(rN);
		if (pR0 == NULL)
			continue;
		pR0->uFogLevel = 0;
	}
	pR->setFog(0.7, MyColor::getUint32(1.0f,1.0f,1.0f));

	mylog("bestRailN=%d at %dx%d, chorde %dx%d=>%dx%d bestDistance=%d\n", bestRailN, (int)pR->ownCoords.pos[0], (int)pR->ownCoords.pos[2], 
		(int)pR->line2d.p0[0], (int)pR->line2d.p0[1],
		(int)pR->line2d.p1[0], (int)pR->line2d.p1[1],
		(int)bestDistance);
		*/
	return bestRailN;
}


int RailMap45::addNewRail(float idxX, float idxZ, float yawIn, float yawOut, int railStatus) {
	Rail* pR = getRail(idxX, idxZ, yawIn, yawOut);
	if (pR != NULL) {
		if (pR->railStatus == railStatus)
			return pR->nInSubjsSet;
		if (railStatus == 0) {
			//free slot
			railsMap.at(pR->nInSubjsSet) = NULL;
			delete pR;
		}
	}
	if (abs(angleDgFromTo(yawIn, yawOut)) == 135) //curve
		pR = new Rail((Rail*)Rail::railModels.at(Rail::railModelCurvedN));
	else {//straight
		if (idxX == (int)idxX) //short straight
			pR = new Rail((Rail*)Rail::railModels.at(Rail::railModelShortN));
		else //long straight
			pR = new Rail((Rail*)Rail::railModels.at(Rail::railModelStraightN));
	}
	pR->pSubjsSet = &railsMap;
	//find empty slot
	int railsN = railsMap.size();
	for (int rN = 0; rN <= railsN; rN++) {
		if (rN == railsN) {
			railsMap.push_back(pR);
			pR->nInSubjsSet = railsN;
			break;
		}
		Rail* pR0 = (Rail*)railsMap.at(rN);
		if (pR0 == NULL) {
			pR->nInSubjsSet = rN;
			railsMap.at(rN) = pR;
			break;
		}
	}
	yawIn = angleDgNorm360(yawIn);
	yawOut = angleDgNorm360(yawOut);
	if (yawIn > yawOut) {//swap
		float a = yawOut;
		yawOut = yawIn;
		yawIn = a;
	}
	pR->yawInOut[0] = yawIn;
	pR->yawInOut[1] = yawOut;
	float modelYaw = (yawOut + yawIn) / 2;
	if (angleDgFromTo(yawIn, modelYaw) > 90)
		modelYaw = angleDgNorm360(modelYaw - 180);
	pR->ownCoords.setEulerDg(0, modelYaw, 0);
	pR->tileIndexXZ[0] = idxX;
	pR->tileIndexXZ[1] = idxZ;
	pR->ownCoords.pos[0] = tileSize * idxX - worldBox.bbRad[0];
	pR->ownCoords.pos[2] = tileSize * idxZ - worldBox.bbRad[2];
	yawIn = angleDgNorm360(yawIn);
	if (pR->railType == 1) //curve
		pR->CCW = signOf(angleDgFromTo(pR->yawInOut[1], pR->yawInOut[0]));

	pR->buildModelMatrix();

	//set in and out points
	float angle00,a0,a1;
	if (pR->railType == 0) {
		//straight
		float vIn[4] = { 0,0,0,0 };
		vIn[0] = -pR->railLength / 2;
		mat4x4_mul_vec4plus(pR->p0, pR->ownModelMatrixUnscaled, vIn, 1, true);
		vIn[0] = pR->railLength / 2;
		mat4x4_mul_vec4plus(pR->p1, pR->ownModelMatrixUnscaled, vIn, 1, true);
	}
	else{//curve
		float vIn[4] = { 0,0,0,0 };
		vIn[2] = Rail::tileCenter2curveCenter;
		mat4x4_mul_vec4plus(pR->curveCenter, pR->ownModelMatrixUnscaled, vIn, 1, true);
		angle00 = angleDgNorm180(modelYaw + 180.0);
		a0 = angle00 - 22.5 * pR->CCW;
		float angleRd = a0 * degrees2radians;
		pR->p0[0] = pR->curveCenter[0] + sin(angleRd) * Rail::curveRadius;
		pR->p0[2] = pR->curveCenter[2] + cos(angleRd) * Rail::curveRadius;
		a1 = angle00 + 22.5 * pR->CCW;
		angleRd = a1 * degrees2radians;
		pR->p1[0] = pR->curveCenter[0] + sin(angleRd) * Rail::curveRadius;
		pR->p1[2] = pR->curveCenter[2] + cos(angleRd) * Rail::curveRadius;
		//radiusAngles
		pR->radiusAngle[0] = a0;
		pR->radiusAngle[1] = a1;
		ArcXY::initArc(&pR->arc2d, pR->curveCenter[0], pR->curveCenter[2], Rail::curveRadius, pR->radiusAngle[0], pR->radiusAngle[1], pR->CCW);
	}
	/*
	mylog("\nrail %.1f x %.1f, %d->%d modelYaw=%d\n", pR->tileIndexXZ[0], pR->tileIndexXZ[1], (int)pR->yawIn, (int)pR->yawOut, (int)modelYaw);
	mylog("xx: %s\n", v3toStrXZ(pR->ownCoords.pos));
	if(pR->railType==1)
		mylog("curve: %s a00=%5.1f, from %5.1f to %5.1f CCW=%d\n", v3toStrXZ(pR->curveCenter), angle00, a0, a1, pR->CCW);
	mylog("p0: %s\n", v3toStrXZ(pR->p0));
	mylog("p1: %s\n", v3toStrXZ(pR->p1));
	*/
	LineXY::initLineXZ(&pR->line2d, pR->p0, pR->p1);
	
	fillRailEnd(pR, 0);
	fillRailEnd(pR, 1);
	checkRailCrossing(pR);

	return pR->nInSubjsSet;
}


int RailMap45::getRailN(float idxX, float idxZ, float yawIn, float yawOut) {
	float idxXZ[2];
	idxXZ[0] = idxX;
	idxXZ[1] = idxZ;
	float yawInOut[2];
	yawIn = angleDgNorm360(yawIn);
	yawOut = angleDgNorm360(yawOut);
	if (yawIn > yawOut) {//swap
		float a = yawOut;
		yawOut = yawIn;
		yawIn = a;
	}
	yawInOut[0] = yawIn;
	yawInOut[1] = yawOut;
	//find
	int railsN = railsMap.size();
	int railsInTile = 0;
	for (int rN = 0; rN < railsN; rN++) {
		Rail* pR0 = (Rail*)railsMap.at(rN);
		if (pR0 == NULL)
			continue;
		if (!v2match(pR0->tileIndexXZ, idxXZ))
			continue;
		if (!v2match(pR0->yawInOut, yawInOut))
			continue;
		return rN;
	}
	return -1;
}

Rail* RailMap45::getRail(float idxX, float idxZ, float yawIn, float yawOut) {
	int railN = getRailN(idxX, idxZ, yawIn, yawOut);
	if (railN < 0)
		return NULL;
	return (Rail*)railsMap.at(railN);
}
int RailMap45::setRail(float idxX, float idxZ, float yawIn, float yawOut, int railStatus) {
	int railN00 = addNewRail(idxX, idxZ, yawIn, yawOut, railStatus);
	if (railStatus == 0) {
		addDiagonalShortTo(idxX, idxZ, yawIn, railStatus);
		addDiagonalShortTo(idxX, idxZ, yawOut, railStatus);
	}
	return railN00;
}
int RailMap45::addDiagonalShortTo(float idxX, float idxZ, float yaw2extend, int railStatus) {
	yaw2extend = angleDgNorm360(yaw2extend);
	if (idxX == (int)idxX) //short itself
		return -1;
	if (yaw2extend == 45) {
		idxX += 0.5;
		idxZ += 0.5;
	}
	else if (yaw2extend == 135) {
		idxX += 0.5;
		idxZ -= 0.5;
	}
	else if (yaw2extend == 225) {
		idxX -= 0.5;
		idxZ -= 0.5;
	}
	else if (yaw2extend == 315) {
		idxX -= 0.5;
		idxZ += 0.5;
	}
	else
		return -1; //not diagonal

	return addNewRail(idxX, idxZ, yaw2extend, yaw2extend - 180, railStatus);
}
int RailMap45::fillRailEnd(Rail* pR, int whichEnd) {
	RailEnd* pRE = &pR->railEnd[whichEnd];
	pRE->endYaw = pR->yawInOut[whichEnd];
	pRE->ownRailN = pR->nInSubjsSet;
	//define connected tile index
	v2copy(pRE->toTileIndexXZ, pR->tileIndexXZ);
	if (pRE->endYaw == 0)
		pRE->toTileIndexXZ[1] += 1.0;
	else if (pRE->endYaw == 180)
		pRE->toTileIndexXZ[1] -= 1.0;
	else if (pRE->endYaw == 90)
		pRE->toTileIndexXZ[0] += 1.0;
	else if (pRE->endYaw == 270)
		pRE->toTileIndexXZ[0] -= 1.0;
	else if (pRE->endYaw == 45) {
		pRE->toTileIndexXZ[0] += 0.5;
		pRE->toTileIndexXZ[1] += 0.5;
	}
	else if (pRE->endYaw == 135) {
		pRE->toTileIndexXZ[0] += 0.5;
		pRE->toTileIndexXZ[1] -= 0.5;
	}
	else if (pRE->endYaw == 225) {
		pRE->toTileIndexXZ[0] -= 0.5;
		pRE->toTileIndexXZ[1] -= 0.5;
	}
	else if (pRE->endYaw == 315) {
		pRE->toTileIndexXZ[0] -= 0.5;
		pRE->toTileIndexXZ[1] += 0.5;
	}
	//find connected rail end
	pRE->toRailN = -1;
	float look4yaw = angleDgNorm360(pRE->endYaw + 180);
	int railsN = railsMap.size();
	for (int rN = 0; rN < railsN; rN++) {
		Rail* pR2 = (Rail*)railsMap.at(rN);
		if (pR2 == NULL)
			continue;
		if (rN == pR->nInSubjsSet)
			continue;
		if (pR2->railStatus != 0)
			continue;
		if (!v2match(pR2->tileIndexXZ, pRE->toTileIndexXZ))
			continue;
		//rail is in connected tile
		int endN = -1;
		if (look4yaw == pR2->yawInOut[0])
			endN = 0;
		else if (look4yaw == pR2->yawInOut[1])
			endN = 1;
		if (endN < 0)
			continue;
		//connected rail found
		pRE->toRailN = rN;
		pRE->toRailEndN = endN;
		RailEnd* pRE2 = &pR2->railEnd[endN];
		pRE2->toRailN = pRE->ownRailN;
		pRE2->toRailEndN = whichEnd;
		return 1;
	}
	return 0;
}
int RailMap45::checkRailCrossing(Rail* pR) {
	//check if switch or crossing
	pR->railEnd[0].isSwitch = 0;
	pR->railEnd[1].isSwitch = 0;
	pR->crossingRailN = -1;
	int railsN = railsMap.size();
	for (int rN = 0; rN < railsN; rN++) {
		Rail* pR2 = (Rail*)railsMap.at(rN);
		if (pR2 == NULL)
			continue;
		if (rN == pR->nInSubjsSet)
			continue;
		if (pR2->railStatus != 0)
			continue;
		if (!v2match(pR2->tileIndexXZ, pR->tileIndexXZ))
			continue;
		//same tile rail found
		int endN = -1;
		int endN2 = -1;
		for (int i0 = 0; i0 < 2; i0++)
			for (int i2 = 0; i2 < 2; i2++)
				if (pR->railEnd[i0].endYaw == pR2->railEnd[i2].endYaw) {
					//switch
					RailEnd* pRE = &pR->railEnd[i0];
					pRE->isSwitch = true;
					pRE->isOn = false;
					pRE->altRailN = rN;
					pRE->altRailEndN = i2;
					//alt rail
					pRE = &pR2->railEnd[i2];
					pRE->isSwitch = true;
					pRE->isOn = true;
					pRE->altRailN = pR->nInSubjsSet;
					pRE->altRailEndN = i0;
					return 1;
				}
		//crossing?
		pR->crossingRailN = rN;
		pR2->crossingRailN= pR->nInSubjsSet;
		return 1;
	}
	return 0;
}
void RailMap45::fillRailCoordsFromPercent(RailCoord* pRC, Rail* pR) {
	//find position and yaw on the rail
	pRC->xyz[1] = pR->ownCoords.pos[1];
	if (pR->railType == 0) {
		//straight
		pRC->currentYaw = pR->yawInOut[1];
		for (int i = 0; i < 3; i += 2)
			pRC->xyz[i] = pR->p0[i] * (1.0 - pRC->percentOnRail) + pR->p1[i] * pRC->percentOnRail;
	}
	else {//curve
		pRC->currentYaw = angleDgNorm180(pR->yawInOut[0] + 180 + 45.0 * pRC->percentOnRail * pR->CCW);
		float radAngleDg = pR->radiusAngle[0] + 45.0 * pRC->percentOnRail * pR->CCW;
		float radAngleRd = radAngleDg * degrees2radians;
		pRC->xyz[0] = sin(radAngleRd) * pR->curveRadius;
		pRC->xyz[2] = cos(radAngleRd) * pR->curveRadius;
		for (int i = 0; i < 3; i += 2)
			pRC->xyz[i] += pR->curveCenter[i];
	}
	if (pRC->alignedWithRail < 0)
		pRC->currentYaw = angleDgNorm180(pRC->currentYaw + 180.0);
}

int RailMap45::shiftRailCoord(RailCoord* pRCout, RailCoord* pRCin, RailMap45* pMap, float shiftZ) {
	memcpy(pRCout, pRCin, sizeof(RailCoord));
	return shiftRailCoord(pRCout, pMap, shiftZ);
}

int RailMap45::shiftRailCoord(RailCoord * pRC, RailMap45 * pMap, float shiftZ) {
	float alignedWithRail = signOf(shiftZ) * pRC->alignedWithRail; //1-moving to rail end, -1-to start
	float remaningShiftZ = abs(shiftZ);
	while (1) {
		Rail* pR = (Rail*)pMap->railsMap.at(pRC->railN);
		float railLeft = 0;
		if (alignedWithRail > 0) //moving to rail end
			railLeft = pR->railLength * (1.0 - pRC->percentOnRail);
		else //moving to rail start
			railLeft = pR->railLength * pRC->percentOnRail;
		if (railLeft >= remaningShiftZ) {
			//adjust percent on rail
			float remainingPercent = remaningShiftZ / pR->railLength;
			pRC->percentOnRail = pRC->percentOnRail + remainingPercent * alignedWithRail;
			fillRailCoordsFromPercent(pRC, pR);
			return 1;
		}
		//if here-move to next rail
		remaningShiftZ -= railLeft;
		int whichEnd0 = 0;
		if (alignedWithRail > 0) //moved to end
			whichEnd0 = 1;
		RailEnd* pRE0 = &pR->railEnd[whichEnd0];
		pRC->railN = pRE0->toRailN;
		pR = (Rail*)pMap->railsMap.at(pRC->railN);
		int whichEnd2 = pRE0->toRailEndN;
		RailEnd* pRE2 = &pR->railEnd[whichEnd2];
		if (pRE2->isSwitch && !pRE2->isOn) {
			pRC->railN = pRE2->altRailN;
			pR = (Rail*)pMap->railsMap.at(pRC->railN);
			whichEnd2 = pRE2->altRailEndN;
			RailEnd* pRE2 = &pR->railEnd[whichEnd2];
		}
		if (whichEnd0 == whichEnd2) {
			alignedWithRail *= -1.0;
			pRC->alignedWithRail *= -1.0;
		}
		if (whichEnd2 == 0)
			pRC->percentOnRail = 0.0;
		else
			pRC->percentOnRail = 1.0;
	}
	//return 1;
}
float RailMap45::offset2railCoord(RailCoord* pRC1, RailCoord* pRC2, RailMap45* pMap, float distLimit) {
	RailCoord rc1;
	memcpy(&rc1, pRC1, sizeof(RailCoord));
	float distChecked = 0;
	while (distChecked < distLimit) {
		Rail* pR = (Rail*)pMap->railsMap.at(rc1.railN);
		if (rc1.railN == pRC2->railN) {
			//same rail
			float percentD = pRC2->percentOnRail - rc1.percentOnRail;
			if (rc1.alignedWithRail<0)
				percentD = -percentD;
			if (percentD < 0)
				return 1000000;//overlap
			float dist = pR->railLength* percentD;
			distChecked += dist;

			if (rc1.alignedWithRail == pRC2->alignedWithRail)
				return 1000000; //wrong direction

			return distChecked;
		}
		//if here-move to next rail
		float railLeft = 0;
		if (rc1.alignedWithRail > 0) //moving to rail end
			railLeft = pR->railLength * (1.0 - rc1.percentOnRail);
		else //moving to rail start
			railLeft = pR->railLength * rc1.percentOnRail;
		distChecked += railLeft;
		int whichEnd0 = 0;
		if (rc1.alignedWithRail > 0) //moved to end
			whichEnd0 = 1;
		RailEnd* pRE0 = &pR->railEnd[whichEnd0];
		rc1.railN = pRE0->toRailN;
		pR = (Rail*)pMap->railsMap.at(rc1.railN);
		int whichEnd2 = pRE0->toRailEndN;
		RailEnd* pRE2 = &pR->railEnd[whichEnd2];
		if (pRE2->isSwitch && !pRE2->isOn) {
			rc1.railN = pRE2->altRailN;
			pR = (Rail*)pMap->railsMap.at(rc1.railN);
			whichEnd2 = pRE2->altRailEndN;
			RailEnd* pRE2 = &pR->railEnd[whichEnd2];
		}
		if (whichEnd0 == whichEnd2) {
			rc1.alignedWithRail *= -1.0;
		}
		if (whichEnd2 == 0)
			rc1.percentOnRail = 0.0;
		else
			rc1.percentOnRail = 1.0;
	}
	return distChecked;
}