#ifndef GSHADER_H
#define GSHADER_H
#include "gmath.h"
#include "gbuffer.h"
#include <tuple>
#include "tgaimage.h"
#include "ggraphiclibdefine.h"

enum GShaderAppDataType
{
    kSADTDefault,
};


// POSITION   vertex position
// TANGENT    vertex tangent
// NORMAL     vertex normal
// TEXCOORD0  uv
// COLOR      vertex color
enum GSlotType
{
    kSlotTPosition,
    kSlotTTangent,
    kSlotTNormal,
    kSlotTColor,
    kSlotTUV0,
    kSlotTUV1,
};

struct S_SlotInfo
{
    int slot;
    int datumCount;
    GSlotType slotType;
};

struct S_appdata;
struct S_abs_appdata
{
    S_abs_appdata(GShaderAppDataType appdataType):appdataType(appdataType){}
    virtual ~S_abs_appdata(){}

    static S_abs_appdata* CreateAppData(GShaderAppDataType appdataType);
    static const std::vector<S_SlotInfo>& GetSlotInfoArr(GShaderAppDataType appdataType);
    static const S_SlotInfo& GetSlotInfo(GShaderAppDataType appdataType, int slot);

    const std::vector<S_SlotInfo>& GetSlotInfoArr()
    {
        return GetSlotInfoArr(appdataType);
    }

    const S_SlotInfo& GetSlotInfo(int slot)
    {
        return GetSlotInfo(appdataType, slot);
    }

    virtual void SetSlotDatum(int slot, int datumIdx, double value) = 0;
    GShaderAppDataType appdataType;
};

struct S_appdata : public S_abs_appdata
{
    S_appdata():S_abs_appdata(GShaderAppDataType::kSADTDefault){}
    // slot info arr
    static const std::vector<S_SlotInfo> _slotInfoArr;

    virtual void SetSlotDatum(int slot, int datumIdx, double value);

    GMath::vec3 vert;
    GMath::vec2 uv;
    GMath::vec3 normal;
};

struct S_abs_v2f
{
    GMath::vec4 gl_position;

    GMath::vec4 ndc()
    {
        return gl_position / gl_position[3];
    }
};

struct S_v2f : public S_abs_v2f
{
    virtual ~S_v2f(){}

    GMath::vec3 wPos;
    GMath::vec3 wTangent;
    GMath::vec3 wNormal;
    GMath::vec2 uv;
};

struct S_fout
{
    std::vector<GMath::vec4> colors;
};

class GGraphicLibAPI;
struct IShader
{
    IShader(GShaderAppDataType appdataType):appdataType(appdataType)
    {}

    virtual ~IShader(){}
    virtual GMath::vec4 vertex(GGraphicLibAPI* GLAPI, S_abs_appdata* vert_in, int vertIdx) = 0;
    virtual void fragment(S_abs_v2f& frag_in, S_fout& frag_out) = 0;
    virtual S_abs_v2f* GetV2f(int idx) = 0;
    virtual S_abs_v2f* interpolation(GGraphicLibAPI* GLAPI, GMath::vec3 lerpFactor, int fragIdx) = 0;

    S_abs_appdata* CreateAppData()
    {
        return S_abs_appdata::CreateAppData(appdataType);
    }

    const std::vector<S_SlotInfo>& GetSlotInfoArr()
    {
        return S_abs_appdata::GetSlotInfoArr(appdataType);
    }

    GShaderAppDataType appdataType;

    // status
    GCullFaceType cullFaceType;
    bool useEarlyPerFragementTest;
};

enum GShaderType
{
    kSTDefault,
};

struct GLightInfo;
struct GShader : public IShader
{
    GShader():IShader(GShaderAppDataType::kSADTDefault)
    {
        useEarlyPerFragementTest = true;
        cullFaceType = GCullFaceType::kFTBack;
    }

    // uniform attribute
    GMath::mat4f obj2World;
    GMath::mat4f world2Obj;
    GMath::mat4f world2View;
    GMath::mat4f projMat;
    GMath::mat4f invertProjMat;
    std::vector<GLightInfo*> lights;

    TGAImage* diffusemap_;
    TGAImage* normalmap_;
    TGAImage* specularmap_;

    S_abs_v2f* GetV2f(int idx)
    {
        return &(v2f_data_arr[idx]);
    }
    S_v2f v2f_data_arr[3];
    S_v2f v2f_interpolated[4];

    virtual GMath::vec4 vertex(GGraphicLibAPI* GLAPI, S_abs_appdata* vert_in, int vertIdx);
    virtual S_abs_v2f* interpolation(GGraphicLibAPI* GLAPI, GMath::vec3 lerpFactor, int fragIdx);

    virtual void fragment(S_abs_v2f& frag_in, S_fout& frag_out);
};

#endif // GSHADER_H
