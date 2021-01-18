#ifndef GGRAPHICLIBAPI_H
#define GGRAPHICLIBAPI_H
#include "glog.h"
#include "gbuffer.h"
#include "grastergpupipeline.h"
#include "ggraphiclibdefine.h"
#include "gshader.h"
#include <set>

class GGraphicLibAPI
{
public:
    GGraphicLibAPI();
    ~GGraphicLibAPI();
    GFrameBuffer* GenFrameBuffer();
    void BindFrameBuffer(GFrameBuffer* framebuffer);
    void DeleteFrameBuffer(GFrameBuffer*& framebuffer);

    GColorBuffer* GenRenderBuffer(int width, int height);
    void BindRenderBuffer(GColorBuffer* buffer);
    void DeleteRenderBuffer(GRenderBuffer*& renderbuffer);

    void AttachRenderBufferToFrameBuffer(GFrameBuffer* framebuffer, GColorBuffer* colorbuffer, int attachIndex);
    // Clear Color Buffer
    void Clear(GColor clearColor);
    void ClearDepth(int clearValue);
    void ClearStencil(int clearValue);
    // Clear Depth Stencil
    void ClearBuffer(int drawBuffer, GColor clearColor);
    void ClearDepthBuffer(GRenderBufferType renderBufferType, int clearValue);
    void ClearStencilBuffer(GRenderBufferType renderBufferType, int clearValue);
    // Select Draw Buffer
    void DrawRenderBuffer(GRenderBufferType renderBufferType);
    void DrawRenderBuffer(std::initializer_list<GRenderBufferType> renderBufferTypes);

    void SetFrontFace(GFrontFace frontFace);
    void SetCullFace(GFaceType faceType);

    // Enable Disable Capability
    void SetEnableCullFace(bool enable=true);
    void SetEnableBlend(GRenderBufferType renderBufferType, bool enable=true);
    void SetScissorTest(GRenderBufferType renderBufferType, bool enable=true);

    // Data Buffer
    GDataBuffer* GenDataBuffer(GDataBufferType bufferType);
    void BindDataBuffer(GDataBuffer* buffer);
    void FillDataBuffer(GDataBufferType dataBufferType, void* data, int size, int offset=0);
    void ClearDataBuffer();

    // Vertex Attrib Info Object == OpenGL Vertex Array Object
    bool CheckVertexAttribInfoObject();
    GVertexAttribInfoObject* GenVAO();
    void BindVAO(GVertexAttribInfoObject* vao);
    void VertexAttriPointer(int attriSlot, int attriDatumCount,GDatumType attriDatumType, bool normalize, int stride, int offset);
    void SetEnableVertexAttriSlot(int attriSlot, bool enable);

    // Draw
    bool CheckDrawValid(GPrimitiveType t, int vertexCount);
    void DrawArrays(GPrimitiveType t, int vertexOffsetCount, int vertexCount);
    void DrawElements(GPrimitiveType t, int vertCount, GDatumType indexType, int offsetBytes);
    void DrawElementsBaseVertex(GPrimitiveType t, int count, GDatumType indexType, int offsetBytes, int baseVertex);
    void SetPolygonMode(GPolygonMode mode);

    // Shader
    GShader* CreateProgram(GShaderType shaderType=GShaderType::kSTDefault);
    void UseProgram(GShader* shader);

    GFrameBuffer* activeFramebuffer;
    GColorBuffer* activeColorBuffer;
    GDepthStencilBuffer* activeDepthBuffer;
    GDepthStencilBuffer* activeStencilBuffer;
    GPolygonMode activePolygonMode;
    GPrimitiveType activePrimitiveType;

    GFrontFace currentFrontFace;
    GFaceType cullFaceType;
    bool enableCullFace;

    GVertexAttribInfoObject* activeVertexAttriInfoObject;
    GShader* activeShader;
private:
    // Draw Helper
    void DoDraw();
    void ParseVertexData(int vertexCount, GVertexAttrib::AttriInfo_AttribArr_Arr& vertexAttribArr_Arr);
    void ParseElemData(int vertexCount, GDatumType indexType, std::vector<int>& vertexIndexArr, int offsetBytes);
    void CreateAppData(std::vector<S_abs_appdata*>& appData, GVertexAttrib::AttriInfo_AttribArr_Arr& vertexAttribArr_Arr, std::vector<int>* elemIdexArr=nullptr);

    std::set<GFrameBuffer*> __frameBufferSet;
    std::set<GRenderBuffer*> __renderBufferSet;
    std::set<GDataBuffer*> __dataBufferSet;
    std::set<GVertexAttribInfoObject*> __vertexAttriInfoObjSet;
    std::set<GShader*> __shaderSet;

    std::vector<GColorBuffer*> drawRenderBuffers;
};

#endif // GGRAPHICLIBAPI_H
