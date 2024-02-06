#define TINYGLTF_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "gltf/GLTFparser.h"

#include "platform.h"
#include <jni.h>
#include <EGL/egl.h>

#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>
#include <game-activity/native_app_glue/android_native_app_glue.c>

#include "TheApp.h"

#include <string>
#include <vector>
#include <sys/stat.h> //mkdir for Android

std::string filesRoot;

TheApp theApp;

struct android_app* pAndroidApp = NULL;
EGLDisplay androidDisplay = EGL_NO_DISPLAY;
EGLSurface androidSurface = EGL_NO_SURFACE;
EGLContext androidContext = EGL_NO_CONTEXT;
bool bExitApp = false;
int screenSize[2] = {0,0};

void android_init_display() {
    // Choose your render attributes
    constexpr EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };
    // The default display is probably what you want on Android
    auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, nullptr, nullptr);

    // figure out how many configs there are
    EGLint numConfigs;
    eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);

    // get the list of configurations
    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);

    // Find a config we like.
    // Could likely just grab the first if we don't care about anything else in the config.
    // Otherwise hook in your own heuristic
    auto config = *std::find_if(
            supportedConfigs.get(),
            supportedConfigs.get() + numConfigs,
            [&display](const EGLConfig &config) {
                EGLint red, green, blue, depth;
                if (eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red)
                    && eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &green)
                    && eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blue)
                    && eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depth)) {

                    //aout << "Found config with " << red << ", " << green << ", " << blue << ", "
                    //     << depth << std::endl;
                    return red == 8 && green == 8 && blue == 8 && depth == 24;
                }
                return false;
            });
    // create the proper window surface
    EGLint format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    EGLSurface surface = eglCreateWindowSurface(display, config, pAndroidApp->window, nullptr);

    // Create a GLES 3 context
    EGLint contextAttribs[] = {
            EGL_CONTEXT_MAJOR_VERSION, 3,
            EGL_CONTEXT_MINOR_VERSION, 2,
            EGL_NONE};
    EGLContext context = eglCreateContext(display, config, nullptr, contextAttribs);

    // get some window metrics
    auto madeCurrent = eglMakeCurrent(display, surface, surface, context);
    if(!madeCurrent) {
        ;
    }
    androidDisplay = display;
    androidSurface = surface;
    androidContext = context;
}
void android_term_display() {
    if (androidDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(androidDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (androidContext != EGL_NO_CONTEXT) {
            eglDestroyContext(androidDisplay, androidContext);
            androidContext = EGL_NO_CONTEXT;
        }
        if (androidSurface != EGL_NO_SURFACE) {
            eglDestroySurface(androidDisplay, androidSurface);
            androidSurface = EGL_NO_SURFACE;
        }
        eglTerminate(androidDisplay);
        androidDisplay = EGL_NO_DISPLAY;
    }
}

void handle_cmd(android_app *pApp, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            android_init_display();
            break;
        case APP_CMD_TERM_WINDOW:
            android_term_display();
            break;
        default:
            break;
    }
}
static std::vector<std::string> list_assets(android_app* app, const char* asset_path)
{ //by Marcel Smit, found on https://github.com/android/ndk-samples/issues/603
    std::vector<std::string> result;

    JNIEnv* env = nullptr;
    app->activity->vm->AttachCurrentThread(&env, nullptr);

    auto context_object = app->activity->javaGameActivity;//clazz;
    auto getAssets_method = env->GetMethodID(env->GetObjectClass(context_object), "getAssets", "()Landroid/content/res/AssetManager;");
    auto assetManager_object = env->CallObjectMethod(context_object, getAssets_method);
    auto list_method = env->GetMethodID(env->GetObjectClass(assetManager_object), "list", "(Ljava/lang/String;)[Ljava/lang/String;");

    jstring path_object = env->NewStringUTF(asset_path);
    auto files_object = (jobjectArray)env->CallObjectMethod(assetManager_object, list_method, path_object);
    env->DeleteLocalRef(path_object);
    auto length = env->GetArrayLength(files_object);

    for (int i = 0; i < length; i++)
    {
        jstring jstr = (jstring)env->GetObjectArrayElement(files_object, i);
        const char* filename = env->GetStringUTFChars(jstr, nullptr);
        if (filename != nullptr)
        {
            result.push_back(filename);
            env->ReleaseStringUTFChars(jstr, filename);
        }
        env->DeleteLocalRef(jstr);
    }
    app->activity->vm->DetachCurrentThread();
    return result;
}
int updateAssets() {
    //get APK apkLastUpdateTime timestamp
    JNIEnv* env = nullptr;
    pAndroidApp->activity->vm->AttachCurrentThread(&env, nullptr);
    jobject context_object = pAndroidApp->activity->javaGameActivity;//clazz;
    jmethodID getPackageNameMid_method = env->GetMethodID(env->GetObjectClass(context_object), "getPackageName", "()Ljava/lang/String;");
    jstring packageName = (jstring)env->CallObjectMethod(context_object, getPackageNameMid_method);
    jmethodID getPackageManager_method = env->GetMethodID(env->GetObjectClass(context_object), "getPackageManager", "()Landroid/content/pm/PackageManager;");
    jobject packageManager_object = env->CallObjectMethod(context_object, getPackageManager_method);
    jmethodID getPackageInfo_method = env->GetMethodID(env->GetObjectClass(packageManager_object), "getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
    jobject packageInfo_object = env->CallObjectMethod(packageManager_object, getPackageInfo_method, packageName, 0x0);
    jfieldID updateTimeFid = env->GetFieldID(env->GetObjectClass(packageInfo_object), "lastUpdateTime", "J");
    long int apkLastUpdateTime = env->GetLongField(packageInfo_object, updateTimeFid);
    // APK updateTime timestamp retrieved
    // compare with saved timestamp
    std::string updateTimeFilePath = filesRoot + "/dt/apk_update_time.bin";
    FILE* inFile = fopen(updateTimeFilePath.c_str(), "r");
    if (inFile != NULL)
    {
        long int savedUpdateTime;
        fread(&savedUpdateTime, 1, sizeof(savedUpdateTime), inFile);
        fclose(inFile);
        if (savedUpdateTime == apkLastUpdateTime) {
            mylog("Assets are up to date.\n");
            return 0;
        }
    }
    // if here - need to update assets
    AAssetManager* am = pAndroidApp->activity->assetManager;

    std::vector<std::string> dirsToCheck; //list of assets folders to check
    dirsToCheck.push_back("dt"); //root folder
    std::vector<std::string> filesToUpdate;
    while (dirsToCheck.size() > 0) {
        //open last element from directories vector
        std::string dirPath = dirsToCheck.back();
        dirsToCheck.pop_back(); //delete last element
        //mylog("Scanning directory <%s>\n", dirPath.c_str());
        //make sure folder exists on local drive
        std::string outPath = filesRoot + "/" + dirPath; // .c_str();
        struct stat info;
        int statRC = stat(outPath.c_str(), &info);
        if (statRC == 0)
            ;//mylog("%s folder exists.\n", outPath.c_str());
        else {
            int status = mkdir(outPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (status == 0)
                ;//mylog("%s folder added.\n", outPath.c_str());
            else {
                ;//mylog("ERROR creating, status=%d, errno: %s.\n", status, std::strerror(errno));
            }
        }
        //get folder's content
        std::vector<std::string> dirItems = list_assets(pAndroidApp, dirPath.c_str());
        int itemsN = dirItems.size();
        //scan directory items
        for (int i = 0; i < itemsN; i++) {
            std::string itemPath = dirPath + "/" + dirItems.at(i);
            //try to open it to see if it's a file
            AAsset* asset = AAssetManager_open(am, itemPath.c_str(), AASSET_MODE_UNKNOWN);
            if (asset != NULL) {
                //it's a file
                filesToUpdate.push_back(itemPath);
                AAsset_close(asset);
            }
            else {
                dirsToCheck.push_back(itemPath);
            }
        }
        dirItems.clear();
    }

    //save files
    int buffSize = 4000000; //2Mb, guess, should be enough?
    char* buff = new char[buffSize];
    int filesN = filesToUpdate.size();
    mylog("%d files to update.\n",filesN);
    for(int i=0;i<filesN;i++){
        std::string assetPath = filesToUpdate.at(i);
        AAsset* asset = AAssetManager_open(am, assetPath.c_str(), AASSET_MODE_UNKNOWN);
        uint32_t assetSize = AAsset_getLength(asset);
        if (assetSize > buffSize) {
            mylog("ERROR in main.cpp->updateAssets(): File %s is too big (%d).\n", assetPath.c_str(),assetSize);
            return -1;
        }
        AAsset_read(asset, buff, assetSize);
        std::string outPath = filesRoot + "/" + assetPath;
        FILE* outFile = fopen(outPath.c_str(), "w");
        if (outFile == NULL) {
            mylog("ERROR in main.cpp->updateAssets(): Can't create file %s\n", assetPath.c_str());
            return -1;
        }
        fwrite(buff, 1, assetSize, outFile);
        fflush(outFile);
        fclose(outFile);
        //mylog("%d: %s, size %d\n", i+1, assetPath.c_str(),assetSize);

        AAsset_close(asset);
    }
    delete[] buff;
    // save updateTime
    FILE* outFile = fopen(updateTimeFilePath.c_str(), "w");
    if (outFile != NULL)
    {
        fwrite(&apkLastUpdateTime, 1, sizeof(apkLastUpdateTime), outFile);
        fflush(outFile);
        fclose(outFile);
    }
    else
        mylog("ERROR creating %s\n", updateTimeFilePath.c_str());
    return 1;
}


/*!
 * This the main entry point for a native activity
 */

void android_main(struct android_app *pApp) {
    pAndroidApp = pApp;

    // register an event handler for Android events
    pApp->onAppCmd = handle_cmd;

    myPollEvents(); //this will wait for display initialization

    //retrieving files root
    filesRoot.assign(pAndroidApp->activity->internalDataPath);
    mylog("filesRoot = %s\n", filesRoot.c_str());

    updateAssets();

    theApp.run();

    android_term_display();
    std::terminate();
}

