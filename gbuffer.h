#ifndef GBUFFER_H
#define GBUFFER_H

#include "glog.h"
#include <vector>
class GRenderBuffer;
class GColorBuffer;

struct GColor
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

    bool IsBlack()
    {
        return (r==0 && g==0 && b==0);
    }

    static GColor white;
    static GColor black;
    static GColor red;
    static GColor green;
    static GColor blue;
};

class GRenderBuffer
{
public:
    enum GRenderBufferType
    {
        kNon=-1,
        kFront,
        kBack,
        kFrontAndBack,
        kColorAttachment0,
        kColorAttachment1,
        kColorAttachment2,
        kColorAttachment3,
        kColorAttachment4,
        kMax,
    };
    bool CheckRangeValid(int x, int y)
    {
        if(data == nullptr)
        {
            GLog::LogError("buffer is nullptr!");
            return false;
        }
        if(x<0 || y<0 || x>=width || y>=height)
        {
            GLog::LogError("width = ", width, " height = ", height, "x = ", x, " y = ", y);
            return false;
        }
        return true;
    }

    int width;
    int height;

protected:
    void* data;
};

class GFrameBuffer
{
public:
    GFrameBuffer() {}
    static const int MAX_COLORBUFF_COUNT = GRenderBuffer::GRenderBufferType::kMax;
    static bool CheckAttachIndexValid(int index);

    void AttachRenderBuffer(GColorBuffer* colorbuffer, int index);
    void ClearRenderBuffer(int index, GColor clearColor);
private:
    GColorBuffer* colorBuffer[MAX_COLORBUFF_COUNT];
    GRenderBuffer* depthBuffer;
    GRenderBuffer* stencilBuffer;
};

class GColorBuffer : GRenderBuffer
{
public:
    GColorBuffer(int w, int h)
    {
        data = nullptr;
        if(w>0 && h>0)
        {
            width = w;
            height = h;
            data = new GColor[w*h];
        }
        else
        {
            GLog::LogError("w = ", w, " h = ", h);
        }
    }
    ~GColorBuffer()
    {
        if(data!=nullptr)
        {
            delete data;
        }
        data = nullptr;
    }



    void Clear(GColor color)
    {
        for(int i=0; i<width; i++)
        {
            for(int j=0; j<height; j++)
            SetColor(i, j, color);
        }
    }

    void SetColor(int x, int y, GColor color)
    {
        if(!CheckRangeValid(x,y)) return;

        data[y*width+x] = color;
    }

    GColor GetColor(int x, int y)
    {
        if(!CheckRangeValid(x,y)) return GColor::black;

        return data[y*width+x];
    }
private:
    GColor* data;
};

class GDepthBuffer : GRenderBuffer
{
public:
    GDepthBuffer(int w, int h)
    {
        data = nullptr;
        if(w>0 && h>0)
        {
            width = w;
            height = h;
            data = new int[w*h];
        }
        else
        {
            GLog::LogError("w = ", w, " h = ", h);
        }
    }
    ~GDepthBuffer()
    {
        if(data!=nullptr)
        {
            delete data;
        }
        data = nullptr;
    }
    void Clear(int depth)
    {
        for(int i=0; i<width; i++)
        {
            for(int j=0; j<height; j++)
            SetDepth(i, j, depth);
        }
    }
    void SetDepth(int x, int y, int depth)
    {
        if(!CheckRangeValid(x,y)) return;

        data[y*width+x] = depth;
    }

    int GetDepth(int x, int y)
    {
        if(!CheckRangeValid(x,y)) return 1;
        return data[y*width+x];
    }
private:
    int* data;
};

class GDataBuffer
{
public:
    enum GDataBufferType
    {
        kArrayBuffer,
        kElementArrayBuffer,
        kPixelPackBuffer,
        kPixelUnpackBuffer,
        kTextureBuffer,
        kTransformFeedbackBuffer,
        kUniformBuffer,
    };

    GDataBuffer(GDataBufferType t)
    {
        bufferType = t;
    }

    ~GDataBuffer()
    {
        InvalidateData();
    }

    void SetData(void* data, int size)
    {
        if(data==nullptr || size<1)
        {
            GLog::LogError("data = ", data, " size = ", size);
            return;
        }

        if(buffer!=nullptr)
        {
            delete[] buffer;
        }
        buffer = nullptr;

        buffer = new unsigned char[size]();
        memcpy(buffer, data, size);
    }

    void InvalidateData()
    {
        if(buffer!=nullptr)
        {
            delete[] buffer;
        }
        buffer = nullptr;
    }

    GDataBufferType bufferType;
    int datumType;
    unsigned char* buffer;
};

class GVertexAttribInfoObject
{
public:
    static const int MAX_VERTEX_ATTRIB_COUNT = 10;
    struct VertexAttribInfo
    {
        bool enable;
        int datumCount;
        int datumType;
        bool normalize;
        int stride;
        int offset;
    };

    VertexAttribInfo vertexAttribInfoArray[MAX_VERTEX_ATTRIB_COUNT];
    GDataBuffer* vertexBuffer;
};

#endif // GBUFFER_H
