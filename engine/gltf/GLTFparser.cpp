#include "GLTFparser.h"
#include "utils.h"
#include "platform.h"
#include "TheApp.h"
#include "Texture.h"
#include "Shader.h"
#include "GLTFskin.h"
#include "GLTFanimation.h"
#include "GLTFanim2nodesMap.h"

extern TheApp theApp;
extern float degrees2radians;

GLTFparser::~GLTFparser() {
    texturesNs.clear();

    //delete all MeshDJs
    int itemsN = meshDJs.size();
    for (int i = 0; i < itemsN; i++) {
        MeshDJs* pMDJ = meshDJs.at(i);
        delete pMDJ;
    }
    meshDJs.clear(); //djStartN/djTotalN
}
int GLTFparser::loadModel(std::vector<SceneSubj*>* pSubjs, std::vector<DrawJob*>* pDrawJobs0, std::vector<unsigned int>* pBuffersIds0,
    std::string sourceFile, std::string subjClass, ProgressBar* pPBar) {

    int subjN = pSubjs->size();
    SceneSubj* pSS = theApp.newSceneSubj(subjClass, sourceFile, pSubjs, pDrawJobs0);

    GLTFparser* pParser = new GLTFparser();
    pParser->pNodes = pSubjs;

    if (pDrawJobs0 == NULL)
        pParser->pDrawJobs = &DrawJob::drawJobs_default;
    else
        pParser->pDrawJobs = pDrawJobs0;

    if (pBuffersIds0 == NULL)
        pParser->pBuffersIds = &DrawJob::buffersIds_default;
    else
        pParser->pBuffersIds = pBuffersIds0;

    readGLB(pParser, sourceFile);
    pPBar->nextStep(pPBar, "readGLB");
    int rootNodeN = parseGLTFmodel(pParser, pPBar);
    delete pParser;

    //attach GLTF model to root SceneSubj
    SceneSubj* pRootNode = pSubjs->at(rootNodeN);
    pRootNode->d2parent = rootNodeN - subjN;

    pSS->totalNativeElements = pSubjs->size() - subjN;
    pSS->totalElements = pSS->totalNativeElements;
    return subjN;
}
int GLTFparser::readGLB(GLTFparser* pParser, std::string fullPath) {
    tinygltf::Model* pModel = &pParser->gltf_model;

    tinygltf::TinyGLTF gltf_ctx; // glTF Parser/Serializer
    std::string err;
    std::string warn;

    bool ret = gltf_ctx.LoadBinaryFromFile(pModel, &err, &warn,
        fullPath.c_str());

    if (ret)
        return 1;
    if (!err.empty())
        mylog("ERROR in GLTFparser::loadGLB %s\n%s\n", fullPath.c_str(), err.c_str());
    if (!warn.empty())
        mylog("WARNING in GLTFparser::loadGLB %s\n%s\n", fullPath.c_str(), warn.c_str());
    return -1;
}

