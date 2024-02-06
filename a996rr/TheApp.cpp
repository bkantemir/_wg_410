#include "TheApp.h"
#include "platform.h"
#include "utils.h"
#include "linmath.h"
#include "Texture.h"
#include "DrawJob.h"
#include "ModelBuilder.h"
#include "TexCoords.h"
#include "rr/ModelLoaderRR.h"
#include "Shadows.h"
#include "ProgressBar.h"
//#include "gltf/GLTFparser.h"
//#include "gltf/GLTFskinShader.h"
#include "model_car/CarWheel.h"
#include "rr/RollingStock.h"
#include "rr/WheelPair.h"
#include "rr/Gear.h"
#include "rr/Bell.h"
#include "rr/Whistle.h"
#include "rr/Coupler.h"
#include "DirLight.h"

extern std::string filesRoot;
extern float degrees2radians;

std::vector<SceneSubj*> TheApp::sceneSubjs;
std::vector<SceneSubj*> TheApp::staticSubjs;
Camera TheApp::collisionCamera;

int TheApp::getReady() {

    Shader::loadBasicShaders();
    UISubj::init();

    ProgressBar* pPBar = new ProgressBar(filesRoot + "/dt/cfg/progressbar00.bin");

    bExitApp = false;
    frameN = 0;
    Shader::loadShaders(pPBar);
    //GLTFskinShader::loadGLTFshaders(pPBar);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);//default
    glFrontFace(GL_CCW);//default

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);

    float carSize = 70;// 130;// 55;//
    float tileSize = round(carSize * 0.8 / 5) * 5; //36 minimum
    RollingStock::maxTrainSpeed = tileSize * 0.02;
    gameTable.initTable(tileSize, 100, 20, 5, 5);
    gameTable.initRailModels();
    gameTable.initRailModelsVirtual();
    gameTable.basicOval();
    gameTable.initVirtualNet();

    int subjN = 0;
    SceneSubj* pSS = NULL;

    /*
    float scale = 64.0f / 150;
    subjN = ModelLoader::loadModelStandard(&sceneSubjs, NULL, NULL, "/dt/md/cars/998_1929_deusenberg_j/root01.txt", "", "", pPBar);
    pSS = sceneSubjs.at(subjN);
    pSS->scaleMe(scale);
    pSS->ownCoords.setEulerDg(0, getRandom(0.0f, 360.0f), 0);
    pSS->deployModel(pSS, "tableAt='2.5,2.5'");
    pSS->ownSpeed.setEulerDg(0, 1, 0);
    */
    std::vector<SceneSubj*>* pSubjs = &sceneSubjs;

    std::vector<std::string> locoOpts;
    locoOpts.push_back("/dt/md/rr/us/lc/999_0-6-0dockside/01reading/root01.txt");
    locoOpts.push_back("/dt/md/rr/us/lc/999_0-6-0dockside/02atsf/root01.txt");

    //locoOpts.push_back("/dt/md/rr/us/lc/998_alc-42/01amtrak_blue/root01.txt");

    subjN = ModelLoaderRR::loadModelRR(pSubjs, NULL, NULL, locoOpts.at(getRandom(0, locoOpts.size() - 1)), "","RollingStock", "", pPBar);
    pSS = pSubjs->at(subjN);
    gameTable.placeAt(pSS->ownCoords.pos, getRandom(0.0f, (float)gameTable.tableTiles[0]), gameTable.groundLevel, getRandom(0.0f, (float)gameTable.tableTiles[1]));//Rail::railsLevel
    mylog("RollingStock at %dx%d\n", (int)pSS->ownCoords.pos[0], (int)pSS->ownCoords.pos[2]);
    pSS->ownCoords.setEulerDg(0, getRandom(0.0f, 360.0f), 0);
    pSS->deployModel(pSS, "");
    pSS->desirableZdir = 1;
    if (getRandom(0, 1) == 0)
        pSS->desirableZdir *= -1;
    RollingStock* pLoco = (RollingStock*)pSS;
    pLoco->activeLoco = true;
    RollingStock::reinspectTrain(pLoco->pCouplerFront);

    
    std::vector<std::string> wagonOpts;
    wagonOpts.push_back("/dt/md/rr/us/fr/999_tank3dome/01gulf/root01.txt");
    wagonOpts.push_back("/dt/md/rr/us/fr/999_tank3dome/02goodyear/root01.txt");
    wagonOpts.push_back("/dt/md/rr/us/fr/999_tank3dome/03protex/root01.txt");
    for (int i = 0; i < 6; i++) {
        subjN = ModelLoaderRR::loadModelRR(pSubjs, NULL, NULL, wagonOpts.at(getRandom(0, wagonOpts.size() - 1)), "","RollingStock", "", pPBar);
        pSS = pSubjs->at(subjN);
        gameTable.placeAt(pSS->ownCoords.pos, getRandom(0.0f, (float)gameTable.tableTiles[0]), gameTable.groundLevel, getRandom(0.0f, (float)gameTable.tableTiles[1]));//Rail::railsLevel
        mylog("RollingStock at %dx%d\n", (int)pSS->ownCoords.pos[0], (int)pSS->ownCoords.pos[2]);
        pSS->ownCoords.setEulerDg(0, getRandom(0.0f, 360.0f), 0);
        pSS->deployModel(pSS, "");
        pSS->ownSpeed.pos[2] = 1;
        if (getRandom(0, 1) == 0)
            pSS->ownSpeed.pos[2] = -1;
    }
    
    //pSS->setHighLight(0.2, MyColor::getUint32(1.0f, 0.0f, 1.0f));
     
    //showHierarchy(pSS);

    pSubjs = &staticSubjs;

    std::vector<std::string> carOpts;
    carOpts.push_back("/dt/md/cars/us/999_1935_deusenberg_ssj/root01.txt");
    carOpts.push_back("/dt/md/cars/us/998_1929_deusenberg_j/root01.txt");

    float scale = 64.0f / 150;
    scale *= 1.1;
    subjN = ModelLoader::loadModelStandard(pSubjs, NULL, NULL, carOpts.at(getRandom(0, carOpts.size() - 1)), "", "", "", pPBar);
    pSS = pSubjs->at(subjN);
    pSS->scaleMe(scale);
    pSS->ownCoords.setEulerDg(0, getRandom(0.0f, 360.0f), 0);
    pSS->deployModel(pSS,"tableAt='3.5,2.5'");

    subjN = ModelLoader::loadModelStandard(pSubjs, NULL, NULL, carOpts.at(getRandom(0, carOpts.size() - 1)), "", "", "", pPBar);
    pSS = pSubjs->at(subjN);
    pSS->scaleMe(scale);
    pSS->ownCoords.setEulerDg(0, getRandom(0.0f, 360.0f), 0);
    pSS->deployModel(pSS, "tableAt='2.5,2.5'");

    subjN = ModelLoader::loadModelStandard(pSubjs, NULL, NULL, carOpts.at(getRandom(0, carOpts.size() - 1)), "", "", "", pPBar);
    pSS = pSubjs->at(subjN);
    pSS->scaleMe(scale);
    pSS->ownCoords.setEulerDg(0, getRandom(0.0f, 360.0f), 0);
    pSS->deployModel(pSS, "tableAt='1,4'");

    subjN = ModelLoader::loadModelStandard(pSubjs, NULL, NULL, "/dt/md/misc/us/999_marlboro01red/root00.txt", "", "", "", pPBar);
    pSS = pSubjs->at(subjN);
    pSS->ownCoords.setEulerDg(0, getRandom(0, 360), 0);
    pSS->deployModel(pSS,"tableAt='1.5,2' align=bottom");
    
    //===== set up light
    //v3set(dirToMainLight, getRandom(-0.7f, 0.7f), 1, getRandom(-0.1f, 0.7f));
    //v3set(dirToMainLight, 1, 1, 0.5);
    v3set(dirToMainLight, 0.3, 0.7, 0.8);
    v3norm(dirToMainLight);

    Shadows::init();
    Camera::setCollisionCamera(&collisionCamera, &gameTable.worldBox);
    //===== set up MAIN camera
    mainCamera.ownCoords.setEulerDg(30, 165, 0); //set camera angles/orientation
    //mainCamera.ownCoords.setEulerDg(getRandom(20, 45), getRandom(0, 360), 0); //set camera angles/orientation
    mat4x4_from_quat(mainCamera.ownCoords.rotationMatrix, mainCamera.ownCoords.getRotationQuat());
    mainCamera.viewRangeDg = 30;
    mainCamera.stageSize[0] = gameTable.suggestedStageSize * 0.8;
    mainCamera.stageSize[1] = mainCamera.stageSize[0] * 0.55;// 120;// 45;
    mainCamera.lookAtPoint[1] = Rail::railsLevel - gameTable.tileSize * 0.3;
    mainCamera.reset(&mainCamera, &gameTable.worldBox);

    pPBar->nextStep(pPBar);
    UISubj::cleanUp();

    //=============arrays to process
    pSubjArrays2process.push_back(&sceneSubjs);
    pSubjArrays2process.push_back(&staticSubjs);
    
    pSubjArrays2draw.push_back(&sceneSubjs);
    pSubjArrays2draw.push_back(&staticSubjs);
    pSubjArrays2draw.push_back(&gameTable.tableParts);
    pSubjArrays2draw.push_back(&gameTable.railsMap);

    pSubjArraysPairs4collisionDetection.push_back(&sceneSubjs);
    pSubjArraysPairs4collisionDetection.push_back(&sceneSubjs);
    //======= set UI
    //UISubj::addZBufferSubj("test", 20, 10, 600, 600, "top left", Shadows::depthMapTexN);

     return 1;
}

