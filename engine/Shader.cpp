#include "Shader.h"
#include "platform.h"
#include "utils.h"
#include "FileLoader.h"

extern std::string filesRoot;

std::vector<Shader*> Shader::shaders;


int Shader::loadBasicShaders() {
    Shader sampleShader;
    FileLoader* pFLvertex = new FileLoader("/dt/shaders/phong_v.txt");
    FileLoader* pFLfragment = new FileLoader("/dt/shaders/phong_f.txt");
    loadShadersGroup(&sampleShader, "flat", "FLAT; COLOR | TEXTURE; NONE | OVERMASK", pFLvertex->pData, pFLfragment->pData, NULL);
    delete pFLvertex;
    delete pFLfragment;
    return 1;
}
int Shader::loadShaders(ProgressBar* pPB) {
    Shader sampleShader;
    /*
    FileLoader* pFLvertex = new FileLoader("/dt/shaders/phong_v.txt");
    FileLoader* pFLfragment = new FileLoader("/dt/shaders/phong_f.txt");
    loadShadersGroup("flat", "FLAT; COLOR | TEXTURE; NONE | OVERMASK; NONE | USE_TEXMOD", pFLvertex->pData, pFLfragment->pData, pPB);
    loadShadersGroup("phong", "PHONG; COLOR | TEXTURE; NONE | OVERMASK; NONE | USE_TEXMOD; NONE | WITHSHADOWS", pFLvertex->pData, pFLfragment->pData, pPB);
    loadShadersGroup("mirror", "PHONG;MIRROR; NONE | OVERMASK; NONE | USE_TEXMOD;  NONE | WITHSHADOWS", pFLvertex->pData, pFLfragment->pData, pPB);
    loadShadersGroup("wire", "WIRE;PHONG; COLOR | TEXTURE;  NONE | WITHSHADOWS", pFLvertex->pData, pFLfragment->pData, pPB);
    delete pFLvertex;
    delete pFLfragment;
    //Normal Maps
    pFLvertex = new FileLoader("/dt/shaders/nm_v.txt");
    pFLfragment = new FileLoader("/dt/shaders/nm_f.txt");
    loadShadersGroup("phong", "COLOR | TEXTURE; NONE | OVERMASK; NONE | USE_TEXMOD; NONE | WITHSHADOWS", pFLvertex->pData, pFLfragment->pData, pPB);
    loadShadersGroup("mirror", "MIRROR; NONE | OVERMASK; NONE | USE_TEXMOD; NONE | WITHSHADOWS", pFLvertex->pData, pFLfragment->pData, pPB);
    delete pFLvertex;
    delete pFLfragment;
    //shadows
    pFLvertex = new FileLoader("/dt/shaders/depthmap_v.txt");
    pFLfragment = new FileLoader("/dt/shaders/depthmap_f.txt");
    loadShadersGroup("depthmap", "FLAT; COLOR | TEXTURE; NONE | OVERMASK; NONE | USE_TEXMOD", pFLvertex->pData, pFLfragment->pData, pPB);
    loadShadersGroup("depthmap", "PHONG; COLOR | TEXTURE; NONE | OVERMASK; NONE | USE_NORMALMAP; NONE | USE_TEXMOD", pFLvertex->pData, pFLfragment->pData, pPB);
    delete pFLvertex;
    delete pFLfragment;
    */
    FileLoader* pFLvertex = new FileLoader("/dt/shaders/phong_v.txt");
    FileLoader* pFLfragment = new FileLoader("/dt/shaders/phong_f.txt");
    //loadShadersGroup(&sampleShader, "flat", "FLAT; COLOR | TEXTURE; NONE | OVERMASK", pFLvertex->pData, pFLfragment->pData, pPB);
    loadShadersGroup(&sampleShader, "phong", "PHONG; WITHSHADOWS; COLOR | TEXTURE; NONE | OVERMASK", pFLvertex->pData, pFLfragment->pData, pPB);
    loadShadersGroup(&sampleShader, "mirror", "PHONG;MIRROR;WITHSHADOWS; NONE | OVERMASK", pFLvertex->pData, pFLfragment->pData, pPB);
    loadShadersGroup(&sampleShader, "wire", "WIRE;PHONG;WITHSHADOWS; COLOR | TEXTURE", pFLvertex->pData, pFLfragment->pData, pPB);
    //USE_TEXMOD
    loadShadersGroup(&sampleShader, "mirror", "USE_TEXMOD;PHONG;MIRROR;WITHSHADOWS", pFLvertex->pData, pFLfragment->pData, pPB);
    delete pFLvertex;
    delete pFLfragment;
    //Normal Maps
    mylog("Now with Normal Maps:\n");
    pFLvertex = new FileLoader("/dt/shaders/nm_v.txt");
    pFLfragment = new FileLoader("/dt/shaders/nm_f.txt");
    loadShadersGroup(&sampleShader, "phong", "WITHSHADOWS; COLOR | TEXTURE; NONE | OVERMASK", pFLvertex->pData, pFLfragment->pData, pPB);
    loadShadersGroup(&sampleShader, "mirror", "MIRROR;WITHSHADOWS", pFLvertex->pData, pFLfragment->pData, pPB);
    //USE_TEXMOD
    loadShadersGroup(&sampleShader, "phong", "USE_TEXMOD;WITHSHADOWS;COLOR", pFLvertex->pData, pFLfragment->pData, pPB);
    delete pFLvertex;
    delete pFLfragment;
    //shadows
    pFLvertex = new FileLoader("/dt/shaders/depthmap_v.txt");
    pFLfragment = new FileLoader("/dt/shaders/depthmap_f.txt");
    //loadShadersGroup("depthmap", "FLAT; COLOR | TEXTURE; NONE | OVERMASK; NONE | USE_TEXMOD", pFLvertex->pData, pFLfragment->pData, pPB);
    loadShadersGroup(&sampleShader, "depthmap", "PHONG; COLOR | TEXTURE; NONE | OVERMASK; NONE | USE_NORMALMAP", pFLvertex->pData, pFLfragment->pData, pPB);
    //USE_TEXMOD
    loadShadersGroup(&sampleShader, "depthmap", "USE_TEXMOD;PHONG; TEXTURE", pFLvertex->pData, pFLfragment->pData, pPB);
    loadShadersGroup(&sampleShader, "depthmap", "USE_TEXMOD;PHONG; COLOR;USE_NORMALMAP", pFLvertex->pData, pFLfragment->pData, pPB);
    delete pFLvertex;
    delete pFLfragment;


    mylog("  %d shaders total\n", shaders.size());
    return 1;
}