int GLTFparser::parseGLTFmodel(GLTFparser* pParser, ProgressBar* pPBar) {
    tinygltf::Model* pModel = &pParser->gltf_model;
    /*
      std::vector<Accessor> accessors;
      std::vector<Animation> animations;
      std::vector<Buffer> buffers;          //buffers - raw data, including VBOs and IBOs, maybe mixed, used as data holders for bufferViews
      std::vector<BufferView> bufferViews;  //bufferView describes a “chunk” or a “slice” of the whole raw buffer data, such as VBO or EBO
      std::vector<Material> materials;
      std::vector<Mesh> meshes;
      std::vector<Node> nodes;
      std::vector<Texture> textures;
      std::vector<Image> images;
      std::vector<Skin> skins;
      std::vector<Sampler> samplers;
      std::vector<Camera> cameras;
      std::vector<Scene> scenes;
      std::vector<Light> lights;

      int defaultScene = -1;
    */

    //images
    int imagesN = pModel->images.size();
    for (int imageN = 0; imageN < imagesN; imageN++) {
        tinygltf::Image* pImage = &pModel->images.at(imageN);
        /*
          std::string name;
          int width;
          int height;
          int component;
          int bits;        // bit depth per channel. 8(byte), 16 or 32.
          int pixel_type;  // pixel type(TINYGLTF_COMPONENT_TYPE_***). usually
                           // UBYTE(bits = 8) or USHORT(bits = 16)
          std::vector<unsigned char> image;
          int bufferView;        // (required if no uri)
          std::string mimeType;  // (required if no uri) ["image/jpeg", "image/png",
                                 // "image/bmp", "image/gif"]
          std::string uri;       // (required if no mimeType) uri is not decoded(e.g.
                                 // whitespace may be represented as %20)
        */
        int texN = -1;
        if (pImage->uri.empty()) {
            //load from memory
            tinygltf::BufferView* pBV = &pModel->bufferViews.at(pImage->bufferView);
            texN = Texture::generateTexture(pImage->name, pImage->width, pImage->height, pImage->image.data());// , int glRepeatH, int glRepeatV);
        }
        else {
            mylog("ATTN from GLTFparser::parseGLTF: pImage->uri=%s", pImage->uri.c_str());
            return -1;
        }
        pParser->texturesNs.push_back(texN);
        pPBar->nextStep(pPBar, "image");
    }
    mylog(" %d images loaded.\n", imagesN);

    //textures
    int texturesN = pModel->textures.size();
    for (int textureN = 0; textureN < texturesN; textureN++) {
        tinygltf::Texture* pTexture = &pModel->textures.at(textureN);
        /*
          std::string name;
          int sampler;
          int source; //pointer to Image
        */
        tinygltf::Sampler* pSampler = &pModel->samplers.at(pTexture->sampler);
        /*
          std::string name;
          // glTF 2.0 spec does not define default value for `minFilter` and
          // `magFilter`. Set -1 in TinyGLTF(issue #186)
          int minFilter =
              -1;  // optional. -1 = no filter defined. ["NEAREST", "LINEAR",
                   // "NEAREST_MIPMAP_NEAREST", "LINEAR_MIPMAP_NEAREST",
                   // "NEAREST_MIPMAP_LINEAR", "LINEAR_MIPMAP_LINEAR"]
          int magFilter =
              -1;  // optional. -1 = no filter defined. ["NEAREST", "LINEAR"]
          int wrapS =
              TINYGLTF_TEXTURE_WRAP_REPEAT;  // ["CLAMP_TO_EDGE", "MIRRORED_REPEAT",
                                             // "REPEAT"], default "REPEAT"
          int wrapT =
              TINYGLTF_TEXTURE_WRAP_REPEAT;  // ["CLAMP_TO_EDGE", "MIRRORED_REPEAT",
        */
        Texture* pTex = Texture::textures.at(pParser->texturesNs.at(pTexture->source));
        glBindTexture(GL_TEXTURE_2D, pTex->GLid);
        // set the texture wrapping/filtering options (on the currently bound texture object)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, pSampler->wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, pSampler->wrapT);
        if (pSampler->magFilter >= 0)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, pSampler->magFilter);
        if (pSampler->minFilter >= 0)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, pSampler->minFilter);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    mylog(" %d textures loaded.\n", texturesN);

    int primitivesTotal = 0;
    int meshesN = pModel->meshes.size();
    for (int meshN = 0; meshN < meshesN; meshN++) {
        MeshDJs* pMDJs = new MeshDJs();
        pParser->meshDJs.push_back(pMDJs);
        pMDJs->djStartN = pParser->pDrawJobs->size();
        tinygltf::Mesh* pMesh = &pModel->meshes.at(meshN);
        /*
        struct Mesh {
          std::string name;
          std::vector<Primitive> primitives;
          std::vector<double> weights;  // weights to be applied to the Morph Targets
        */
        int primitivesN = pMesh->primitives.size();
        primitivesTotal += primitivesN;
        for (int primitiveN = 0; primitiveN < primitivesN; primitiveN++) {
            tinygltf::Primitive* pPrimitive = &pMesh->primitives.at(primitiveN);

            DrawJob* pDJ = new DrawJob(pParser->pDrawJobs);
            pDJ->primitiveType = pPrimitive->mode; //GL_TRIANGLES and so on

            //create VAO
            glGenVertexArrays(1, &pDJ->glVAOid);
            glBindVertexArray(pDJ->glVAOid);

            checkGLerrors("GLTFparser::parseGLTF create VAO");
            /*
            struct Primitive {
              std::map<std::string, int> attributes;  // (required) A dictionary object of
                                                      // integer, where each integer
                                                      // is the index of the accessor
                                                      // containing an attribute.
              int material;  // The index of the material to apply to this primitive
                             // when rendering.
              int indices;   // The index of the accessor that contains the indices.
              int mode;      // one of TINYGLTF_MODE_***
              std::vector<std::map<std::string, int> > targets;  // array of morph targets,
              // where each target is a dict with attributes in ["POSITION, "NORMAL",
              // "TANGENT"] pointing
              // to their corresponding accessors
            */

            //indices accessor
            if (pPrimitive->indices >= 0) {
                tinygltf::Accessor* pAccessor = &pModel->accessors.at(pPrimitive->indices);
                pDJ->glEBOid = bufferIdAtGPU(pParser, pAccessor->bufferView);
                pDJ->pointsN = pAccessor->count;
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pDJ->glEBOid);

                checkGLerrors("GLTFparser::parseGLTF attaching EBO");
            }
            else { //no indices - use vers #
                pDJ->glEBOid = -1;
                int accessorN = pPrimitive->attributes["POSITION"];
                tinygltf::Accessor* pAccessor = &pModel->accessors.at(accessorN);
                pDJ->pointsN = pAccessor->count;
            }

            //attributes
            /*
            layout (location = 0) in vec3 aPos; // position attribute (x,y,z)
            layout (location = 1) in vec2 aTuv; //attribute TUV (texture coordinates)
            layout (location = 2) in vec2 aTuv2; //attribute TUV2 (for normal map)
            layout (location = 3) in vec3 aNormal; // normal attribute (x,y,z)
            layout (location = 4) in vec3 aTangent;//normal map
            layout (location = 5) in vec3 aBinormal;//normal map

            layout (location = 6) in float aJoints;
            layout (location = 7) in float aWeights;
            */
            glTF_attachAttribute(pParser, pPrimitive, "POSITION", 0, 3, GL_FLOAT);
            glTF_attachAttribute(pParser, pPrimitive, "TEXCOORD_0", 1, 2, GL_FLOAT);
            glTF_attachAttribute(pParser, pPrimitive, "NORMAL", 3, 3, GL_FLOAT);
            //glTF_attachAttribute(pParser, pPrimitive, "TANGENT", 4, 3, GL_FLOAT); //moved down to normal map check
            glTF_attachAttribute(pParser, pPrimitive, "JOINTS_0", 6, 4, GL_UNSIGNED_BYTE);
            glTF_attachAttribute(pParser, pPrimitive, "WEIGHTS_0", 7, 4, GL_FLOAT);

            buildBoundingBoxFromVerts(&pDJ->gabarites, pParser, pPrimitive);

            //material
            Material* pMt = &pDJ->mt;

            //pPrimitive->material = -1;

            if (pPrimitive->material < 0) {
                //material not defined - use default
                strcpy_s(pMt->materialName32, 32, "default");
                pMt->uColor.setRGBA(0, 255, 0, 255);
                Shader::assignShader(pMt, "phong");

                //shadow renderer
                if (pMt->dropsShadow > 0) {
                    Material* pMtS = &pDJ->mtShadow;
                    memcpy(pMtS, pMt, sizeof(Material));
                    pMtS->takesShadow = 0;
                    Shader::assignShader(pMtS, "depthmap");
                }

                checkGLerrors("GLTFparser::parseGLTF parsing primitive");
            }
            else {
                //material defined
                tinygltf::Material* pMaterial = &pModel->materials.at(pPrimitive->material);
                /*
                tinygltf::Material:
                      std::string name;
                      std::vector<double> emissiveFactor;  // length 3. default [0, 0, 0]
                      std::string alphaMode;               // default "OPAQUE"
                      double alphaCutoff;                  // default 0.5
                      bool doubleSided;                    // default false;
                      PbrMetallicRoughness pbrMetallicRoughness;
                      NormalTextureInfo normalTexture;
                      OcclusionTextureInfo occlusionTexture;
                      TextureInfo emissiveTexture;
                      ExtensionMap extensions;
                      Value extras;
                */
                strcpy_s(pMt->materialName32, 32, pMaterial->name.c_str());

                tinygltf::PbrMetallicRoughness* pPBRMetallicRoughness = &pMaterial->pbrMetallicRoughness;
                /*
                tinygltf::PbrMetallicRoughness:
                      std::vector<double> baseColorFactor;  // len = 4. default [1,1,1,1]
                      TextureInfo baseColorTexture;
                      double metallicFactor;   // default 1
                      double roughnessFactor;  // default 1
                      TextureInfo metallicRoughnessTexture; - ignore so far
                */
                //color texture
                tinygltf::TextureInfo* pPBRMetallicRoughness_baseColorTexture = &pPBRMetallicRoughness->baseColorTexture;
                int baseColorTextureIndex = pPBRMetallicRoughness_baseColorTexture->index;
                if (baseColorTextureIndex < 0) { //uColor
                    std::vector<double>* pClr = &pPBRMetallicRoughness->baseColorFactor;
                    pMt->uColor.setRGBA((float)pClr->at(0), (float)pClr->at(1), (float)pClr->at(2), (float)pClr->at(3));
                    //pMt->uColor.setRGBA(0.0f, 1.0f, 0.0f, 1.0f);
                }
                else { //uTex0
                    tinygltf::Texture* pTexture = &pModel->textures.at(baseColorTextureIndex);
                    /*
                      std::string name;
                      int sampler;
                      int source; //pointer to Image
                    */
                    pMt->uTex0 = pParser->texturesNs.at(pTexture->source);

                    //usesAlpha?
                    Texture* pTex = Texture::textures.at(pMt->uTex0);

                    pTex->usesAlpha = 0;

                    if (pTex->usesAlpha > 0) {
                        pMt->uAlphaBlending = 1;
                        pMt->zBuffer = 1;
                        pMt->zBufferUpdate = 0;
                    }

                }
                float metallicFactor = (float)pPBRMetallicRoughness->metallicFactor;   // default 1
                float roughnessFactor = (float)pPBRMetallicRoughness->roughnessFactor;  // default 1

                tinygltf::TextureInfo* pPBRMetallicRoughness_metallicRoughnessTexture = &pPBRMetallicRoughness->metallicRoughnessTexture;
                int metallicRoughnessTextureIndex = pPBRMetallicRoughness_metallicRoughnessTexture->index;
                if (metallicRoughnessTextureIndex >= 0) {
                    mylog("ATTN from GLTFparser::parseGLTF: has metallicRoughnessTexture\n");

                    tinygltf::Texture* pTexture = &pModel->textures.at(metallicRoughnessTextureIndex);

                    int texN = pParser->texturesNs.at(pTexture->source);
                    if (texN == pMt->uTex0) {
                        mylog("ERROR in GLTFparser::parseGLTF: metallicRoughnessTexture is same as color map (uTex0)\n");
                        metallicFactor = 0;
                        roughnessFactor = 0.5;
                    }
                    else {
                        tinygltf::Image* pImage = &pModel->images.at(pTexture->source);
                        int pixelsN = pImage->width * pImage->height;
                        float sumR = 0;
                        float sumG = 0;
                        for (int pxN = pixelsN - 1; pxN >= 0; pxN--) {
                            sumR += (float)pImage->image.at(pxN * 4) / 255.0f;
                            sumG += (float)pImage->image.at(pxN * 4 + 1) / 255.0f;
                        }
                        metallicFactor = sumR / pixelsN;// 0;
                        roughnessFactor = sumG / pixelsN;//0.5;
                    }
                }

                //normal map?
                tinygltf::NormalTextureInfo* pNormalTexture = &pMaterial->normalTexture;
                int normalTextureIndex = pNormalTexture->index;

                if (normalTextureIndex >= 0)
                    if (pPrimitive->attributes.count("TANGENT") < 1) {
                        normalTextureIndex = -1;
                        mylog("ERROR in GLTFparser::parseGLTF: TANGENTs missed\n");
                    }

                if (normalTextureIndex >= 0) { //has normal map
                    tinygltf::Texture* pTexture = &pModel->textures.at(normalTextureIndex);
                    pMt->uTex2nm = pParser->texturesNs.at(pTexture->source);
                    //pMt->uTex0 = pParser->texturesNs.at(pTexture->source);
                    glTF_attachAttribute(pParser, pPrimitive, "TEXCOORD_0", 2, 2, GL_FLOAT); //TUV for NM
                    glTF_attachAttribute(pParser, pPrimitive, "TANGENT", 4, 3, GL_FLOAT);
                    pDJ->uHaveBinormal = 0; //no binormal provided
                }

                setMetallicRoughness(pDJ, metallicFactor, roughnessFactor);

                /* ignore extra textures?
                tinygltf::OcclusionTextureInfo* pOcclusionTexture = &pMaterial->occlusionTexture;
                tinygltf::TextureInfo* pEmissiveTexture = &pMaterial->emissiveTexture;
                */

                //shadow renderer
                if (pMt->dropsShadow > 0) {
                    Material* pMtS = &pDJ->mtShadow;
                    memcpy(pMtS, pMt, sizeof(Material));
                    pMtS->takesShadow = 0;
                    Shader::assignShader(pMtS, "depthmap");
                }
            }
            checkGLerrors("GLTFparser::parseGLTF parsing primitive");
        }
        pMDJs->djTotalN = pParser->pDrawJobs->size() - pMDJs->djStartN;

        //Mesh -> std::vector<double> weights;  // weights to be applied to the Morph Targets
        int weightsN = pMesh->weights.size();
        if (weightsN > 0) {
            mylog("ATTN in GLTFparser::parseGLTF: pMesh->weights.size() = %d\n", pMesh->weights.size());
            /*
            for (int weightN = 0; weightN < weightsN; weightsN++) {
                double weight = pMesh->weights.at(weightN);
            }
            */
        }
    }
    pPBar->nextStep(pPBar, "meshes");

    mylog(" %d meshes / %d primitivesTotal loaded.\n", meshesN, primitivesTotal);
    mylog("\n");

    int rootNodeN = pParser->pNodes->size();
    int nodesN = pModel->nodes.size();
    //check if nodes in reversed order
    bool reversedOrder = false;
    for (int nodeN = 0; nodeN < nodesN; nodeN++) {
        tinygltf::Node* pNode = &pModel->nodes.at(nodeN);
        if (pNode->children.size() == 0)
            continue;
        int childN = pNode->children.at(0);
        if (childN > nodeN)
            reversedOrder = false;
        else
            reversedOrder = true;
        break;
    }

    //skins
    std::vector<GLTFskin*>* pGLTFskins = &GLTFskin::GLTFskins_default;
    int skinsStartN = pGLTFskins->size();
    int skinsN = pModel->skins.size();

    for (int skinN = 0; skinN < skinsN; skinN++) {
        tinygltf::Skin* pSkin = &pModel->skins.at(skinN);
        GLTFskin* pGLTFskin = new GLTFskin();
        pGLTFskins->push_back(pGLTFskin);

        //std::string name;
        //int inverseBindMatrices;  // required here but not in the spec // index of the accessor to matrices array
        //int skeleton;             // The index of the node used as a skeleton root
        //std::vector<int> joints;  // Indices of skeleton nodes

        strcpy_s(pGLTFskin->skinName, 32, pSkin->name.c_str());

        pGLTFskin->sceletonRootN = pSkin->skeleton;
        if (pGLTFskin->sceletonRootN >= 0)
            if (reversedOrder)
                pGLTFskin->sceletonRootN = nodesN - pGLTFskin->sceletonRootN - 1;
        //get inverseBindMatrices accessor
        tinygltf::Accessor* pAccessor = &pModel->accessors.at(pSkin->inverseBindMatrices);
        //inverseBindMatrices array
        pGLTFskin->jointsN = pAccessor->count;
        tinygltf::BufferView* pBufferView = &pModel->bufferViews.at(pAccessor->bufferView);
        pGLTFskin->jointsMatrices = new float[pGLTFskin->jointsN * 16];
        pGLTFskin->inverseBindMatrices = new float[pGLTFskin->jointsN * 16];
        tinygltf::Buffer* pBuf = &pModel->buffers.at(pBufferView->buffer);
        memcpy(pGLTFskin->inverseBindMatrices, &pBuf->data[pBufferView->byteOffset], pGLTFskin->jointsN * 16 * sizeof(float));

        //joints
        pGLTFskin->joints = new int[pGLTFskin->jointsN];
        for (int jN = 0; jN < pGLTFskin->jointsN; jN++) {
            int n = pSkin->joints.at(jN);
            if (reversedOrder)
                n = nodesN - n - 1;
            pGLTFskin->joints[jN] = n;
        }

        mylog("Skin %d '%s' inverseBindMatrices=%d skeleton=%d", skinN, pSkin->name.c_str(), pAccessor->count, pSkin->skeleton);
        int jointsN = pSkin->joints.size();
        mylog(", %d joints: ", jointsN);
        for (int i = 0; i < jointsN; i++)
            mylog("%d, ", pSkin->joints.at(i));
        mylog("\n");

    }

    mylog("\n");

    pPBar->nextStep(pPBar);

    //nodes: first pass - create nodes
    for (int nodeN = 0; nodeN < nodesN; nodeN++) {
        tinygltf::Node* pNode = &pModel->nodes.at(nodeN);
        /*
          int camera;  // the index of the camera referenced by this node
          std::string name;
          int skin;
          int mesh;
          std::vector<int> children;
          std::vector<double> rotation;     // length must be 0 or 4
          std::vector<double> scale;        // length must be 0 or 3
          std::vector<double> translation;  // length must be 0 or 3
          std::vector<double> matrix;       // length must be 0 or 16
          std::vector<double> weights;  // The weights of the instantiated Morph Target
        */

        mylog("Node %d '%s'", nodeN, pNode->name.c_str());
        int childsN = pNode->children.size();
        if (childsN > 0) {
            mylog(", %d children: ", childsN);
            for (int i = 0; i < childsN; i++)
                mylog("%d, ", pNode->children.at(i));
        }
        if (pNode->skin >= 0)
            mylog(", SKIN=%d", pNode->skin);
        if (pNode->mesh >= 0)
            mylog(", MESH=%d", pNode->mesh);
        mylog("\n");

        GLTFnode* pAN = newGLTFnode(pParser->pNodes, pParser->pDrawJobs, "", reversedOrder, rootNodeN);
        if (pNode->camera >= 0) {
            mylog("ATTN in GLTFparser::parseGLTF, Node->camera >= 0");
            return -1;
        }

        std::string nodeName = pNode->name;
        int nameStartPos = nodeName.find_last_of(":");
        if (nameStartPos != std::string::npos)
            nodeName = nodeName.substr(nameStartPos + 1);
        strcpy_s(pAN->name64, 64, (char*)nodeName.c_str());

        if (pNode->skin >= 0)
            pAN->skinN = pNode->skin + skinsStartN;

        //pAN->skinN = -1;

        if (pNode->mesh >= 0) {
            MeshDJs* pMDJs = pParser->meshDJs.at(pNode->mesh);
            pAN->djStartN = pMDJs->djStartN;
            pAN->djTotalN = pMDJs->djTotalN;
            DrawJob::buildGabaritesFromDrawJobs(&pAN->gabaritesOnLoad, pParser->pDrawJobs, pAN->djStartN, pAN->djTotalN);
            //reassign shaders to skinned?
            if (pAN->skinN >= 0) {
                for (int djN = 0; djN < pAN->djTotalN; djN++) {
                    DrawJob* pDJ = pAN->pDrawJobs->at(pAN->djStartN + djN);
                    setSkinnedShader(&pDJ->mt);
                    setSkinnedShader(&pDJ->mtLayer2); //pDJ->mtLayer2.shaderN = -1;// 
                    setSkinnedShader(&pDJ->mtShadow);
                }

                //pAN->skinN = -1;
            }
        }

        if (pNode->matrix.size() > 0) {
            float m[16];
            for (int i = 0; i < 16; i++)
                m[i] = pNode->matrix.at(i);
            Coords::getPositionFromMatrix(pAN->ownCoords.pos, (vec4*)m);
            quat q;
            quat_from_mat4x4(q, (vec4*)m);
            pAN->ownCoords.setQuaternion(q);

            mylog("ATTN in GLTFparser::parseGLTF, pNode->matrix.size()>0\n");
            //return -1;
        }
        if (pNode->weights.size() > 0) {
            mylog("ATTN in GLTFparser::parseGLTF, pNode->weights.size()=%d\n", pNode->weights.size());
            //return -1;
        }

        if (pNode->rotation.size() > 0) {
            quat q1;
            for (int i = 0; i < 4; i++)
                q1[i] = pNode->rotation.at(i);
            pAN->ownCoords.setQuaternion(q1);
        }
        if (pNode->scale.size() > 0) {
            for (int i = 0; i < 3; i++)
                pAN->scale[i] = pNode->scale.at(i);
        }
        if (pNode->translation.size() > 0) {
            for (int i = 0; i < 3; i++)
                pAN->ownCoords.pos[i] = pNode->translation.at(i);
        }

    }
    mylog("\n");

    if (reversedOrder) {
        for (int i = rootNodeN; i < (nodesN + rootNodeN); i++) {
            GLTFnode* pAN = (GLTFnode*)pParser->pNodes->at(i);
            pAN->nInSubjsSet = i;
        }
    }

    //second pass - children-parents
    for (int parentN = 0; parentN < nodesN; parentN++) {
        GLTFnode* pParent = (GLTFnode*)pParser->pNodes->at(parentN + rootNodeN);
        pParent->rootN = rootNodeN;

        int parentNinFile = parentN;
        if (reversedOrder)
            parentNinFile = nodesN - parentN - 1;

        tinygltf::Node* pNodeParent = &pModel->nodes.at(parentNinFile);
        int childsN = pNodeParent->children.size();
        for (int i = 0; i < childsN; i++) {
            int childNinFile = pNodeParent->children.at(i);

            int childN = childNinFile;
            if (reversedOrder)
                childN = nodesN - childNinFile - 1;

            GLTFnode* pChild = (GLTFnode*)pParser->pNodes->at(childN + rootNodeN);
            if (pChild->d2parent != 0) {
                mylog("ERROR in GLTFparser::parseGLTF, children-parents, multiple references to 1 child");
                return -1;
            }
            pChild->d2parent = childN - parentN;

        }
    }
    //root node
    SceneSubj* pNode = pParser->pNodes->at(rootNodeN);
    pNode->totalElements = nodesN;
    pNode->totalNativeElements = nodesN;

    pPBar->nextStep(pPBar);

    mylog("\n");
    /*
    //animations
    std::vector<GLTFanimation*>* pAnims = &GLTFanimation::GLTFanimations_default;
    int animsN = pModel->animations.size();
    for (int animN = 0; animN < animsN; animN++) {
        tinygltf::Animation* pAnimation = &pModel->animations.at(animN);
        GLTFanimation* pAnim = new GLTFanimation();
        pAnims->push_back(pAnim);

        //struct Animation {
        //  std::string name;
        //  std::vector<AnimationChannel> channels;
        //  std::vector<AnimationSampler> samplers;


        myStrcpy_s(pAnim->animName, 32, pAnimation->name.c_str());

        int samplersN = pAnimation->samplers.size();
        for (int sN = 0; sN < samplersN; sN++) {
            tinygltf::AnimationSampler* pASampler = &pAnimation->samplers.at(sN);

            //struct AnimationSampler {
            //  int input;                  // accessor id
            //  int output;                 // accessor id
            //  std::string interpolation;  // "LINEAR", "STEP","CUBICSPLINE" or user defined

            GLTFsampler* pS = new GLTFsampler();
            pAnim->samplers.push_back(pS);
            myStrcpy_s(pS->interpolation, 32, pASampler->interpolation.c_str());
            pS->inputN = pASampler->input;
            pS->outputN = pASampler->output;
            //load inputs?
            bool alreadyLoaded = false;
            for (int inN = pAnim->inputs.size() - 1; inN >= 0; inN--) {
                GLTFsamplerInput* pIn = pAnim->inputs.at(inN);
                if (pIn->accessorId != pS->inputN)
                    continue;
                alreadyLoaded = true;
                break;
            }
            if (!alreadyLoaded) {
                GLTFsamplerInput* pSI = new GLTFsamplerInput();
                pAnim->inputs.push_back(pSI);
                pSI->accessorId = pS->inputN;
                //Input contains times (in seconds) for each keyframe.
                tinygltf::Accessor* pAccessor = &pModel->accessors.at(pASampler->input);
                pSI->inputsN = pAccessor->count;
                pSI->input = new float[pAccessor->count];
                tinygltf::BufferView* pBufferView = &pModel->bufferViews.at(pAccessor->bufferView);
                tinygltf::Buffer* pBuf = &pModel->buffers.at(pBufferView->buffer);
                memcpy(pSI->input, &pBuf->data[pBufferView->byteOffset], pBufferView->byteLength);
            }

            //load outputs?
            alreadyLoaded = false;
            for (int oN = pAnim->outputs.size() - 1; oN >= 0; oN--) {
                GLTFsamplerOutput* pO = pAnim->outputs.at(oN);
                if (pO->accessorId != pS->inputN)
                    continue;
                alreadyLoaded = true;
                break;
            }
            if (!alreadyLoaded) {
                GLTFsamplerOutput* pSO = new GLTFsamplerOutput();
                pAnim->outputs.push_back(pSO);
                pSO->accessorId = pS->outputN;
                //Output contains values (of any Accessor.Type) for the animated property at each keyframe.
                tinygltf::Accessor* pAccessor = &pModel->accessors.at(pASampler->output);
                pSO->outputsN = pAccessor->count;
                tinygltf::BufferView* pBufferView = &pModel->bufferViews.at(pAccessor->bufferView);
                tinygltf::Buffer* pBuf = &pModel->buffers.at(pBufferView->buffer);
                pSO->output = new float[pBufferView->byteLength / sizeof(float)];
                memcpy(pSO->output, &pBuf->data[pBufferView->byteOffset], pBufferView->byteLength);

                pSO->strideInFloats = pBufferView->byteLength / sizeof(float) / pSO->outputsN;
            }

            int a = 0;
        }
        //convert in/outputN in GLTFsampler from accessors IDs to actual indices in Anim
        for (int sN = 0; sN < samplersN; sN++) {
            GLTFsampler* pS = pAnim->samplers.at(sN);
            //inputN
            for (int iN = pAnim->inputs.size() - 1; iN >= 0; iN--) {
                GLTFsamplerInput* pI = pAnim->inputs.at(iN);
                if (pI->accessorId != pS->inputN)
                    continue;
                pS->inputN = iN;
                break;
            }
            //ouputN
            for (int oN = pAnim->outputs.size() - 1; oN >= 0; oN--) {
                GLTFsamplerOutput* pO = pAnim->outputs.at(oN);
                if (pO->accessorId != pS->outputN)
                    continue;
                pS->outputN = oN;
                break;
            }
        }
        //total anim time
        pAnim->timeTotal = 0;
        for (int iN = pAnim->inputs.size() - 1; iN >= 0; iN--) {
            GLTFsamplerInput* pI = pAnim->inputs.at(iN);
            float lastTime = pI->input[pI->inputsN - 1];
            if (pAnim->timeTotal < lastTime)
                pAnim->timeTotal = lastTime;
        }

        int channelsN = pAnimation->channels.size();
        for (int cN = 0; cN < channelsN; cN++) {
            tinygltf::AnimationChannel* pAChannel = &pAnimation->channels.at(cN);

            //struct AnimationChannel {
            //  int sampler;              // required
            //  int target_node;          // optional index of the node to target (alternative target should be provided by extension)
            //  std::string target_path;  // required with standard values of ["translation", "rotation", "scale", "weights"]

            GLTFchannel* pC = new GLTFchannel();
            pAnim->channels.push_back(pC);
            pC->samplerN = pAChannel->sampler;
            pC->target_node = pAChannel->target_node;
            if(reversedOrder)
                pC->target_node = nodesN - pC->target_node - 1;
            myStrcpy_s(pC->target_path, 32, pAChannel->target_path.c_str());
        }

    }
    mylog(" %d anims loaded.\n", animsN);

    pPBar->nextStep(pPBar);
    */
    return rootNodeN;
}
int GLTFparser::glTF_attachAttribute(GLTFparser* pParser, tinygltf::Primitive* pPrimitive, std::string attribName,
    int varLocationInShader, int attributeSizeInUnits, int unitType) {

    if (pPrimitive->attributes.count(attribName) < 1)
        return 0;

    int accessorN = pPrimitive->attributes[attribName];
    tinygltf::Model* pModel = &pParser->gltf_model;
    tinygltf::Accessor* pAccessor = &pModel->accessors.at(accessorN);

    /*
    struct Accessor {
      int bufferView;  // optional in spec but required here since sparse accessor
                       // are not supported
      std::string name;
      size_t byteOffset;
      bool normalized;    // optional.
      int componentType;  // (required) One of TINYGLTF_COMPONENT_TYPE_***
      size_t count;       // required
      int type;           // (required) One of TINYGLTF_TYPE_***   ..
    */

    int byteOffset = pAccessor->byteOffset;

    int bufferViewN = pAccessor->bufferView;
    int bufferViewID = bufferIdAtGPU(pParser, bufferViewN); //VBO ID
    //bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, bufferViewID);

    tinygltf::BufferView* pBufferView = &pModel->bufferViews.at(bufferViewN);
    int byteStride = pBufferView->byteLength / pAccessor->count;

    glEnableVertexAttribArray(varLocationInShader);
    if (unitType == GL_UNSIGNED_BYTE)
        glVertexAttribIPointer(varLocationInShader, attributeSizeInUnits, unitType, byteStride, (void*)(long)byteOffset);
    else
        glVertexAttribPointer(varLocationInShader, attributeSizeInUnits, unitType, GL_FALSE, byteStride, (void*)(long)byteOffset);

    checkGLerrors("GLTFparser::glTF_attachAttribute glVertexAttribPointer");

    return 1;
}


