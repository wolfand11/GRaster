#include "grastergpupipeline.h"
#include <QImage>
#include "glog.h"
#include "gshader.h"
#include <algorithm>
#include "ggraphiclibapi.h"

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
        for(int j=0; j<onePrimitiveVertCount; j++)
        {
            auto appdata = appdataArr[i+j];
            GLAPI->activeShader->vertex(GLAPI, appdata, (i-offset)%onePrimitiveVertCount);
        }
        // ndc to viewPort
        // raster
        {
            // invoke pixel shader
            // GLAPI->activeShader->fragment();
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
        __DrawLine(t0.x, t0.y, t1.x, t1.y, color, colorBuffers, depthBuffer, stencilBuffer, shader, mode);
        __DrawLine(t1.x, t1.y, t2.x, t2.y, color, colorBuffers, depthBuffer, stencilBuffer, shader, mode);
        __DrawLine(t2.x, t2.y, t0.x, t0.y, color, colorBuffers, depthBuffer, stencilBuffer, shader, mode);
        return;
    }

    if(t0.y > t1.y) swap(t0, t1);
    if(t0.y > t2.y) swap(t0, t2);
    if(t1.y > t2.y) swap(t1, t2);
    int total_h = t2.y-t0.y;
    int t1_t0_h = t1.y-t0.y;
    for(int y=0; y<total_h; y++)
    {
        bool drawPart2 = (y>(t1.y-t0.y));
        float partH = drawPart2 ? t2.y-t1.y : t1_t0_h;
        float alpha = drawPart2 ? (y-t1_t0_h) : y;
        alpha = alpha / partH;
        float beta = (float)y/total_h;

        vec2i A = drawPart2 ? t1+alpha*(t2 - t1) : t0+alpha*(t1 - t0);
        vec2i B = t0 + beta*(t2-t0);

        if(A.x>B.x) swap(A,B);
        for(int x=A.x; x<=B.x; x++)
        {
            __SetColorBufferArrColor(colorBuffers,x, t0.y+y, color);
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
    for(p.x=bboxmin.x; p.x<=bboxmax.x; p.x++)
    {
        for(p.y=bboxmin.y; p.y<=bboxmax.y; p.y++)
        {
            vec3 p_bcpos = __Barycentric(t0, t1, t2, p);
            if(p_bcpos.x<0 || p_bcpos.y<0 || p_bcpos.z<0) continue;
            __SetColorBufferArrColor(colorBuffers, p.x, p.y, color);
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
    vec3 barycentric = cross(vec3(t1.x-t0.x,t2.x-t0.x, t0.x-p.x), vec3(t1.y-t0.y,t2.y-t0.y, t0.y-p.y));

    // t0 t1 t2 on the same line, dont need generate triangle ?
    if(abs(barycentric.z)<1)
    {
        return vec3(-1,1,1);
    }
    barycentric = vec3(1-(barycentric.x+barycentric.y)/barycentric.z, barycentric.x/barycentric.z, barycentric.y/barycentric.z);
    return barycentric;
}










































