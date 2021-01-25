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
        // raster
        if(GLAPI->activePrimitiveType == GPrimitiveType::kTriangles)
        {
            // triangle
            vec3 triangleSurfaceNormal;
            if(GLAPI->currentFrontFace==GFrontFace::kClockwise)
            {
                triangleSurfaceNormal = cross(embed<double,3>(primitiveScreenPos[2]-primitiveScreenPos[0],0), embed<double,3>(primitiveScreenPos[1]-primitiveScreenPos[0], 0));
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
                GLAPI->activeShader->fragment(*interpolationData[fragIdx], fout);

                // merge: scissor test --> alpha to coverage op --> stencil test --> depth test --> blending --> dithering --> logic op --> addition multisample fragment op --> write to framebuffer
                // depth test
                if(!ZDepthTest(GLAPI, fragScreenPos, interpolationData, fragNeedRendering, fragIdx)) continue;
                for(size_t outColorIdx=0; outColorIdx<fout.colors.size(); outColorIdx++)
                {
                    GColorBuffer* drawRenderBuffer = drawRenderBuffers[outColorIdx];
                    GColor srcColor = GColor::FastTonemap(fout.colors[outColorIdx]);
                    // blending
                    if(GLAPI->activeFramebuffer->IsBlendEnabled(drawRenderBuffer))
                    {
                        GColor desColor = drawRenderBuffer->GetColor(fragScreenPos[fragIdx].x(), fragScreenPos[fragIdx].y());
                    }
                    // write to framebuffer
                    drawRenderBuffer->SetColor(fragScreenPos[fragIdx].x(), fragScreenPos[fragIdx].y(), srcColor);
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
        srcDepth = GGameObject::activeCamera->ToWBufferValue(interpolationData[fragIdx]->ndc().w());
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

void GRasterGPUPipeline::DrawLine(int x0, int y0, int x1, int y1, GColor color, std::vector<GColorBuffer *>& colorBuffers, GDepthStencilBuffer *depthBuffer, GDepthStencilBuffer *stencilBuffer, GShader *shader, GPolygonMode mode)
{
    __DrawLine(x0, y0, x1, y1, color, colorBuffers, depthBuffer, stencilBuffer, shader, mode);
}

void GRasterGPUPipeline::DrawTriangle(GMath::vec2i t0, GMath::vec2i t1, GMath::vec2i t2, GColor color, std::vector<GColorBuffer *>& colorBuffers, GDepthStencilBuffer *depthBuffer, GDepthStencilBuffer *stencilBuffer, GShader *shader, GPolygonMode mode)
{
    __DrawTriangleV2(t0,t1,t2,color,colorBuffers,depthBuffer,stencilBuffer,shader, mode);
}

void GRasterGPUPipeline::__DrawLine(int x0, int y0, int x1, int y1, GColor color, std::vector<GColorBuffer *>& colorBuffers, GDepthStencilBuffer *depthBuffer, GDepthStencilBuffer *stencilBuffer, GShader *shader, GPolygonMode mode)
{
    int xStep = 1;
    if(mode == GPolygonMode::kPMPoint)
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
            __SetColorBufferArrColor(colorBuffers, j, i, color);
        }
        else
        {
            __SetColorBufferArrColor(colorBuffers, i, j, color);
        }
        error += errorStep;
        if(error >= xDelta*xStep)
        {
            j += yDelta>0 ? xStep : -xStep;
            error -= xDelta*xStep;
        }
    }
}

void GRasterGPUPipeline::__DrawTriangleV1(GMath::vec2i t0, GMath::vec2i t1, GMath::vec2i t2, GColor color, std::vector<GColorBuffer *>& colorBuffers, GDepthStencilBuffer *depthBuffer, GDepthStencilBuffer *stencilBuffer, GShader *shader, GPolygonMode mode)
{
    if(mode == GPolygonMode::kPMPoint || mode==GPolygonMode::kPMLine)
    {
        __DrawLine(t0[0], t0[1], t1[0], t1[1], color, colorBuffers, depthBuffer, stencilBuffer, shader, mode);
        __DrawLine(t1[0], t1[1], t2[0], t2[1], color, colorBuffers, depthBuffer, stencilBuffer, shader, mode);
        __DrawLine(t2[0], t2[1], t0[0], t0[1], color, colorBuffers, depthBuffer, stencilBuffer, shader, mode);
        return;
    }

    if(t0[1] > t1[1]) swap(t0, t1);
    if(t0[1] > t2[1]) swap(t0, t2);
    if(t1[1] > t2[1]) swap(t1, t2);
    int total_h = t2[1]-t0[1];
    int t1_t0_h = t1[1]-t0[1];
    for(int y=0; y<total_h; y++)
    {
        bool drawPart2 = (y>(t1[1]-t0[1]));
        float partH = drawPart2 ? t2[1]-t1[1] : t1_t0_h;
        float alpha = drawPart2 ? (y-t1_t0_h) : y;
        alpha = alpha / partH;
        float beta = (float)y/total_h;

        vec2i A = drawPart2 ? t1+alpha*(t2 - t1) : t0+alpha*(t1 - t0);
        vec2i B = t0 + beta*(t2-t0);

        if(A[0]>B[0]) swap(A,B);
        for(int x=A[0]; x<=B[0]; x++)
        {
            __SetColorBufferArrColor(colorBuffers,x, t0[1]+y, color);
        }
    }
}

void GRasterGPUPipeline::__DrawTriangleV2(vec2i t0, vec2i t1, vec2i t2, GColor color, std::vector<GColorBuffer *> &colorBuffers, GDepthStencilBuffer *depthBuffer, GDepthStencilBuffer *stencilBuffer, GShader *shader, GPolygonMode mode)
{
    vec2i vertex_arr[] = {t0, t1, t2};
    vec2i bboxmin(colorBuffers[0]->width-1, colorBuffers[0]->height-1);
    vec2i bboxmax(0, 0);
    vec2i max_boundary(colorBuffers[0]->width-1, colorBuffers[0]->height-1);
    for(int i=0; i<3; i++)
    {
        for(int j=0; j<2; j++)
        {
            bboxmin[j] = max(0,               min(vertex_arr[i][j], bboxmin[j]));
            bboxmax[j] = min(max_boundary[j], max(vertex_arr[i][j], bboxmax[j]));
        }
    }
    vec2i p;
    for(p[0]=bboxmin[0]; p[0]<=bboxmax[0]; p[0]++)
    {
        for(p[1]=bboxmin[1]; p[1]<=bboxmax[1]; p[1]++)
        {
            vec3 p_bcpos = __Barycentric(t0, t1, t2, p);
            if(p_bcpos[0]<0 || p_bcpos[1]<0 || p_bcpos[2]<0) continue;
            __SetColorBufferArrColor(colorBuffers, p[0], p[1], color);
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










