GLTFnode* GLTFparser::newGLTFnode(std::vector<SceneSubj*>* pSubjs0, std::vector<DrawJob*>* pDrawJobs0, std::string nodeClass0,
    bool reversedOrder, int rootNodeN) {
    GLTFnode* pAN = NULL;
    if (nodeClass0.compare("") == 0)
        nodeClass0.assign("GLTFnode");

    if (nodeClass0.compare("GLTFnode") == 0)
        pAN = (new GLTFnode());

    if (pAN == NULL) {
        mylog("ERROR in GLTFparser::newGLTFnode. %s class not found\n", nodeClass0.c_str());
        return NULL;
    }
    //myStrcpy_s(pSS->source, 256, sourceFile.c_str());
    strcpy_s(pAN->className, 32, (char*)nodeClass0.c_str());
    if (pDrawJobs0 != NULL)
        pAN->pDrawJobs = pDrawJobs0;
    if (pSubjs0 != NULL) {
        pAN->pSubjsSet = pSubjs0;
        if (reversedOrder) {
            pSubjs0->insert(pSubjs0->begin() + rootNodeN, pAN);
        }
        else {//normal order
            pAN->nInSubjsSet = pSubjs0->size();
            pSubjs0->push_back(pAN);
        }
    }

    return pAN;
}

