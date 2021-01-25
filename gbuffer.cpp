#include "gbuffer.h"
#include "glog.h"
using namespace std;
using namespace GMath;

GColor GColor::white = {255,255,255,255};
GColor GColor::black = {0,0,0,255};
GColor GColor::red = {255,0,0,255};
GColor GColor::green = {0,255,0,255};
GColor GColor::blue = {0,0,255,255};
GColor GColor::gray = {128,128,128,255};

bool GFrameBuffer::CheckAttachIndexValid(int index)
{
    if(index < 0 || index>=MAX_COLORBUFF_COUNT)
    {
        GLog::LogError("index = ", index);
        return false;
    }
    return true;
}

void GFrameBuffer::AttachRenderBuffer(GColorBuffer *colorbuffer, int index)
{
    if(!CheckAttachIndexValid(index)) return;
    if(colorBuffer[index]!=nullptr)
    {
        SetEnableBlend((GRenderBufferType)index, true);
        SetEnableScissorTest((GRenderBufferType)index, false);
    }
    colorBuffer[index] = colorbuffer;
}

void GFrameBuffer::AttachRenderBuffer(GDepthStencilBuffer *depthStencilBuffer, bool isDepth)
{
    if(isDepth)
    {
        depthBuffer = depthStencilBuffer;
    }
    else
    {
        stencilBuffer = depthStencilBuffer;
    }
}

void GFrameBuffer::ClearRenderBuffer(int index, GColor clearColor)
{
    if(!CheckAttachIndexValid(index)) return;

    if(colorBuffer[index] == nullptr)
    {
        GLog::LogError("colorBuffer is nullptr. index = ", index);
        return;
    }
    colorBuffer[index]->Clear(clearColor);
}

void GFrameBuffer::ClearDepthBuffer(float clearDepthValue)
{
    if(depthBuffer==nullptr)
    {
        GLog::LogError("depthBuffer==nullptr");
        return;
    }
    depthBuffer->ClearF(clearDepthValue);
}

void GFrameBuffer::ClearStencilBuffer(int clearValue)
{
    if(stencilBuffer==nullptr)
    {
        GLog::LogError("stencilBuffer==nullptr");
        return;
    }
    stencilBuffer->Clear(clearValue);
}

void GFrameBuffer::DrawRenderBuffer(std::initializer_list<GRenderBufferType> renderBufferTypes)
{
    this->drawRenderBufferTypes.clear();
    this->drawRenderBufferTypes.assign(renderBufferTypes.begin(),renderBufferTypes.end());

    CheckRenderBufferSizeValid();
}

bool GFrameBuffer::CheckRenderBufferSizeValid()
{
    int w = -1;
    int h = -1;
    std::vector<GColorBuffer*> drawRenderBuffers = GetDrawRenderBuffer();
    for(auto colorBuffer : drawRenderBuffers)
    {
        if(w<0 || h<0)
        {
            w = colorBuffer->width;
            h = colorBuffer->height;
        }
        if(w != colorBuffer->width || h!=colorBuffer->height)
        {
            GLog::LogError("preW = ", w, " preH = ", h, " curW = ", colorBuffer->width, "curH = ", colorBuffer->height);
            return false;
        }
    }
    return true;
}

GColorBuffer *GFrameBuffer::GetRenderBufer(GRenderBufferType renderBufferType)
{
    if(!CheckAttachIndexValid(renderBufferType)) return nullptr;

    return colorBuffer[renderBufferType];
}

const std::vector<GColorBuffer*>& GFrameBuffer::GetDrawRenderBuffer()
{
    drawRenderBuffers.clear();

    for(auto rbt : drawRenderBufferTypes)
    {
        drawRenderBuffers.push_back(GetRenderBufer(rbt));
    }
    return drawRenderBuffers;
}

int GFrameBuffer::GetDrawRenderBufferCount()
{
    return drawRenderBufferTypes.size();
}

GMath::vec2i GFrameBuffer::GetSize()
{
    vec2i size = {-1,-1};
    std::vector<GColorBuffer*> drawRenderBuffers = GetDrawRenderBuffer();
    if(drawRenderBuffers.size()>0)
    {
        size.SetX(drawRenderBuffers[0]->width);
        size.SetY(drawRenderBuffers[0]->height);
    }
    return size;
}

GRenderBufferType GFrameBuffer::GetRenderBufferType(GRenderBuffer *renderBuffer)
{
    for(int rbt = GRenderBufferType::kRBFront; rbt<MAX_COLORBUFF_COUNT; rbt++)
    {
        if(renderBuffer == colorBuffer[rbt])
        {
            return (GRenderBufferType)rbt;
        }
    }
    return GRenderBufferType::kRBNon;
}

