#include "ggraphiclibapi.h"
#include "grastergpupipeline.h"
#include "gmath.h"
using namespace std;
using namespace GMath;

GGraphicLibAPI::GGraphicLibAPI()
    :activePolygonMode(GPolygonMode::kPMFill)
{
}

GGraphicLibAPI::~GGraphicLibAPI()
{
    for(auto frameBuff : __frameBufferSet)
    {
        delete frameBuff;
    }
    for(auto renderBuff : __renderBufferSet)
    {
        delete renderBuff;
    }
    for(auto dataBuffer : __dataBufferSet)
    {
        delete dataBuffer;
    }
    for(auto vao : __vertexAttriInfoObjSet)
    {
        delete vao;
    }
    for(auto shader : __shaderSet)
    {
        delete shader;
    }
}


GFrameBuffer *GGraphicLibAPI::GenFrameBuffer()
{
    auto ret = new GFrameBuffer();
    __frameBufferSet.insert(ret);
    return ret;
}

void GGraphicLibAPI::BindFrameBuffer(GFrameBuffer *framebuffer)
{
    activeFramebuffer = framebuffer;
}

void GGraphicLibAPI::DeleteFrameBuffer(GFrameBuffer*& framebuffer)
{
    __frameBufferSet.erase(framebuffer);
    delete framebuffer;
    framebuffer = nullptr;
}

GColorBuffer *GGraphicLibAPI::GenRenderBuffer(int width, int height)
{
    auto buffer = new GColorBuffer(width, height);
    __renderBufferSet.insert(buffer);
    return buffer;
}

void GGraphicLibAPI::BindRenderBuffer(GColorBuffer *buffer)
{
    activeColorBuffer = buffer;
}

void GGraphicLibAPI::DeleteRenderBuffer(GRenderBuffer *&renderbuffer)
{
    __renderBufferSet.erase(renderbuffer);
    delete renderbuffer;
    renderbuffer = nullptr;
}

void GGraphicLibAPI::AttachRenderBufferToFrameBuffer(GFrameBuffer *framebuffer, GColorBuffer *colorbuffer, int attachIndex)
{
    framebuffer->AttachRenderBuffer(colorbuffer, attachIndex);
}

void GGraphicLibAPI::Clear(GColor clearColor)
{
    for(auto rbt : activeFramebuffer->drawRenderBufferTypes)
    {
        auto rb = activeFramebuffer->GetRenderBufer(rbt);
        rb->Clear(clearColor);
    }
}

void GGraphicLibAPI::ClearBuffer(int drawBuffer, GColor clearColor)
{
    activeFramebuffer->ClearRenderBuffer(drawBuffer, clearColor);
}

void GGraphicLibAPI::DrawRenderBuffer(GRenderBufferType renderBufferType)
{
    activeFramebuffer->DrawRenderBuffer({renderBufferType});
}

