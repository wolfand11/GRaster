#include "grastergpupipeline.h"
#include <QImage>
#include "glog.h"
#include "gshader.h"
#include <algorithm>
#include "ggraphiclibapi.h"
#include "ggameobject.h"

using namespace std;
using namespace GMath;

void GRasterGPUPipeline::ProcessAppData(GGraphicLibAPI* GLAPI, std::vector<S_abs_appdata *> appdataArr, int offset)
{
    int onePrimitiveVertCount = 1;
    if(GLAPI->activePrimitiveType == GPrimitiveType::kTriangles)
    {
        onePrimitiveVertCount = 3;
    }
    else if(GLAPI->activePrimitiveType == GPrimitiveType::kLines)
    {
        onePrimitiveVertCount = 2;
    }

    for(size_t i=offset; i<appdataArr.size(); i=i+onePrimitiveVertCount)
    {
        vec2 primitiveScreenPos[onePrimitiveVertCount];
        vec4 clipPos[onePrimitiveVertCount];
        for(int j=0; j<onePrimitiveVertCount; j++)
        {
            // vertex process
            auto appdata = appdataArr[i+j];
            clipPos[j] = GLAPI->activeShader->vertex(GLAPI, appdata, j);

            // ndc to viewport
            vec3 ndc = proj<double,3>(clipPos[j]/clipPos[j].w());
            primitiveScreenPos[j] = GGameObject::activeCamera->NDCPosToScreenPos(ndc);
        }
        GLAPI->activeShader->calc_tangent(GLAPI);
        // raster
        if(GLAPI->activePrimitiveType == GPrimitiveType::kTriangles)
        {
            // triangle
            vec3 triangleSurfaceNormal;
            if(GLAPI->currentFrontFace==GFrontFace::kClockwise)
            {
                triangleSurfaceNormal = cross(embed<double,3>(primitiveScreenPos[2]-primitiveScreenPos[0],0), embed<double,3>(primitiveScreenPos[1]-primitiveScreenPos[0], 0)).inverse();
            }
            else if(GLAPI->currentFrontFace==GFrontFace::kCounterClockwise)
            {
                triangleSurfaceNormal = cross(embed<double,3>(primitiveScreenPos[2]-primitiveScreenPos[0], 0), embed<double,3>(primitiveScreenPos[1]-primitiveScreenPos[0],0));
            }
            else
            {
                assert(false);
            }
            // HSR hide surface remove
            GCullFaceType curCullFaceType = GLAPI->cullFaceType;
            if(curCullFaceType==GCullFaceType::kFTShaderSetting)
            {
                curCullFaceType = GLAPI->activeShader->cullFaceType;
            }
            if(curCullFaceType == GCullFaceType::kFTBack)
            {
                if(dot(triangleSurfaceNormal, vec3(0,0, 1))>0)
                {
                    continue;
                }
            }
            else if(curCullFaceType == GCullFaceType::kFTFront)
            {
                if(dot(triangleSurfaceNormal, vec3(0,0,-1))>0)
                {
                    continue;
                }
            }
            if(GLAPI->activePolygonMode == GPolygonMode::kPMLine)
            {
                RasterLine(primitiveScreenPos[0].x(),primitiveScreenPos[0].y(),primitiveScreenPos[1].x(),primitiveScreenPos[1].y(),GLAPI);
                RasterLine(primitiveScreenPos[1].x(),primitiveScreenPos[1].y(),primitiveScreenPos[2].x(),primitiveScreenPos[2].y(),GLAPI);
                RasterLine(primitiveScreenPos[2].x(),primitiveScreenPos[2].y(),primitiveScreenPos[0].x(),primitiveScreenPos[0].y(),GLAPI);
            }
            else if(GLAPI->activePolygonMode == GPolygonMode::kPMFill)
            {
                // interpolation attribution
                // invoke pixel shader
                RasterTriangle(clipPos, primitiveScreenPos, GLAPI);
            }
        }
    }
}

