#include "SceneSubj.h"
#include "platform.h"
#include "utils.h"
#include "TheApp.h"
#include "DrawJob.h"
#include "Shadows.h"
#include "ModelLoaderCmd.h"
#include "XMLparser.h"
#include <algorithm>


extern TheApp theApp;
extern float degrees2radians;


SceneSubj::SceneSubj() {
}
SceneSubj::~SceneSubj() {
    if (pCustomColors != NULL) {
        int totalN = pCustomColors->size();
        for (int i = 0; i < totalN; i++)
            delete pCustomColors->at(i);
        pCustomColors->clear();
        delete pCustomColors;
        pCustomColors = NULL;
    }
    if (pCustomMaterials != NULL) {
        int totalN = pCustomMaterials->size();
        for (int i = 0; i < totalN; i++)
            delete pCustomMaterials->at(i);
        pCustomMaterials->clear();
        delete pCustomMaterials;
        pCustomMaterials = NULL;
    }
}
void SceneSubj::buildModelMatrixStandard(SceneSubj* pSS) {
    memcpy(&pSS->absCoordsPrev, &pSS->absCoords,sizeof(Coords));

    if (pSS->d2parent != 0 && pSS->d2headTo != 0) {
        SceneSubj* pParent = pSS->pSubjsSet->at(pSS->nInSubjsSet - pSS->d2parent);
        if (v3equals(pSS->ownCoords.pos, 0))
            v3copy(pSS->absCoords.pos, pParent->absCoords.pos);
        else
            mat4x4_mul_vec4plus(pSS->absCoords.pos, pParent->absModelMatrixUnscaled, pSS->ownCoords.pos, 1);
        SceneSubj* pHeadTo = pSS->pSubjsSet->at(pSS->nInSubjsSet - pSS->d2headTo);
        float posHeadTo[4];
        if (v3equals(pSS->posOnHeadTo, 0))
            v3copy(posHeadTo, pHeadTo->absCoords.pos);
        else
            mat4x4_mul_vec4plus(posHeadTo, pHeadTo->absModelMatrixUnscaled, pSS->posOnHeadTo, 1);
        float yaw = v3yawDgFromTo(pSS->absCoords.pos, posHeadTo);
        pSS->absCoords.setYaw(yaw);
        if (pSS->place2middle)
            for (int i = 0; i < 3; i+=2)
                pSS->absCoords.pos[i] = (pSS->absCoords.pos[i] + posHeadTo[i]) / 2;
        //ignore ownModelMatrixUnscaled
        mat4x4_translate(pSS->absModelMatrixUnscaled, pSS->absCoords.pos[0], pSS->absCoords.pos[1], pSS->absCoords.pos[2]);
        //rotation order: Z-X-Y
        mat4x4_mul(pSS->absModelMatrixUnscaled, pSS->absModelMatrixUnscaled, pSS->absCoords.rotationMatrix);
    }
    else {//normal case
        mat4x4_translate(pSS->ownModelMatrixUnscaled, pSS->ownCoords.pos[0], pSS->ownCoords.pos[1], pSS->ownCoords.pos[2]);
        //rotation order: Z-X-Y
        mat4x4_mul(pSS->ownModelMatrixUnscaled, pSS->ownModelMatrixUnscaled, pSS->ownCoords.rotationMatrix);

        if (pSS->renderMirrorAxis >= 0)
            pSS->ownModelMatrixUnscaled[pSS->renderMirrorAxis][pSS->renderMirrorAxis] *= -1;

        if (pSS->d2parent != 0) {
            SceneSubj* pParent = pSS->pSubjsSet->at(pSS->nInSubjsSet - pSS->d2parent);
            mat4x4_mul(pSS->absModelMatrixUnscaled, pParent->absModelMatrixUnscaled, pSS->ownModelMatrixUnscaled);
        }
        else if (pSS->pStickTo != NULL)
            mat4x4_mul(pSS->absModelMatrixUnscaled, pSS->pStickTo->absModelMatrixUnscaled, pSS->ownModelMatrixUnscaled);
        else
            memcpy(pSS->absModelMatrixUnscaled, pSS->ownModelMatrixUnscaled, sizeof(mat4x4));
    }
    if (v3equals(pSS->scale, 1))
        memcpy(pSS->absModelMatrix, pSS->absModelMatrixUnscaled, sizeof(mat4x4));
    else
        mat4x4_scale_aniso(pSS->absModelMatrix, pSS->absModelMatrixUnscaled, pSS->scale[0], pSS->scale[1], pSS->scale[2]);

    //update absCoords
    if (pSS->d2parent == 0)
        memcpy(&pSS->absCoords, &pSS->ownCoords, sizeof(Coords));
    else {
        Coords::getPositionFromMatrix(pSS->absCoords.pos, pSS->absModelMatrixUnscaled);
    }

    pSS->absCoordsUpdateFrameN = theApp.frameN;
}

