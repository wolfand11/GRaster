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
    if(vertIdx == 0) v2f_data_arr_tmp.clear();
    if((int)v2f_data_arr_tmp.size() <= vertIdx)
    {
        v2f_data_arr_tmp.push_back(S_v2f());
    }

    S_appdata* appdata = (S_appdata*) vert_in;
    GMath::vec4 wPos = GMath::embed<double,4>(appdata->vert, 1);
    wPos = (mat4)obj2World * wPos;
    v2f_data_arr_tmp[vertIdx].wPos = proj<double, 3>(wPos);
    wPos = (mat4)world2View * wPos;
    v2f_data_arr_tmp[vertIdx].gl_position = (mat4)projMat * wPos;
    v2f_data_arr_tmp[vertIdx].uv = appdata->uv;
    v2f_data_arr_tmp[vertIdx].normal = appdata->normal;
    return v2f_data_arr_tmp[vertIdx].gl_position;
}

int GShader::homogenous_clipping(GGraphicLibAPI *GLAPI)
{
    int onePrimitiveVertCount = 0;
    if(GLAPI->activePrimitiveType==GPrimitiveType::kTriangles)
    {
        onePrimitiveVertCount = 3;
    }
    else if(GLAPI->activePrimitiveType==GPrimitiveType::kLines)
    {
        onePrimitiveVertCount = 2;
    }
    int vertCount = clip_vertex(onePrimitiveVertCount);
    return vertCount;
}

int GShader::clip_vertex(int primitiveCount)
{
    std::vector<S_v2f> &vertsIn = v2f_data_arr_tmp;
    std::vector<S_v2f> &vertsOut = v2f_data_arr;
    int vertCount = clip_vertex(GFrustumPlaneType::kFPTFront, vertsIn, vertsOut);
    if(vertCount<primitiveCount)
    {
        return vertCount;
    }
    vertCount = clip_vertex(GFrustumPlaneType::kFPTBack, vertsOut, vertsIn);
    if(vertCount<primitiveCount)
    {
        return vertCount;
    }
    vertCount = clip_vertex(GFrustumPlaneType::kFPTLeft, vertsIn, vertsOut);
    if(vertCount<primitiveCount)
    {
        return vertCount;
    }
    vertCount = clip_vertex(GFrustumPlaneType::kFPTRight, vertsOut, vertsIn);
    if(vertCount<primitiveCount)
    {
        return vertCount;
    }
    vertCount = clip_vertex(GFrustumPlaneType::kFPTBottom, vertsIn, vertsOut);
    if(vertCount<primitiveCount)
    {
        return vertCount;
    }
    vertCount = clip_vertex(GFrustumPlaneType::kFPTTop, vertsOut, vertsIn);
    if(vertCount<primitiveCount)
    {
        return vertCount;
    }
    vertCount = clip_vertex(GFrustumPlaneType::kFPTW, vertsIn, vertsOut);

    // need generate primitive
    if(vertCount>primitiveCount)
    {
        vertCount = 0;
        v2f_data_arr_tmp.clear();
        for(int i=0; i<(int)v2f_data_arr.size()-2; i++)
        {
            int idx0 = 0;
            int idx1 = i + 1;
            int idx2 = i + 2;
            v2f_data_arr_tmp.push_back(v2f_data_arr[idx0]);
            v2f_data_arr_tmp.push_back(v2f_data_arr[idx1]);
            v2f_data_arr_tmp.push_back(v2f_data_arr[idx2]);

            vertCount += 3;
        }
    }
    return vertCount;
}

int GShader::clip_vertex(GFrustumPlaneType planeType, std::vector<S_v2f> &vertsIn, std::vector<S_v2f> &vertsOut)
{
    vertsOut.clear();
    for(size_t vertIdx=0; vertIdx<vertsIn.size(); vertIdx++)
    {
        size_t preIdx = (vertIdx-1+vertsIn.size()) % vertsIn.size();
        size_t curIdx = vertIdx;

        vec4 prePos = vertsIn[preIdx].gl_position;
        vec4 curPos = vertsIn[curIdx].gl_position;

        bool isPreInside = GRasterGPUPipeline::IsInsideFrustumPlane(planeType, prePos);
        bool isCurInside = GRasterGPUPipeline::IsInsideFrustumPlane(planeType, curPos);

        if(isPreInside != isCurInside)
        {
            float lerpFactor = GRasterGPUPipeline::CalcIntersectLerpFactor(planeType, prePos, curPos);
            S_v2f intersectPoint;
            intersectPoint.gl_position = lerp(prePos, curPos, lerpFactor);
            intersectPoint.normal = lerp(vertsIn[preIdx].normal, vertsIn[curIdx].normal, lerpFactor);
            intersectPoint.uv = lerp(vertsIn[preIdx].uv, vertsIn[curIdx].uv, lerpFactor);
            intersectPoint.wPos = lerp(vertsIn[preIdx].wPos, vertsIn[curIdx].wPos, lerpFactor);

            vertsOut.push_back(std::move(intersectPoint));
        }

        if(isCurInside)
        {
            vertsOut.push_back(vertsIn[curIdx]);
        }
    }
    return (int)vertsOut.size();
}

void GShader::calc_tangent(GGraphicLibAPI *GLAPI)
{
    if(GLAPI->activePrimitiveType==GPrimitiveType::kTriangles)
    {
        // tangent-xAxis normal-yAxis binormal-zAxis. the binormals below are all OK.
        //vec3 binormal = embed<double,3>(v2f_data_arr[1].uv - v2f_data_arr[0].uv, 0);
        //vec3 binormal = embed<double,3>(v2f_data_arr[2].uv - v2f_data_arr[0].uv, 0);
        vec3 binormal = vec3(v2f_data_arr[2].uv.x()-v2f_data_arr[0].uv.x(), 0, v2f_data_arr[2].uv.y()-v2f_data_arr[0].uv.y());
        for(int vertIdx=0; vertIdx<3; vertIdx++)
        {
            vec3 normal = v2f_data_arr[vertIdx].normal;
            vec3 tmpTangent = cross(normal, binormal);
            v2f_data_arr[vertIdx].tangent = tmpTangent;
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
    vec4 diffColor = GColor::ToFloat01Color(SampleTex(diffusemaps_, diff_mipmaptype, v2f.uv,fragIdx, diffuseColor));
    double alpha = diffColor.w();
    float NoL = 0;
    float HoN = 0;
    // don't normalize the wTangent!
    //v2f.wTangent.normalize();
    vec3 wBinormal = cross(v2f.wTangent, v2f.wNormal); //cross(v2f.wNormal, v2f.wTangent);
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
    // debug world tangent
    //col.SetXYZ((v2f.wTangent+vec3::one)*0.5f);
    // debug world normal
    //col.SetXYZ((wNormal+vec3::one)*0.5f);

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
        sampleRatio = min(sampleRatio, 20);
        int lod = std::max(0.0f,log2f(maxPixelCount/sampleRatio));
        lod = std::min(lod, (int)mipmaps->size()-1);
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