void GRasterGPUPipeline::RasterTriangle(vec4* vertsClipPos, vec2* vertsScreenPos, GGraphicLibAPI *GLAPI)
{
    int debugPosX = 550;
    int debugPosY = 202;
    const std::vector<GColorBuffer*>& drawRenderBuffers = GLAPI->activeFramebuffer->GetDrawRenderBuffer();
    vec2i bufferSize = GLAPI->activeFramebuffer->GetSize();
    vec2i bboxmin = vec2i(bufferSize[0]-1,bufferSize[1]-1);
    vec2i bboxmax = vec2i::zero;
    vec2i max_boundary = vec2i(bufferSize[0]-1,bufferSize[1]-1);

    for(int vertIdx=0; vertIdx<3; vertIdx++)
    {
        for(int datumIdx=0; datumIdx<2; datumIdx++)
        {
            bboxmin[datumIdx] = max(                     0, min((int)vertsScreenPos[vertIdx][datumIdx], bboxmin[datumIdx]));
            bboxmax[datumIdx] = min(max_boundary[datumIdx], max((int)vertsScreenPos[vertIdx][datumIdx], bboxmax[datumIdx]));
        }
    }

    // four fragment in a group
    vec2i pixelCount = bboxmax - bboxmin + vec2i::one;
    if(pixelCount.x()%2 > 0)
    {
        if(bboxmin.x() > 0)
        {
            bboxmin.SetX(bboxmin.x()-1);
        }
        else
        {
            bboxmax.SetX(bboxmax.x()+1);
        }
    }
    if(pixelCount.y()%2 > 0)
    {
        if(bboxmin.y() > 0)
        {
            bboxmin.SetY(bboxmin.y()-1);
        }
        else
        {
            bboxmax.SetY(bboxmax.y()+1);
        }
    }

    vec2i screenPos;
    vec2i fragScreenPos[4];
    bool fragNeedRendering[4];
    vec3 fragScreenBCPos[4];
    vec3 fragClipBCPos[4];
    for(screenPos.SetX(bboxmin.x()); screenPos.x()<=bboxmax.x(); screenPos.SetX(screenPos.x()+2))
    {
        for(screenPos.SetY(bboxmin.y()); screenPos.y()<=bboxmax.y(); screenPos.SetY(screenPos.y()+2))
        {
            if(screenPos.x()>=debugPosX && screenPos.y()>=debugPosY)
            {
                GLog::LogInfo("");
            }
            fragScreenPos[0] = screenPos;
            fragScreenPos[1] = screenPos+vec2i(1,0);
            fragScreenPos[2] = screenPos+vec2i(0,1);
            fragScreenPos[3] = screenPos+vec2i(1,1);
            for(int fragIdx=0; fragIdx<4; fragIdx++)
            {
                fragScreenBCPos[fragIdx] = __Barycentric(vertsScreenPos[0], vertsScreenPos[1], vertsScreenPos[2], fragScreenPos[fragIdx]);
                fragClipBCPos[fragIdx] = fragScreenBCPos[fragIdx];
            }

            for(int fragIdx=0; fragIdx<4; fragIdx++)
            {
                if(fragScreenBCPos[fragIdx].x()<0 || fragScreenBCPos[fragIdx].y()<0 || fragScreenBCPos[fragIdx].z()<0)
                {
                    fragNeedRendering[fragIdx] = false;
                }
                else
                {
                    fragNeedRendering[fragIdx] = true;
                }
            }

            bool isAnyNeedRendering = false;
            for(int fragIdx=0; fragIdx<4; fragIdx++)
            {
                if(fragNeedRendering[fragIdx])
                {
                    isAnyNeedRendering = true;
                    break;
                }
            }
            // culling
            if(!isAnyNeedRendering) continue;

            // perspective correction
            for(int fragIdx=0; fragIdx<4; fragIdx++)
            {
                if(GGameObject::activeCamera->cameraType == GGameObject::GCameraType::kProjection)
                {
                    fragClipBCPos[fragIdx].SetX(fragScreenBCPos[fragIdx].x()/vertsClipPos[0].w());
                    fragClipBCPos[fragIdx].SetY(fragScreenBCPos[fragIdx].y()/vertsClipPos[1].w());
                    fragClipBCPos[fragIdx].SetZ(fragScreenBCPos[fragIdx].z()/vertsClipPos[2].w());
                    fragClipBCPos[fragIdx] = fragClipBCPos[fragIdx]/(fragClipBCPos[fragIdx].x()+fragClipBCPos[fragIdx].y()+fragClipBCPos[fragIdx].z());
                }
            }

            // interpolation attribution
            S_abs_v2f* interpolationData[4];
            for(int fragIdx=0; fragIdx<4; fragIdx++)
            {
                interpolationData[fragIdx] = GLAPI->activeShader->interpolation(GLAPI, fragClipBCPos[fragIdx], fragIdx);
            }

            // early-fragment test: scissor test -> multisample fragment op -> stencil test -> depth test --> occlusion query sample counting
            if(GLAPI->activeShader->useEarlyPerFragementTest)
            {
                for(int fragIdx=0; fragIdx<4; fragIdx++)
                {
                    ZDepthTest(GLAPI, fragScreenPos, interpolationData, fragNeedRendering, fragIdx);
                }
            }
            // invoke pixel shader
            for(int fragIdx=0; fragIdx<4; fragIdx++)
            {
                if(!fragNeedRendering[fragIdx]) continue;

                S_fout fout;
                GLAPI->activeShader->fragment(*interpolationData[fragIdx], fout, fragIdx);

                // merge: scissor test --> alpha to coverage op --> stencil test --> depth test --> blending --> dithering --> logic op --> addition multisample fragment op --> write to framebuffer
                // depth test
                if(!GLAPI->activeShader->useEarlyPerFragementTest)
                {
                    if(!ZDepthTest(GLAPI, fragScreenPos, interpolationData, fragNeedRendering, fragIdx)) continue;
                }
                for(size_t outColorIdx=0; outColorIdx<fout.colors.size(); outColorIdx++)
                {
                    GColorBuffer* drawRenderBuffer = drawRenderBuffers[outColorIdx];
                    vec4 srcColor = GColor::FastTonemap(fout.colors[outColorIdx]);
                    // blending
                    if(GLAPI->activeFramebuffer->IsBlendEnabled(drawRenderBuffer))
                    {
                        vec4 desColor = GColor::ToFloat01Color(drawRenderBuffer->GetColor(fragScreenPos[fragIdx].x(), fragScreenPos[fragIdx].y()));
                        srcColor = srcColor * srcColor.w() + desColor * (1-srcColor.w());
                    }
                    // write to framebuffer
                    drawRenderBuffer->SetColor(fragScreenPos[fragIdx].x(), fragScreenPos[fragIdx].y(), GColor::FromFloat01Color(srcColor));
                }
            }
        }
    }
}

