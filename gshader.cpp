#include "gshader.h"
#include "ggraphiclibapi.h"
using namespace GMath;

S_abs_appdata *S_abs_appdata::CreateAppData(GShaderAppDataType appdataType)
{
    switch (appdataType)
    {
    case GShaderAppDataType::kSADTDefault:
        return new S_appdata();
    }
    return nullptr;
}

const std::vector<S_SlotInfo> &S_abs_appdata::GetSlotInfoArr(GShaderAppDataType appdataType)
{
    switch (appdataType)
    {
    case GShaderAppDataType::kSADTDefault:
        return S_appdata::_slotInfoArr;
    }
    assert(false);
}

const S_SlotInfo &S_abs_appdata::GetSlotInfo(GShaderAppDataType appdataType, int slot)
{
    const std::vector<S_SlotInfo>& slotInfoArr = GetSlotInfoArr(appdataType);
    return slotInfoArr[slot];
}

const std::vector<S_SlotInfo> S_appdata::_slotInfoArr = {
    {0, 3, GSlotType::kSlotTPosition},
    {1, 2, GSlotType::kSlotTUV0},
    {2, 3, GSlotType::kSlotTNormal},
};


void S_appdata::SetSlotDatum(int slot, int datumIdx, double value)
{
    if(slot==0)
    {
        vert[datumIdx] = value;
    }
    else if(slot==1)
    {
        uv[datumIdx] = value;
    }
    else if(slot==2)
    {
        normal[datumIdx] = value;
    }
}

vec4 GShader::vertex(GGraphicLibAPI *GLAPI, S_abs_appdata *vert_in, int vertIdx)
{
    S_appdata* appdata = (S_appdata*) vert_in;
    GMath::vec4 wPos = GMath::embed<double,4>(appdata->vert, 1);
    wPos = (mat4)obj2World * wPos;

    v2f_data_arr[vertIdx].gl_position = (mat4)projMat * wPos;
    v2f_data_arr[vertIdx].uv = appdata->uv;
    v2f_data_arr[vertIdx].normal = appdata->normal;
}