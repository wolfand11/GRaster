#include "gutils.h"
#include <QCoreApplication>
#include <QDir>
using namespace std;

#define PROJ_NAME "GRaster"

int GUtils::screenWidth = 400;
int GUtils::screenHeight = 400;

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