int Shader::buildShaderObjectFromFiles(Shader* pS0, std::string shaderType, std::string filePathVertexS, std::string filePathFragmentS) {
    //create shader object
    Shader* pSh = NULL;
    if (pS0 == NULL)
        pSh = new Shader();
    else
        pSh = pS0->newShader();
    shaders.push_back(pSh);
    strcpy_s(pSh->shaderType32, 32, (char*)shaderType.c_str());
    pSh->GLid = linkShaderProgramFromFiles((filesRoot + filePathVertexS).c_str(), (filesRoot + filePathFragmentS).c_str());
    //common variables. If not presented, = -1;
    pSh->fillLocations();

    return (shaders.size() - 1);
}

void Shader::fillLocationsStandard(Shader* pSh) {
    //common variables. If not presented, = -1;
    //attributes
    pSh->l_aPos = glGetAttribLocation(pSh->GLid, "aPos"); //attribute position (3D coordinates)
    pSh->l_aNormal = glGetAttribLocation(pSh->GLid, "aNormal"); //attribute normal (3D vector)
    pSh->l_aTangent = glGetAttribLocation(pSh->GLid, "aTangent"); //for normal map
    pSh->l_aBinormal = glGetAttribLocation(pSh->GLid, "aBinormal"); //for normal map
    pSh->l_uHaveBinormal = glGetUniformLocation(pSh->GLid, "uHaveBinormal"); //for normal map
    pSh->l_aTuv = glGetAttribLocation(pSh->GLid, "aTuv"); //attribute TUV (texture coordinates)
    pSh->l_aTuv2 = glGetAttribLocation(pSh->GLid, "aTuv2"); //attribute TUV (texture coordinates)
    pSh->l_aJoints = glGetAttribLocation(pSh->GLid, "aJoints"); //for animations
    pSh->l_aWeights = glGetAttribLocation(pSh->GLid, "aWeights"); //for animations
    //uniforms
    pSh->l_uMVP = glGetUniformLocation(pSh->GLid, "uMVP"); // transform matrix (Model-View-Projection)
    pSh->l_uMV3x3 = glGetUniformLocation(pSh->GLid, "uMV3x3"); // Model-View matrix for normals
    pSh->l_uMM = glGetUniformLocation(pSh->GLid, "uMM"); // Model matrix for HalfVector
    pSh->l_uVectorToLight = glGetUniformLocation(pSh->GLid, "uVectorToLight"); // 
    pSh->l_uTranslucency = glGetUniformLocation(pSh->GLid, "uTranslucency"); // 
    pSh->l_uVectorToTranslucent = glGetUniformLocation(pSh->GLid, "uVectorToTranslucent"); // 
    pSh->l_uCameraPosition = glGetUniformLocation(pSh->GLid, "uCameraPosition"); // required for specular light
    //material's properties
    pSh->l_uColor = glGetUniformLocation(pSh->GLid, "uColor");
    pSh->l_uColor1 = glGetUniformLocation(pSh->GLid, "uColor1");
    pSh->l_uColor2 = glGetUniformLocation(pSh->GLid, "uColor2");
    pSh->l_uTex0 = glGetUniformLocation(pSh->GLid, "uTex0"); //texture id
    pSh->l_uTex1mask = glGetUniformLocation(pSh->GLid, "uTex1mask"); //texture id
    pSh->l_uTex2nm = glGetUniformLocation(pSh->GLid, "uTex2nm"); //texture id
    pSh->l_uTex3 = glGetUniformLocation(pSh->GLid, "uTex3"); //texture id
    //tex coords modifiers
    pSh->l_uTexMod = glGetUniformLocation(pSh->GLid, "uTexMod");

    //if (pSh->l_uTexMod>=0)
    //    int a = 0;

    //shadows
    pSh->l_uTex4dm = glGetUniformLocation(pSh->GLid, "uTex4dm");
    pSh->l_uShadow = glGetUniformLocation(pSh->GLid, "uShadow");
    pSh->l_uMVP4dm = glGetUniformLocation(pSh->GLid, "uMVP4dm");

    pSh->l_uTex1alphaChannelN = glGetUniformLocation(pSh->GLid, "uTex1alphaChannelN");
    pSh->l_uTex1alphaNegative = glGetUniformLocation(pSh->GLid, "uTex1alphaNegative");
    pSh->l_uTex0translateChannelN = glGetUniformLocation(pSh->GLid, "uTex0translateChannelN");
    pSh->l_uAlphaFactor = glGetUniformLocation(pSh->GLid, "uAlphaFactor"); // for semi-transparency
    pSh->l_uAlphaBlending = glGetUniformLocation(pSh->GLid, "uAlphaBlending"); // for semi-transparency
    pSh->l_uAmbient = glGetUniformLocation(pSh->GLid, "uAmbient"); // ambient light
    pSh->l_uSpecularIntencity = glGetUniformLocation(pSh->GLid, "uSpecularIntencity"); // 
    pSh->l_uSpecularMinDot = glGetUniformLocation(pSh->GLid, "uSpecularMinDot"); // 
    pSh->l_uBleach = glGetUniformLocation(pSh->GLid, "uBleach"); // 
    pSh->l_uShadingK = glGetUniformLocation(pSh->GLid, "uShadingK"); // 

    pSh->l_uHighLightLevel = glGetUniformLocation(pSh->GLid, "uHighLightLevel"); // 
    pSh->l_uHighLightColor = glGetUniformLocation(pSh->GLid, "uHighLightColor"); // 

    pSh->l_uSpecularPowerOf = glGetUniformLocation(pSh->GLid, "uSpecularPowerOf"); // 
    pSh->l_uDiscardNormalsOut = glGetUniformLocation(pSh->GLid, "uDiscardNormalsOut"); // 

    pSh->l_uDepthBias = glGetUniformLocation(pSh->GLid, "uDepthBias"); // for shadow map
    pSh->l_uConstZ = glGetUniformLocation(pSh->GLid, "uConstZ");
}
int Shader::cleanUp() {
    int shadersN = shaders.size();
    if (shadersN < 1)
        return -1;
    glUseProgram(0);
    for (int i = 0; i < shadersN; i++) {
        Shader* pSh = shaders.at(i);
        glDeleteProgram(pSh->GLid);
        delete pSh;
    }
    shaders.clear();
    return 1;
}

