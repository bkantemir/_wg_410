#include "GLTFskinShader.h"
#include "FileLoader.h"


int GLTFskinShader::loadGLTFshaders(ProgressBar* pPB) {
    GLTFskinShader sampleShader;
    FileLoader* pFLvertex = new FileLoader("/dt/shaders/phong_v.txt");
    FileLoader* pFLfragment = new FileLoader("/dt/shaders/phong_f.txt");
    loadShadersGroup(&sampleShader, "skin phong", "GLTF_SKIN; PHONG; WITHSHADOWS; COLOR | TEXTURE", pFLvertex->pData, pFLfragment->pData, pPB);
    loadShadersGroup(&sampleShader, "skin mirror", "GLTF_SKIN;MIRROR;WITHSHADOWS", pFLvertex->pData, pFLfragment->pData, pPB);
    //USE_TEXMOD
    loadShadersGroup(&sampleShader, "skin mirror", "GLTF_SKIN;MIRROR;USE_TEXMOD;WITHSHADOWS", pFLvertex->pData, pFLfragment->pData, pPB);
    delete pFLvertex;
    delete pFLfragment;

    //Normal Maps
    pFLvertex = new FileLoader("/dt/shaders/nm_v.txt");
    pFLfragment = new FileLoader("/dt/shaders/nm_f.txt");
    loadShadersGroup(&sampleShader, "skin phong", "GLTF_SKIN;PHONG;WITHSHADOWS; COLOR | TEXTURE", pFLvertex->pData, pFLfragment->pData, pPB);
    loadShadersGroup(&sampleShader, "skin mirror", "GLTF_SKIN;MIRROR;WITHSHADOWS", pFLvertex->pData, pFLfragment->pData, pPB);
    //USE_TEXMOD
    loadShadersGroup(&sampleShader, "skin phong", "GLTF_SKIN;PHONG;USE_TEXMOD;WITHSHADOWS;COLOR", pFLvertex->pData, pFLfragment->pData, pPB);
    loadShadersGroup(&sampleShader, "skin mirror", "GLTF_SKIN;MIRROR;USE_TEXMOD;WITHSHADOWS", pFLvertex->pData, pFLfragment->pData, pPB);
    delete pFLvertex;
    delete pFLfragment;

    //shadows
    pFLvertex = new FileLoader("/dt/shaders/depthmap_v.txt");
    pFLfragment = new FileLoader("/dt/shaders/depthmap_f.txt");
    //loadShadersGroup("depthmap", "FLAT; COLOR | TEXTURE; NONE | OVERMASK; NONE | USE_TEXMOD", pFLvertex->pData, pFLfragment->pData, pPB);
    loadShadersGroup(&sampleShader, "skin depthmap", "GLTF_SKIN; PHONG; COLOR | TEXTURE; NONE | USE_NORMALMAP", pFLvertex->pData, pFLfragment->pData, pPB);
    delete pFLvertex;
    delete pFLfragment;


    mylog("  %d shaders total\n", shaders.size());
    return 1;
}

int GLTFskinShader::fillLocationsGLTFskin(GLTFskinShader* pSh) {
    pSh->l_uboBones = glGetUniformBlockIndex(pSh->GLid, "uboBones");
    //pSh->l_uBoneTransforms = glGetUniformLocation(pSh->GLid, "uBoneTransforms");
    pSh->l_uVPcamera = glGetUniformLocation(pSh->GLid, "uVPcamera");
    pSh->l_uLookAt = glGetUniformLocation(pSh->GLid, "uLookAt");
    pSh->l_uVPshadow = glGetUniformLocation(pSh->GLid, "uVPshadow");
    Shader::fillLocationsStandard(pSh);
    return 1;
}

