#ifndef GGAMEOBJECT_H
#define GGAMEOBJECT_H
#include "gmath.h"
#include "gmodel.h"
#include "gbuffer.h"
#include "gshader.h"

enum GLightType
{
    kLTDirection,
    kLTPoint,
};

struct GLightInfo
{
    GColor lightColor;
    float lightIntensity;
    GLightType lightType;
    GMath::vec3 lightPosOrDir;
};

class GGraphicLibAPI;
class GGameObject
{
public:
    enum GGameObjectType
    {
        kCamera,
        kLight,
        kModel,
    };

    enum GCameraType
    {
        kOrthographic,
        kProjection,
    };


    GGameObject()=default;
    GGameObject(GGameObjectType t,int subType);

    // common
    void SetT(GMath::vec3f pos);
    void SetR(GMath::vec3f rotation);
    void SetS(GMath::vec3f scale);
    void SetTRS(GMath::vec3f pos,GMath::vec3f rotation,GMath::vec3f scale);
    void TRSInvertTRS(const GMath::mat4f*& trs, const GMath::mat4f*& invertTRS);
    const GMath::vec3f& position() { return _position; }
    const GMath::vec3f& rotation() { return _rotation; }
    const GMath::vec3f& scale() { return _scale; }

    GGameObjectType mtype;
    GMath::vec3f right();
    GMath::vec3f up();
    GMath::vec3f forward();

    // camera
    static GGameObject CreateProjCamera(float near, float far, float fov);
    static GGameObject* activeCamera;
    void SetViewport(int x, int y, int w, int h);
    void SetFov(float fov);
    GMath::vec2 NDCPosToScreenPos(GMath::vec3 ndc);
    float ToWBufferValue(float wValue);
    int viewportX;
    int viewportY;
    int viewportW;
    int viewportH;
    GCameraType cameraType;
    float fov{60};
    float near{0.1};
    float far{200.0f};
    float aspectRatio{1.0f};
    GMath::mat4f& LookAt(GMath::vec3f eyePos, GMath::vec3f lookAtPoint, GMath::vec3f up);
    void ProjInvertProj(const GMath::mat4f*& tproj,const GMath::mat4f*& tinvertProj);

    // light
    static GGameObject& CreateLightGObj(GLightType lightType, GColor lColor=GColor::white, float lIntensity=1);
    static std::vector<GGameObject> lights;
    GLightInfo lightInfo;

    // model
    static GGameObject CreateModelGObj(GModelType modelType, std::string modelPath="", GShaderType shaderType=GShaderType::kSTDefault);
    void SetupDraw(GGraphicLibAPI* GLAPI);
    void DrawModel(GGraphicLibAPI* GLAPI);
    GShaderType shaderType;
    GGLModel model;
    GVertexAttribInfoObject* modelVAO;
    GShader* modelShader;

private:
    // common
    GMath::vec3f _position;
    GMath::vec3f _rotation;
    GMath::vec3f _scale = {1,1,1};
    bool _trs_dirty = true;
    GMath::mat4f transform;
    GMath::mat4f invertTransform;

    // camera
    GMath::mat4f projMat;
    GMath::mat4f invertProjMat;
    bool _proj_dirty = true;

    // light
    void FillLightData(GGraphicLibAPI* GLAPI);
};

#endif // GGAMEOBJECT_H