int SceneSubj::moveStandard(SceneSubj* pSS) {
    pSS->applySpeeds();
    return 1;
}
int SceneSubj::applySpeedsStandard(SceneSubj* pSS) {
    //set up speeds here or use pre-set
    if (pSS->ownSpeed.noRotation())
        return 0;
    quat q2;
    quat_mul(q2, pSS->ownSpeed.getRotationQuat(), pSS->ownCoords.getRotationQuat());
    pSS->ownCoords.setQuaternion(q2);
    return 1;
}



int SceneSubj::scaleStandard(SceneSubj* pSS, float k) {
    for (int i = 0; i < 3; i++)
        pSS->scale[i] *= k;
    int subjsN = pSS->pSubjsSet->size();
    for (int sN = pSS->nInSubjsSet + 1; sN < subjsN; sN++) {
        SceneSubj* pSS2 = pSS->pSubjsSet->at(sN);
        if (pSS2 == NULL)
            continue;
        if (pSS2->nInSubjsSet - pSS2->d2parent != pSS->nInSubjsSet)
            continue;
        for (int i = 0; i < 3; i++)
            pSS2->ownCoords.pos[i] *= k;
        pSS2->zOffsetFromRoot *= k;
        pSS2->lever *= k;
        pSS2->scaleMe(k);
    }
    return 1;
}
int SceneSubj::processSubjStandart(SceneSubj* pSS) {
    if (pSS->d2parent == 0)
        pSS->rootN = pSS->nInSubjsSet;
    else {
        SceneSubj* pParent = pSS->pSubjsSet->at(pSS->nInSubjsSet - pSS->d2parent);
        pSS->rootN = pParent->rootN;
    }
    pSS->moveSubj();
    if (pSS->absCoordsUpdateFrameN != theApp.frameN)
        pSS->buildModelMatrix();
    return 1;
}
int SceneSubj::renderDepthMapStandard(SceneSubj* pSS, Camera* pCam) {
    if (pSS->pDrawJobs == NULL)
        pSS->pDrawJobs = &DrawJob::drawJobs_default;

    if (pSS->dropsShadow < 1)
        return 0;
    mat4x4 mMVP;
    mat4x4_mul(mMVP, pCam->mViewProjection, pSS->absModelMatrix);
    mat4x4 mMV4x4;
    mat4x4_mul(mMV4x4, pCam->lookAtMatrix, pSS->absModelMatrixUnscaled);
    //convert to 3x3 matrix
    float mMV3x3[3][3];
    for (int y = 0; y < 3; y++)
        for (int x = 0; x < 3; x++)
            mMV3x3[y][x] = mMV4x4[y][x];
    //render subject
    for (int i = 0; i < pSS->djTotalN; i++) {
        DrawJob* pDJ = pSS->pDrawJobs->at(pSS->djStartN + i);
        if (pDJ->mt.dropsShadow < 1)
            continue;
        Material* pMT = &pDJ->mtShadow;

        executeDJstandard(pDJ, (float*)mMVP, *mMV3x3, NULL, NULL, NULL, NULL, NULL,
            Shadows::sizeUnitPixelsSize * pSS->scale[0], pMT);
    }

    /*
    float vIn[4] = { 0,0,-1,0 };
    float vOut[4];
    mat4x4_mul_vec4plus(vOut, mMVP, vIn, 1);
    */
    return 1;
}


