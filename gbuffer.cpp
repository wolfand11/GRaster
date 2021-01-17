#include "gbuffer.h"
#include "glog.h"
using namespace std;

GColor GColor::white = {255,255,255,255};
GColor GColor::black = {0,0,0,255};
GColor GColor::red = {255,0,0,255};
GColor GColor::green = {0,255,0,255};
GColor GColor::blue = {0,0,255,255};

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

    colorBuffer[index] = colorbuffer;
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

void GFrameBuffer::ClearRenderBuffer(int clearValue, bool isDepthBuffer)
{
    if(isDepthBuffer)
    {
        if(depthBuffer==nullptr)
        {
            GLog::LogError("depthBuffer==nullptr");
            return;
        }
        depthBuffer->Clear(clearValue);
    }
    else
    {
        if(stencilBuffer==nullptr)
        {
            GLog::LogError("stencilBuffer==nullptr");
            return;
        }
        depthBuffer->Clear(clearValue);
    }
}

void GFrameBuffer::DrawRenderBuffer(std::initializer_list<GRenderBufferType> renderBufferTypes)
{
    this->drawRenderBufferTypes.clear();
    this->drawRenderBufferTypes.assign(renderBufferTypes.begin(),renderBufferTypes.end());
}

GColorBuffer *GFrameBuffer::GetRenderBufer(GRenderBufferType renderBufferType)
{
    if(!CheckAttachIndexValid(renderBufferType)) return nullptr;

    return colorBuffer[renderBufferType];
}

void GFrameBuffer::GetDrawRenderBuffer(std::vector<GColorBuffer *> &drawRenderBuffers)
{
    drawRenderBuffers.clear();

    for(auto rbt : drawRenderBufferTypes)
    {
        drawRenderBuffers.push_back(GetRenderBufer(rbt));
    }
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