int GLTFparser::setMetallicRoughness(DrawJob* pDJ, float metallicFactor, float roughnessFactor) {
    //metallicFactor = 1;
    //roughnessFactor = 0.1;

    Material* pMt = &pDJ->mt;
    pMt->uAmbient = 0.3f + 0.4f * (1.0f - metallicFactor);

    pMt->uSpecularPowerOf = 9.0f + std::pow(1.9f * (metallicFactor + 1.0f - roughnessFactor), 3);
    pMt->uSpecularIntencity = metallicFactor * 0.7f + (1.0f - roughnessFactor) * 0.3f;
    pMt->uSpecularMinDot[0] = 0.7f + 0.25 * (1.0f - roughnessFactor);// 0.95;
    //pMt->uTranslucency = 0.5;
    Shader::assignShader(pMt, "phong");

    pMt->uBleach = ((1.0f - metallicFactor) * roughnessFactor) * 0.5f;

    //2-nd layer
    Material* pMt2 = &pDJ->mtLayer2;
    if (metallicFactor > 0.5) {
        pMt2->uAlphaFactor = std::pow(metallicFactor * (1.0f - roughnessFactor), 5);
        pMt2->uSpecularIntencity = 1;
        //pMt2->uAmbient = 1;
        pMt2->uTex0 = Texture::loadTexture("/dt/mt/wn64_blur2.bmp");
        pMt2->uTex0translateChannelN = 1;
        //pMt2->uTex3 = Texture::loadTexture("/dt/mt/chrome02dark.bmp");
        pMt2->uTex3 = Texture::loadTexture("/dt/mt/chrome01.bmp");
        pMt2->uAlphaBlending = 1;
        //scale reflected wn to blur
        pMt2->uTexMod[0] = 0;
        pMt2->uTexMod[1] = pMt2->uAlphaFactor * 0.8f + 0.5f;
    }
    else { //non metallic
        //uAlphaFactor=0.7 uTex0_src="/dt/mt/wn32_256.bmp" uTex0translateChannelN=0 
        //uTex3_src="/dt/mt/glass01_16.bmp" uAmbient=0.7 uSpecularIntencity=0 />
        pMt2->uAlphaFactor = std::powf((1.0f - metallicFactor) * (1.0f - roughnessFactor), 5) * 0.7f;
        pMt2->uSpecularIntencity = 0;
        pMt2->uAmbient = 0.7f;
        pMt2->uTex0 = Texture::loadTexture("/dt/mt/wn32_256.bmp");
        pMt2->uTex0translateChannelN = 0;
        pMt2->uTex3 = Texture::loadTexture("/dt/mt/glass01_8.bmp");
        pMt2->uAlphaBlending = 1;
    }
    if (pMt2->uAlphaFactor > 0.09f)
        Shader::assignShader(pMt2, "mirror");
    else
        pMt2->shaderN = -1;
    /*
    mylog("\nmetallicFactor=%f roughnessFactor=%f uAmbient=%f uBleach=%f\n", metallicFactor, roughnessFactor, pMt->uAmbient, pMt->uBleach);
    mylog("uSpecularPowerOf=%f Intencity=%f MinDot=%f pMt2->uAlphaFactor=%f\n", pMt->uSpecularPowerOf, pMt->uSpecularIntencity, pMt->uSpecularMinDot[0], pMt2->uAlphaFactor);
    if (pMt->uTex2nm >= 0) //has normal map
        mylog("     with NM\n");
    */
    return 1;
}
void GLTFparser::buildBoundingBoxFromVerts(Gabarites* pGB, GLTFparser* pParser, tinygltf::Primitive* pPrimitive) {
    //Gabarites* pGB = &pDJ->gabarites;
    int accessorN = pPrimitive->attributes["POSITION"];
    tinygltf::Model* pModel = &pParser->gltf_model;
    tinygltf::Accessor* pAccessor = &pModel->accessors.at(accessorN);
    for (int i = 0; i < 3; i++) {
        pGB->bbMin[i] = pAccessor->minValues.at(i);
        pGB->bbMax[i] = pAccessor->maxValues.at(i);
    }
    Gabarites::adjustMidRad(pGB);
}
int GLTFparser::bufferIdAtGPU(GLTFparser* pParser, int bufferViewN) {
    //bufferViews -  A bufferView describes a “chunk” or a “slice” of the whole raw buffer data, such as VBO or EBO
    tinygltf::Model* pModel = &pParser->gltf_model;
    tinygltf::BufferView* pBufferView = &pModel->bufferViews.at(bufferViewN);
    int bufferViewID = DrawJob::newBufferId(pParser->pBuffersIds);
    //locate data buffer
    tinygltf::Buffer* pBuffer = &pModel->buffers.at(pBufferView->buffer);

    glBindBuffer(GL_ARRAY_BUFFER, bufferViewID); // A buffer must be bound to be used
    glBufferData(GL_ARRAY_BUFFER, pBufferView->byteLength, &pBuffer->data[pBufferView->byteOffset], GL_STATIC_DRAW); // Assuming data is a std::vector containing some data
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Generally a good idea to cleanup the binding point after

    checkGLerrors("GLTFparser::bufferIdAtGPU");

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Generally a good idea to cleanup the binding point after
    return bufferViewID;
}
int GLTFparser::setSkinnedShader(Material* pMt) {
    if (pMt->shaderN < 0)
        return 0;
    Shader::assignShader(pMt, "skin " + std::string(pMt->shaderType32));
    return 1;
}

