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
        for(int j=0; j<onePrimitiveVertCount; j++)
        {
            // vertex process
            auto appdata = appdataArr[i+j];
            GLAPI->activeShader->vertex(GLAPI, appdata, (i-offset)%onePrimitiveVertCount);

            // ndc to viewport
            vec3 ndc = proj<double,3>(GLAPI->activeShader->GetV2f(j)->ndc());
            const mat3* viewportMat;
            GGameObject::activeCamera->Viewport(viewportMat);
            primitiveScreenPos[j] = proj<double,2>((*viewportMat) * ndc);
        }
        // raster
        if(GLAPI->activePrimitiveType == GPrimitiveType::kTriangles)
        {
            // interpolation attribution
            // invoke pixel shader
            RasterTriangle(primitiveScreenPos, GLAPI);
        }
    }
}

void GRasterGPUPipeline::RasterTriangle(vec2* verts, GGraphicLibAPI *GLAPI)
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
            bboxmin[datumIdx] = max(                     0, min((int)verts[vertIdx][datumIdx], bboxmin[datumIdx]));
            bboxmax[datumIdx] = min(max_boundary[datumIdx], max((int)verts[vertIdx][datumIdx], bboxmax[datumIdx]));
        }
    }

    vec2i screenPos;
    for(screenPos[0] = bboxmin[0]; screenPos[0]<bboxmax[0]; screenPos[0]++)
    {
        for(screenPos[1] = bboxmin[1]; screenPos[1]<bboxmax[1]; screenPos[1]++)
        {
            vec3 p_bcpos = __Barycentric(verts[0], verts[1], verts[2], screenPos);
            // culling
            if(p_bcpos[0]<0 || p_bcpos[1]<0 || p_bcpos[2]<0) continue;

            // interpolation attribution
            S_abs_v2f& interpolationData = GLAPI->activeShader->interpolation(GLAPI, p_bcpos);
            // early-fragment test: scissor test -> multisample fragment op -> stencil test -> depth test --> occlusion query sample counting
            // invoke pixel shader
            S_fout fout;
            GLAPI->activeShader->fragment(interpolationData, fout);
            // merge: scissor test --> alpha to coverage op --> stencil test --> depth test --> blending --> dithering --> logic op --> addition multisample fragment op --> write to framebuffer
            for(size_t outColorCount=0; outColorCount<fout.colors.size(); outColorCount++)
            {
                GColorBuffer* drawRenderBuffer = drawRenderBuffers[outColorCount];
                GColor srcColor = GColor::FastTonemap(fout.colors[outColorCount]);
                // blending
                if(GLAPI->activeFramebuffer->IsBlendEnabled(drawRenderBuffer))
                {
                    GColor desColor = drawRenderBuffer->GetColor(screenPos[0], screenPos[1]);
                }
                // write to framebuffer
                drawRenderBuffer->SetColor(screenPos[0], screenPos[1], srcColor);
            }
        }
    }
}

void GRasterGPUPipeline::DrawLine(int x0, int y0, int x1, int y1, GColor color, std::vector<GColorBuffer *>& colorBuffers, GDepthStencilBuffer *depthBuffer, GDepthStencilBuffer *stencilBuffer, GShader *shader, GPolygonMode mode)
{
    __DrawLine(x0, y0, x1, y1, color, colorBuffers, depthBuffer, stencilBuffer, shader, mode);
}

void GRasterGPUPipeline::DrawTriangle(GMath::vec2i t0, GMath::vec2i t1, GMath::vec2i t2, GColor color, std::vector<GColorBuffer *>& colorBuffers, GDepthStencilBuffer *depthBuffer, GDepthStencilBuffer *stencilBuffer, GShader *shader, GPolygonMode mode)
{
    //__DrawTriangleV1(t0,t1,t2,color,colorBuffers,depthBuffer,stencilBuffer,shader, mode);
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

void GRasterGPUPipeline::__SetColorBufferArrColor(std::vector<GColorBuffer *> &colorBuffers, int i, int j, GColor color)
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










































