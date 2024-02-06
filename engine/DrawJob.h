#pragma once
#include "Material.h"
#include <vector>
#include "platform.h"
#include "Gabarites.h"


struct AttribRef //attribute reference/description
{
	unsigned int glVBOid = 0; //buffer object id
	int offset = 0; //variable's offset inside of VBO's element
	int stride = 0; //Buffer's element size in bytes
};

class DrawJob
{
public:
	Material mt;
	Material mtLayer2;
	Material mtShadow;

	unsigned int glVAOid = 0; //will hold data stream attributes mapping/positions
	int pointsN = 0; //N of points to draw
	unsigned int glEBOid = 0; //Element Buffer Object (vertex indices)
	unsigned int primitiveType = GL_TRIANGLES;

	Gabarites gabarites;

	//common attributes
	AttribRef aPos;
	AttribRef aNormal;
	AttribRef aTuv;
	AttribRef aTuv2; //for normal map
	AttribRef aTangent; //for normal map
	AttribRef aBinormal; //for normal map
	int uHaveBinormal = 1; //in glTF - haven't
	AttribRef aJoints; //for animations
	AttribRef aWeights; //for animations

	//static arrays (vectors) of all loaded DrawJobs, VBO ids
	static std::vector<DrawJob*> drawJobs_default;
	static std::vector<unsigned int> buffersIds_default;
public:
	DrawJob(std::vector<DrawJob*>* pDrawJobs);
	virtual ~DrawJob(); //destructor
	static int cleanUp();
	static int newBufferId(std::vector<unsigned int>* pBuffersIds);
/*
	int execute(float* uMVP, float* uMV, float* uMM, float* uMVP4dm, float* uVectorToLight, float* uVectorToTranslucent, float* uCameraPosition,
		float line1pixSize = 1, Material* pMt = NULL) {
		return executeDrawJob(this, uMVP, uMV, uMM, uMVP4dm, uVectorToLight, uVectorToTranslucent,
			uCameraPosition, line1pixSize, pMt);
	};
	static int executeDrawJob(DrawJob* pDJ, float* uMVP, float* uMV, float* uMM, float* uMVP4dm,
		float* uVectorToLight, float* uVectorToTranslucent, float* uCameraPosition, float line1pixSize = 1, Material* pMt = NULL);
*/
	static bool lineWidthIsImportant(int primitiveType);

	static int setDesirableOffsetsForSingleVBO(DrawJob* pDJ, int* pStride, int shaderN, int VBOid);
	static int buildVAOforShader(DrawJob* pDJ, int shaderN);
	static int attachAttribute(int varLocationInShader, int attributeSizeInUnits, int unitType, AttribRef* pAttribRef);
	static int setAttribRef(AttribRef* pAR, unsigned int glVBOid, int offset, int stride);

	static int buildGabaritesFromDrawJobs(Gabarites* pGB, std::vector<DrawJob*>* pDrawJobs, int djStartN, int djTotalN);
};
