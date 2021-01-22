#include "ggameobject.h"
#include "glog.h"
#include "ggraphiclibapi.h"
#include "gmathutils.h"
#include "gutils.h"
using namespace GMath;
using namespace std;

GGameObject* GGameObject::activeCamera = nullptr;
vector<GGameObject> GGameObject::lights;

GGameObject::GGameObject(GGameObject::GGameObjectType t, int subType)
    :mtype(t)
{
    switch (mtype)
    {
    case GGameObjectType::kCamera:
        cameraType = (GCameraType)subType;
        break;
    case GGameObjectType::kLight:
        lightInfo.lightType = (GLightType)subType;
        break;
    case GGameObjectType::kModel:
        break;
    }
}

void GGameObject::SetT(vec3f pos)
{
    this->_position = pos;
    _trs_dirty = true;
}

void GGameObject::SetR(vec3f rotation)
{
    this->_rotation.SetX(GMathUtils::Deg2Rad(rotation.x()));
    this->_rotation.SetY(GMathUtils::Deg2Rad(rotation.y()));
    this->_rotation.SetZ(GMathUtils::Deg2Rad(rotation.z()));
    _trs_dirty = true;
}

void GGameObject::SetS(vec3f scale)
{
    this->_scale = scale;
    _trs_dirty = true;
}

void GGameObject::SetTRS(vec3f pos, vec3f rotation, vec3f scale)
{
    SetT(pos);
    SetR(rotation);
    SetS(scale);
}

void GGameObject::TRSInvertTRS(const mat4f* &trs, const mat4f* &invertTRS)
{
    if(_trs_dirty)
    {
        transform = GMathUtils::TRS(_position, _rotation, _scale);
        invertTransform = transform.invert();
        _trs_dirty = false;
    }
    trs = &transform;
    invertTRS = &invertTransform;
}

GGameObject GGameObject::CreateProjCamera(float near, float far, float fov)
{
    float aspectRatio = GUtils::screenAspectRatio();
    GGameObject cameraGObj(GGameObjectType::kCamera, GCameraType::kProjection);
    cameraGObj.near = near;
    cameraGObj.far = far;
    cameraGObj.fov = fov;
    assert(far>near);
    cameraGObj.aspectRatio = aspectRatio;
    cameraGObj._proj_dirty = true;
    return cameraGObj;
}

void GGameObject::SetViewport(int x, int y, int w, int h)
{
    viewportX = x;
    viewportY = y;
    viewportW = w;
    viewportH = h;
}

vec2 GGameObject::NDCPosToScreenPos(vec3 ndc)
{
    vec2 ret;
    ret.SetX((ndc.x()+1.0)*viewportW*0.5+viewportX);
    ret.SetY((ndc.y()+1.0)*viewportH*0.5+viewportY);
    return ret;
}

GMath::mat4f &GGameObject::LookAt(GMath::vec3f eyePos, GMath::vec3f lookAtPoint, GMath::vec3f up)
{
    _position = eyePos;
    _scale = vec3f::one;

    transform.identity();
    vec3f forward = (lookAtPoint - eyePos).normalize();
    vec3f right = cross(up, forward).normalize();
    up = cross(forward, right).normalize();
    transform.set_col(0, embed<float,4>(right,0));
    transform.set_col(1, embed<float,4>(up,0));
    transform.set_col(2, embed<float,4>(forward,0));
    _rotation = GMathUtils::RotationMatrixToEulerAngle(transform);

    transform[0][3] = eyePos[0];
    transform[1][3] = eyePos[1];
    transform[2][3] = eyePos[2];
    // TODO calc invertTransform to set _trs_dirty false
    _trs_dirty = true;
    return transform;
}

void GGameObject::ProjInvertProj(const mat4f*& tproj,const mat4f*& tinvertProj)
{
    if(_proj_dirty)
    {
        // fov axis is yAxis
        float zoomY = 1.0/std::tan(GMathUtils::Deg2Rad(fov/2.0));
        float zoomX = zoomY / aspectRatio;
        projMat.zero();
        projMat[0][0] = zoomX;
        projMat[1][1] = zoomY;
        projMat[2][2] = (far+near)/(far-near);
        projMat[2][3] = (-2*near*far)/(far-near);
        projMat[3][2] = 1;
        projMat[3][3] = 0;

        invertProjMat.zero();
        invertProjMat[0][0] = 1.0f/zoomX;
        invertProjMat[1][1] = 1.0f/zoomY;
        invertProjMat[2][2] = 0;
        invertProjMat[2][3] = 1;
        invertProjMat[3][2] = (near-far)/(2*near*far);
        invertProjMat[3][3] = (near+far)/(2*near*far);
        _proj_dirty = false;
    }
    tproj = &projMat;
    tinvertProj = &invertProjMat;
}

GGameObject& GGameObject::CreateLightGObj(GLightType lightType, GColor lColor, float lIntensity)
{
    GGameObject light = GGameObject(GGameObject::GGameObjectType::kLight, lightType);
    light.lightInfo.lightColor = lColor;
    light.lightInfo.lightIntensity = lIntensity;
    lights.push_back(std::move(light));
    return lights[lights.size()-1];
}

GGameObject GGameObject::CreateModelGObj(GModelType modelType, std::string modelPath)
{
    GGameObject gObj(GGameObjectType::kModel, modelType);
    GOBJModel gObjModel(modelType, modelPath);
    gObj.model = GGLModel::CreateWithObjModel(&gObjModel);
    return gObj;
}