int SceneSubj::onDeployStandatd(SceneSubj* pSS,std::string tagStr) {
    
    if (strlen(pSS->fileOnDeploy) > 0) {
        std::string fileOnDeployStr(pSS->fileOnDeploy);
        ModelLoaderCmd* pML = new ModelLoaderCmd(pSS, fileOnDeployStr);
        pML->processSource(pML);
        delete pML;
    }
    //tag instructions
    if (tagStr.length() == 0)
        return 0;

    float pos[2] = { 0,0 };
    XMLparser::setFloatArray(pos, 2, "tableAt", tagStr);
    float posY = theApp.gameTable.groundLevel;
    std::string alignStr = XMLparser::getStringValue("align", tagStr);
    if (alignStr.find("bottom") != std::string::npos)
        posY += (pSS->gabaritesOnLoad.bbRad[1] * pSS->scale[1]);
    theApp.gameTable.placeAt(pSS->ownCoords.pos, pos[0], posY, pos[1]);

    if (XMLparser::varExists("ay", tagStr)) {
        float ay = XMLparser::getFloatValue("ay", tagStr);
        pSS->ownCoords.setEulerDg(0, ay, 0);
        mat4x4_from_quat(pSS->ownCoords.rotationMatrix, pSS->ownCoords.getRotationQuat());
    }


    return 1;
}
int SceneSubj::addToRenderQueue(std::vector<SceneSubj*>* pQueueOpaque, std::vector<SceneSubj*>* pQueueTransparent, std::vector<SceneSubj*>* pSubjs) {
    int subjsN = pSubjs->size();
    int added = 0;
    for (int sN = 0; sN < subjsN; sN++) {
        SceneSubj* pSS = pSubjs->at(sN);
        if (pSS == NULL)
            continue;
        if (pSS->hide > 0)
            continue;
        if (pSS->djTotalN < 1)
            continue;
        //if (pSS->isOnScreen < 0)
          //  continue;
        //check render order/priority
        if (pSS->renderOrder < 0) {
            int lastOpaqueN = -1;
            int lastAlphaN = -1;
            for (int djN = 0; djN < pSS->djTotalN; djN++) {
                DrawJob* pDJ = pSS->pDrawJobs->at(pSS->djStartN + djN);
                Material* pMt = &pDJ->mt;
                if (pMt->uAlphaBlending > 0) //transparent
                    lastAlphaN = djN;
                else //opaque
                    lastOpaqueN = djN;
            }
            if (lastAlphaN < 0)//all DJs opaque
                pSS->renderOrder = 0;
            else if (lastOpaqueN < 0)
                pSS->renderOrder = 1; //all w/alpha
            else
                pSS->renderOrder = 2; //some DJs w/alpha, some - opaque, mixed
        }

        if (pQueueTransparent == NULL)
            pQueueOpaque->push_back(pSS);
        else if (pSS->renderOrder == 0)
            pQueueOpaque->push_back(pSS);
        else if (pSS->renderOrder == 1)
            pQueueTransparent->push_back(pSS);
        else { //mixed
            pQueueOpaque->push_back(pSS);
            pQueueTransparent->push_back(pSS);
        }
        added++;
    }
    return added;
}
int SceneSubj::sortRenderQueue(std::vector<SceneSubj*>* pQueue, int direction) {
    //direction: 0-closer 1st, 1-farther 1st
    if (direction == 0) //0-closer 1st
        std::sort(pQueue->begin(), pQueue->end(),
            [](SceneSubj* pSS0, SceneSubj* pSS1) {
                //if < - go 1st
                return pSS0->gabaritesOnScreen.bbMin[2] < pSS1->gabaritesOnScreen.bbMin[2];
            });
    else //1-farther 1st
        std::sort(pQueue->begin(), pQueue->end(),
            [](SceneSubj* pSS0, SceneSubj* pSS1) {
                //if > - go 1st
                return pSS0->gabaritesOnScreen.bbMin[2] > pSS1->gabaritesOnScreen.bbMin[2];
            });
    return 1;
}

