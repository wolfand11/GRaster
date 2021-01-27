#include "gshader.h"
#include "ggraphiclibapi.h"
#include "gutils.h"
#include "ggameobject.h"
#include <algorithm>
#include "gmathutils.h"
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
    v2f_data_arr[vertIdx].wPos = proj<double, 3>(wPos);
    wPos = (mat4)world2View * wPos;
    v2f_data_arr[vertIdx].gl_position = (mat4)projMat * wPos;
    v2f_data_arr[vertIdx].uv = appdata->uv;
    v2f_data_arr[vertIdx].normal = appdata->normal;
    return v2f_data_arr[vertIdx].gl_position;
}

void GShader::calc_tangent(GGraphicLibAPI *GLAPI)
{
    if(GLAPI->activePrimitiveType==GPrimitiveType::kTriangles)
    {
        // tangent-xAxis normal-yAxis binormal-zAxis
        //vec3 tangent = embed<double,3>(v2f_data_arr[2].uv - v2f_data_arr[0].uv, 0);
        vec3 binormal = embed<double,3>(v2f_data_arr[1].uv - v2f_data_arr[0].uv, 0);
        for(int vertIdx=0; vertIdx<3; vertIdx++)
        {
            vec3 normal = v2f_data_arr[vertIdx].normal;
            vec3 tmpTangent = cross(normal, binormal);
            v2f_data_arr[vertIdx].wNormal = (mat3)(world2Obj.get_minor(3,3).transpose()) * normal;
            v2f_data_arr[vertIdx].wTangent = (mat3)(obj2World.get_minor(3,3)) * tmpTangent;
        }
    }
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
    v2f_interpolated[fragIdx].wTangent = vec3::zero;
    for(int i=0; i<vertCount; i++)
    {
        v2f_interpolated[fragIdx].gl_position = v2f_interpolated[fragIdx].gl_position + v2f_data_arr[i].gl_position * lerpFactor[i];
        v2f_interpolated[fragIdx].uv = v2f_interpolated[fragIdx].uv + v2f_data_arr[i].uv * lerpFactor[i];
        v2f_interpolated[fragIdx].wPos = v2f_interpolated[fragIdx].wPos + v2f_data_arr[i].wPos * lerpFactor[i];
        v2f_interpolated[fragIdx].wNormal = v2f_interpolated[fragIdx].wNormal + v2f_data_arr[i].wNormal * lerpFactor[i];
        v2f_interpolated[fragIdx].wTangent = v2f_interpolated[fragIdx].wTangent + v2f_data_arr[i].wTangent * lerpFactor[i];
    }
    return &(v2f_interpolated[fragIdx]);
}

void GShader::fragment(S_abs_v2f& frag_in, S_fout &frag_out, int fragIdx)
{
    S_v2f& v2f = (S_v2f&)frag_in;
    vec4 col;
    vec4 diffColor = GColor::ToFloat01Color(SampleTex(diffusemaps_, diff_mipmaptype, v2f.uv,fragIdx));
    double alpha = diffColor.w();
    float NoL = 0;
    float HoN = 0;
    vec3 wBinormal = cross(v2f.wNormal, v2f.wTangent);
    vec3 tNormal = proj<float,3>(GColor::ToFloat01Color(SampleTex(normalmaps_, norm_mipmaptype, v2f.uv,fragIdx, GColor::normal)));
    tNormal = (tNormal*2) - vec3::one;
    vec3 wNormal = tNormal.x() * v2f.wTangent + tNormal.y() * wBinormal + tNormal.z() * v2f.wNormal;
    vec3 viewDir = ((vec3)wCamPos - v2f.wPos).normalize();
    vec3 H;
    for(GLightInfo* light : lights)
    {
        NoL = 1;
        HoN = 0;
        if(light->lightType == GLightType::kLTDirection)
        {
            NoL = GMathUtils::saturate(GMath::dot(wNormal, light->lightPosOrDir));
            H = (viewDir + light->lightPosOrDir).normalize();
            HoN = GMathUtils::saturate(GMath::dot(H, wNormal));
        }
        vec4 lightColor = GColor::ToFloat01Color(light->lightColor) * light->lightIntensity;
        col = (diffColor * lightColor) * NoL + lightColor * std::pow(HoN, 50);
    }

    col.SetW(alpha);
    frag_out.colors.push_back(col);
}

GColor GShader::SampleTex(std::vector<TGAImage> *mipmaps, GMipmapType mipmapType, vec2f uv, int fragIdx, GColor defaultColor)
{
    if(mipmaps==nullptr)
    {
        return defaultColor;
    }
    if(mipmaps->size()<1)
    {
        return defaultColor;
    }

    if(mipmapType == GMipmapType::kMipmapIsotropy)
    {
        vec2f pixelCount = GetPixelCountPerTexel(&(mipmaps->at(0)), fragIdx);
        float lod = max(pixelCount.x(),pixelCount.y());
        lod = log2f(lod);
        int lodLevel = lod+0.5;
        return GUtils::SampleImage(mipmaps, uv, lodLevel);
    }
    else if(mipmapType== GMipmapType::kMipmapAnisotropy)
    {
        vec2f pixelCount = GetPixelCountPerTexel(&(mipmaps->at(0)), fragIdx);
        bool xDirPixelMore = pixelCount.x() > pixelCount.y();
        float maxPixelCount = xDirPixelMore ? pixelCount.x() : pixelCount.y();
        float minPixelCount = xDirPixelMore ? pixelCount.y() : pixelCount.x();
        int sampleRatio = maxPixelCount/minPixelCount + 0.5;
        int lod = log2f(maxPixelCount/sampleRatio);
        TGAImage* curLodImage = &(mipmaps->at(lod));
        vec2f curLodUVStep = vec2f::one / vec2f(curLodImage->get_width(), curLodImage->get_height());
        vec4f texelValue = vec4::zero;
        vec2f sampleUV;
        for(float i=1; i<=sampleRatio; i++)
        {
            sampleUV = xDirPixelMore ? vec2f(uv.x()+(i/(sampleRatio+i)-0.5)*curLodUVStep.x(), uv.y()) : vec2f(uv.x(), uv.y()+(i/(sampleRatio+i)-0.5)*curLodUVStep.y());
            texelValue = texelValue + GUtils::SampleImage(mipmaps, sampleUV, lod).ToFloatColor();
        }
        texelValue = texelValue / sampleRatio;
        return GColor::FromFloatColor(texelValue);
    }
    else
    {
        return GUtils::SampleImage(&(mipmaps->at(0)), uv);
    }
}

vec2f GShader::GetPixelCountPerTexel(const TGAImage* mipmap0Tex, int fragIdx)
{
    vec2f ddxUV;
    vec2f ddyUV;
    if(fragIdx==0 || fragIdx==1)
    {
        ddxUV = v2f_interpolated[1].uv - v2f_interpolated[0].uv;
    }
    else
    {
        ddxUV = v2f_interpolated[3].uv - v2f_interpolated[2].uv;
    }

    if(fragIdx==0 || fragIdx==2)
    {
        ddyUV = v2f_interpolated[2].uv - v2f_interpolated[0].uv;
    }
    else
    {
        ddyUV = v2f_interpolated[3].uv - v2f_interpolated[1].uv;
    }
    vec2f mipmap0Size = vec2f(mipmap0Tex->get_width(), mipmap0Tex->get_height());
    ddxUV = ddxUV * mipmap0Size;
    ddyUV = ddyUV * mipmap0Size;
    return vec2f(ddxUV.length(), ddyUV.length());
}































