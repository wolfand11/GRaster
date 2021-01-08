#include "ggraphiclibapi.h"
#include "grasterapi.h"

GGraphicLibAPI::GGraphicLibAPI()
{

}


GFrameBuffer *GGraphicLibAPI::GenFrameBuffer()
{
    return new GFrameBuffer();
}

void GGraphicLibAPI::BindFrameBuffer(GFrameBuffer *framebuffer)
{
    activeFramebuffer = framebuffer;
}

void GGraphicLibAPI::DeleteFrameBuffer(GFrameBuffer*& framebuffer)
{
    delete framebuffer;
    framebuffer = nullptr;
}

GColorBuffer *GGraphicLibAPI::GenRenderBuffer(int width, int height)
{
    auto buffer = new GColorBuffer(width, height);
    return buffer;
}

void GGraphicLibAPI::BindRenderBuffer(GColorBuffer *buffer)
{
    activeColorBuffer = buffer;
}

void GGraphicLibAPI::DeleteRenderBuffer(GRenderBuffer *&renderbuffer)
{
    delete renderbuffer;
    renderbuffer = nullptr;
}

void GGraphicLibAPI::AttachRenderBufferToFrameBuffer(GFrameBuffer *framebuffer, GColorBuffer *colorbuffer, int attachIndex)
{
    framebuffer->AttachRenderBuffer(colorbuffer, attachIndex);
}

void GGraphicLibAPI::ClearBuffer(int drawBuffer, GColor clearColor)
{
    activeFramebuffer->ClearRenderBuffer(drawBuffer, clearColor);
}

void GGraphicLibAPI::SetFrontFace(GFrontFace frontFace)
{
    currentFrontFace = frontFace;
}

void GGraphicLibAPI::SetCullFace(GFaceType faceType)
{
    cullFaceType = faceType;
}

void GGraphicLibAPI::SetEnableCullFace(bool enable)
{
    enableCullFace = enable;
}

void GGraphicLibAPI::SetEnableBlend(int renderBufferIndex, bool enable)
{
    if(!GFrameBuffer::CheckAttachIndexValid(renderBufferIndex)) return;

    enableBlend[renderBufferIndex] = enable;
}

void GGraphicLibAPI::SetScissorTest(int renderBufferIndex, bool enable)
{
    if(!GFrameBuffer::CheckAttachIndexValid(renderBufferIndex)) return;
    enableScissorTest[renderBufferIndex] = enable;
}

GDataBuffer* GGraphicLibAPI::GenDataBuffer(GDataBuffer::GDataBufferType bufferType)
{
    return new GDataBuffer(bufferType);
}

void GGraphicLibAPI::BindDataBuffer(GDataBuffer *buffer)
{
    if(buffer->bufferType==GDataBuffer::GDataBufferType::kArrayBuffer)
    {
        if(!CheckVertexAttribInfoObject()) return;
        activeVertexAttriInfoObject->vertexBuffer = buffer;
    }
}

void GGraphicLibAPI::FillDataBuffer(GDataBuffer::GDataBufferType dataBufferType, void *data, int size)
{
    if(dataBufferType==GDataBuffer::GDataBufferType::kArrayBuffer)
    {
        if(!CheckVertexAttribInfoObject()) return;
        activeVertexAttriInfoObject->vertexBuffer->SetData(data, size);
    }
}

bool GGraphicLibAPI::CheckVertexAttribInfoObject()
{
    if(activeVertexAttriInfoObject == nullptr)
    {
        GLog::LogError("activeVertexAttriInfoObject = null");
        return false;
    }
    return true;
}

GVertexAttribInfoObject *GGraphicLibAPI::GenVAO()
{
    return new GVertexAttribInfoObject();
}

void GGraphicLibAPI::BindVAO(GVertexAttribInfoObject *vao)
{
    activeVertexAttriInfoObject = vao;
}

void GGraphicLibAPI::VertexAttriPointer(int attriSlot, int attriDatumCount, int attriDatumType, bool normalize, int stride, int offset)
{
    activeVertexAttriInfoObject->vertexAttribInfoArray[attriSlot].datumCount = attriDatumCount;
    activeVertexAttriInfoObject->vertexAttribInfoArray[attriSlot].datumType = attriDatumType;
    activeVertexAttriInfoObject->vertexAttribInfoArray[attriSlot].normalize = normalize;
    activeVertexAttriInfoObject->vertexAttribInfoArray[attriSlot].stride = stride;
    activeVertexAttriInfoObject->vertexAttribInfoArray[attriSlot].offset = offset;
}

void GGraphicLibAPI::SetEnableVertexAttriSlot(int attriSlot, bool enable)
{
    activeVertexAttriInfoObject->vertexAttribInfoArray[attriSlot].enable = enable;
}

void GGraphicLibAPI::DrawArrays(GGraphicLibAPI::GPrimitiveType t, int vertexOffsetCount, int vertexCount)
{
    if(!CheckVertexAttribInfoObject()) return;
    if(t == GPrimitiveType::kTriangles)
    {
    }
}
