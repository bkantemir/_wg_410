#include <android/log.h>
#include "stdio.h"
#include "TheApp.h"
#include <EGL/egl.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <sys/stat.h>	//mkdir for Android

extern struct android_app* pAndroidApp;
extern EGLDisplay androidDisplay;
extern EGLSurface androidSurface;

extern TheApp theApp;
 
void mylog(const char* _Format, ...) {
    char outStr[4096];
    va_list _ArgList;
    va_start(_ArgList, _Format);
    vsprintf(outStr, _Format, _ArgList);
    va_end(_ArgList);
    int outStrLen = strlen(outStr);
    int printLimit = 1000;
    if(outStrLen < printLimit)
        __android_log_print(ANDROID_LOG_INFO, "mylog", outStr, NULL);
    else{ //text is too big, split
        int printed = 0;
        std::string outString = std::string(outStr);
        while(printed <= outStrLen){
            std::string part2print = outString.substr(printed,printLimit);
            __android_log_print(ANDROID_LOG_INFO, "mylog", part2print.c_str(), NULL);
            __android_log_print(ANDROID_LOG_INFO, "mylog", "---\n", NULL);
            printed += printLimit;
        }
    }
};
 
void mySwapBuffers() {
    eglSwapBuffers(androidDisplay, androidSurface);
}
void myPollEvents() {
    // Process all pending events before running game logic.
    int events;
    android_poll_source *pSource;
    if (ALooper_pollAll(0, nullptr, &events, (void **) &pSource) >= 0)
        if (pSource)
            pSource->process(pAndroidApp, pSource);
    //if no display - wait for it
    while (androidDisplay == EGL_NO_DISPLAY)
        if (ALooper_pollAll(0, nullptr, &events, (void **) &pSource) >= 0)
            if (pSource)
                pSource->process(pAndroidApp, pSource);

    // handle all queued inputs
    for (auto i = 0; i < pAndroidApp->motionEventsCount; i++) {

        // cache the current event
        auto &motionEvent = pAndroidApp->motionEvents[i];

        // cache the current action
        auto action = motionEvent.action;

        // Find the pointer index, mask and bitshift to turn it into a readable value
        auto pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
                >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
        //aout << "Pointer " << pointerIndex << ":";

        // get the x and y position of this event
        auto &pointer = motionEvent.pointers[pointerIndex];
        auto x = GameActivityPointerAxes_getX(&pointer);
        auto y = GameActivityPointerAxes_getY(&pointer);
        //aout << "(" << x << ", " << y << ") ";

        // Only consider touchscreen events, like touches
        auto actionMasked = action & AINPUT_SOURCE_TOUCHSCREEN;

        // determine the kind of event it is
        switch (actionMasked) {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
                //aout << "Pointer Down";
                break;

            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_POINTER_UP:
                //aout << "Pointer Up";
                break;

            default:
                ;//aout << "Pointer Move";
        }
    }
    android_app_clear_motion_events(pAndroidApp);

    // handle key inputs
    for (auto i = 0; i < pAndroidApp->keyUpEventsCount; i++) {
        // cache the current event
        auto &keyEvent = pAndroidApp->keyUpEvents[i];
        if (keyEvent.keyCode == AKEYCODE_BACK) {
            // actions on back key
            theApp.bExitApp = true;
        }
    }
    android_app_clear_key_up_events(pAndroidApp);
	
	
    //updateRenderArea
    EGLint width,height;
    eglQuerySurface(androidDisplay, androidSurface, EGL_WIDTH, &width);
    eglQuerySurface(androidDisplay, androidSurface, EGL_HEIGHT, &height);
    //screenSize[0] = 0;
    //screenSize[1] = 0;
        //mylog(">>>>>>>>>>>>>>>APP_CMD_INIT_WINDOW %d x %d\n",width,height);
    theApp.onScreenResize(width,height);

}
void strcpy_s(char* dst, int maxSize, const char* src) {
	strcpy(dst, src);
	//fill tail by zeros
	int strLen = strlen(dst);
	if (strLen < maxSize)
		for (int i = strLen; i < maxSize; i++)
			dst[i] = 0;
}
int fopen_s(FILE** pFile, const char* filePath, const char* mode) {
	*pFile = fopen(filePath, mode);
	if (*pFile == NULL) {
		mylog("ERROR: can't open file %s\n", filePath);
		return -1;
	}
	return 1;
}
int myMkDir(const char* outPath) {
	struct stat info;
	if (stat(outPath, &info) == 0)
		return 0; //exists already
	int status = mkdir(outPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (status == 0)
		return 1; //Successfully created
	mylog("ERROR creating, status=%d, errno: %s.\n", status, std::strerror(errno));
	return -1;
}

void sprintf_s(char *buffer, size_t sizeOfBuffer,const char* _Format, ...) {
    va_list _ArgList;
    va_start(_ArgList, _Format);
    vsprintf(buffer, _Format, _ArgList);
    va_end(_ArgList);
};
