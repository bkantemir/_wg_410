#include "TheTable.h"
#include "ModelBuilder.h"
#include "Texture.h"

extern std::string filesRoot;


TheTable::~TheTable() {
	cleanUp();
}



void TheTable::cleanUp() {
	for (int i = tableParts.size() - 1; i >= 0; i--)
		delete tableParts.at(i);
	tableParts.clear();

	for (int i = table_drawJobs.size() - 1; i >= 0; i--)
		delete table_drawJobs.at(i);
	table_drawJobs.clear();

	for (int i = table_buffersIds.size() - 1; i >= 0; i--) {
		unsigned int id = table_buffersIds.at(i);
		glDeleteBuffers(1, &id);
	}
	table_buffersIds.clear();
}
int TheTable::initTable(float tileSizeXZ, float tileSizeUp, float tileSizeDown, int tilesNx, int tilesNz) {
	tileSize = tileSizeXZ;
	tableTiles[0] = tilesNx;
	tableTiles[1] = tilesNz;
	groundLevel = tileSizeDown;

	suggestedStageSize = tileSize * 2;

	//define worldBox 
	worldBox.bbMin[1] = 0;
	worldBox.bbMax[1] = tileSizeUp + tileSizeDown;
	worldBox.bbMax[0] = tileSizeXZ * 0.5 * tilesNx;
	worldBox.bbMax[2] = tileSizeXZ * 0.5 * tilesNz;
	worldBox.bbMin[0] = -worldBox.bbMax[0];
	worldBox.bbMin[2] = -worldBox.bbMax[2];
	Gabarites::adjustMidRad(&worldBox);

	ModelBuilder* pMB = new ModelBuilder();
	Material mt;
	strcpy_s(mt.shaderType32, 32, "phong");
	strcpy_s(mt.materialName32, 32, "table");
	mt.dropsShadow = 0;
	mt.uSpecularIntencity = 0;
	//mt.uColor.setRGBA(255, 0, 0);
	//mt.uTex0 = Texture::loadTexture(filesRoot+"/dt/mt/wood01.jpg");
	VirtualShape vs;
	v3set(vs.whl, tileSize, groundLevel, tileSize);
	float y0 = groundLevel / 2;
	//table top
	pMB->lockGroup(pMB);
	for (int zN = 0; zN < tableTiles[1]; zN++)
		for (int xN = 0; xN < tableTiles[0]; xN++) {
			SceneSubj* pSS = new SceneSubj();
			tableParts.push_back(pSS);
			pSS->pDrawJobs = &table_drawJobs;
			pSS->pSubjsSet = &tableParts;
			pSS->nInSubjsSet = tableParts.size() - 1;
			pSS->buildModelMatrix();
			pSS->gabaritesWorld.chordType = -1;
			pSS->gabaritesOnScreen.chordType = -1;
			pMB->useSubjN(pMB, pSS->nInSubjsSet);
			/*
			if ((zN + xN) % 2 == 0)
				mt.uColor.setRGBA(255, 0, 0);
			else
				mt.uColor.setRGBA(0, 0, 255);
			//mt.uColor.setRGBA(0.5f, 0.5f, 0.5f, 1.0f);
			*/
			if ((zN + xN) % 2 == 0)
				mt.uColor.setRGBA(0.4f, 0.4f, 0.4f);
			else
				mt.uColor.setRGBA(0.3f, 0.3f, 0.3f);

			pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);

			float z0 = tileSize * zN - worldBox.bbRad[2] + tileSize / 2;
			float x0 = tileSize * xN - worldBox.bbRad[0] + tileSize / 2;

			pMB->lockGroup(pMB);
			pMB->buildBoxFace(pMB, "top", &vs);
			pMB->moveGroupDg(pMB, 0, 0, 0, x0, y0, z0);
			pMB->releaseGroup(pMB);
		}
	/*
	TexCoords tc;
	tc.set_GL(&tc, 0,0,10,10,"");
	pMB->groupApplyTexture(pMB, "top", &tc);//, TexCoords * pTC = NULL, TexCoords * pTC2nm = NULL);
	*/
	pMB->releaseGroup(pMB);
	pMB->buildDrawJobs(pMB, &tableParts, &table_drawJobs, &table_buffersIds);
	delete pMB;
	return 1;
}
void TheTable::placeAt(float* pos, float x, float y, float z) {
	pos[0] = x * tileSize - worldBox.bbRad[0];
	pos[1] = y;
	pos[2] = z * tileSize - worldBox.bbRad[2];
}