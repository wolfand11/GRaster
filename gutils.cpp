#include "gutils.h"
#include <QCoreApplication>
#include <QDir>
using namespace std;

#define PROJ_NAME "GRaster"

int GUtils::screenWidth = 800;
int GUtils::screenHeight = 800;

float GUtils::screenAspectRatio()
{
    return (float)screenHeight/screenWidth;
}

std::string GUtils::GetProjRootPath()
{
    QDir dir(QCoreApplication::applicationDirPath()+"/../../GRaster");
    if(dir.exists())
    {
        return dir.absolutePath().toStdString();
    }
    return "";
}

string GUtils::GetAbsPath(const string &relativePath)
{
    return GetProjRootPath()+"/"+relativePath;
}

GColor GUtils::SampleImage(TGAImage *img, GMath::vec2 uv, GTextureWrapMode wrapMode)
{
    if(img->get_width()<1 || img->get_height()<1)
    {
        GLog::LogError("img w = ", img->get_width(), " h = ", img->get_height());
        return GColor::gray;
    }

    if(uv.x()>1)
    {
        if(wrapMode == GTextureWrapMode::kTWMClamp)
        {
            uv.SetX(1);
        }
        else if(wrapMode == GTextureWrapMode::kTWMRepeat)
        {
            uv.SetX(uv.x()-(int)uv.x());
        }
        else
        {
            GLog::LogError("wrapMode = ", wrapMode);
        }
    }
    if(uv.y()>1)
    {
        if(wrapMode == GTextureWrapMode::kTWMClamp)
        {
            uv.SetY(1);
        }
        else if(wrapMode == GTextureWrapMode::kTWMRepeat)
        {
            uv.SetY(uv.y()-(int)uv.y());
        }
        else
        {
            GLog::LogError("wrapMode = ", wrapMode);
        }
    }

    int w = img->get_width() - 1;
    int h = img->get_height() - 1;

    float hLerpFactor = w*uv.x();
    int rX = w*uv.x();
    hLerpFactor = hLerpFactor - rX;
    int lX = rX >= w ? rX : rX+1;
    float vLerpFactor = h*uv.y();
    int bY = h*uv.y();
    vLerpFactor = vLerpFactor - bY;
    int tY = bY >= h ? bY : bY+1;

    GColor hColor1 = GColor::Lerp(img->get(rX,bY), img->get(lX,bY), hLerpFactor);
    GColor hColor2 = GColor::Lerp(img->get(rX,tY), img->get(lX,tY), hLerpFactor);
    GColor ret = GColor::Lerp(hColor1, hColor2, vLerpFactor);
    if(img->get_bytespp()!=TGAImage::Format::RGBA)
    {
        ret.a = 255;
    }
    return ret;
}
