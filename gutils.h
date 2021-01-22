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

    // sample image
    static GColor SampleImage(TGAImage* img, GMath::vec2 uv, GTextureWrapMode wrapMode=GTextureWrapMode::kTWMClamp);
};

#endif // GUTILS_H
