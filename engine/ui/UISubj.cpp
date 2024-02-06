#include "UISubj.h"
#include "TheApp.h"
#include "Shader.h"
#include "Texture.h"

int UISubj::djNtex = -1;
int UISubj::djNclr = -1;
int UISubj::djNdepthmap = -1;
mat4x4 UISubj::mOrthoViewProjection;

std::vector<UISubj*> UISubj::uiSubjs_default;
float UISubj::screenSize[2];
float UISubj::screenAspectRatio;

extern TheApp theApp;
extern float degrees2radians;

UISubj::UISubj(std::vector<UISubj*>* pSubjs) {
    if (pSubjs == NULL)
        pSubjs = &uiSubjs_default;
    UISubj* pS = this;
    pS->pSubjs = pSubjs;
    pS->nInSubjs = pSubjs->size();
    pSubjs->push_back(pS);
}
UISubj::~UISubj() {
    //UISubj* pS = this;
    //pS->pSubjs->erase(pS->pSubjs->begin() + pS->nInSubjs);
}

int UISubj::init() {
    std::vector<DrawJob*>* pDrawJobs = &DrawJob::drawJobs_default;
    std::vector<unsigned int>* pBuffersIds = &DrawJob::buffersIds_default;

    AttribRef aPos;
    AttribRef aTuv;

    //djTexture
    struct
    {
        float x, y, z, tu, tv;
    } vertsTex[4] =
    {
        { -0.5f,  0.5f, 0.f, 0.f, 0.f }, //top-left
        { -0.5f, -0.5f, 0.f, 0.f, 1.f }, //bottom-left
        {  0.5f,  0.5f, 0.f, 1.f, 0.f }, //top-right
        {  0.5f, -0.5f, 0.f, 1.f, 1.f }  //bottom-right
    };
    djNtex = pDrawJobs->size();
    DrawJob* pDJ = new DrawJob(pDrawJobs);
    pDJ->pointsN = 4;
    int vertex_buffer = DrawJob::newBufferId(pBuffersIds);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertsTex), vertsTex, GL_STATIC_DRAW);
    //VBO is ready
    pDJ->setAttribRef(&pDJ->aPos, vertex_buffer, 0, sizeof(float) * 5);
    pDJ->setAttribRef(&pDJ->aTuv, vertex_buffer, sizeof(float) * 3, sizeof(float) * 5);

    Material* pMT = &pDJ->mt;
    pMT->primitiveType = GL_TRIANGLE_STRIP;
    pMT->uTex0 = 22;
    pMT->dropsShadow = 0;
    pMT->zBuffer = 0;
    pMT->zBufferUpdate = 0;
    Shader::assignShader(pMT, "flat");
    //pMT->shaderN = Shader::progN_flat_uTex;

    DrawJob::buildVAOforShader(pDJ, pMT->shaderN);

    //djClr
    struct
    {
        float x, y, z;
    } vertsClr[4] =
    {
        { -0.5f,  0.5f, 0.f, }, //top-left
        { -0.5f, -0.5f, 0.f, }, //bottom-left
        {  0.5f,  0.5f, 0.f, }, //top-right
        {  0.5f, -0.5f, 0.f, }  //bottom-right
    };
    djNclr = pDrawJobs->size();
    pDJ = new DrawJob(pDrawJobs);
    pDJ->pointsN = 4;
    //unsigned int vertex_buffer;
    vertex_buffer = DrawJob::newBufferId(pBuffersIds);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertsClr), vertsClr, GL_STATIC_DRAW);
    //VBO is ready
    DrawJob::setAttribRef(&pDJ->aPos, vertex_buffer, 0, sizeof(float) * 3);

    pMT = &pDJ->mt;
    pMT->primitiveType = GL_TRIANGLE_STRIP;
    pMT->uColor.setRGBA(255, 0, 0, 255);
    pMT->dropsShadow = 0;
    pMT->zBuffer = 0;
    pMT->zBufferUpdate = 0;
    Shader::assignShader(pMT, "flat");
    //pMT->shaderN = Shader::progN_flat_uClr;

    DrawJob::buildVAOforShader(pDJ, pMT->shaderN);


    //djDepthmap

    Shader::buildShaderObjectFromFiles(NULL, "z-buffer", "/dt/shaders/flat_utex_v.txt", "/dt/shaders/flat_depthmap_f.txt");

    struct
    {
        float x, y, z, tu, tv;
    } vertsDM[4] =
    {   //image is upside down
        { -0.5f,  0.5f, 0.f, 0.f, 1.f }, //top-left
        { -0.5f, -0.5f, 0.f, 0.f, 0.f }, //bottom-left
        {  0.5f,  0.5f, 0.f, 1.f, 1.f }, //top-right
        {  0.5f, -0.5f, 0.f, 1.f, 0.f }  //bottom-right
    };
    djNdepthmap = pDrawJobs->size();
    pDJ = new DrawJob(pDrawJobs);
    pDJ->pointsN = 4;
    vertex_buffer = DrawJob::newBufferId(pBuffersIds);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertsTex), vertsDM, GL_STATIC_DRAW);
    //VBO is ready
    pDJ->setAttribRef(&pDJ->aPos, vertex_buffer, 0, sizeof(float) * 5);
    pDJ->setAttribRef(&pDJ->aTuv, vertex_buffer, sizeof(float) * 3, sizeof(float) * 5);

    pMT = &pDJ->mt;
    pMT->primitiveType = GL_TRIANGLE_STRIP;
    pMT->uTex0 = 22;
    pMT->takesShadow = 0;
    pMT->dropsShadow = 0;
    pMT->zBuffer = 0;
    pMT->zBufferUpdate = 0;
    Shader::assignShader(pMT, "z-buffer");
    //pMT->shaderN = Shader::progN_flat_depthmap;

    DrawJob::buildVAOforShader(pDJ, pMT->shaderN);


    return 1;
}

