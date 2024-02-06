#pragma once
/*
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
*/
#include "gltf/tiny_gltf.h"
#include "DrawJob.h"
#include "SceneSubj.h"
#include "gltf/GLTFnode.h"
#include "ProgressBar.h"

struct MeshDJs
{
	int djStartN = 0; //primitive start #
	int djTotalN = 0; //total primitives involved
};

class GLTFparser
{
public:
	tinygltf::Model gltf_model;

	std::vector<MeshDJs*> meshDJs; //djStartN/djTotalN

	std::vector<SceneSubj*>* pNodes;
	std::vector<DrawJob*>* pDrawJobs;
	std::vector<unsigned int>* pBuffersIds;
	std::vector<int> texturesNs;


public:
	virtual ~GLTFparser(); //destructor
	static int loadModel(std::vector<SceneSubj*>* pNodes0, std::vector<DrawJob*>* pDrawJobs, std::vector<unsigned int>* pBuffersIds,
		std::string sourceFile, std::string subjClass, ProgressBar* pPB);
	static int loadAnimations(std::string sourceFile, ProgressBar* pPB, int useSkeletonMapN = -1);
private:
	static int readGLB(GLTFparser* pParser, std::string fullPath);
	static int parseGLTFmodel(GLTFparser* pParser, ProgressBar* pPBar);
	static int glTF_attachAttribute(GLTFparser* pParser, tinygltf::Primitive* pPrimitive, std::string attribName,
		int varLocationInShader, int attributeSizeInUnits, int unitType);
	static GLTFnode* newGLTFnode(std::vector<SceneSubj*>* pNodes, std::vector<DrawJob*>* pDrawJobs, std::string className,
		bool reversedOrder, int rootNodeN);
	static int setMetallicRoughness(DrawJob* pDJ, float metallicFactor, float roughnessFactor);
	static void buildBoundingBoxFromVerts(Gabarites* pGB, GLTFparser* pParser, tinygltf::Primitive* pPrimitive);
	static int bufferIdAtGPU(GLTFparser* pParser, int bufferViewN);
	static int setSkinnedShader(Material* pMt);

	static int parseGLTFskeletonMap(GLTFparser* pParser, ProgressBar* pPBar);
	static int parseGLTFanimations(GLTFparser* pParser, ProgressBar* pPBar, int useSkeletonMapN = -1);

};


