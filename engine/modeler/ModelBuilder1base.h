#pragma once
#include <string>
#include <vector>
#include "Vertex01.h"
#include "Triangle01.h"
#include "VirtualShape.h"
#include "Group01.h"
#include "Material.h"
#include "MaterialAdjust.h"
#include "SceneSubj.h"
#include <map>


class ModelBuilder1base
{
public:
	std::vector<Vertex01*> vertices;
	std::vector<Triangle01*> triangles;
	std::vector<int> subjNumbersList;
	int usingSubjN = -1;
	std::vector<int> usingSubjsStack;

	std::vector<Group01*> groupsStack;
	Group01* pCurrentGroup = NULL;
	Group01* pLastClosedGroup = NULL;
	
	std::vector<VirtualShape*> vShapesStack;
	VirtualShape* pCurrentVShape = NULL;

	std::vector<Material*> materialsList;
	int usingMaterialN = -1;
	std::vector<int> materialsStack;

	std::vector<MaterialAdjust*> materialAdjustsList0;
	std::vector<MyColor*> colorsList00;

	std::map<std::string, float> floatsHashMap;

public:
	virtual ~ModelBuilder1base();
	static int useSubjN(ModelBuilder1base* pMB, int subjN);
	static int getMaterialN(ModelBuilder1base* pMB, Material* pMT);
	static void lockGroup(ModelBuilder1base* pMB);
	static void releaseGroup(ModelBuilder1base* pMB);
	static int addVertex(ModelBuilder1base* pMB, float kx = 0, float ky = 0, float kz = 0, float nx=0, float ny=0, float nz=1);
	static int add2triangles(ModelBuilder1base* pMB, int nNW, int nNE, int nSW, int nSE, int n);
	static int addTriangle(ModelBuilder1base* pMB, int n0, int n1, int n2);
	static int buildDrawJobs(ModelBuilder1base* pMB, std::vector<SceneSubj*>* pSceneSubjs, 
		std::vector<DrawJob*>* pDrawJobs, std::vector<unsigned int>* pBuffersIds);
	static int rearrangeArraysForDrawJob(std::vector<Vertex01*>* pAllVertices, std::vector<Vertex01*>* pUseVertices, std::vector<Triangle01*>* pUseTriangles);
	static int buildSingleDrawJob(std::vector<DrawJob*>* pDrawJobs, std::vector<unsigned int>* pBuffersIds, 
		Material* pMT, std::vector<Vertex01*>* pVertices, std::vector<Triangle01*>* pTriangles);
	static int calculateTangentSpace(std::vector<Vertex01*>* pUseVertices, std::vector<Triangle01*>* pUseTriangles);
	static int finalizeLine(std::vector<Vertex01*>* pVerts, int lineStartsAt = 0, int lineEndsAt = 0);
	static int finalizeShorts(std::vector<Vertex01*>* pVerts);
	static int optimizeMesh(std::vector<Vertex01*>* pVertices, std::vector<Triangle01*>* pTriangles);
	static int debug_showNormals(ModelBuilder1base* pMB);
	static int buildBoundingBoxFromVerts(Gabarites* pGB, std::vector<Vertex01*>* pVerts);
	static int moveGroupDg(ModelBuilder1base* pMB, float aX, float aY, float aZ, float kX = 0, float kY = 0, float kZ = 0, Group01* pGroup = NULL);
	static int rotateGroupDg(ModelBuilder1base* pMB, float aX, float aY, float aZ, Group01* pGroup = NULL);
	static int shiftGroup(ModelBuilder1base* pMB, float x, float y, float z, Group01* pGroup=NULL);
	static int scaleGroup(ModelBuilder1base* pMB, float x, float y, float z, Group01* pGroup = NULL);
};