int GLTFparser::parseGLTFanimations(GLTFparser* pParser, ProgressBar* pPBar, int useSkeletonMapN) {
    tinygltf::Model* pModel = &pParser->gltf_model;

    std::vector<GLTFanimation*>* pAnims = &GLTFanimation::GLTFanimations_default;
    int firstAnimN = pAnims->size();

    if (useSkeletonMapN < 0)
        useSkeletonMapN = parseGLTFskeletonMap(pParser, pPBar);

    //animations
    int animsN = pModel->animations.size();
    for (int animN = 0; animN < animsN; animN++) {
        tinygltf::Animation* pAnimation = &pModel->animations.at(animN);
        GLTFanimation* pAnim = new GLTFanimation();
        pAnims->push_back(pAnim);

        pAnim->animSkeletonMapN = useSkeletonMapN;

        //struct Animation {
        //  std::string name;
        //  std::vector<AnimationChannel> channels;
        //  std::vector<AnimationSampler> samplers;

        strcpy_s(pAnim->animName, 32, pAnimation->name.c_str());

        int samplersN = pAnimation->samplers.size();
        for (int sN = 0; sN < samplersN; sN++) {
            tinygltf::AnimationSampler* pASampler = &pAnimation->samplers.at(sN);

            //struct AnimationSampler {
            //  int input;                  // accessor id
            //  int output;                 // accessor id
            //  std::string interpolation;  // "LINEAR", "STEP","CUBICSPLINE" or user defined

            GLTFsampler* pS = new GLTFsampler();
            pAnim->samplers.push_back(pS);
            strcpy_s(pS->interpolation, 32, pASampler->interpolation.c_str());
            pS->inputN = pASampler->input;
            pS->outputN = pASampler->output;
            //load inputs?
            bool alreadyLoaded = false;
            for (int inN = pAnim->inputs.size() - 1; inN >= 0; inN--) {
                GLTFsamplerInput* pIn = pAnim->inputs.at(inN);
                if (pIn->accessorId != pS->inputN)
                    continue;
                alreadyLoaded = true;
                break;
            }
            if (!alreadyLoaded) {
                GLTFsamplerInput* pSI = new GLTFsamplerInput();
                pAnim->inputs.push_back(pSI);
                pSI->accessorId = pS->inputN;
                //Input contains times (in seconds) for each keyframe.
                tinygltf::Accessor* pAccessor = &pModel->accessors.at(pASampler->input);
                pSI->inputsN = pAccessor->count;
                pSI->input = new float[pAccessor->count];
                tinygltf::BufferView* pBufferView = &pModel->bufferViews.at(pAccessor->bufferView);
                tinygltf::Buffer* pBuf = &pModel->buffers.at(pBufferView->buffer);
                memcpy(pSI->input, &pBuf->data[pBufferView->byteOffset], pBufferView->byteLength);
            }

            //load outputs?
            alreadyLoaded = false;
            for (int oN = pAnim->outputs.size() - 1; oN >= 0; oN--) {
                GLTFsamplerOutput* pO = pAnim->outputs.at(oN);
                if (pO->accessorId != pS->inputN)
                    continue;
                alreadyLoaded = true;
                break;
            }
            if (!alreadyLoaded) {
                GLTFsamplerOutput* pSO = new GLTFsamplerOutput();
                pAnim->outputs.push_back(pSO);
                pSO->accessorId = pS->outputN;
                //Output contains values (of any Accessor.Type) for the animated property at each keyframe.
                tinygltf::Accessor* pAccessor = &pModel->accessors.at(pASampler->output);
                pSO->outputsN = pAccessor->count;
                tinygltf::BufferView* pBufferView = &pModel->bufferViews.at(pAccessor->bufferView);
                tinygltf::Buffer* pBuf = &pModel->buffers.at(pBufferView->buffer);
                pSO->output = new float[pBufferView->byteLength / sizeof(float)];
                memcpy(pSO->output, &pBuf->data[pBufferView->byteOffset], pBufferView->byteLength);

                pSO->strideInFloats = pBufferView->byteLength / sizeof(float) / pSO->outputsN;
            }

        }
        //convert in/outputN in GLTFsampler from accessors IDs to actual indices in Anim
        for (int sN = 0; sN < samplersN; sN++) {
            GLTFsampler* pS = pAnim->samplers.at(sN);
            //inputN
            for (int iN = pAnim->inputs.size() - 1; iN >= 0; iN--) {
                GLTFsamplerInput* pI = pAnim->inputs.at(iN);
                if (pI->accessorId != pS->inputN)
                    continue;
                pS->inputN = iN;
                break;
            }
            //ouputN
            for (int oN = pAnim->outputs.size() - 1; oN >= 0; oN--) {
                GLTFsamplerOutput* pO = pAnim->outputs.at(oN);
                if (pO->accessorId != pS->outputN)
                    continue;
                pS->outputN = oN;
                break;
            }
        }
        //total anim time
        pAnim->timeTotal = 0;
        for (int iN = pAnim->inputs.size() - 1; iN >= 0; iN--) {
            GLTFsamplerInput* pI = pAnim->inputs.at(iN);
            float lastTime = pI->input[pI->inputsN - 1];
            if (pAnim->timeTotal < lastTime)
                pAnim->timeTotal = lastTime;
        }

        int channelsN = pAnimation->channels.size();
        for (int cN = 0; cN < channelsN; cN++) {
            tinygltf::AnimationChannel* pAChannel = &pAnimation->channels.at(cN);

            //struct AnimationChannel {
            //  int sampler;              // required
            //  int target_node;          // optional index of the node to target (alternative target should be provided by extension)
            //  std::string target_path;  // required with standard values of ["translation", "rotation", "scale", "weights"]

            GLTFchannel* pC = new GLTFchannel();
            pAnim->channels.push_back(pC);
            pC->samplerN = pAChannel->sampler;
            pC->target_node = pAChannel->target_node;
            strcpy_s(pC->target_path, 32, pAChannel->target_path.c_str());
        }
        pPBar->nextStep(pPBar);
    }
    mylog(" %d anims loaded.\n", animsN);
    return firstAnimN;
}
int GLTFparser::loadAnimations(std::string sourceFile, ProgressBar* pPBar, int useSkeletonMapN) {

    GLTFparser* pParser = new GLTFparser();

    readGLB(pParser, sourceFile);
    pPBar->nextStep(pPBar, "readGLB");

    int firstAnimN = parseGLTFanimations(pParser, pPBar, useSkeletonMapN);
    delete pParser;

    return firstAnimN;
}