void GGameObject::SetupDraw(GGraphicLibAPI *GLAPI)
{
    auto tShader = GLAPI->CreateProgram();
    modelShader = tShader;
    auto tVAO = GLAPI->GenVAO();
    modelVAO = tVAO;
    GLAPI->BindVAO(tVAO);
    auto tArrBuffer = GLAPI->GenDataBuffer(GDataBufferType::kArrayBuffer);
    GLAPI->BindDataBuffer(tArrBuffer);
    auto tElemArrBuffer = GLAPI->GenDataBuffer(GDataBufferType::kElementArrayBuffer);
    GLAPI->BindDataBuffer(tElemArrBuffer);

    int offset = 0;
    int vertCount = model.nverts();
    int dataSize = 0;

    auto slotInfoArr = tShader->GetSlotInfoArr();
    for(auto slotInfo : slotInfoArr)
    {
        switch (slotInfo.slotType)
        {
        case GSlotType::kSlotTColor:
        {
            break;
        }
        case GSlotType::kSlotTNormal:
        {
            dataSize = sizeof(vec3)*vertCount;
            GLAPI->FillDataBuffer(GDataBufferType::kArrayBuffer, model.norms_p(), dataSize, offset);
            // keep mesh data layout == slot data layout
            assert(slotInfo.slot == 2 && slotInfo.datumCount==3);
            GLAPI->VertexAttriPointer(2, 3, GDatumType::kDouble, false, 0, offset);
            offset += dataSize;
            GLAPI->SetEnableVertexAttriSlot(slotInfo.slot, true);
            break;
        }
        case GSlotType::kSlotTPosition:
        {
            dataSize = sizeof(vec3)*vertCount;
            GLAPI->FillDataBuffer(GDataBufferType::kArrayBuffer, model.verts_p(), dataSize);
            // keep mesh data layout == slot data layout
            assert(slotInfo.slot == 0 && slotInfo.datumCount==3);
            GLAPI->VertexAttriPointer(0, 3, GDatumType::kDouble, false, 0, offset);
            offset += dataSize;
            GLAPI->SetEnableVertexAttriSlot(slotInfo.slot, true);
            break;
        }
        case GSlotType::kSlotTTangent:
        {
            break;
        }
        case GSlotType::kSlotTUV0:
        {
            dataSize = sizeof(vec2)*vertCount;
            GLAPI->FillDataBuffer(GDataBufferType::kArrayBuffer, model.uv_p(), dataSize, offset);
            // keep mesh data layout == slot data layout
            assert(slotInfo.slot == 1 && slotInfo.datumCount==2);
            GLAPI->VertexAttriPointer(1, 2, GDatumType::kDouble, false, 0, offset);
            offset += dataSize;
            GLAPI->SetEnableVertexAttriSlot(slotInfo.slot, true);
            break;
        }
        case GSlotType::kSlotTUV1:
        {
            break;
        }
        }
    }

    dataSize = sizeof(int)*model.indexCount();
    GLAPI->FillDataBuffer(GDataBufferType::kElementArrayBuffer, model.index_p(), dataSize, 0);

    GLAPI->BindVAO(nullptr);
}

void GGameObject::DrawModel(GGraphicLibAPI *GLAPI)
{
    GLAPI->UseProgram(modelShader);
    // set obj uniform variable
    const mat4f* tMat;
    const mat4f* tInvertMat;
    TRSInvertTRS(tMat, tInvertMat);
    GLAPI->activeShader->world2Obj = *tInvertMat;
    GLAPI->activeShader->obj2World = *tMat;
    // set camera uniform variable
    GGameObject::activeCamera->TRSInvertTRS(tMat, tInvertMat);
    GLAPI->activeShader->world2View = *tInvertMat;
    GGameObject::activeCamera->ProjInvertProj(tMat, tInvertMat);
    GLAPI->activeShader->projMat = *tMat;
    GLAPI->activeShader->invertProjMat = *tInvertMat;
    GLAPI->activeShader->diffusemap_ = &(model.diffusemap_);
    GLAPI->activeShader->normalmap_ = &(model.normalmap_);
    GLAPI->activeShader->specularmap_ = &(model.specularmap_);
    FillLightData(GLAPI);

    GLAPI->BindVAO(modelVAO);
    GLAPI->DrawElements(GPrimitiveType::kTriangles, model.indexCount(), GDatumType::kInt, 0);
    GLAPI->BindVAO(nullptr);
}

void GGameObject::FillLightData(GGraphicLibAPI* GLAPI)
{
    GLAPI->activeShader->lights.clear();
    for(size_t i=0; i<lights.size(); i++)
    {
        GGameObject& light = lights[i];
        GLightInfo* lightInfo = &(light.lightInfo);
        if(lightInfo->lightType == GLightType::kLTPoint)
        {
            lightInfo->lightPosOrDir = light.position();
        }
        else if(lightInfo->lightType == GLightType::kLTDirection)
        {
            const mat4f* tMat;
            const mat4f* tInvertMat;
            light.TRSInvertTRS(tMat, tInvertMat);
            lightInfo->lightPosOrDir = (*tInvertMat).get_minor(3,3).transpose() * vec3f(0,0,1);
            lightInfo->lightPosOrDir.normalize();
        }
        GLAPI->activeShader->lights.push_back(lightInfo);
    }
}
