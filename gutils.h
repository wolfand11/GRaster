#ifndef GUTILS_H
#define GUTILS_H
#include<string>
#include "gmath.h"
#include "gbuffer.h"
#include "tgaimage.h"


enum GTextureWrapMode
{
    kTWMRepeat,
    kTWMClamp
};

class GUtils
{
public:
    static int screenWidth;
    static int screenHeight;
    static float screenAspectRatio();

    static std::string GetProjRootPath();
    static std::string GetAbsPath(const std::string& relativePath);
    static bool IsFileExist(const std::string filepath);

    // sample image
    static GColor SampleImage(TGAImage* img, GMath::vec2 uv, GTextureWrapMode wrapMode=GTextureWrapMode::kTWMClamp, GColor defaultColor=GColor::black);
    static GColor SampleImage(std::vector<TGAImage>* mipmaps, GMath::vec2 uv, int mipmapLevel, GTextureWrapMode wrapMode=GTextureWrapMode::kTWMClamp, GColor defaultColor=GColor::black);
};

#endif // GUTILS_H
