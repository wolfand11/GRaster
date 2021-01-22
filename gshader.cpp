#include "gshader.h"
#include "ggraphiclibapi.h"
#include "gutils.h"
#include "ggameobject.h"
#include <algorithm>
using namespace GMath;
using namespace std;

S_abs_appdata *S_abs_appdata::CreateAppData(GShaderAppDataType appdataType)
{
    switch (appdataType)
    {
    case GShaderAppDataType::kSADTDefault:
        return new S_appdata();
    }
    return nullptr;
}

const std::vector<S_SlotInfo> &S_abs_appdata::GetSlotInfoArr(GShaderAppDataType appdataType)
{
    switch (appdataType)
    {
    case GShaderAppDataType::kSADTDefault:
        return S_appdata::_slotInfoArr;
    }
    assert(false);
}

const S_SlotInfo &S_abs_appdata::GetSlotInfo(GShaderAppDataType appdataType, int slot)
{
    const std::vector<S_SlotInfo>& slotInfoArr = GetSlotInfoArr(appdataType);
    return slotInfoArr[slot];
}

const std::vector<S_SlotInfo> S_appdata::_slotInfoArr = {
    {0, 3, GSlotType::kSlotTPosition},
    {1, 2, GSlotType::kSlotTUV0},
    {2, 3, GSlotType::kSlotTNormal},
};


void S_appdata::SetSlotDatum(int slot, int datumIdx, double value)
{
    if(slot==0)
    {
        vert[datumIdx] = value;
    }
    else if(slot==1)
    {
        uv[datumIdx] = value;
    }
    else if(slot==2)
    {
        normal[datumIdx] = value;
    }
}

vec4 GShader::vertex(GGraphicLibAPI *GLAPI, S_abs_appdata *vert_in, int vertIdx)
{
    S_appdata* appdata = (S_appdata*) vert_in;
    GMath::vec4 wPos = GMath::embed<double,4>(appdata->vert, 1);
    wPos = (mat4)obj2World * wPos;
    v2f_data_arr[vertIdx].wPos = proj<double, 3>(wPos/wPos.w());
    v2f_data_arr[vertIdx].wNormal = (mat3)(world2Obj.get_minor(3,3).transpose()) * appdata->normal;
    wPos = (mat4)world2View * wPos;
    v2f_data_arr[vertIdx].gl_position = (mat4)projMat * wPos;
    v2f_data_arr[vertIdx].uv = appdata->uv;
    return v2f_data_arr[vertIdx].gl_position;
}

S_abs_v2f* GShader::interpolation(GGraphicLibAPI *GLAPI, vec3 lerpFactor, int fragIdx)
{
    int vertCount = 1;
    if(GLAPI->activePrimitiveType==GPrimitiveType::kLines)
    {
        vertCount = 2;
    }
    else if(GLAPI->activePrimitiveType==GPrimitiveType::kTriangles)
    {
        vertCount = 3;
    }
    v2f_interpolated[fragIdx].gl_position = vec4::zero;
    v2f_interpolated[fragIdx].uv = vec2::zero;
    v2f_interpolated[fragIdx].wPos = vec3::zero;
    v2f_interpolated[fragIdx].wNormal = vec3::zero;
    for(int i=0; i<vertCount; i++)
    {
        v2f_interpolated[fragIdx].gl_position = v2f_interpolated[fragIdx].gl_position + v2f_data_arr[i].gl_position * lerpFactor[i];
        v2f_interpolated[fragIdx].uv = v2f_interpolated[fragIdx].uv + v2f_data_arr[i].uv * lerpFactor[i];
        v2f_interpolated[fragIdx].wPos = v2f_interpolated[fragIdx].wPos + v2f_data_arr[i].wPos * lerpFactor[i];
        v2f_interpolated[fragIdx].wNormal = v2f_interpolated[fragIdx].wNormal + v2f_data_arr[i].wNormal * lerpFactor[i];
    }
    return &(v2f_interpolated[fragIdx]);
}

void GShader::fragment(S_abs_v2f& frag_in, S_fout &frag_out)
{
    S_v2f& v2f = (S_v2f&)frag_in;
    vec4 col;
    vec4 diffColor = GColor::ToFloatColor(GUtils::SampleImage(diffusemap_,v2f.uv));
    double alpha = diffColor.w();
    float NoL = 0;
    v2f.wNormal.normalize();
    for(GLightInfo* light : lights)
    {
        NoL = 1;
        if(light->lightType == GLightType::kLTDirection)
        {
            NoL = std::max(GMath::dot(v2f.wNormal, light->lightPosOrDir), 0.0);
        }
        vec4 lightColor = GColor::ToFloatColor(light->lightColor) * light->lightIntensity;
        col = (diffColor * lightColor) * NoL;
    }

    col.SetW(alpha);
    frag_out.colors.push_back(col);
}































