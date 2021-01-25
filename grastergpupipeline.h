#ifndef GRASTERGPUPIPELINE_H
#define GRASTERGPUPIPELINE_H
#include <iostream>
#include "gbuffer.h"
#include <vector>
#include "gmath.h"
#include "gshader.h"

class GGraphicLibAPI;
class GRasterGPUPipeline
{
public:
    static void ProcessAppData(GGraphicLibAPI* GLAPI, std::vector<S_abs_appdata*> appdataArr, int offset);
    static void RasterTriangle(GMath::vec4* vertsClipPos, GMath::vec2* vertsScreenPos, GGraphicLibAPI* GLAPI);
    static bool ZDepthTest(GGraphicLibAPI *GLAPI, GMath::vec2i* fragScreenPos, S_abs_v2f* interpolationData[4], bool* fragNeedRendering, int fragIdx);
    static void RasterLine(int x0, int y0, int x1, int y1, GGraphicLibAPI* GLAPI);

    static void DrawLine(int x0, int y0, int x1, int y1, GColor color, std::vector<GColorBuffer*>& colorBuffers, GDepthStencilBuffer* depthBuffer, GDepthStencilBuffer *stencilBuffer, GShader* shader, GPolygonMode mode);
    static void DrawTriangle(GMath::vec2i t0, GMath::vec2i t1, GMath::vec2i t2, GColor color, std::vector<GColorBuffer*>& colorBuffers, GDepthStencilBuffer* depthBuffer, GDepthStencilBuffer *stencilBuffer, GShader* shader, GPolygonMode mode);

private:
    static void _RasterTriangle(GMath::vec2 verts[3], GGraphicLibAPI* GLAPI);

    static void __DrawLine(int x0, int y0, int x1, int y1, GColor color,std::vector<GColorBuffer*>& colorBuffers, GDepthStencilBuffer* depthBuffer, GDepthStencilBuffer *stencilBuffer, GShader* shader, GPolygonMode mode);
    static void __DrawTriangleV1(GMath::vec2i t0, GMath::vec2i t1, GMath::vec2i t2, GColor color, std::vector<GColorBuffer*>& colorBuffers, GDepthStencilBuffer* depthBuffer, GDepthStencilBuffer *stencilBuffer, GShader* shader, GPolygonMode mode);
    static void __DrawTriangleV2(GMath::vec2i t0, GMath::vec2i t1, GMath::vec2i t2, GColor color, std::vector<GColorBuffer*>& colorBuffers, GDepthStencilBuffer* depthBuffer, GDepthStencilBuffer *stencilBuffer, GShader* shader, GPolygonMode mode);
    static void __SetColorBufferArrColor(const std::vector<GColorBuffer*>& colorBuffers, int i, int j, GColor color);
    static  GMath::vec3 __Barycentric(GMath::vec2i t0, GMath::vec2i t1, GMath::vec2i t2, GMath::vec2i p);
};

#endif // GRASTERGPUPIPELINE_H
