#include "gmathutils.h"
#include <algorithm>
using namespace std;
using namespace GMath;

int GMathUtils::FloatNegativOneToOne2Int32(float src)
{
    src = min(src, 1.0f);
    src = max(src, -1.0f);
    return (int)(src*(double)INT_MAX);
}

GMath::mat4 GMathUtils::LookAt(GMath::vec3 eyePos, GMath::vec3 lookAtPoint, GMath::vec3 up)
{
    mat4 ret;
    ret.identity();
    ret[0][3] = eyePos.x;
    ret[1][3] = eyePos.y;
    ret[2][3] = eyePos.z;

    vec3 forward = (lookAtPoint - eyePos).normalize();
    vec3 right = cross(up, forward).normalize();
    up = cross(forward, right).normalize();
    for(int ncol=0; ncol<3; ncol++)
    {
        ret[0][ncol] = right[ncol];
        ret[1][ncol] = up[ncol];
        ret[2][ncol] = forward[ncol];
    }
    return ret;
}

void GMathUtils::DecomposeMatrix(const mat4f &mat, mat4f &translate, mat4f &rotation, mat4f &scale)
{
    translate.identity();
    rotation.identity();
    scale.identity();
    translate[0][3] = mat.rows[0][3];
    translate[1][3] = mat.rows[1][3];
    translate[2][3] = mat.rows[2][3];

    mat4f RS = mat;
    for(int i=0; i<3; i++)
    {
        RS.rows[i][3] = RS.rows[3][i] = 0.f;
    }
    RS.rows[3][3] = 1.0;

    float delta = 0.0f;
    int count = 0;
    rotation = RS;
    do
    {
        mat4f Rnext;
        mat4f Rit = rotation.transpose().invert();
        for(int i=0; i<4; i++)
        {
            for(int j=0; j<4; j++)
            {
                Rnext[i][j] = 0.5f * (rotation[i][j] + Rit[i][j]);
            }
        }
        delta = 0.f;
        for(int i=0; i<3; i++)
        {
            delta = std::abs(rotation[i][0]-Rnext[i][0]) +
                    std::abs(rotation[i][1]-Rnext[i][1]) +
                    std::abs(rotation[i][2]-Rnext[i][2]);
            delta = std::max(delta, 0.0f);
        }
        rotation = Rnext;
    } while(++count<100 && delta>0.0001);

    scale = RS * rotation.invert();
}

void GMathUtils::DecomposeMatrix(const mat4f& mat, vec3f &translate, vec3f &rotation, vec3f &scale)
{
    mat4f _trans, _rot, _scale;
    DecomposeMatrix(mat, _trans, _rot, _scale);
    translate[0] = mat.rows[0][3];
    translate[1] = mat.rows[1][3];
    translate[2] = mat.rows[2][3];

    rotation = RotationMatrixToEulerAngle(_rot);
    scale.x = _scale[0][0];
    scale.y = _scale[1][1];
    scale.z = _scale[2][2];
}

vec3f GMathUtils::RotationMatrixToEulerAngle(mat4f matrix)
{
    vec3f eulerAngle;
    float sp = -matrix.rows[3][2];
    if(sp<=-1.0f)
    {
        eulerAngle.x = -M_PI / 2.0f;
    }
    else if(sp>=1.0f)
    {
        eulerAngle.x = M_PI/2.0f;
    }
    else
    {
        eulerAngle.x = asin(sp);
    }

    if(fabs(sp) > 0.9999f)
    {
        eulerAngle.z = 0.0f;
        eulerAngle.y = atan2(-matrix.rows[1][3],matrix.rows[1][1]);
    }
    else
    {
        eulerAngle.z = atan2(-matrix.rows[1][2],matrix.rows[2][2]);
        eulerAngle.y = atan2(-matrix.rows[3][1],matrix.rows[3][3]);
    }
    return eulerAngle;
}

mat4f GMathUtils::EulerAngleToRotationMatrix(vec3f eulerAngle)
{
    mat4f rotMat;
    rotMat.identity();
    float ch = cos(eulerAngle.y);
    float cp = cos(eulerAngle.x);
    float cb = cos(eulerAngle.z);
    float sh = sin(eulerAngle.y);
    float sp = sin(eulerAngle.x);
    float sb = sin(eulerAngle.z);
    rotMat.rows[0][0] = ch*cb+sh*sp*sb;
    rotMat.rows[0][1] = sb*cp;
    rotMat.rows[0][2] = -sh*cb+ch*sp*sb;
    rotMat.rows[1][0] = -ch*sb+sh*sp*cb;
    rotMat.rows[1][1] = cb*cp;
    rotMat.rows[1][2] = sb*sh+ch*sp*cb;
    rotMat.rows[2][0] = sh*cp;
    rotMat.rows[2][1] = -sp;
    rotMat.rows[2][2] = ch*cp;
    return rotMat;
}

mat4f GMathUtils::TRS(vec3f &translate, vec3f &rotation, vec3f &scale)
{
    mat4f ret;
    ret.identity();
    ret[0][3] = translate.x;
    ret[1][3] = translate.y;
    ret[2][3] = translate.z;
    ret = EulerAngleToRotationMatrix(rotation) * ret;
    ret[0][0] *= scale.x;
    ret[1][1] *= scale.y;
    ret[2][2] *= scale.z;
    return ret;
}

float GMathUtils::Rad2Deg(float rad)
{
    return rad * 180.0f / M_PI;
}

float GMathUtils::Deg2Rad(float degree)
{
    return degree * M_PI / 180.0f;
}

vec3f GMathUtils::Rad2Deg(vec3f rad)
{
    vec3f angle;
    angle.x = Rad2Deg(rad.x);
    angle.y = Rad2Deg(rad.y);
    angle.z = Rad2Deg(rad.z);
    return angle;
}

vec3f GMathUtils::Deg2Rad(vec3f degree)
{
    vec3f angle;
    angle.x = Deg2Rad(degree.x);
    angle.y = Deg2Rad(degree.y);
    angle.z = Deg2Rad(degree.z);
    return angle;
}