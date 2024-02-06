#pragma once
#include "platform.h"
#include "utils.h"
#include <vector>

class MyColor
{
public:
    char colorName[32] = "";

private:
    float rgba[4] = { 0,0,0,0 };
    union {
        uint32_t uint32value = 0;
        uint8_t channel[4]; //4 channels
    } RGBA;

public:
    static std::vector<MyColor*> colorsList;

public:
    MyColor() {};
    MyColor(MyColor* pV0) { memcpy((void*)this, (void*)pV0, sizeof(MyColor)); };
    static int cleanUp();
    float* forGL() { return rgba; };
    static void cloneIntToFloat(MyColor* pCl);
    static void cloneFloatToInt(MyColor* pCl);
    void setRGBA(float* rgba) { return setRGBA(this, rgba); };
    static void setRGBA(MyColor* pCl, float* rgba);
    void setRGBA(float r, float g, float b, float a = 1.0) { return setRGBA(this, r, g, b, a); };
    static void setRGBA(MyColor* pCl, float r, float g, float b, float a);
    void setRGBA(int R, int G, int B, int A = 255) { return setRGBA(this, R, G, B, A); };
    static void setRGBA(MyColor* pCl, int R, int G, int B, int A = 255);
    void setRGBA(unsigned char* RGBA) { return setRGBA(this, RGBA); };
    static void setRGBA(MyColor* pCl, unsigned char* RGBA);
    void setUint32(unsigned int RGBA) { return setUint32(this, RGBA); };
    static void setUint32(MyColor* pCl, unsigned int RGBA);
    bool isZero() { return (RGBA.uint32value == 0); };
    bool isSet() { return (RGBA.uint32value != 0); };
    void clear() { setUint32(this, 0); };
    unsigned int getUint32() { return (unsigned int)RGBA.uint32value; };
    static MyColor* findColor(const char colorName[], std::vector<MyColor*>* pList);
    static unsigned int getUint32(int R, int G, int B, int A=255);
    static unsigned int getUint32(float r, float g, float b, float a=1.0);
    static void toLog(MyColor* pCl);
    //float getChannelFloat(int chN) { return rgba[chN]; };
    //int getChannelInt(int chN) { return RGBA.channel[chN]; };
    void getrgba(float* vOut) { v4copy(vOut, rgba); };
};
