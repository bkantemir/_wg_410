#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION  //required by stb_image.h
#include "stb_image.h"
#include "platform.h"
#include "utils.h"
#include "MyColor.h"
#include "FileLoader.h"

//static array (vector) of all loaded textures
std::vector<Texture*> Texture::textures;

int Texture::loadTexture(std::string filePath, unsigned int ckey, int glRepeatH, int glRepeatV) {
    filePath = FileLoader::getFullPath(filePath);
    int texN = findTexture(filePath);
    if (texN >= 0)
        return texN;
    //if here - texture wasn't loaded
    // load an image
    int nrChannels, w, h;
    unsigned char* imgData = stbi_load(filePath.c_str(),
        &w, &h, &nrChannels, 4); //"4"-convert to 4 channels -RGBA
    if (imgData == NULL) {
        mylog("ERROR in Texture::loadTexture loading image %s\n", filePath.c_str());
    }
    if (ckey != 0)
        applyCkey(imgData, w, h, ckey);
    // generate texture
    generateTexture(filePath, w, h, imgData, glRepeatH, glRepeatV);
    // release image data
    stbi_image_free(imgData);

    return (textures.size() - 1);
}
int Texture::findTexture(std::string filePath) {
    int texturesN = textures.size();
    if (texturesN < 1)
        return -1;
    for (int i = 0; i < texturesN; i++) {
        Texture* pTex = textures.at(i);
        if (pTex->source.compare(filePath) == 0)
            return i;
    }
    return -1;
}
int Texture::cleanUp() {
    int texturesN = textures.size();
    if (texturesN < 1)
        return -1;
    //detach all textures
    glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE4); //depthMap
    glBindTexture(GL_TEXTURE_2D, 0);
    //release all textures
    for (int i = 0; i < texturesN; i++) {
        Texture* pTex = textures.at(i);
        detachRenderBuffer(pTex);
        glDeleteTextures(1, (GLuint*)&pTex->GLid);
        pTex->GLid = 0;
        delete pTex;
    }
    textures.clear();
    return 1;
}

int Texture::generateTexture(std::string imgID, int w, int h, unsigned char* imgData, int glRepeatH, int glRepeatV) {
    //glRepeat options: GL_REPEAT, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT
    if (!imgID.empty()) {
        int texN = findTexture(imgID);
        if (texN >= 0)
            return texN;
    }
    //if here - texture wasn't generated
    //create Texture object
    Texture* pTex = new Texture();
    textures.push_back(pTex);
    pTex->size[0] = w;
    pTex->size[1] = h;
    pTex->source.assign(imgID);
    // generate texture
    glGenTextures(1, (GLuint*)&pTex->GLid);
    glBindTexture(GL_TEXTURE_2D, pTex->GLid);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glRepeatH);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glRepeatV);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    if (imgData != NULL) {
        // attach image data (if provided)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pTex->size[0], pTex->size[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, imgData);
        //mipmap
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);// GL_LINEAR);  //
        glGenerateMipmap(GL_TEXTURE_2D);

        //transparency?
        int usesAlpha = -1;
        int pixelsN = pTex->size[0] * pTex->size[1];
        for (int i = 0; i < pixelsN; i++) {
            unsigned char a = imgData[i * 4 + 3];
            if (a != 255) {
                if (a == 0)
                    usesAlpha = 0;
                else {
                    usesAlpha = 1;
                    break;
                }
            }
        }
        pTex->usesAlpha = usesAlpha;
    }
    else
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, pTex->size[0], pTex->size[1]);
    return (textures.size() - 1);
}
int Texture::detachRenderBuffer(Texture* pTex) {
    if (pTex->frameBufferId == 0)
        return 0;
    if (pTex->depthBufferId > 0) {
        glDeleteRenderbuffers(1, (GLuint*)&pTex->depthBufferId);
        pTex->depthBufferId = 0;
    }
    glDeleteFramebuffers(1, (GLuint*)&pTex->frameBufferId);
    pTex->frameBufferId = 0;
    return 1;
}

