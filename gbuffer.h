#ifndef GBUFFER_H
#define GBUFFER_H

#include "glog.h"
#include <vector>
#include <tuple>
#include <functional>
#include "ggraphiclibdefine.h"
class GRenderBuffer;
class GColorBuffer;
class GDepthStencilBuffer;

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
    virtual ~GRenderBuffer(){}
    bool CheckRangeValid(int x, int y)
    {
        if(data == nullptr)
        {
            GLog::LogError("buffer is nullptr! buffer type = ",typeid(*this).name());
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
    static const int MAX_COLORBUFF_COUNT = GRenderBufferType::kRBMax;
    static bool CheckAttachIndexValid(int index);

    void AttachRenderBuffer(GColorBuffer* colorbuffer, int index);
    void ClearRenderBuffer(int index, GColor clearColor);
    void ClearRenderBuffer(int clearValue, bool isDepthBuffer=true);
    void DrawRenderBuffer(std::initializer_list<GRenderBufferType> renderBufferTypes);
    std::vector<GRenderBufferType> drawRenderBufferTypes;
    GColorBuffer* GetRenderBufer(GRenderBufferType renderBufferType);
    void GetDrawRenderBuffer(std::vector<GColorBuffer*>& drawRenderBuffers);
private:
    GColorBuffer* colorBuffer[MAX_COLORBUFF_COUNT];
    GDepthStencilBuffer* depthBuffer;
    GDepthStencilBuffer* stencilBuffer;
};

#define BUFFER_ZERO_COORD_AT_LEFT_BOTTOM \
    y = height - y - 1; \

class GColorBuffer : public GRenderBuffer
{
public:
    GColorBuffer(int w, int h)
    {
        GLog::LogInfo("Create Color Buffer w = ",w," h = ", h);
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
    virtual ~GColorBuffer()
    {
        if(data!=nullptr)
        {
            delete colorData();
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
        BUFFER_ZERO_COORD_AT_LEFT_BOTTOM;
        colorData()[y*width+x] = color;
    }

    GColor GetColor(int x, int y)
    {
        if(!CheckRangeValid(x,y)) return GColor::black;
        return colorData()[y*width+x];
    }

    unsigned char* GetData()
    {
        return (unsigned char*)data;
    }
private:
    GColor* colorData()
    {
        return (GColor*)data;
    }
};

class GDepthStencilBuffer : public GRenderBuffer
{
public:
    GDepthStencilBuffer(int w, int h)
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
    virtual ~GDepthStencilBuffer()
    {
        if(data!=nullptr)
        {
            delete depthStencilData();
        }
        data = nullptr;
    }
    void Clear(int depth)
    {
        for(int i=0; i<width; i++)
        {
            for(int j=0; j<height; j++)
            SetValue(i, j, depth);
        }
    }
    void SetValue(int x, int y, int depth)
    {
        if(!CheckRangeValid(x,y)) return;

        BUFFER_ZERO_COORD_AT_LEFT_BOTTOM;
        depthStencilData()[y*width+x] = depth;
    }

    int GetValue(int x, int y)
    {
        if(!CheckRangeValid(x,y)) return 1;
        return depthStencilData()[y*width+x];
    }
private:
    int* depthStencilData()
    {
        return (int*)data;
    }
};

class GDataBuffer
{
public:
    GDataBuffer(GDataBufferType t)
    {
        bufferType = t;
    }

    ~GDataBuffer()
    {
        InvalidateData();
    }

    void SetData(void* data, int size, int offset=0)
    {
        if(data==nullptr || size<1)
        {
            GLog::LogError("data = ", data, " size = ", size);
            return;
        }
        unsigned char* start = (unsigned char*)data;
        _buffer.insert(_buffer.begin()+offset, start, start+size);
    }

    void InvalidateData()
    {
        _buffer.clear();
    }

    template<typename T>T* GetData(int offset)
    {
        return (T*)(_buffer.data()+offset);
    }

    void* buffer()
    {
        return _buffer.data();
    }

    GDataBufferType bufferType;
    std::vector<unsigned char> _buffer;
};

struct GVertexAttribInfo
{
    int slot;
    bool enable;
    int datumCount;
    GDatumType datumType;
    bool normalize;
    int stride;
    int offset;
};

class GVertexAttribInfoObject;
struct GVertexAttrib
{
    int datumCount;
    GDatumType datumType;
    double data[4];

    typedef std::vector<GVertexAttrib> VertexAttribArr;
    typedef std::tuple<GVertexAttribInfo, VertexAttribArr> AttriInfo_AttribArr;
    typedef std::vector<AttriInfo_AttribArr> AttriInfo_AttribArr_Arr;

    static void InitArr_Arr(AttriInfo_AttribArr_Arr& slotAtrribArr_Arr, GVertexAttribInfoObject* vao);
    static void AppendAttribToArr_Arr(AttriInfo_AttribArr_Arr& slotAtrribArr_Arr, int slot, GVertexAttrib& attrib);
};

class GVertexAttribInfoObject
{
public:
    bool IsSlotExist(int slot);
    GVertexAttribInfo* GetVertexAttriInfo(int slot);
    int slotCount()
    {
        return vertexAttribInfoArray.size();
    }

    std::vector<GVertexAttribInfo> vertexAttribInfoArray;
    GDataBuffer* vertexBuffer;
    GDataBuffer* elemBuffer;
};

#endif // GBUFFER_H