void GGraphicLibAPI::DrawRenderBuffer(std::initializer_list<GRenderBufferType> renderBufferTypes)
{
    activeFramebuffer->DrawRenderBuffer(renderBufferTypes);
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

GDataBuffer* GGraphicLibAPI::GenDataBuffer(GDataBufferType bufferType)
{
    auto buffer = new GDataBuffer(bufferType);
    __dataBufferSet.insert(buffer);
    return buffer;
}

void GGraphicLibAPI::BindDataBuffer(GDataBuffer *buffer)
{
    if(buffer->bufferType==GDataBufferType::kArrayBuffer)
    {
        if(!CheckVertexAttribInfoObject()) return;
        activeVertexAttriInfoObject->vertexBuffer = buffer;
    }
    else if(buffer->bufferType==GDataBufferType::kElementArrayBuffer)
    {
        if(!CheckVertexAttribInfoObject()) return;
        activeVertexAttriInfoObject->elemBuffer = buffer;
    }
}

void GGraphicLibAPI::FillDataBuffer(GDataBufferType dataBufferType, void *data, int size, int offset)
{
    if(dataBufferType==GDataBufferType::kArrayBuffer)
    {
        if(!CheckVertexAttribInfoObject()) return;
        activeVertexAttriInfoObject->vertexBuffer->SetData(data, size, offset);
    }
    else if(dataBufferType==GDataBufferType::kElementArrayBuffer)
    {
        if(!CheckVertexAttribInfoObject()) return;
        activeVertexAttriInfoObject->elemBuffer->SetData(data, size, offset);
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
    auto ret = new GVertexAttribInfoObject();
    __vertexAttriInfoObjSet.insert(ret);
    return ret;
}

void GGraphicLibAPI::BindVAO(GVertexAttribInfoObject *vao)
{
    activeVertexAttriInfoObject = vao;
}

void GGraphicLibAPI::VertexAttriPointer(int attriSlot, int attriDatumCount, GDatumType attriDatumType, bool normalize, int stride, int offset)
{
    if(activeVertexAttriInfoObject->IsSlotExist(attriSlot))
    {
        GLog::LogError("slot exist!");
        return;
    }
    GVertexAttribInfo attriInfo;
    attriInfo.slot = attriSlot;
    attriInfo.datumCount = attriDatumCount;
    attriInfo.datumType = attriDatumType;
    attriInfo.normalize = normalize;
    attriInfo.stride = stride;
    attriInfo.offset = offset;
    activeVertexAttriInfoObject->vertexAttribInfoArray.push_back(attriInfo);
}

void GGraphicLibAPI::SetEnableVertexAttriSlot(int attriSlot, bool enable)
{
    activeVertexAttriInfoObject->vertexAttribInfoArray[attriSlot].enable = enable;
}

bool GGraphicLibAPI::CheckDrawValid(GPrimitiveType t, int vertexCount)
{
    bool ret = true;
    if(t == GPrimitiveType::kTriangles)
    {
        ret = (vertexCount%3 == 0);
    }
    else if(t==GPrimitiveType::kLines)
    {
        ret = (vertexCount%2 == 0);
    }
    if(!ret)
    {
        GLog::LogError("primitiveType = ",t," vertexCount = ",vertexCount);
    }
    return ret;
}

void GGraphicLibAPI::DrawArrays(GPrimitiveType t, int vertexOffsetCount, int vertexCount)
{
    if(!CheckDrawValid(t, vertexCount)) return;
    if(!CheckVertexAttribInfoObject()) return;
    const std::vector<GColorBuffer*>& drawRenderBuffers = activeFramebuffer->GetDrawRenderBuffer();
    activePrimitiveType = t;

    GVertexAttrib::AttriInfo_AttribArr_Arr slotVertexAttribArr_Arr;
    vector<S_abs_appdata*> appdataArr;
    ParseVertexData(vertexCount, slotVertexAttribArr_Arr);
    CreateAppData(appdataArr, slotVertexAttribArr_Arr);
    GRasterGPUPipeline::ProcessAppData(this, appdataArr, vertexOffsetCount);

//    for(int i=0; i<activeVertexAttriInfoObject->MAX_VERTEX_ATTRIB_COUNT; i++)
//    {
//        if(t == GPrimitiveType::kLines)
//        {
//            for(int drawIdx=0; drawIdx<vertexCount/2; drawIdx++)
//            {
//                GRasterGPUPipeline::DrawLine(
//                    posElemArr[drawIdx*4 + 0],
//                    posElemArr[drawIdx*4 + 1],
//                    posElemArr[drawIdx*4 + 2],
//                    posElemArr[drawIdx*4 + 3],
//                    GColor::blue, drawRenderBuffers, activeDepthBuffer, activeStencilBuffer, activeShader, activePolygonMode);
//            }
//        }
//        else if(t == GPrimitiveType::kTriangles)
//        {
//            for(int drawIdx=0; drawIdx<vertexCount/3; drawIdx++)
//            {
//                GRasterGPUPipeline::DrawTriangle(
//                    vec2(posElemArr[drawIdx*6 + 0], posElemArr[drawIdx*6 + 1]),
//                    vec2(posElemArr[drawIdx*6 + 2], posElemArr[drawIdx*6 + 3]),
//                    vec2(posElemArr[drawIdx*6 + 4], posElemArr[drawIdx*6 + 5]),
//                    GColor::blue, drawRenderBuffers, activeDepthBuffer, activeStencilBuffer, activeShader, activePolygonMode);
//            }
//        }
//    }
}

void GGraphicLibAPI::DrawElements(GPrimitiveType t, int vertCount, GDatumType indexType, int offsetBytes)
{
    if(!CheckDrawValid(t, vertCount)) return;
    if(!CheckVertexAttribInfoObject()) return;
    const std::vector<GColorBuffer*>& drawRenderBuffers = activeFramebuffer->GetDrawRenderBuffer();
    activePrimitiveType = t;

    vector<GVertexAttrib::AttriInfo_AttribArr> slotVertexAttribArr_Arr;
    GVertexAttrib::InitArr_Arr(slotVertexAttribArr_Arr, activeVertexAttriInfoObject);

    vector<S_abs_appdata*> appdataArr;
    ParseVertexData(vertCount, slotVertexAttribArr_Arr);
    std::vector<int> elemIndexArr;
    ParseElemData(vertCount, indexType, elemIndexArr, offsetBytes);
    CreateAppData(appdataArr, slotVertexAttribArr_Arr, &elemIndexArr);
    GRasterGPUPipeline::ProcessAppData(this, appdataArr, 0);
}

void GGraphicLibAPI::SetPolygonMode(GPolygonMode mode)
{
    activePolygonMode = mode;
}

GShader *GGraphicLibAPI::CreateProgram(GShaderType shaderType)
{
    GShader* shader = nullptr;
    switch (shaderType)
    {
        case GShaderType::kSTDefault:
        default:
            shader = new GShader();
            break;
    }
    if(shader!=nullptr)
    {
        __shaderSet.insert(shader);
    }
    return shader;
}

void GGraphicLibAPI::UseProgram(GShader *shader)
{
    activeShader = shader;
}

void GGraphicLibAPI::ParseVertexData(int vertexCount,GVertexAttrib::AttriInfo_AttribArr_Arr& slotVertexAttribArr_Arr)
{
    auto dataBuffer = activeVertexAttriInfoObject->vertexBuffer;

    int totalOffset = 0;
    int vertexDataOffset = 0;
    vector<int> attriDataOffset(activeVertexAttriInfoObject->slotCount(), 0);

    for(int vertexIdx=0; vertexIdx<vertexCount; vertexIdx++)
    {
        for(int i=0; i<activeVertexAttriInfoObject->slotCount(); i++)
        {
            auto attriInfo = activeVertexAttriInfoObject->vertexAttribInfoArray[i];

            if(attriInfo.stride==0 && attriDataOffset[i]==0)
            {
                attriDataOffset[i] = attriInfo.offset;
            }
            GVertexAttrib vertexAttrib;
            vertexAttrib.datumType = attriInfo.datumType;
            vertexAttrib.datumCount = attriInfo.datumCount;
            for(int datumIdx = 0; datumIdx<attriInfo.datumCount; datumIdx++)
            {
                totalOffset = vertexIdx * attriInfo.stride + attriDataOffset[i];
                switch (attriInfo.datumType)
                {
                case GDatumType::kDouble:
                {
                    if(attriInfo.enable)
                    {
                        double* datumPtr = dataBuffer->GetData<double>(totalOffset);
                        if(attriInfo.stride==0)
                        {
                            vertexAttrib.data[datumIdx] = *datumPtr;
                        }
                        else
                        {
                            vertexAttrib.data[datumIdx] = *(datumPtr+datumIdx);
                        }
                    }
                    if(attriInfo.stride==0)
                    {
                        attriDataOffset[i] += sizeof(double);
                    }
                    break;
                }
                case GDatumType::kFloat:
                {
                    if(attriInfo.enable)
                    {
                        float* datumPtr = dataBuffer->GetData<float>(totalOffset);
                        if(attriInfo.stride==0)
                        {
                            vertexAttrib.data[datumIdx] = *datumPtr;
                        }
                        else
                        {
                            vertexAttrib.data[datumIdx] = *(datumPtr+datumIdx);
                            //appdata->SetSlotDatum(i, datumIdx, *(datumPtr+datumIdx));
                        }
                    }
                    if(attriInfo.stride==0)
                    {
                        attriDataOffset[i] += sizeof(float);
                    }
                    break;
                }
                default:
                {
                    assert(false);
                    break;
                }
                }
            }
            vertexDataOffset += attriInfo.stride;
            GVertexAttrib::AppendAttribToArr_Arr(slotVertexAttribArr_Arr, attriInfo.slot, vertexAttrib);
        }
    }
}

void GGraphicLibAPI::ParseElemData(int vertexCount, GDatumType indexType, std::vector<int> &vertexIndexArr, int offsetBytes)
{
    vertexIndexArr.clear();
    auto indexBuffer = activeVertexAttriInfoObject->elemBuffer;
    for(int vertexIdx=0; vertexIdx<vertexCount; vertexIdx++)
    {
        int idx = 0;
        switch (indexType)
        {
        case GDatumType::kInt:
        {
            idx = *(indexBuffer->GetData<int>(offsetBytes));
            offsetBytes += sizeof (int);
            break;
        }
        case GDatumType::kUInt:
        {
            idx = *(indexBuffer->GetData<unsigned int>(offsetBytes));
            offsetBytes += sizeof (unsigned int);
            break;
        }
        default:
            assert(false);
        }
        vertexIndexArr.push_back(idx);
    }
}

void GGraphicLibAPI::CreateAppData(std::vector<S_abs_appdata*>& appData, GVertexAttrib::AttriInfo_AttribArr_Arr& slotVertexAttribArr_Arr, std::vector<int> *elemIdxArr)
{
    int appdataCount = slotVertexAttribArr_Arr.size();
    if(elemIdxArr!=nullptr)
    {
        appdataCount = elemIdxArr->size();
    }

    for(int i=0; i<appdataCount; i++)
    {
        S_abs_appdata* t_appdata = S_abs_appdata::CreateAppData(activeShader->appdataType);
        int vertexIdx = i;
        if(elemIdxArr!=nullptr)
        {
            vertexIdx = elemIdxArr->at(i);
        }

        for(auto slotVertexAttribArr : slotVertexAttribArr_Arr)
        {
            GVertexAttribInfo& attribInfo = std::get<0>(slotVertexAttribArr);
            GVertexAttrib::VertexAttribArr attribArr = std::get<1>(slotVertexAttribArr);
            GVertexAttrib& attrib = attribArr[vertexIdx];
            for(int datumIdx=0; datumIdx<attribInfo.datumCount; datumIdx++)
            {
                t_appdata->SetSlotDatum(attribInfo.slot, datumIdx, attrib.data[datumIdx]);
            }
        }
        appData.push_back(t_appdata);
    }
}