int Texture::attachRenderBuffer(Texture* pTex, bool zBuffer) {
    if (pTex->frameBufferId > 0)
        return 0; //attached already
    //generate frame buffer
    glGenFramebuffers(1, (GLuint*)&pTex->frameBufferId);
    if (zBuffer) {
        //generate depth buffer
        glGenRenderbuffers(1, (GLuint*)&pTex->depthBufferId);
        // create render buffer and bind 16-bit depth buffer
        glBindRenderbuffer(GL_RENDERBUFFER, pTex->depthBufferId);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, pTex->size[0], pTex->size[1]);
        glBindRenderbuffer(GL_RENDERBUFFER, 0); //release
        /*
        glGenTextures(1, (GLuint*)&pTex->depthBufferId);
        glBindTexture(GL_TEXTURE_2D, pTex->depthBufferId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
            pTex->size[0], pTex->size[1], 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
        */
    }
    return 1;
}
int Texture::setRenderToTexture(Texture* pTex) {
    if (pTex->frameBufferId == 0) {
        mylog("ERROR in Texture::setRenderToTexture: %s not renderable", pTex->source.c_str());
        return -1;
    }
    // Bind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, pTex->frameBufferId);

    if (pTex->GLid >= 0) {
        // detach mipmap
        glBindTexture(GL_TEXTURE_2D, pTex->GLid);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        // specify texture as color attachment
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pTex->GLid, 0);
    }
    // attach render buffer as depth buffer
    if (pTex->depthBufferId > 0) {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pTex->depthBufferId);
        //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, pTex->depthBufferId, 0);
        glDepthMask(GL_TRUE);
    }
    else
        glDepthMask(GL_FALSE);
    // check status
    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::string str;
        if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
            str.assign("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
        else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
            str.assign("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
        else if (status == GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE)
            str.assign("GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE");
        else if (status == GL_FRAMEBUFFER_UNSUPPORTED)
            str.assign("GL_FRAMEBUFFER_UNSUPPORTED");
        else
            str.assign("hz");
        mylog("Modeler.setRenderToTextureBind to texture %s failed: %s\n", pTex->source.c_str(), str.c_str());
        return -1;
    }
    glViewport(0, 0, pTex->size[0], pTex->size[1]);
    return 1;
}
int Texture::getImageFromTexture(int texN, unsigned char* imgData) {
    Texture* pTex = textures.at(texN);
    glBindTexture(GL_TEXTURE_2D, pTex->GLid);
    glBindFramebuffer(GL_FRAMEBUFFER, pTex->frameBufferId);

    glReadPixels(0, 0, pTex->size[0], pTex->size[1], GL_RGBA, GL_UNSIGNED_BYTE, imgData);
    return 1;
}
int Texture::blurRGBA(unsigned char* imgData, int w0, int h0, int blurLevel) {
    unsigned char* imgTemp = new unsigned char[w0 * h0 * 4];
    int w00 = blurLevel * 2 + 1;
    for (int y0 = 0; y0 < h0; y0++) {
        int y1 = y0 - blurLevel;
        int h1 = w00;
        if (y1 < 0) {
            int d = -y1;
            y1 += d;
            h1 -= d;
        }
        else if (y1 > h0 - w00) {
            int d = y1 - (h0 - w00);
            h1 -= d;
        }
        for (int x0 = 0; x0 < w0; x0++) {
            int x1 = x0 - blurLevel;
            int w1 = w00;
            if (x1 < 0) {
                int d = -x1;
                x1 += d;
                w1 -= d;
            }
            else if (x1 > w0 - w00) {
                int d = x1 - (w0 - w00);
                w1 -= d;
            }
            int sum[4] = { 0,0,0,0 };
            for (int y = y1; y < y1 + h1; y++) {
                for (int x = x1; x < x1 + w1; x++) {
                    int idx = (y * w0 + x) * 4;
                    for (int ch = 0; ch < 4; ch++)
                        sum[ch] += imgData[idx + ch];
                }
            }
            int n = w1 * h1;
            int idx = (y0 * w0 + x0) * 4;
            for (int ch = 0; ch < 4; ch++)
                imgTemp[idx + ch] = (unsigned char)(sum[ch] / n);
        }
    }
    memcpy(imgData, imgTemp, w0 * h0 * 4);
    delete[] imgTemp;
    return 1;
}

int Texture::applyCkey(unsigned char* imgData, int w0, int h0, unsigned int ckey) {
    if (ckey == 0)
        return 0;
    MyColor transparentPixel;
    transparentPixel.setRGBA(127, 127, 127, 0);
    unsigned int transparentValue = transparentPixel.getUint32();
    unsigned int* pIntData = (unsigned int*)imgData;
    int dataIntsN = w0 * h0;
    int transparentPixelsN = 0;
    for (int i = 0; i < dataIntsN; i++) {
        if (pIntData[i] != ckey)
            continue;
        //here - have ckey pixel
        transparentPixelsN++;
        pIntData[i] = transparentValue;
    }
    if (transparentPixelsN == 0)
        return 0;
    //re-calculate transparent RGBs
    int dataCharsN = dataIntsN * 4;
    //duplicate image
    unsigned char* imgTemp = new unsigned char[dataCharsN];
    memcpy(imgTemp, imgData, dataCharsN);
    //scan
    int blurLevel = 1;
    int w00 = blurLevel * 2 + 1;
    for (int y0 = 0; y0 < h0; y0++) {
        int y1 = y0 - blurLevel;
        int h1 = w00;
        if (y1 < 0) {
            int d = -y1;
            y1 += d;
            h1 -= d;
        }
        else if (y1 > h0 - w00) {
            int d = y1 - (h0 - w00);
            h1 -= d;
        }
        for (int x0 = 0; x0 < w0; x0++) {
            if (imgData[(y0 * w0 + x0) * 4 + 3] != 0) //check current pixel's alpha component
                continue; //non-transparent pixel
            int x1 = x0 - blurLevel;
            int w1 = w00;
            if (x1 < 0) {
                int d = -x1;
                x1 += d;
                w1 -= d;
            }
            else if (x1 > w0 - w00) {
                int d = x1 - (w0 - w00);
                w1 -= d;
            }
            int sum[3] = { 0,0,0 };
            int opaquePixelsN = 0;
            for (int y = y1; y < y1 + h1; y++) {
                for (int x = x1; x < x1 + w1; x++) {
                    int idx = (y * w0 + x) * 4;
                    if (imgTemp[idx + 3] == 0) //check alpha channel
                        continue; //transparent pixel
                    opaquePixelsN++;
                    for (int ch = 0; ch < 3; ch++)
                        sum[ch] += imgTemp[idx + ch];
                }
            }
            if (opaquePixelsN == 0)
                continue;
            int idx = (y0 * w0 + x0) * 4;
            for (int ch = 0; ch < 3; ch++)
                imgData[idx + ch] = (unsigned char)(sum[ch] / opaquePixelsN);
        }
    }
    delete[] imgTemp;
    //saveTGA("/dt/02.tga", imgData, w0, h0, 3);
    return 1;
}
