#include "gbuffer.h"
#include "glog.h"

GColor GColor::white = {1,1,1,1};
GColor GColor::black = {0,0,0,1};
GColor GColor::red = {1,0,0,1};
GColor GColor::green = {1,0,0,1};
GColor GColor::blue = {1,0,0,1};

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

