#pragma once
#include "platform.h"
#include <string>
#include <vector>
#include "ProgressBar.h"

class Shader
{
public:
    //Shader program's individual descriptor:
    unsigned int GLid = -1; // GL shader id
    char shaderType32[32] = "";
    //common variables, "l_" for "location"
    //attributes
    int l_aPos; //attribute position (3D coordinates)   layout (location = 0)
    int l_aTuv; //attribute TUV (texture coordinates)   layout (location = 1)
    int l_aTuv2; //attribute TUV (texture coordinates for normal map)   layout (location = 2)
    int l_aNormal; //attribute normal (3D vector)       layout (location = 3)
    int l_aTangent; //for normal map                    layout (location = 4)
    int l_aBinormal; //for normal map                    layout (location = 5)
    int l_uHaveBinormal;
    int l_aJoints; //for animations                    layout (location = 6)
    int l_aWeights; //for animations                    layout (location = 7)
    //uniforms
    int l_uMVP; // transform matrix (Model-View-Projection)
    int l_uMV3x3; // Model-View matrix for normals
    int l_uMM; // Model matrix for HalfVector
    int l_uVectorToLight; //required for light
    int l_uTranslucency; //translucent light
    int l_uVectorToTranslucent; //translucent light
    int l_uCameraPosition; //required for specular light
    //material's properties
    int l_uColor;
    int l_uColor1; //for 2-tone
    int l_uColor2; //for 2-tone
    int l_uTex0; //texture id
    int l_uTex1mask; //transparency map
    int l_uTex2nm; //normal map
    int l_uTex3; //texture id
    //tex coords modifier
    int l_uTexMod;
    //shadow/depth map
    int l_uTex4dm; //texture id
    int l_uShadow;
    int l_uMVP4dm; //mvp for light space

    int l_uTex1alphaChannelN; //alpha channel for mask
    int l_uTex1alphaNegative; //alpha channel negative
    int l_uTex0translateChannelN; //translate tex0 to tex3 by channelN.
    int l_uAlphaFactor; //for semi-transparency
    int l_uAlphaBlending; //for semi-transparency
    //light:
    int l_uAmbient; //ambient light
    //specular light parameters
    int l_uSpecularIntencity;
    int l_uSpecularMinDot;
    int l_uSpecularPowerOf;
    int l_uBleach;
    int l_uShadingK;
    int l_uHighLightLevel;
    int l_uHighLightColor;

    int l_uDiscardNormalsOut;

    int l_uDepthBias; //for depthMap
    float l_uConstZ;
    //end of descriptor

    //static array (vector) of all loaded shaders
    static std::vector<Shader*> shaders;

public:
    virtual Shader* newShader() { return new Shader(); };
    static int loadBasicShaders();
    static int loadShaders(ProgressBar* pPB);
    static int cleanUp();
    static unsigned int getGLid(int shN) { return shaders.at(shN)->GLid; };
    static int shaderErrorCheck(int shaderId, std::string ref);
    static int programErrorCheck(int programId, std::string ref);

    virtual void fillLocations() { fillLocationsStandard(this); };
    static void fillLocationsStandard(Shader* pSh);

    static int buildShaderObjectFromFiles(Shader* pS0, std::string shaderType, std::string filePathVertexS, std::string filePathFragmentS);
    static int linkShaderProgramFromFiles(const char* filePathVertexS, const char* filePathFragmentS);
    static int compileShaderFromFile(const char* filePath, GLenum shaderType);

    static int buildShaderObjectWithDefines(Shader* pS0, std::string shaderType, std::string definesString, char* sourceVertex, char* sourceFragment);
    static int linkShaderProgramWithDefines(std::string definesString, char* sourceVertex, char* sourceFragment);
    static int compileShaderWithDefines(std::string definesString, char* shaderSource, GLenum shaderType);

    static int loadShadersGroup(Shader* pS0, std::string shaderType, std::string optionsString, char* sourceVertex, char* sourceFragment,
        ProgressBar* pPB);

    static void setShaderType(Material* pMT, std::string needType) { strcpy_s(pMT->shaderType32, 32, (char*)needType.c_str()); };
    static int pickShaderNumber(Material* pMT);
    static int assignShader(Material* pMT, std::string needType);

};
