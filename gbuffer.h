#ifndef GBUFFER_H
#define GBUFFER_H

#include "glog.h"
#include <vector>
#include <tuple>
#include <functional>
#include "ggraphiclibdefine.h"
#include "gmath.h"
#include "tgaimage.h"
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
    static GColor gray;
    static GColor pink;
    static GColor normal;

    static GColor FastTonemap(GMath::vec4 color)
    {
        GColor ret;
        ret.r = (unsigned char)((color[0] * 1.0/(color[0]+1.0))*255);
        ret.g = (unsigned char)((color[1] * 1.0/(color[1]+1.0))*255);
        ret.b = (unsigned char)((color[2] * 1.0/(color[2]+1.0))*255);
        ret.a = (unsigned char)(std::max(0.0, std::min(color[3], 1.0)) * 255);
        return ret;
    }

    static GMath::vec4f ToFloat01Color(const GColor& color)
    {
        GMath::vec4f ret;
        ret.SetX(((float)color.r)/255.0);
        ret.SetY(((float)color.g)/255.0);
        ret.SetZ(((float)color.b)/255.0);
        ret.SetW(((float)color.a)/255.0);
        return ret;
    }

    static GMath::vec4f ToFloatColor(const GColor& color)
    {
        GMath::vec4f ret;
        ret.SetX((float)color.r);
        ret.SetY((float)color.g);
        ret.SetZ((float)color.b);
        ret.SetW((float)color.a);
        return ret;
    }

    static GColor FromFloat01Color(GMath::vec4 color)
    {
        GColor ret;
        ret.r = (unsigned char)(color[0] * 255);
        ret.g = (unsigned char)(color[1] * 255);
        ret.b = (unsigned char)(color[2] * 255);
        ret.a = (unsigned char)(color[3] * 255);
        return ret;
    }

    static GColor FromFloatColor(GMath::vec4 color)
    {
        GColor ret;
        ret.r = (unsigned char)(color[0]);
        ret.g = (unsigned char)(color[1]);
        ret.b = (unsigned char)(color[2]);
        ret.a = (unsigned char)(color[3]);
        return ret;
    }

    static GColor TgaColor(TGAColor tgaColor)
    {
        GColor color;
        color.r = tgaColor.bgra[2];
        color.g = tgaColor.bgra[1];
        color.b = tgaColor.bgra[0];
        color.a = tgaColor.bgra[3];
        return color;
    }

    static GColor Lerp(TGAColor color1, TGAColor color2, float f);
    static GColor Lerp(GColor color1, GColor color2, float f);

    GMath::vec4f ToFloat01Color()
    {
        return ToFloat01Color(*this);
    }

    GMath::vec4f ToFloatColor()
    {
        return ToFloatColor(*this);
    }
};

struct GHDRColor
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

    float intensity;

    static GMath::vec4f ToFloatColor(const GHDRColor& hdrColor)
    {
        GMath::vec4f ret;
        float factor = std::pow(2, hdrColor.intensity);
        ret.SetX(factor * float(hdrColor.r)/255);
        ret.SetY(factor * float(hdrColor.g)/255);
        ret.SetY(factor * float(hdrColor.b)/255);
        ret.SetW(float(hdrColor.a)/255);
        return ret;
    }

    GMath::vec4f ToFloatColor()
    {
        return ToFloatColor(*this);
    }
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
    std::vector<GColorBuffer*> drawRenderBuffers;

    void AttachRenderBuffer(GColorBuffer* colorbuffer, int index);
    void AttachRenderBuffer(GDepthStencilBuffer* depthStencilBuffer, bool isDepth=true);
    void ClearRenderBuffer(int index, GColor clearColor);
    void ClearDepthBuffer(float clearDepthValue);
    void ClearStencilBuffer(int clearValue);
    void DrawRenderBuffer(std::initializer_list<GRenderBufferType> renderBufferTypes);
    std::vector<GRenderBufferType> drawRenderBufferTypes;
    GColorBuffer* GetRenderBufer(GRenderBufferType renderBufferType);
    const std::vector<GColorBuffer*>& GetDrawRenderBuffer();
    int GetDrawRenderBufferCount();
    GMath::vec2i GetSize();

    GRenderBufferType GetRenderBufferType(GRenderBuffer *renderBuffer);
    bool IsBlendEnabled(GRenderBufferType renderBufferType);
    bool IsBlendEnabled(GRenderBuffer* renderBuffer);
    void SetEnableBlend(GRenderBufferType renderBufferType, bool enable=true);
    bool IsScissorTestEnabled(GRenderBufferType renderBufferType);
    bool IsScissorTestEnabled(GRenderBuffer* renderBuffer);
    void SetEnableScissorTest(GRenderBufferType renderBufferType, bool enable=true);

    GDepthStencilBuffer* depthBuffer;
    GDepthStencilBuffer* stencilBuffer;
private:
    bool CheckRenderBufferSizeValid();
    GColorBuffer* colorBuffer[MAX_COLORBUFF_COUNT];
    std::vector<std::tuple<bool, GRenderBufferType>> enableBlend;
    std::vector<std::tuple<bool, GRenderBufferType>> enableScissorTest;
};


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
    void Clear(unsigned int depth=~0)
    {
        for(int i=0; i<width; i++)
        {
            for(int j=0; j<height; j++)
            SetValue(i, j, depth);
        }
    }
    void ClearF(float depth=1.0f)
    {
        Clear(DepthFToUInt(depth));
    }
    void SetValue(int x, int y, unsigned int depth)
    {
        if(!CheckRangeValid(x,y)) return;

        depthStencilData()[y*width+x] = depth;
    }
    void SetFValue(int x, int y, float depth)
    {
        SetValue(x, y, DepthFToUInt(depth));
    }
    unsigned int GetValue(int x, int y)
    {
        if(!CheckRangeValid(x,y)) return 1;
        return depthStencilData()[y*width+x];
    }
    float GetFValue(int x, int y)
    {
        unsigned int ivalue = GetValue(x, y);
        return DepthUIntToF(ivalue);
    }
private:
    unsigned int DepthFToUInt(float fdepth)
    {
        fdepth = std::max(std::min(fdepth, 1.0f), -1.0f);
        unsigned int idepth = (fdepth + 1.0) * 0.5 * UINT_MAX;
        return idepth;
    }
    float DepthUIntToF(unsigned int idepth)
    {
        float fvalue = (double)idepth / UINT_MAX * 2.0 - 1.0;
        return fvalue;
    }
    unsigned int* depthStencilData()
    {
        return (unsigned int*)data;
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