int GLTFparser::parseGLTFskeletonMap(GLTFparser* pParser, ProgressBar* pPBar) {
    tinygltf::Model* pModel = &pParser->gltf_model;

    std::vector<GLTFanim2nodesMap*>* pSMaps = &GLTFanim2nodesMap::anim2nodesMaps_default;
    GLTFanim2nodesMap* pSMap = new GLTFanim2nodesMap();
    pSMap->animSkeletonMapN = pSMaps->size();
    pSMaps->push_back(pSMap);
    std::vector<GLTFanim2nodesBone*>* pBones = &pSMap->anim2nodesBones;
    int nodesN = pModel->nodes.size();
    //nodes: first pass - create nodes
    for (int nodeN = 0; nodeN < nodesN; nodeN++) {
        tinygltf::Node* pNode = &pModel->nodes.at(nodeN);
        GLTFanim2nodesBone* pBone = new GLTFanim2nodesBone();
        pBone->animBoneN = pBones->size();
        pBones->push_back(pBone);
        /*
          int camera;  // the index of the camera referenced by this node
          std::string name;
          int skin;
          int mesh;
          std::vector<int> children;
          std::vector<double> rotation;     // length must be 0 or 4
          std::vector<double> scale;        // length must be 0 or 3
          std::vector<double> translation;  // length must be 0 or 3
          std::vector<double> matrix;       // length must be 0 or 16
          std::vector<double> weights;  // The weights of the instantiated Morph Target
        */

        std::string nodeName = pNode->name;
        int nameStartPos = nodeName.find_last_of(":");
        if (nameStartPos != std::string::npos)
            nodeName = nodeName.substr(nameStartPos + 1);
        strcpy_s(pBone->name, 64, (char*)nodeName.c_str());

        if (pNode->matrix.size() > 0) {
            float m[16];
            for (int i = 0; i < 16; i++)
                m[i] = pNode->matrix.at(i);
            Coords::getPositionFromMatrix(pBone->ownCoords.pos, (vec4*)m);
            quat q;
            quat_from_mat4x4(q, (vec4*)m);
            pBone->ownCoords.setQuaternion(q);
        }
        if (pNode->rotation.size() > 0) {
            quat q1;
            for (int i = 0; i < 4; i++)
                q1[i] = pNode->rotation.at(i);
            pBone->ownCoords.setQuaternion(q1);
        }
        if (pNode->scale.size() > 0) {
            for (int i = 0; i < 3; i++)
                pBone->ownScale[i] = pNode->scale.at(i);
        }
        if (pNode->translation.size() > 0) {
            for (int i = 0; i < 3; i++)
                pBone->ownCoords.pos[i] = pNode->translation.at(i);
        }
        pBone->boneLever = v3length(pBone->ownCoords.pos);

        mylog("Bone %d '%s' lever=%f\n", nodeN, pBone->name, pBone->boneLever);
    }
    pPBar->nextStep(pPBar);

    mylog("skeletalMapN %d loaded.\n", pSMap->animSkeletonMapN);

    return pSMap->animSkeletonMapN;
}

