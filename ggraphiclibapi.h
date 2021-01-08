#ifndef GGRAPHICLIBAPI_H
#define GGRAPHICLIBAPI_H
#include "glog.h"
#include "gbuffer.h"

class GGraphicLibAPI
{
public:
    enum GFrontFace
    {
        kCounterClockwise,
        kClockwise
    };

    enum GFaceType
    {
        kFront,
        kBack,
        kFrontAndBack
    };

    enum GPrimitiveType
    {
        kPoints,
        kLines,
        kLineLoop,
        kTriangles,
    };

    GGraphicLibAPI();
    GFrameBuffer* GenFrameBuffer();
    void BindFrameBuffer(GFrameBuffer* framebuffer);
    void DeleteFrameBuffer(GFrameBuffer*& framebuffer);

    GColorBuffer* GenRenderBuffer(int width, int height);
    void BindRenderBuffer(GColorBuffer* buffer);
    void DeleteRenderBuffer(GRenderBuffer*& renderbuffer);

    void AttachRenderBufferToFrameBuffer(GFrameBuffer* framebuffer, GColorBuffer* colorbuffer, int attachIndex);
    // Clear Color Buffer
    void ClearBuffer(int drawBuffer, GColor clearColor);
    // Clear Depth Stencil
    void ClearBuffer(int bufferType, int clearValue);
    void DrawRenderBuffer(int drawBufferIndex);

    void SetFrontFace(GFrontFace frontFace);
    void SetCullFace(GFaceType faceType);

    // Enable Disable Capability
    void SetEnableCullFace(bool enable=true);
    void SetEnableBlend(int renderBufferIndex, bool enable=true);
    void SetScissorTest(int renderBufferIndex, bool enable=true);

    // Data Buffer
    GDataBuffer* GenDataBuffer(GDataBuffer::GDataBufferType bufferType);
    void BindDataBuffer(GDataBuffer* buffer);
    void FillDataBuffer(GDataBuffer::GDataBufferType dataBufferType, void* data, int size);
    void ClearDataBuffer();

    // Vertex Attrib Info Object == OpenGL Vertex Array Object
    bool CheckVertexAttribInfoObject();
    GVertexAttribInfoObject* GenVAO();
    void BindVAO(GVertexAttribInfoObject* vao);
    void VertexAttriPointer(int attriSlot, int attriDatumCount, int attriDatumType, bool normalize, int stride, int offset);
    void SetEnableVertexAttriSlot(int attriSlot, bool enable);

    // Draw
    void DrawArrays(GPrimitiveType t, int vertexOffsetCount, int vertexCount);
    void DrawElements(GPrimitiveType t, int count, int indexType, int offsetBytes);
    void DrawElementsBaseVertex(GPrimitiveType t, int count, int indexType, int offsetBytes, int baseVertex);
private:
    GFrameBuffer* activeFramebuffer;
    GColorBuffer* activeColorBuffer;

    GFrontFace currentFrontFace;
    GFaceType cullFaceType;
    bool enableCullFace;
    bool enableBlend[GFrameBuffer::MAX_COLORBUFF_COUNT];
    bool enableScissorTest[GFrameBuffer::MAX_COLORBUFF_COUNT];

    GVertexAttribInfoObject* activeVertexAttriInfoObject;
};

#endif // GGRAPHICLIBAPI_H
