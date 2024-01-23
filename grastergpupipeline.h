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
    static void ProcessAppData(GGraphicLibAPI* GLAPI, std::vector<S_abs_appdata*>& appdataArr, int offset);
    static void RasterTriangle(std::vector<GMath::vec4>& vertsClipPos, std::vector<GMath::vec2>& vertsScreenPos, GGraphicLibAPI* GLAPI);
    static bool ZDepthTest(GGraphicLibAPI *GLAPI, GMath::vec2i* fragScreenPos, S_abs_v2f* interpolationData[4], bool* fragNeedRendering, int fragIdx);
    static void RasterLine(int x0, int y0, int x1, int y1, GGraphicLibAPI* GLAPI);

    static bool IsInsideFrustumPlane(GFrustumPlaneType planeType, GMath::vec4 pos);
    static bool IsInsideFrustumPlane(GMath::vec4 pos);
    static float CalcIntersectLerpFactor(GFrustumPlaneType planeType, GMath::vec4 pos0, GMath::vec4 pos1);
private:
    static bool ClipVertex(GGraphicLibAPI* GLAPI, int primitiveCount, std::vector<S_abs_appdata*>& vertsIn, std::vector<S_abs_appdata*>& vertsOut);
    static bool ClipVertex(GGraphicLibAPI* GLAPI, GFrustumPlaneType planeType, int primitiveCount, std::vector<S_abs_appdata*>& vertsIn, std::vector<S_abs_appdata*>& vertsOut);
    static void __SetColorBufferArrColor(const std::vector<GColorBuffer*>& colorBuffers, int i, int j, GColor color);
    static  GMath::vec3 __Barycentric(GMath::vec2i t0, GMath::vec2i t1, GMath::vec2i t2, GMath::vec2i p);
};

#endif // GRASTERGPUPIPELINE_H
