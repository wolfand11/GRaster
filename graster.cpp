#include "graster.h"
#include "ui_graster.h"
#include "glog.h"
#include <QResizeEvent>
#include "ggraphiclibapi.h"
#include "gmath.h"
#include "gutils.h"
using namespace GMath;

const int defaultBufferW = 400;
const int defaultBufferH = 400;
GFrameBuffer* frameBuffer;
GColorBuffer* colorBuffer;
GDataBuffer*  arrayBuffer; // geometry
GVertexAttribInfoObject* vao;
GShader* shader;

static void CopyColorBufferToImage(GColorBuffer* colorBuffer, QImage* image)
{
    image->setColorCount(colorBuffer->width*colorBuffer->height);
    for(int i=0; i<colorBuffer->width; i++)
    {
        for(int j=0; j<colorBuffer->width; j++)
        {
            GColor color = colorBuffer->GetColor(i, j);
            image->setPixelColor(i, j, QColor(color.r,color.g,color.b,color.a));
        }
    }
}

GRaster::GRaster(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GRaster),
    middleBuffer(GUtils::screenWidth,GUtils::screenHeight,QImage::Format_RGBA8888)
{
    ui->setupUi(this);

    SetupGRaster();
    CreateScene();
    OnDraw();
    OnPostDraw();
}

GRaster::~GRaster()
{
    delete ui;
    delete GLAPI;
}

void GRaster::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    GLog::LogInfo("size changed! w = ", event->oldSize().width(), " h = ", event->oldSize().height());
}

void GRaster::CreateScene()
{
    // light
    // camera
    GGameObject cameraGObj = GGameObject::CreateProjCamera(1, 2000, 60);
    cameraGObj.SetViewport(0, 0, GUtils::screenWidth, GUtils::screenHeight);
    cameras.push_back(cameraGObj);

    // models
    models.push_back(GGameObject::CreateModelGObj(GModelType::kMTCube));
    models.push_back(GGameObject::CreateModelGObj(GModelType::kMTObj, GUtils::GetAbsPath("obj/floor.obj")));
}

void GRaster::SetupGRaster()
{
    GLAPI = new GGraphicLibAPI();
    frameBuffer = GLAPI->GenFrameBuffer();
    assert(frameBuffer!=nullptr);
    GLAPI->BindFrameBuffer(frameBuffer);
    colorBuffer = GLAPI->GenRenderBuffer(GUtils::screenWidth, GUtils::screenHeight);
    assert(colorBuffer!=nullptr);
    GLAPI->AttachRenderBufferToFrameBuffer(frameBuffer, colorBuffer, GRenderBufferType::kRBFront);
    GLAPI->DrawRenderBuffer({GRenderBufferType::kRBFront});
    GLAPI->Clear(GColor::black);

    for(auto& model: models)
    {
        model.SetupDraw(GLAPI);
    }
}

void GRaster::OnDraw()
{
    for(auto& camera : cameras)
    {
        GGameObject::activeCamera = &camera;

        for(auto& model: models)
        {
            model.DrawModel(GLAPI);
        }
    }
}

void GRaster::OnPostDraw()
{
    CopyColorBufferToImage(colorBuffer, &middleBuffer);
    ui->canvas->setPixmap(QPixmap::fromImage(middleBuffer));
}
