#include "gutils.h"
#include <QCoreApplication>
#include <QDir>
#include <sys/stat.h>
using namespace std;

#define PROJ_NAME "GRaster"

int GUtils::screenWidth = 500;
int GUtils::screenHeight = 500;

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

bool GUtils::IsFileExist(const std::string filepath)
{
    struct stat s;
    if(stat(filepath.c_str(), &s) == 0)
    {
        if(s.st_mode & S_IFREG)
        {
            return true;
        }
        else if(s.st_mode & S_IFDIR)
        {
            GLog::LogWarning(filepath, " is directory!");
        }
    }
    return false;
}


GColor GUtils::SampleImage(TGAImage *img, GMath::vec2 uv, GTextureWrapMode wrapMode, GColor defaultColor)
{
    if(img->get_width()<1 || img->get_height()<1)
    {
        GLog::LogError("img w = ", img->get_width(), " h = ", img->get_height());
        return defaultColor;
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

GColor GUtils::SampleImage(std::vector<TGAImage> *mipmaps, GMath::vec2 uv, int mipmapLevel, GTextureWrapMode wrapMode, GColor defaultColor)
{
    if(mipmaps==nullptr)
    {
        GLog::LogError("mipmaps == nullptr");
        return defaultColor;
    }

    if((size_t)mipmapLevel>=mipmaps->size())
    {
        mipmapLevel = mipmaps->size() - 1;
    }
    return SampleImage(&(mipmaps->at(mipmapLevel)), uv, wrapMode, defaultColor);
}