void UISubj::buildModelMatrix(UISubj* pUI) {
    if (pUI->transformMatrixIsReady)
        return;
    memcpy(&pUI->absCoords, &pUI->ownCoords, sizeof(Coords2D));
    if (strstr(pUI->countFrom, "left") != NULL)
        pUI->absCoords.pos[0] = pUI->ownCoords.pos[0] - screenSize[0] / 2 + pUI->scale[0] / 2;
    else if (strstr(pUI->countFrom, "right") != NULL)
        pUI->absCoords.pos[0] = -(pUI->ownCoords.pos[0] - screenSize[0] / 2 + pUI->scale[0] / 2);

    if (strstr(pUI->countFrom, "top") != NULL)
        pUI->absCoords.pos[1] = pUI->ownCoords.pos[1] - screenSize[1] / 2 + pUI->scale[1] / 2;
    else if (strstr(pUI->countFrom, "bottom") != NULL)
        pUI->absCoords.pos[1] = -(pUI->ownCoords.pos[1] - screenSize[1] / 2 + pUI->scale[1] / 2);

    //screen y - from bottom up
    pUI->absCoords.pos[1] = -pUI->absCoords.pos[1];

    //count parent here
    //...
    mat4x4 m, mr;
    mat4x4_translate(m, pUI->absCoords.pos[0], pUI->absCoords.pos[1], 0);
    mat4x4_rotate_Z(mr, m, pUI->absCoords.aZdg * degrees2radians);
    mat4x4_scale_aniso(m, mr, pUI->scale[0], pUI->scale[1], 1);



    mat4x4_mul(pUI->transformMatrix, mOrthoViewProjection, m);
    pUI->transformMatrixIsReady = true;
}
int UISubj::renderStandard(UISubj* pUI) {
    std::vector<DrawJob*>* pDrawJobs = &DrawJob::drawJobs_default;

    buildModelMatrix(pUI);


    //render subject
    for (int i = 0; i < pUI->djTotalN; i++) {
        DrawJob* pDJ = pDrawJobs->at(pUI->djStartN + i);
        Material* pMt = &pDJ->mt;
        if (i == 0)
            pMt = &pUI->mt0;

        executeDJbasic(pDJ, (float*)pUI->transformMatrix, pMt);
    }
    return 1;
}
int UISubj::executeDJbasic(DrawJob* pDJ, float* uMVP, Material* pMt) {

    if (pMt == NULL)
        pMt = &(pDJ->mt);

    int shaderN = pMt->shaderN;
    if (shaderN < 0)
        return 0;

    glLineWidth(1);

    glBindVertexArray(pDJ->glVAOid);

    Shader* pShader = Shader::shaders.at(shaderN);
    glUseProgram(pShader->GLid);

    //input uniforms
    glUniformMatrix4fv(pShader->l_uMVP, 1, GL_FALSE, (const GLfloat*)uMVP);

    //attach textures
    if (pShader->l_uTex0 >= 0) {
        int textureId = Texture::getGLid(pMt->uTex0);
        //pass textureId to shader program
        glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
        glBindTexture(GL_TEXTURE_2D, textureId);
        // Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 0.    
        glUniform1i(pShader->l_uTex0, 0);
    }
    if (pShader->l_uTex1mask >= 0) {
        int textureId = Texture::getGLid(pMt->uTex1mask);
        //pass textureId to shader program
        glActiveTexture(GL_TEXTURE1); // activate the texture unit first before binding texture
        glBindTexture(GL_TEXTURE_2D, textureId);
        // Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 1.    
        glUniform1i(pShader->l_uTex1mask, 1);
    }

    //material uniforms
    if (pShader->l_uTex1alphaChannelN >= 0)
        glUniform1i(pShader->l_uTex1alphaChannelN, pMt->uTex1alphaChannelN);
    if (pShader->l_uTex1alphaNegative >= 0)
        glUniform1i(pShader->l_uTex1alphaNegative, pMt->uTex1alphaNegative);
    if (pShader->l_uColor >= 0)
        glUniform4fv(pShader->l_uColor, 1, pMt->uColor.forGL());
    if (pShader->l_uAlphaFactor >= 0)
        glUniform1f(pShader->l_uAlphaFactor, pMt->uAlphaFactor);
    if (pShader->l_uAlphaBlending >= 0)
        glUniform1i(pShader->l_uAlphaBlending, pMt->uAlphaBlending);

    //adjust render settings
    if (pMt->zBuffer > 0) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }
    else
        glDisable(GL_DEPTH_TEST);

    if (pMt->zBufferUpdate > 0)
        glDepthMask(GL_TRUE);
    else
        glDepthMask(GL_FALSE);

    if (pShader->l_uAlphaBlending >= 0 && pMt->uAlphaBlending > 0 || pMt->uAlphaFactor < 1) {
        glDepthMask(GL_FALSE); //don't update z-buffer
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
        glDisable(GL_BLEND);

    //execute
    if (pDJ->glEBOid > 0)
        glDrawElements(pMt->primitiveType, pDJ->pointsN, GL_UNSIGNED_SHORT, 0);
    else
        glDrawArrays(pMt->primitiveType, 0, pDJ->pointsN);

    glBindVertexArray(0);


    //checkGLerrors("DrawJob::executeDrawJob end");

    return 1;
}