GLchar infoLog[1024];
int logLength;
int Shader::shaderErrorCheck(int shaderId, std::string ref) {
    //use after glCompileShader()
    if (checkGLerrors(ref) > 0)
        return -1;
    glGetShaderInfoLog(shaderId, 1024, &logLength, infoLog);
    if (logLength == 0)
        return 0;
    mylog("%s shader infoLog:\n%s\n", ref.c_str(), infoLog);
    return -1;
}
int Shader::programErrorCheck(int programId, std::string ref) {
    //use after glLinkProgram()
    if (checkGLerrors(ref) > 0)
        return -1;
    glGetProgramInfoLog(programId, 1024, &logLength, infoLog);
    if (logLength == 0)
        return 0;
    mylog("%s program infoLog:\n%s\n", ref.c_str(), infoLog);
    return -1;
}

int Shader::compileShaderFromFile(const char* filePath, GLenum shaderType) {
    int shaderId = glCreateShader(shaderType);
    FILE* pFile;
    fopen_s(&pFile, filePath, "rt");
    if (pFile != NULL)
    {
        // obtain file size:
        fseek(pFile, 0, SEEK_END);
        int fSize = ftell(pFile);
        rewind(pFile);
        // size obtained, create buffer
        char* shaderSource = new char[fSize + 1];
        fSize = fread(shaderSource, 1, fSize, pFile);
        shaderSource[fSize] = 0;
        fclose(pFile);
        // source code loaded, compile
        glShaderSource(shaderId, 1, (const GLchar**)&shaderSource, NULL);
        //myglErrorCheck("glShaderSource");
        glCompileShader(shaderId);
        if (shaderErrorCheck(shaderId, "glCompileShader") < 0)
            return -1;
        delete[] shaderSource;
    }
    else {
        mylog("ERROR loading %s\n", filePath);
        return -1;
    }
    return shaderId;
}
int Shader::linkShaderProgramFromFiles(const char* filePathVertexS, const char* filePathFragmentS) {
    int vertexShaderId = compileShaderFromFile(filePathVertexS, GL_VERTEX_SHADER);
    int fragmentShaderId = compileShaderFromFile(filePathFragmentS, GL_FRAGMENT_SHADER);
    int programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glLinkProgram(programId);
    if (programErrorCheck(programId, "glLinkProgram") < 0)
        return -1;
    //don't need shaders any longer - detach and delete them
    glDetachShader(programId, vertexShaderId);
    glDetachShader(programId, fragmentShaderId);
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);
    return programId;
}