int TheApp::drawFrame() {
    checkGameStatus();

    myPollEvents(); 

    mat4x4_invert(collisionMViewInversed, collisionCamera.lookAtMatrix);
    
    //scan subjects
    for (int arrayN = 0; arrayN < pSubjArrays2process.size(); arrayN++) {
        std::vector<SceneSubj*>* pSubjs = pSubjArrays2process.at(arrayN);
        int subjsN = pSubjs->size();
        for (int subjN = 0; subjN < subjsN; subjN++) {
            SceneSubj* pSS = pSubjs->at(subjN);
            if (pSS == NULL)
                continue;
            if (pSS->hidden() > 0)
                continue;

            pSS->processSubj();

            //prepare for collision detection
            if (pSS->djTotalN > 0)
                if (pSS->gabaritesWorld.chordType >= 0)
                    if (pSS->pSubjsSet != &staticSubjs) {
                        Gabarites::fillGabarites(&pSS->gabaritesWorld, pSS->absModelMatrix, &pSS->gabaritesOnLoad, collisionCamera.lookAtMatrix, NULL);
                    }
        }
    }
    if (frameN > 0) {
        RollingStock::checkTrains(&sceneSubjs);
        SceneSubj::checkCollisions(pSubjArraysPairs4collisionDetection);
    }


    //translucent light
    float vecToLightInEyeSpace[4];
    mat4x4_mul_vec4plus(vecToLightInEyeSpace, (vec4*)mainCamera.mViewProjection, dirToMainLight, 0);
    float dirToTranslucent[3];
    if (vecToLightInEyeSpace[2] <= 0) { //light from behind
        for (int i = 0; i < 3; i++) {
            if (i == 2)
                dirToTranslucent[i] = dirToMainLight[i];
            else
                dirToTranslucent[i] = -dirToMainLight[i];
        }
    }
    else { //light to eyes
        for (int i = 0; i < 3; i++) {
            dirToTranslucent[i] = -dirToMainLight[i];
        }
    }
    dirToTranslucent[2] *= 0.5;
    v3norm(dirToTranslucent);

    int arraysN = pSubjArrays2draw.size();
    for (int nA = 0; nA < arraysN; nA++) {
        std::vector<SceneSubj*>* pSubjs = pSubjArrays2draw.at(nA);
        int subjsN = pSubjs->size();
        //calculate screen coords/gabarites
        for (int subjN = 0; subjN < subjsN; subjN++) {
            SceneSubj* pSS = pSubjs->at(subjN);
            if (pSS == NULL)
                continue;
            if (pSS->hidden() > 0)
                continue;
            Gabarites::fillGabarites(&pSS->gabaritesOnScreen, pSS->absModelMatrix, &pSS->gabaritesOnLoad, mainCamera.mViewProjection, mainCamera.targetRads);
        }
    }

    Shadows::addToShadowsQueue(pSubjArrays2draw);
    Shadows::renderDepthMap();
			checkGLerrors("after renderDepthMap");
 
    //set render to screen
    glViewport(0, 0, mainCamera.targetDims[0], mainCamera.targetDims[1]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDepthMask(GL_TRUE);
    //glClearColor(0.0, 0.0, 0.0, 1.0);
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //sort for rendering
    std::vector<SceneSubj*> renderQueueOpaque;
    std::vector<SceneSubj*> renderQueueTransparent;
    arraysN = pSubjArrays2draw.size();
    for (int nA = 0; nA < arraysN; nA++) {
        std::vector<SceneSubj*>* pSubjs = pSubjArrays2draw.at(nA);
        SceneSubj::addToRenderQueue(&renderQueueOpaque, &renderQueueTransparent, pSubjs);
    }
    //draw opaque subjects
    SceneSubj::sortRenderQueue(&renderQueueOpaque, 0); //0-closer 1st
    int subjsN = renderQueueOpaque.size();
		//mylog("frame%d, %d opaques\n",frameN,subjsN);
    for (int subjN = 0; subjN < subjsN; subjN++) {
        SceneSubj* pSS = renderQueueOpaque.at(subjN);
        pSS->render(&mainCamera, dirToMainLight, dirToTranslucent, 0); //0 - render opaque only
 			checkGLerrors("after lrender opaque");
   }
    renderQueueOpaque.clear();

    //draw transparent subjects
    SceneSubj::sortRenderQueue(&renderQueueTransparent, 1); //1-farther 1st
    subjsN = renderQueueTransparent.size();
		//mylog("%d tranparents\n",subjsN);
    for (int subjN = 0; subjN < subjsN; subjN++) {
        SceneSubj* pSS = renderQueueTransparent.at(subjN);
        pSS->render(&mainCamera, dirToMainLight, dirToTranslucent, 1); //1 - render transparent only
 			checkGLerrors("after lrender transparent");
    }
    renderQueueTransparent.clear();

    UISubj::renderAll();
			//mylog("after render UI\n");
			

    //synchronization
    while (1) {
        uint64_t currentMillis = getSystemMillis();
        uint64_t millisSinceLastFrame = currentMillis - lastFrameMillis;
        if (millisSinceLastFrame >= millisPerFrame) {
            lastFrameMillis = currentMillis;
            break;
        }
    }
    mySwapBuffers();
    frameN++;
    return 1;
}

int TheApp::cleanUp() {
    int itemsN = sceneSubjs.size();
    //delete all UISubjs
    for (int i = 0; i < itemsN; i++) {
        SceneSubj* pSS = sceneSubjs.at(i);
        if(pSS!=NULL)
            delete pSS;
    }
    sceneSubjs.clear();
    //clear all other classes
    MaterialAdjust::cleanUp();
    Texture::cleanUp();
    Shader::cleanUp();
    DrawJob::cleanUp();
    MyColor::cleanUp();
    UISubj::cleanUp();
    gameTable.cleanUp();
    return 1;
}
int TheApp::onScreenResize(int width, int height) {
			//mylog("onScreenResize %d x %d\n",width,height);
    if (mainCamera.targetDims[0] == width && mainCamera.targetDims[1] == height)
        return 0;
    if (gameTable.worldBox.boxRad != 0) {
        Camera::onTargetResize(&mainCamera, width, height, &gameTable.worldBox);
        Camera::onTargetResize(&collisionCamera, width, height, &gameTable.worldBox);
    }
    UISubj::onScreenResize(width, height);
    mylog(" screen size %d x %d\n", width, height);
    return 1;
}

int TheApp::run() {
    {//random pick
        int choice = getRandom(0, 6);
        if (choice == 0) { //ebay
            printf("ebay.");
            choice = getRandom(0, 4);
            if (choice == 0) printf("uk ");
            else if (choice == 1) printf("de ");
            else printf("com ");
            printf("page %d item %d", getRandom(1, 10), getRandom(1, 10));
        }
        else if (choice == 1) { //pict
            printf("picts ");
            for (int lvN = 0; lvN < 6; lvN++) {
                printf("%d ", getRandom(1, 10));
            }
        }
        else { //real
            std::vector<std::string> terms;
            terms.push_back("top box,mid box,lower box,table");
            terms.push_back("South,SE,East,NE,North,NW,West,SW,center");
            terms.push_back("South,SE,East,NE,North,NW,West,SW,center");
            terms.push_back("layer 1,layer 1,layer 2,layer 3,junk,bulk");
            for (int lvN = 0; lvN < terms.size(); lvN++) {
                std::vector<std::string>* pOpts = splitString(terms.at(lvN), ",");
                int optN = getRandom(0, pOpts->size() - 1);
                printf("%s -> ", pOpts->at(optN).c_str());
            }
        }
        printf("\n");
    }

    getReady();
    /*
    //10 sec delay
    lastFrameMillis = getSystemMillis();
    while (1) {
        uint64_t currentMillis = getSystemMillis();
        uint64_t millisSinceLastFrame = currentMillis - lastFrameMillis;
        if (millisSinceLastFrame >= 10000) {
            lastFrameMillis = currentMillis;
            break;
        }
    }
    */
    while (!bExitApp) {
        drawFrame();
    }
    cleanUp();
    return 1;
}
void TheApp::mylogObjSize(std::string objName, int objSize) {
    mylog("%s %d", objName.c_str(), objSize);
    int divider = 2;
    while (1) {
        int rest = objSize % divider;
        if (rest != 0) {
            mylog(" dividable by %d\n", divider / 2);
            break;
        }
        divider *= 2;
    }
}
void TheApp::showHierarchy(SceneSubj* pSS) {
    //std::vector<SceneSubj*>* pSubjsSet = pSS->pSubjsSet;
    int subjN = pSS->nInSubjsSet;
    int acanTo = subjN+pSS->totalElements;
    mylog("------------%s-----\n", pSS->source256);
    for (int elN = subjN; elN < acanTo; elN++) {
        int eN = elN;
        SceneSubj* pSS0 = sceneSubjs.at(eN);
        mylog("<%d:%s:%s:%s>", elN, pSS0->className, pSS0->name64, pSS0->source256);
        if (pSS0->d2parent != 0) {
            int pN = pSS0->nInSubjsSet - pSS0->d2parent;
            SceneSubj* pParent = sceneSubjs.at(pN);
            mylog(" parent <%d:%s:%s>",pN, pParent->className, pParent->name64);

            int a = 0;
        }
        else
            mylog(" root");
        mylog("\n");
    }
    mylog("/////////////////////////////////////\n");
}

SceneSubj* TheApp::newSceneSubj(std::string subjClass, std::string sourceFile,
    std::vector<SceneSubj*>* pSubjsSet0, std::vector<DrawJob*>* pDrawJobs0) {
    SceneSubj* pSS = NULL;
    if (subjClass.compare("") == 0)
        pSS = (new SceneSubj());
    else if (subjClass.find("Car") == 0) {
        if (subjClass.compare("CarWheel") == 0)
            pSS = (new CarWheel());
    }
    else if (subjClass.compare("coupler") == 0)
        pSS = (new Coupler());
    else if (subjClass.compare("whistle") == 0)
        pSS = (new Whistle());
    else if (subjClass.compare("bell") == 0)
        pSS = (new Bell());
    else if (subjClass.compare("DirLight") == 0)
        pSS = (new DirLight());
    else if (subjClass.compare("Gear") == 0)
        pSS = (new Gear());
    else if (subjClass.compare("OnRails") == 0)
        pSS = (new OnRails());
    else if (subjClass.compare("WheelPair") == 0)
        pSS = (new WheelPair());
    else if (subjClass.find("RollingStock") == 0) {
        if (subjClass.compare("RollingStock") == 0)
            pSS = (new RollingStock());
    }
    if (pSS == NULL) {
        mylog("ERROR in TheProject::newSceneSubj. %s class not found\n", subjClass.c_str());
        return NULL;
    }
    strcpy_s(pSS->className, 32, subjClass.c_str());
    strcpy_s(pSS->source256, 256, sourceFile.c_str());
    if (pDrawJobs0 != NULL)
        pSS->pDrawJobs = pDrawJobs0;
    if (pSubjsSet0 != NULL) {
        pSS->nInSubjsSet = pSubjsSet0->size();
        pSS->pSubjsSet = pSubjsSet0;
        pSubjsSet0->push_back(pSS);
    }
    return pSS;
}
int TheApp::checkGameStatus() {
    int wagonsN = 7;
    //RollingStock* pWagon = (RollingStock*)sceneSubjs.at(0);
    RollingStock* pTrainRoot = (RollingStock*)sceneSubjs.at(0); // (RollingStock*)sceneSubjs.at(pWagon->trainRootN);
    Train* pTrain = &pTrainRoot->tr;
    RollingStock* pLoco = pTrainRoot;// (RollingStock*)sceneSubjs.at(0);

    if (pTrain->carsTotal == wagonsN) {
        //all cars collected
        Coupler* pCpFront = pLoco->pCouplerFront;
        if (pCpFront->connected < 1) {
            //no cars ahead
            if (pLoco->desirableZdir > 0)
                return 1; //moving forward
            else {
                pLoco->desirableZdir = 1;
                return 1; //go forward
            }
        }
        else {
            //have cars in front - stop
            if (pLoco->desirableZdir != 0) {
                pLoco->desirableZdir = 0;
                return 1;
            }
            else {//train about to stop
                if (pTrainRoot->ownSpeed.pos[2] == 0) {
                    //stopped
                    RollingStock::decouple(pCpFront);
                    //mylog("<<<%d decouple %d:%d\n", (int)frameN, pCpFront->rootN, pCpFront->alignedWithRoot);
                    pLoco->desirableZdir = -1;
                    return 1;
                }
                else {
                    //keeps backing
                    return 1;
                }
            }
        }
    }
    else {
        //not all cars collected - keep moving
        if (pLoco->desirableZdir == 0) {
            pLoco->desirableZdir = -1;
            return 1;
        }
    }
    return 1;
}

