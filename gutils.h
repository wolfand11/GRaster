#ifndef GUTILS_H
#define GUTILS_H
#include<string>

class GUtils
{
public:
    static int screenWidth;
    static int screenHeight;
    static float screenAspectRatio();

    static std::string GetProjRootPath();
    static std::string GetAbsPath(const std::string& relativePath);
};

#endif // GUTILS_H