bool GRasterGPUPipeline::ZDepthTest(GGraphicLibAPI *GLAPI, vec2i* fragScreenPos, S_abs_v2f* interpolationData[4], bool* fragNeedRendering, int fragIdx)
{
    if(!fragNeedRendering[fragIdx]) return false;

    float desDepth = GLAPI->activeFramebuffer->depthBuffer->GetFValue(fragScreenPos[fragIdx].x(),fragScreenPos[fragIdx].y());
    float srcDepth = 0;
    if(GLAPI->enableWBuffer)
    {
        srcDepth = GGameObject::activeCamera->ToWBufferValue(interpolationData[fragIdx]->gl_position.w());
    }
    else
    {
        srcDepth = interpolationData[fragIdx]->ndc().z();
    }
    if(srcDepth>desDepth)
    {
        fragNeedRendering[fragIdx] = false;
    }
    else
    {
        if(GLAPI->depthMask)
            GLAPI->activeFramebuffer->depthBuffer->SetFValue(fragScreenPos[fragIdx].x(),fragScreenPos[fragIdx].y(), srcDepth);
    }
    return fragNeedRendering[fragIdx];
}

void GRasterGPUPipeline::RasterLine(int x0, int y0, int x1, int y1, GGraphicLibAPI *GLAPI)
{
    int xStep = 1;
    if(GLAPI->activePolygonMode == GPolygonMode::kPMPoint)
    {
        xStep = 2;
    }
    float xDelta = x1-x0;
    float yDelta = y1-y0;
    bool yDeltaBigger = abs(yDelta) > abs(xDelta);
    if(yDeltaBigger)
    {
        // make x always bigger than y
        std::swap(x0,y0);
        std::swap(x1,y1);
        std::swap(xDelta, yDelta);
    }
    if(xDelta<0)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
        xDelta = -xDelta;
        yDelta = -yDelta;
    }

    float error = 0.0f;
    float errorStep = abs(yDelta)*xStep;
    for(int i=x0,j=y0; i<=x1; i+=xStep)
    {
        if(yDeltaBigger)
        {
            __SetColorBufferArrColor(GLAPI->activeFramebuffer->GetDrawRenderBuffer(), j, i, GColor::white);
        }
        else
        {
            __SetColorBufferArrColor(GLAPI->activeFramebuffer->GetDrawRenderBuffer(), i, j, GColor::white);
        }
        error += errorStep;
        if(error >= xDelta*xStep)
        {
            j += yDelta>0 ? xStep : -xStep;
            error -= xDelta*xStep;
        }
    }
}

void GRasterGPUPipeline::__SetColorBufferArrColor(const std::vector<GColorBuffer *> &colorBuffers, int i, int j, GColor color)
{
    for(auto cb : colorBuffers)
    {
        cb->SetColor(i, j, color);
    }
}
vec3 GRasterGPUPipeline::__Barycentric(vec2i t0, vec2i t1, vec2i t2, vec2i p)
{
    vec3 barycentric = cross(vec3(t1[0]-t0[0],t2[0]-t0[0], t0[0]-p[0]), vec3(t1[1]-t0[1],t2[1]-t0[1], t0[1]-p[1]));

    // t0 t1 t2 on the same line, dont need generate triangle ?
    if(abs(barycentric[2])<1)
    {
        return vec3(-1,1,1);
    }
    barycentric = vec3(1-(barycentric[0]+barycentric[1])/barycentric[2], barycentric[0]/barycentric[2], barycentric[1]/barycentric[2]);
    return barycentric;
}










