bool GFrameBuffer::IsBlendEnabled(GRenderBufferType renderBufferType)
{
    if(!CheckAttachIndexValid(renderBufferType)) return false;
    for(size_t i=0; i<enableBlend.size(); i++)
    {
        if(get<1>(enableBlend[i]) == renderBufferType)
        {
            return get<0>(enableBlend[i]);
        }
    }
    return false;
}

bool GFrameBuffer::IsBlendEnabled(GRenderBuffer *renderBuffer)
{
    return IsBlendEnabled(GetRenderBufferType(renderBuffer));
}

void GFrameBuffer::SetEnableBlend(GRenderBufferType renderBufferType, bool enable)
{
    if(!CheckAttachIndexValid(renderBufferType)) return;
    for(size_t i=0; i<enableBlend.size(); i++)
    {
        if(get<1>(enableBlend[i]) == renderBufferType)
        {
            get<0>(enableBlend[i]) = enable;
            return;
        }
    }
    enableBlend.push_back(make_tuple(enable,renderBufferType));
}

bool GFrameBuffer::IsScissorTestEnabled(GRenderBufferType renderBufferType)
{
    if(!CheckAttachIndexValid(renderBufferType)) return false;
    for(size_t i=0; i<enableScissorTest.size(); i++)
    {
        if(get<1>(enableScissorTest[i]) == renderBufferType)
        {
            return get<0>(enableScissorTest[i]);
        }
    }
    return false;
}

void GFrameBuffer::SetEnableScissorTest(GRenderBufferType renderBufferType, bool enable)
{
    if(!CheckAttachIndexValid(renderBufferType)) return;
    for(size_t i=0; i<enableScissorTest.size(); i++)
    {
        if(get<1>(enableScissorTest[i]) == renderBufferType)
        {
            get<0>(enableScissorTest[i]) = enable;
            return;
        }
    }
    enableBlend.push_back(make_tuple(enable,renderBufferType));
}

bool GVertexAttribInfoObject::IsSlotExist(int slot)
{
    for(auto attriInfo : vertexAttribInfoArray)
    {
        if(attriInfo.slot == slot)
        {
            return true;
        }
    }
    return false;
}

GVertexAttribInfo* GVertexAttribInfoObject::GetVertexAttriInfo(int slot)
{
    for(size_t i=0; i<vertexAttribInfoArray.size(); i++)
    {
        if(vertexAttribInfoArray[i].slot == slot)
            return &(vertexAttribInfoArray[i]);
    }
    return nullptr;
}

void GVertexAttrib::InitArr_Arr(GVertexAttrib::AttriInfo_AttribArr_Arr &slotAtrribArr_Arr, GVertexAttribInfoObject *vao)
{
    for(int i=0; i<vao->slotCount(); i++)
    {
        auto attriInfo = vao->vertexAttribInfoArray[i];
        if(attriInfo.enable)
        {
            slotAtrribArr_Arr.push_back(make_tuple(attriInfo, vector<GVertexAttrib>()));
        }
    }
}

void GVertexAttrib::AppendAttribToArr_Arr(GVertexAttrib::AttriInfo_AttribArr_Arr &slotAtrribArr_Arr, int slot, GVertexAttrib &attrib)
{
    for(size_t i=0; i<slotAtrribArr_Arr.size(); i++)
    {
        AttriInfo_AttribArr& slot_VertexAttribArr = slotAtrribArr_Arr.at(i);
        GVertexAttribInfo& attribInfo = std::get<0>(slot_VertexAttribArr);
        if(attribInfo.slot == slot)
        {
            VertexAttribArr& arr = std::get<1>(slot_VertexAttribArr);
            arr.push_back(attrib);
        }
    }
}

GColor GColor::Lerp(TGAColor color1, TGAColor color2, float f)
{
    GColor ret;
    ret.r = color1.bgra[2] * f + color2.bgra[2] * (1-f);
    ret.g = color1.bgra[1] * f + color2.bgra[1] * (1-f);
    ret.b = color1.bgra[0] * f + color2.bgra[0] * (1-f);
    ret.a = color1.bgra[3] * f + color2.bgra[3] * (1-f);
    return ret;
}

GColor GColor::Lerp(GColor color1, GColor color2, float f)
{
    GColor ret;
    ret.r = color1.r * f + color2.r * (1-f);
    ret.g = color1.g * f + color2.g * (1-f);
    ret.b = color1.b * f + color2.b * (1-f);
    ret.a = color1.a * f + color2.a * (1-f);
    return ret;

}
