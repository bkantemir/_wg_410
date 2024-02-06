#pragma once
#include "SceneSubj.h"
#include "GLTFskin.h"

class GLTFnode : public SceneSubj
{
public:
    int skinN = -1;

    int uboBonesId = -1; //for pAnimRoot only
    uint32_t uboBonesUpdateFrameN = -1; //for pAnimRoot only

    int gltfAnimN = -1;
    int anim2skinMapN = -1;
    float anim2nodeMapScale = 1;
    float gltfAnimTimePass = 0; //in seconds
    int animCycle = 0; //0-no cycle, stop; 1 - straight cycle; 2 - back-and-forth
public:
    virtual ~GLTFnode();
    virtual int processSubj() { return processGLTFnode(this); };
    static int processGLTFnode(GLTFnode* pNode);
    static int startAnimation(GLTFnode* pNode, int animN, int cycleType = 1, int anim2skinMapN = -1);
    static int applyAnimation(GLTFnode* pNode);
    static int slerpQuat(float* curQuat, float* previousQuat, float* nextQuat, float interpolationValue);
    static int buildJointsMatrices(GLTFskin* pSkin, GLTFnode* pNode);
    virtual int render(Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter);
    static int renderGLTFskinned(GLTFnode* pGS, Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter);
    static int executeDJskinned(DrawJob* pDJ, int uboBonesId, mat4x4 uVPmain, mat4x4 uLookAt, mat4x4 uVPshadow,
        float* uVectorToLight, float* uVectorToTranslucent, float* uCameraPosition, float line1pixSize, Material* pMt);
    virtual int renderDepthMap(Camera* pCam);
    static int renderDepthMapSkinned(GLTFnode* pGS, Camera* pCam);

};


