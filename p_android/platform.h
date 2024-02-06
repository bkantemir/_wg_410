#pragma once
#include <GLES3/gl32.h>
#include <stdio.h>

void mylog(const char* _Format, ...);
void mySwapBuffers();
void myPollEvents();
void strcpy_s(char* dst, int maxSize, const char* src);
int fopen_s(FILE** pFile, const char* filePath, const char* mode);
int myMkDir(const char* outPath);
void sprintf_s(char *buffer, size_t sizeOfBuffer,const char* _Format, ...);