int Shader::buildShaderObjectWithDefines(Shader* pS0, std::string shaderType, std::string definesString, char* sourceVertex, char* sourceFragment) {
    //create shader object
    Shader* pSh = NULL;
    if (pS0 == NULL)
        pSh = new Shader();
    else
        pSh = pS0->newShader();
    shaders.push_back(pSh);
    strcpy_s(pSh->shaderType32, 32, (char*)shaderType.c_str());

    pSh->GLid = linkShaderProgramWithDefines(definesString, sourceVertex, sourceFragment);
    //common variables. If not presented, = -1;
    pSh->fillLocations();

    return (shaders.size() - 1);
}
int Shader::linkShaderProgramWithDefines(std::string definesString00, char* sourceVertex, char* sourceFragment) {
    //build extended definesString
    bool bUSE_NORMALS = false;
    bool bUSE_TEX0 = false;
    bool bUSE_TUV0 = false;
    if (definesString00.find(" PHONG\n") != std::string::npos)
        bUSE_NORMALS = true;
    if (definesString00.find(" TEXTURE\n") != std::string::npos) {
        bUSE_TEX0 = true;
        bUSE_TUV0 = true;
    }
    if (definesString00.find(" MIRROR\n") != std::string::npos) {
        bUSE_NORMALS = true;
        bUSE_TEX0 = true;
    }
    if (definesString00.find(" OVERMASK\n") != std::string::npos) {
        bUSE_TUV0 = true;
    }
    std::string definesString;
    definesString.assign("#version 320 es\n");
    definesString.append(definesString00);
    if (bUSE_NORMALS)
        definesString.append("#define USE_NORMALS\n");
    if (bUSE_TEX0)
        definesString.append("#define USE_TEX0\n");
    if (bUSE_TUV0)
        definesString.append("#define USE_TUV0\n");

    //mylog("--------------\n%s\n", definesString.c_str());

    int vertexShaderId = compileShaderWithDefines(definesString, sourceVertex, GL_VERTEX_SHADER);
    int fragmentShaderId = compileShaderWithDefines(definesString, sourceFragment, GL_FRAGMENT_SHADER);

    int programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glLinkProgram(programId);
    if (programErrorCheck(programId, "glLinkProgram") < 0)
        return -1;
    //don't need shaders any longer - detach and delete them
    glDetachShader(programId, vertexShaderId);
    glDetachShader(programId, fragmentShaderId);
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);
    //mylog("linking program\n%s\n", definesString.c_str());
    return programId;
}
int Shader::compileShaderWithDefines(std::string definesString, char* shaderSource, GLenum shaderType) {
    int shaderId = glCreateShader(shaderType);
    if (definesString.empty())
        glShaderSource(shaderId, 1, (const GLchar**)&shaderSource, NULL);
    else { //2 strings
        const char* sourceStrings[2];
        sourceStrings[0] = definesString.c_str();
        sourceStrings[1] = shaderSource;
        // source code loaded, compile
        glShaderSource(shaderId, 2, (const GLchar**)sourceStrings, NULL);
    }
    //myglErrorCheck("glShaderSource");
    glCompileShader(shaderId);
    if (shaderErrorCheck(shaderId, "glCompileShader") < 0) {
        mylog("ERROR in compileShader,\n%s\n%s\n", definesString.c_str(), shaderSource);
        return -1;
    }
    return shaderId;
}