int UISubj::addZBufferSubj(std::string uiName, float x, float y, float w, float h, std::string alignTo, int uTex0) {
    std::vector<UISubj*>* pSubjs = &uiSubjs_default;
    std::vector<DrawJob*>* pDrawJobs = &DrawJob::drawJobs_default;

    UISubj* pUI = new UISubj();

    setCoords(pUI, x, y, w, h, alignTo);

    pUI->djStartN = djNdepthmap;
    memcpy((void*)&pUI->mt0, (void*)&pDrawJobs->at(pUI->djStartN)->mt, sizeof(Material));
    pUI->mt0.uTex0 = uTex0;
    return pUI->nInSubjs;
}
int UISubj::addTexSubj(std::string uiName, float x, float y, float w, float h, std::string alignTo, int uTex0) {
    std::vector<UISubj*>* pSubjs = &uiSubjs_default;
    std::vector<DrawJob*>* pDrawJobs = &DrawJob::drawJobs_default;

    UISubj* pUI = new UISubj();

    setCoords(pUI, x, y, w, h, alignTo);

    pUI->djStartN = djNtex;
    memcpy((void*)&pUI->mt0, (void*)&pDrawJobs->at(pUI->djStartN)->mt, sizeof(Material));
    pUI->mt0.uTex0 = uTex0;
    return pUI->nInSubjs;
}
int UISubj::addClrSubj(std::string uiName, float x, float y, float w, float h, std::string alignTo, unsigned int rgba) {
    std::vector<UISubj*>* pSubjs = &uiSubjs_default;
    std::vector<DrawJob*>* pDrawJobs = &DrawJob::drawJobs_default;

    UISubj* pUI = new UISubj();

    setCoords(pUI, x, y, w, h, alignTo);

    pUI->djStartN = djNclr;
    memcpy((void*)&pUI->mt0, (void*)&pDrawJobs->at(pUI->djStartN)->mt, sizeof(Material));
    pUI->mt0.uColor.setUint32(rgba);

    return pUI->nInSubjs;
}
int UISubj::renderAll() {
    std::vector<UISubj*>* pSubjs = &uiSubjs_default;
    int subjsN = pSubjs->size();
		//mylog("%d UI subjs\n",subjsN);
    for (int subjN = 0; subjN < subjsN; subjN++) {
        UISubj* pUI = pSubjs->at(subjN);

        pUI->render();
    }
    return 1;
}
int UISubj::onScreenResize(int width, int height) {
    std::vector<UISubj*>* pSubjs = &uiSubjs_default;
    screenSize[0] = (float)width;
    screenSize[1] = (float)height;
    screenAspectRatio = (float)width / (float)height;
    mat4x4 mProjection;
    float w = screenSize[0] / 2;
    float h = screenSize[1] / 2;
    mat4x4_ortho(mProjection, -w, w, -h, h, -1, 1);

    mat4x4 lookAtMatrix;
    float cameraUp[4] = { 0,1,0,0 }; //y - up
    float lookAtPoint[4] = { 0,0,-1,0 }; //z - away
    float cameraPos[4] = { 0,0,1,0 };
    mat4x4_look_at(lookAtMatrix, cameraPos, lookAtPoint, cameraUp);

    mat4x4_mul(mOrthoViewProjection, mProjection, lookAtMatrix);

    int subjsN = pSubjs->size();
    for (int subjN = 0; subjN < subjsN; subjN++) {
        UISubj* pUI = pSubjs->at(subjN);
        pUI->transformMatrixIsReady = false;
    }
    return 1;
}

int UISubj::cleanUp() {
    std::vector<UISubj*>* pSubjs = &uiSubjs_default;
    int subjsN = pSubjs->size();
    for (int subjN = 0; subjN < subjsN; subjN++)
        delete pSubjs->at(subjN);
    pSubjs->clear();
    return 1;
}
int UISubj::setCoords(UISubj* pUI, float x, float y, float w, float h, std::string countFrom) {
    strcpy_s(pUI->countFrom, 32, (char*)countFrom.c_str());
    pUI->ownCoords.pos[0] = x;
    pUI->ownCoords.pos[1] = y;
    pUI->scale[0] = w;
    pUI->scale[1] = h;
    pUI->transformMatrixIsReady = false;
    buildModelMatrix(pUI);
    return 1;
}
