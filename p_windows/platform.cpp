#include <stdarg.h>
#include <stdio.h>
#include <GLFW/glfw3.h>
#include "platform.h"
#include "TheApp.h"
#include <direct.h> //myMkDir: mkdir
#include <sys/stat.h> //myMkDir: if file esists

extern GLFWwindow* myMainWindow;
extern TheApp theApp;

void mylog(const char* _Format, ...) {
#ifdef _DEBUG
    va_list _ArgList;
    va_start(_ArgList, _Format);
    vprintf(_Format, _ArgList);
    va_end(_ArgList);
#endif
};
void mySwapBuffers() {
    glfwSwapBuffers(myMainWindow);
}
void myPollEvents() {
    glfwPollEvents();
    //check if closing the window
    theApp.bExitApp = glfwWindowShouldClose(myMainWindow);
    //check screen size
    int width, height;
    glfwGetFramebufferSize(myMainWindow, &width, &height);
    theApp.onScreenResize(width, height);
}
int myMkDir(const char* outPath) {
    struct stat info;
    if (stat(outPath, &info) == 0)
        return 0; //exists already
    int status = _mkdir(outPath);
    if (status == 0)
        return 1; //Successfully created
    mylog("ERROR creating, status=%d\n", status);
    return -1;
}