int SceneSubj::customizeMaterial(Material** ppMt, Material** ppMt2, Material** ppAltMt, Material** ppAltMt2, SceneSubj* pSS) {
    Material* pMt = *ppMt;
    Material* pMt2 = *ppMt2;
    Material* pAltMt = *ppAltMt;
    Material* pAltMt2 = *ppAltMt2;

    SceneSubj* pRoot = pSS->pSubjsSet->at(pSS->rootN);
    if (pRoot->pCustomMaterials != NULL) {
        int customMaterialsN = pRoot->pCustomMaterials->size();
        if (strlen(pMt->materialName32) != 0) {
            for (int oN = 0; oN <= customMaterialsN; oN++) {
                if (oN == customMaterialsN)
                    break;
                MaterialAdjust* pMA = pRoot->pCustomMaterials->at(oN); //custom color option
                if (strcmp(pMA->materialName32, pMt->materialName32) == 0) {
                    pAltMt = new Material(pMt);
                    pMt = pAltMt;
                    pMA->adjust(pMt, pMA);
                    Shader::pickShaderNumber(pMt);
                    //2-nd layer
                    if (strlen(pMt->layer2as) == 0)
                        //pMt2 = NULL;
                        pMt2->shaderN = -1;
                    else if (strcmp(pMt->layer2as, pMt2->materialName32) != 0) {
                        pAltMt2 = new Material(pMt);
                        pMt2 = pAltMt2;
                        pMt2->shaderN = -1;
                        strcpy_s(pMt2->materialName32, 32, pMt->layer2as);
                        //look in
                        std::vector<MaterialAdjust*>* pMAlist = &MaterialAdjust::materialAdjustsList;
                        customMaterialsN = pMAlist->size();
                        for (int oN = 0; oN < customMaterialsN; oN++) {
                            MaterialAdjust* pMA = pMAlist->at(oN); //custom color option
                            if (strcmp(pMA->materialName32, pMt2->materialName32) == 0) {
                                pMA->adjust(pMt2, pMA);
                                Shader::pickShaderNumber(pMt2);
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    if (pRoot->pCustomColors != NULL) {
        int customColorsN = pRoot->pCustomColors->size();
        //customize colors?
        MyColor* pColors[3];
        pColors[0] = &pMt->uColor;
        pColors[1] = &pMt->uColor1;
        pColors[2] = &pMt->uColor2;
        for (int cN = 0; cN < 3; cN++) {
            MyColor* pCL = pColors[cN];
            if (strlen(pCL->colorName) == 0)
                continue;
            for (int oN = 0; oN <= customColorsN; oN++) {
                if (oN == customColorsN)
                    break;
                MyColor* pCC = pRoot->pCustomColors->at(oN); //custom color option
                if (strcmp(pCC->colorName, pCL->colorName) == 0) {
                    memcpy(pCL, pCC, sizeof(MyColor));
                    break;
                }
            }
        }
    }
    *ppMt = pMt;
    *ppMt2 = pMt2;
    *ppAltMt = pAltMt;
    *ppAltMt2 = pAltMt2;

    return 1;
}
bool SceneSubj::lineWidthIsImportant(int primitiveType) {
    if (primitiveType == GL_TRIANGLES) return false;
    if (primitiveType == GL_TRIANGLE_STRIP) return false;
    if (primitiveType == GL_TRIANGLE_FAN) return false;
    return true;
}
int SceneSubj::executeDJstandard(DrawJob* pDJ,
    float* uMVP, float* uMV3x3, float* uMM, float* uMVP4dm,
    float* uVectorToLight, float* uVectorToTranslucent, float* uCameraPosition, float line1pixSize, Material* pMt, 
    float uFogLevel, unsigned int uFogColor32) {

    if (pMt == NULL)
        pMt = &(pDJ->mt);

    int shaderN = pMt->shaderN;
    if (shaderN < 0)
        return 0;

    if (lineWidthIsImportant(pMt->primitiveType)) {
        if(pMt->lineWidthFixed==1) 
            glLineWidth(pMt->lineWidth);
        else {
            float lw = line1pixSize * pMt->lineWidth;
            if (lw < 1.0) {
                if (pMt->lineWidthFixed == 2) //not less than 1
                    lw = 1;
                else
                    return 0;
            }
            glLineWidth(lw);
        }
    }

    Shader* pShader = Shader::shaders.at(shaderN);
    glUseProgram(pShader->GLid);

    //input uniform matrices
    if (pShader->l_uMVP >= 0)
        glUniformMatrix4fv(pShader->l_uMVP, 1, GL_FALSE, (const GLfloat*)uMVP);
    if (pShader->l_uMV3x3 >= 0)
        glUniformMatrix3fv(pShader->l_uMV3x3, 1, GL_FALSE, (const GLfloat*)uMV3x3);
    if (pShader->l_uMM >= 0)
        glUniformMatrix4fv(pShader->l_uMM, 1, GL_FALSE, (const GLfloat*)uMM);
    //shadow uniform matrix
    if (pShader->l_uTex4dm >= 0)
        glUniformMatrix4fv(pShader->l_uMVP4dm, 1, GL_FALSE, (const GLfloat*)uMVP4dm);

    executeDJcommon(pDJ, pShader, uVectorToLight, uVectorToTranslucent, uCameraPosition, pMt, uFogLevel, uFogColor32);
    return 1;
}
int SceneSubj::executeDJcommon(DrawJob* pDJ, Shader* pShader,
    float* uVectorToLight, float* uVectorToTranslucent, float* uCameraPosition, Material* pMt, float uFogLevel, unsigned int uFogColor32) {
    //assuming that shader program set, transform matrices provided
    glBindVertexArray(pDJ->glVAOid);

    if (pShader->l_uDepthBias >= 0) //for depthMap
        glUniformMatrix4fv(pShader->l_uDepthBias, 1, GL_FALSE, (const GLfloat*)Shadows::uDepthBias);

    //shadow uniforms
    if (pShader->l_uTex4dm >= 0) {
        //translate to texture
        int textureId = Texture::getGLid(Shadows::depthMapTexN);
        //pass textureId to shader program
        glActiveTexture(GL_TEXTURE4); // activate the texture unit first before binding texture
        glBindTexture(GL_TEXTURE_2D, textureId);
        // Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 4.
        glUniform1i(pShader->l_uTex4dm, 4);

        glUniform1f(pShader->l_uShadow, Shadows::shadowLight);
    }

    if (pShader->l_uVectorToLight >= 0)
        glUniform3fv(pShader->l_uVectorToLight, 1, (const GLfloat*)uVectorToLight);

    if (pShader->l_uTranslucency >= 0) {
        glUniform1f(pShader->l_uTranslucency, pMt->uTranslucency);
        if (pMt->uTranslucency > 0)
            glUniform3fv(pShader->l_uVectorToTranslucent, 1, (const GLfloat*)uVectorToTranslucent);
    }

    if (pShader->l_uCameraPosition >= 0)
        glUniform3fv(pShader->l_uCameraPosition, 1, (const GLfloat*)uCameraPosition);

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
    if (pShader->l_uTex2nm >= 0) {
        int textureId = Texture::getGLid(pMt->uTex2nm);
        //pass textureId to shader program
        glActiveTexture(GL_TEXTURE2); // activate the texture unit first before binding texture
        glBindTexture(GL_TEXTURE_2D, textureId);
        // Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 2.    
        glUniform1i(pShader->l_uTex2nm, 2);
    }
    if (pShader->l_uTex0translateChannelN >= 0) {
        glUniform1i(pShader->l_uTex0translateChannelN, pMt->uTex0translateChannelN);
        if (pMt->uTex0translateChannelN >= 4) {
            //translate to 2-tone
            if (pShader->l_uColor1 >= 0) {
                glUniform4fv(pShader->l_uColor1, 1, pMt->uColor1.forGL());
                glUniform4fv(pShader->l_uColor2, 1, pMt->uColor2.forGL());
            }
        }
        else
            if (pShader->l_uTex3 >= 0 && pMt->uTex3 >= 0) {
                //translate to texture
                int textureId = Texture::getGLid(pMt->uTex3);
                //pass textureId to shader program
                glActiveTexture(GL_TEXTURE3); // activate the texture unit first before binding texture
                glBindTexture(GL_TEXTURE_2D, textureId);
                // Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 3.    
                glUniform1i(pShader->l_uTex3, 3);
            }
    }

    //tex coords modifiers
    if (pShader->l_uTexMod >= 0)
        glUniform4fv(pShader->l_uTexMod, 1, pMt->uTexMod);

    //for normal map
    if (pShader->l_uHaveBinormal >= 0)
        glUniform1i(pShader->l_uHaveBinormal, pDJ->uHaveBinormal);

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
    if (pShader->l_uAmbient >= 0)
        glUniform1f(pShader->l_uAmbient, pMt->uAmbient);
    if (pShader->l_uSpecularIntencity >= 0)
        glUniform1f(pShader->l_uSpecularIntencity, pMt->uSpecularIntencity);
    if (pShader->l_uSpecularMinDot >= 0)
        glUniform2fv(pShader->l_uSpecularMinDot, 1, pMt->uSpecularMinDot);
    if (pShader->l_uSpecularPowerOf >= 0)
        glUniform1f(pShader->l_uSpecularPowerOf, pMt->uSpecularPowerOf);
    if (pShader->l_uBleach >= 0)
        glUniform1f(pShader->l_uBleach, pMt->uBleach);
    if (pShader->l_uShadingK >= 0)
        glUniform1f(pShader->l_uShadingK, pMt->uShadingK);
    if (pShader->l_uDiscardNormalsOut >= 0)
        glUniform1i(pShader->l_uDiscardNormalsOut, pMt->uDiscardNormalsOut);
    if (pShader->l_uConstZ >= 0) {
        float constZ = Shadows::uConstZ * 2;
        if (pMt->isLine())
            constZ = constZ + Shadows::uConstZ * pMt->lineWidth;
        glUniform1f(pShader->l_uConstZ, constZ);
    }
    if (pShader->l_uHighLightLevel >= 0)
        glUniform1f(pShader->l_uHighLightLevel, uFogLevel);
    if (pShader->l_uHighLightColor >= 0) {
        MyColor uHighLightColor;
        uHighLightColor.setUint32(uFogColor32);
        glUniform4fv(pShader->l_uHighLightColor, 1, uHighLightColor.forGL());
    }

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

int SceneSubj::onLoadStandatd(SceneSubj* pSS, std::string tagStr) {
    return 1;
}
void SceneSubj::setHighLight(SceneSubj* pSS, float highLightlevel, unsigned int highLightColor32) {
    pSS->uHighLightLevel = highLightlevel;
    pSS->uHighLightColor.setUint32(highLightColor32);
}
int SceneSubj::renderStandard_prepare(SceneSubj* pSS, Camera* pCam, float* dirToMainLight, float* dirToTranslucent, 
    mat4x4 mMVP, mat4x4 mMVP4dm, float* pSizeUnitPixelsSize) {

    //build mMVP4dm (shadow MVP) matrix for given subject
    mat4x4_mul(mMVP4dm, Shadows::shadowCamera.mViewProjection, pSS->absModelMatrix);
    //build MVP matrix for given subject
    mat4x4_mul(mMVP, pCam->mViewProjection, pSS->absModelMatrix);

    float sizeUnitPixelsSize = getUnitPixelsSize(pSS, pCam);
    *pSizeUnitPixelsSize = sizeUnitPixelsSize;
    return 1;
}
float SceneSubj::getUnitPixelsSize(SceneSubj * pSS, Camera * pCam){
    //subj's distance from camera
    float cameraSpacePos[4];
    mat4x4_mul_vec4plus(cameraSpacePos, pCam->lookAtMatrix, pSS->absCoords.pos, 1);
    float zDistance = abs(cameraSpacePos[2]);
    float viewRangeDg = pCam->viewRangeDg;
    if (viewRangeDg == 0)
        viewRangeDg = 30;
    float cotangentA = 1.0f / tanf(degrees2radians * viewRangeDg / 2.0f);
    float halfScreenVertSizeInUnits = zDistance / cotangentA;
    float sizeUnitPixelsSize = (float)theApp.mainCamera.targetRads[1] / halfScreenVertSizeInUnits;
    sizeUnitPixelsSize *= pSS->scale[0];
    return sizeUnitPixelsSize;
}
int SceneSubj::renderStandard(SceneSubj* pSS, Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter) {
    mat4x4 mMVP;
    mat4x4 mMVP4dm;
    float sizeUnitPixelsSize;
    renderStandard_prepare(pSS, pCam, dirToMainLight, dirToTranslucent, mMVP, mMVP4dm, &sizeUnitPixelsSize);
    renderStandard_execute(pSS, mMVP, mMVP4dm, pCam, dirToMainLight, dirToTranslucent, sizeUnitPixelsSize,renderFilter);
    return 1;
}
int SceneSubj::renderStandard_execute(SceneSubj* pSS, mat4x4 mMVP, mat4x4 mMVP4dm, Camera* pCam, float* dirToMainLight, float* dirToTranslucent,
    float sizeUnitPixelsSize, int renderFilter){

    if (renderFilter == pSS->renderOrder)
        renderFilter = -1; //render all

    glCullFace(pSS->cullFace);

    //build Model-View (rotation) matrix for normals
    mat4x4 mMV4x4;
    mat4x4_mul(mMV4x4, pCam->lookAtMatrix, pSS->absModelMatrixUnscaled);
    //convert to 3x3 matrix
    float mMV3x3[3][3];
    for (int y = 0; y < 3; y++)
        for (int x = 0; x < 3; x++)
            mMV3x3[y][x] = mMV4x4[y][x];

    if (pSS->pDrawJobs == NULL)
        pSS->pDrawJobs = &DrawJob::drawJobs_default;

    for (int i0 = 0; i0 < pSS->djTotalN; i0++) {
        int i = i0;
        if (renderFilter > 0) //render transparent only, most likely - GLTF reversed order
            i = pSS->djTotalN - 1 - i0;

        DrawJob* pDJ = pSS->pDrawJobs->at(pSS->djStartN + i);

        Material* pMt = &pDJ->mt;

        if (renderFilter >= 0)
            if (renderFilter != pMt->uAlphaBlending > 0)
                continue;

        Material* pMt2 = &pDJ->mtLayer2;
        if (i == 0) {
            Material mt;
            memcpy(&mt, &pDJ->mt, sizeof(Material));
            pMt = &mt;
            //2-nd layer
            Material mt2;
            memcpy(&mt2, &pDJ->mtLayer2, sizeof(Material));
            pMt2 = &mt2;
        }
        Material* pAltMt = NULL;
        Material* pAltMt2 = NULL;
        if (i == 0 && pSS->mt0isSet > 0) { //already customized
            pMt = &pSS->mt0;
            pMt2 = &pSS->mt0Layer2;
        }
        else { //customize?
            customizeMaterial(&pMt, &pMt2, &pAltMt, &pAltMt2, pSS);
            if (i == 0) {
                memcpy(&pSS->mt0, pMt, sizeof(Material));
                memcpy(&pSS->mt0Layer2, pMt2, sizeof(Material));
                pSS->mt0isSet = 1;
            }
        }
 
        executeDJstandard(pDJ, (float*)mMVP, *mMV3x3, (float*)pSS->absModelMatrix, (float*)mMVP4dm, dirToMainLight, dirToTranslucent,
            pCam->ownCoords.pos, sizeUnitPixelsSize, pMt, pSS->uHighLightLevel, pSS->uHighLightColor.getUint32());

        //have 2-nd layer ?
        if (pMt2 != NULL) {
            if (pMt2->shaderN >= 0) {
                executeDJstandard(pDJ, (float*)mMVP, *mMV3x3, (float*)pSS->absModelMatrix, (float*)mMVP4dm, dirToMainLight, dirToTranslucent,
                    pCam->ownCoords.pos, sizeUnitPixelsSize, pMt2, pSS->uHighLightLevel, pSS->uHighLightColor.getUint32());
            }
        }
        if (pAltMt != NULL) {
            delete pAltMt;
            pAltMt = NULL;
        }
        if (pAltMt2 != NULL) {
            delete pAltMt2;
            pAltMt2 = NULL;
        }
    }
    return 1;
}
void SceneSubj::deployModel(SceneSubj * pSS00, std::string tagStr){
    for (int elN = pSS00->totalElements - 1; elN >= 0; elN--) {
        SceneSubj* pSS = pSS00->pSubjsSet->at(pSS00->nInSubjsSet + elN);
        if(pSS == NULL)
            continue;
        if(elN==0)
            pSS->onDeploy(tagStr);
        else
            pSS->onDeploy("");
    }
}
std::vector<int>* SceneSubj::entireFamily(SceneSubj* pSS00) {
    std::vector<int>* pv = new std::vector<int>;
    pv->push_back(pSS00->nInSubjsSet);
    //array limits
    //SceneSubj* pRoot = pSS00->pSubjsSet->at(pSS00->rootN);
    //int lastN2check = pRoot->nInSubjsSet + pRoot->totalElements - 1;
    int lastN2check = pSS00->pSubjsSet->size() - 1;
    for (int checkSlotN = 0; checkSlotN < pv->size(); checkSlotN++) {
        int parentN = pv->at(checkSlotN);
        for (int elN = lastN2check; elN > parentN; elN--) {
            SceneSubj* pSS = pSS00->pSubjsSet->at(elN);
            if (elN - pSS->d2parent != parentN)
                continue;
            //here-have a child
            pv->push_back(elN);
        }
    }
    return pv;
}
void SceneSubj::setHide(SceneSubj* pSS00, int val) {
    std::vector<int>* pFamilyList = entireFamily(pSS00);
    for (int slotN = pFamilyList->size() - 1; slotN >= 0; slotN--) {
        int elN = pFamilyList->at(slotN);
        SceneSubj* pSS = pSS00->pSubjsSet->at(elN);
        pSS->hide = val;
    }
    pFamilyList->clear();
    delete pFamilyList;
}

void SceneSubj::resetRoot(SceneSubj* pSS00,int rootN) {
    if (rootN < 0) {    //find root
        rootN = pSS00->nInSubjsSet;
        while (pSS00->d2parent != 0) {
            rootN = pSS00->nInSubjsSet - pSS00->d2parent;
            pSS00 = pSS00->pSubjsSet->at(rootN);
        }
    }
    //reset
    std::vector<int>* pFamilyList = entireFamily(pSS00);
    for (int slotN = pFamilyList->size() - 1; slotN >= 0; slotN--) {
        int elN = pFamilyList->at(slotN);
        SceneSubj* pSS = pSS00->pSubjsSet->at(elN);
        pSS->rootN = rootN;
    }
    pFamilyList->clear();
    delete pFamilyList;
}
int SceneSubj::checkCollisions(std::vector<std::vector<SceneSubj*>*> pSubjArraysPairs4collisionDetection) {
    for (int arrayN = 0; arrayN < pSubjArraysPairs4collisionDetection.size(); arrayN += 2) {
        std::vector<SceneSubj*>* pSet1 = pSubjArraysPairs4collisionDetection.at(arrayN);
        std::vector<SceneSubj*>* pSet2 = pSubjArraysPairs4collisionDetection.at(arrayN+1);
        int set1size = pSet1->size();
        int set2size = pSet2->size();
        bool sameSet = (pSet1 == pSet2);
        for (int n1 = 0; n1 < set1size; n1++) {
            SceneSubj* pS1 = pSet1->at(n1);
            if (pS1 == NULL)
                continue;
            if (pS1->hidden() > 0)
                continue;
            if (pS1->d2parent != 0)
                continue;
            int startFrom = 0;
            if (sameSet)
                startFrom = n1 + 1;
            for (int n2 = startFrom; n2 < set2size; n2++) {
                SceneSubj* pS2 = pSet2->at(n2);
                if (pS2 == NULL)
                    continue;
                if (pS2->hidden() > 0)
                    continue;
                if (pS2->d2parent != 0)
                    continue;
                if (pS1->ignoreCollision(pS2))
                    continue;
                float penetrationDepth = checkMacroObjsCollision(pS1, pS2);
                if (penetrationDepth > 0)
                    break;
            }
        }
    }
    return 1;
}
float SceneSubj::checkMacroObjsCollision(SceneSubj* pS01, SceneSubj* pS02) {
    for (int n1 = 0; n1 < pS01->totalElements; n1++) {
        int sN1 = n1 + pS01->nInSubjsSet;
        SceneSubj* pS1 = pS01->pSubjsSet->at(sN1);
        if (pS1 == NULL)
            continue;
        if (pS1->hidden() > 0)
            continue;
        if (pS1->djTotalN<1)
            continue;
        if (pS1->gabaritesWorld.chordType < 0)
            continue;
        if (pS1->rootN != pS01->rootN)
            continue;
        for (int n2 = 0; n2 < pS02->totalElements; n2++) {
            int sN2 = n2 + pS02->nInSubjsSet;
            SceneSubj* pS2 = pS02->pSubjsSet->at(sN2);
            if (pS2 == NULL)
                continue;
            if (pS2->hidden() > 0)
                continue;
            if (pS2->djTotalN < 1)
                continue;
            if (pS2->gabaritesWorld.chordType < 0)
                continue;
            if (pS2->rootN != pS02->rootN)
                continue;
            float collisionPoint[4];
            float penetrationDepth = Gabarites::checkCollision(collisionPoint, &pS1->gabaritesWorld, &pS2->gabaritesWorld);
            if (penetrationDepth <= 0)
               continue;
            //have 2 objs collision here
            float collisionPointWorld[4];
            float closestPoint1[4];
            float closestPoint2[4];
            mat4x4_mul_vec4plus(collisionPointWorld, theApp.collisionMViewInversed, collisionPoint, 1, true);
            mat4x4_mul_vec4plus(closestPoint1, theApp.collisionMViewInversed, pS1->gabaritesWorld.chord.closestPoint, 1, true);
            mat4x4_mul_vec4plus(closestPoint2, theApp.collisionMViewInversed, pS2->gabaritesWorld.chord.closestPoint, 1, true);
            float hitPointNormal1[4];
            float hitPointNormal2[4];
            for (int i = 0; i < 3; i++) {
                hitPointNormal1[i] = collisionPointWorld[i] - closestPoint1[i];
                hitPointNormal2[i] = collisionPointWorld[i] - closestPoint2[i];
            }
            v3norm(hitPointNormal1);
            v3norm(hitPointNormal2);
            //hit speed
            float speedS1[3];
            float speedS2[3];
            float hitSpeed2S1[3];
            float hitSpeed2S2[3];
            for (int i = 0; i < 3; i++) {
                speedS1[i] = pS1->absCoords.pos[i] - pS1->absCoordsPrev.pos[i];
                speedS2[i] = pS2->absCoords.pos[i] - pS2->absCoordsPrev.pos[i];
                hitSpeed2S1[i] = -speedS1[i] + speedS2[i];
                hitSpeed2S2[i] = -hitSpeed2S1[i];
            }
            SceneSubj* pS1base = pS1;
            while (pS1base->d2parent != 0) {
                pS1base = pS1base->pSubjsSet->at(pS1base->nInSubjsSet - pS1base->d2parent);
            }
            SceneSubj* pS2base = pS2;
            while (pS2base->d2parent != 0) {
                pS2base = pS2base->pSubjsSet->at(pS2base->nInSubjsSet - pS2base->d2parent);
            }
            pS1base->processCollision(penetrationDepth,pS2base, collisionPointWorld, hitPointNormal1, hitSpeed2S1);
            pS2base->processCollision(penetrationDepth,pS1base, collisionPointWorld, hitPointNormal2, hitSpeed2S2);
            return penetrationDepth;
        }
    }
    return 0;
}
int SceneSubj::processCollisionStandard(float penetrationDepth, SceneSubj* pS1, SceneSubj* pS2, float* collisionPointWorld, float* hitPointNormal, float* hitSpeed) {
    return 1;
}
int lastId = 0;
int SceneSubj::newId() {
    lastId++;
    return lastId;
}