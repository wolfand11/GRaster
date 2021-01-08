#ifndef GRASTERAPI_H
#define GRASTERAPI_H
#include <iostream>
#include "gbuffer.h"

class GRasterAPI
{
public:
    GRasterAPI();

    static void DrawLine(int x0, int y0, int x1, int y1, GColor color, GColorBuffer* colorBuffer, GDepthBuffer* depthBuffer);
};

#endif // GRASTERAPI_H