int Shader::loadShadersGroup(Shader* pS0, std::string shaderType, std::string optionsString,
    char* sourceVertex, char* sourceFragment, ProgressBar* pPB) {

    struct TermsGroup {
        std::vector<std::string>* pTermsChoice;
        int totalN = 0;
        int currentN = 0;
    };
    std::vector<TermsGroup*> terms;
    std::vector<std::string>* pTermAllGroups = splitString(optionsString, ";");
    int groupsN = pTermAllGroups->size();
    for (int groupN = 0; groupN < groupsN; groupN++) {
        TermsGroup* pTermsGroup = new TermsGroup();
        terms.push_back(pTermsGroup);
        pTermsGroup->pTermsChoice = splitString(pTermAllGroups->at(groupN), "|");
        pTermsGroup->totalN = pTermsGroup->pTermsChoice->size();
    }
    delete pTermAllGroups;
    while (1) {
        std::string definesString = "";
        for (int groupN = 0; groupN < groupsN; groupN++) {
            TermsGroup* pTermsGroup = terms.at(groupN);
            std::string term = pTermsGroup->pTermsChoice->at(pTermsGroup->currentN);
            if (term.compare("NONE") != 0) {
                definesString.append("#define ");
                definesString.append(term);
                definesString.append("\n");
            }
        }
        int shaderObjN = buildShaderObjectWithDefines(pS0, shaderType, definesString, sourceVertex, sourceFragment);
        ProgressBar::nextStep(pPB);

        //go to next terms combo
        bool noMoreOptions = false;
        for (int groupN = groupsN - 1; groupN >= 0; groupN--) {
            TermsGroup* pTermsGroup = terms.at(groupN);
            if (pTermsGroup->currentN < pTermsGroup->totalN - 1) {
                pTermsGroup->currentN++;
                break;
            }
            else { // the level exhausted
                pTermsGroup->currentN = 0;
                //proceed to upper level
                if (groupN == 0) {
                    noMoreOptions = true;
                    break;
                }
            }
        }
        if (noMoreOptions)
            break;
    }
    //clear memory
    for (int groupN = 0; groupN < groupsN; groupN++) {
        TermsGroup* pTermsGroup = terms.at(groupN);
        pTermsGroup->pTermsChoice->clear();
        delete pTermsGroup->pTermsChoice;
        delete pTermsGroup;
    }
    terms.clear();

    mylog("Shader::loadShadersGroup('%s', '%s', ..);\n", shaderType.c_str(), optionsString.c_str());
    return 1;
}

