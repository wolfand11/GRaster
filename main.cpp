#include "graster.h"
#include <QApplication>
#include "glog.h"
#include "gmathutils.h"
#include "gutils.h"
using namespace GMath;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    std::cout << "Test Code" << std::endl;
    // test code
    mat4f trs;
    trs.rows[0] = {2.f, 0.f, 0.f, 10.f};
    trs.rows[1] = {0.f, 3.f, 0.f, 20.f};
    trs.rows[2] = {0.f, 0.f, 4.f, 30.f};
    trs.rows[3] = {0.f, 0.f, 0.f, 1.f};

    mat4f mpos, mrotation, mscale;
    vec3f pos, rotation, scale;
    GMathUtils::DecomposeMatrix(trs, mpos, mrotation, mscale);
    GMathUtils::DecomposeMatrix(trs, pos, rotation, scale);

    std::cout << trs << std::endl;
    std::cout << "mpos : \n" << mpos << std::endl;
    std::cout << "mrot : \n" << mrotation << std::endl;
    std::cout << "mscl : \n" << mscale << std::endl;
    std::cout << "pos : " << pos << std::endl;
    std::cout << "rot : " << rotation << std::endl;
    std::cout << "scl : " << scale << std::endl << std::endl;

    GLog::LogInfo("proj path = ", GUtils::GetProjRootPath());

    GRaster w;
    w.show();

    return a.exec();
}
