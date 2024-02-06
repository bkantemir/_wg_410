#pragma once
#include "Shader.h"

class GLTFskinShader : public Shader
{
public:
	int l_uboBones; //binding = 0
	//int l_uBoneTransforms;
	int l_uVPcamera;
	int l_uLookAt;
	int l_uVPshadow;
public:
	virtual GLTFskinShader* newShader() { return new GLTFskinShader(); };
	static int loadGLTFshaders(ProgressBar* pPB);

	virtual void fillLocations() { fillLocationsGLTFskin(this); };
	static 	int fillLocationsGLTFskin(GLTFskinShader* pSh);

};