int Shader::pickShaderNumber(Material* pMT) {
    int shadersN = Shader::shaders.size();
    if (strstr(pMT->shaderType32, "flat") != NULL)
        pMT->takesShadow = 0;
    //else if (strstr(pMT->shaderType, "depthmap") != NULL)
    //    pMT->takesShadow = 0;
    for (int i = 0; i < shadersN; i++) {
        Shader* pSH = Shader::shaders.at(i);
        if (strcmp(pMT->shaderType32, pSH->shaderType32) != 0)
            continue;
        if (pMT->uColor.isZero() != (pSH->l_uColor < 0))
            continue;
        if ((pMT->uTex0 < 0) != (pSH->l_uTex0 < 0))
            continue;
        if ((pMT->uTex1mask < 0) != (pSH->l_uTex1mask < 0))
            continue;
        if ((pMT->uTex2nm < 0) != (pSH->l_uTex2nm < 0))
            continue;
        if ((pMT->takesShadow < 1) != (pSH->l_uTex4dm < 0))
            continue;
        if ((pMT->uTexMod[0] < 0) != (pSH->l_uTexMod < 0))
            continue;
        if ((strstr(pMT->shaderType32, "flat") != NULL || strstr(pMT->shaderType32, "z-buffer") != NULL) != (pSH->l_aNormal < 0))
            continue;
        pMT->shaderN = i;
        return i;
    }
    pMT->shaderN = -1;
    mylog("ERROR in Shader::pickShaderNumber:\n");
    mylog("Can't find '%s' shader for uColor=%08x uTex0=%d uTex1mask=%d uTex2nm=%d takesShadow=%d uTexMod[0]=%d\n",
        pMT->shaderType32, pMT->uColor.getUint32(), pMT->uTex0, pMT->uTex1mask, pMT->uTex2nm, pMT->takesShadow, (int)pMT->uTexMod[0]);
    return -1;
}
int Shader::assignShader(Material* pMT, std::string shaderType) {
    Shader::setShaderType(pMT, shaderType);
    return Shader::pickShaderNumber(pMT);
}



