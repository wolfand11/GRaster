#include "graster.h"
#include "ui_graster.h"
#include "ui_gmodalmessage.h"
#include "glog.h"
#include <QResizeEvent>
#include "ggraphiclibapi.h"
#include "gmath.h"
#include "gutils.h"
using namespace GMath;

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
    updateTimer(this),
    middleBuffer(GUtils::screenWidth,GUtils::screenHeight,QImage::Format_RGBA8888)
{
    ui->setupUi(this);
    msgLabel = new QLabel;
    modalMsg = new GModalMessage(this);
    modalMsg->setModal(true);
    modalMsg->hide();

    SetupGRaster();
    CreateScene();

    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(_update()));
    updateTimer.start(1000);

    isNeedExist = false;
    isRenderingCompleted = true;
    renderingThread = std::thread(&GRaster::DoRendering, this);
    renderingThread.detach();

    ui->poxXLineEdit->setValidator(new QDoubleValidator(this));
    ui->poxYLineEdit->setValidator(new QDoubleValidator(this));
    ui->poxZLineEdit->setValidator(new QDoubleValidator(this));
    ui->rotationXLineEdit->setValidator(new QDoubleValidator(this));
    ui->rotationYLineEdit->setValidator(new QDoubleValidator(this));
    ui->rotationZLineEdit->setValidator(new QDoubleValidator(this));
    RefreshUI();
    connect(ui->poxXLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_cameraTRS_changed(const QString&)));
    connect(ui->poxYLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_cameraTRS_changed(const QString&)));
    connect(ui->poxZLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_cameraTRS_changed(const QString&)));
    connect(ui->rotationXLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_cameraTRS_changed(const QString&)));
    connect(ui->rotationYLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_cameraTRS_changed(const QString&)));
    connect(ui->rotationZLineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(on_cameraTRS_changed(const QString&)));
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

void GRaster::closeEvent(QCloseEvent *event)
{
    isNeedExist = true;
    hasDrawTask.notify_one();
}

void GRaster::CreateScene()
{
    // light
    GGameObject& lightGObj = GGameObject::CreateLightGObj(GLightType::kLTDirection);
    lightGObj.SetR(vec3f(50,-50,0));

    // camera
    GGameObject cameraGObj = GGameObject::CreateProjCamera(1, 2000, 90);
    cameraGObj.LookAt(vec3f(0,0,-2), vec3f(0,0,0), vec3f(0,1,0));
    cameraGObj.SetViewport(0, 0, GUtils::screenWidth, GUtils::screenHeight);
    cameras.push_back(cameraGObj);

    // models
    //models.push_back(GGameObject::CreateModelGObj(GModelType::kMTCube));
    models.push_back(GGameObject::CreateModelGObj(GModelType::kMTObj, GUtils::GetAbsPath("models/floor.obj")));
    auto diablo3GObj = GGameObject::CreateModelGObj(GModelType::kMTObj, GUtils::GetAbsPath("models/diablo3_pose/diablo3_pose.obj"));
    diablo3GObj.SetT(vec3f(0,0,0));
    models.push_back(diablo3GObj);
    auto sphereGObj = GGameObject::CreateModelGObj(GModelType::kMTObj, GUtils::GetAbsPath("models/sphere.obj"));
    sphereGObj.SetT(vec3f(0,0,-0.9));
    //models.push_back(sphereGObj);

    //models.push_back(GGameObject::CreateModelGObj(GModelType::kMTObj, GUtils::GetAbsPath("obj/triangle.obj")));

    for(auto& model: models)
    {
        model.SetupDraw(GLAPI);
    }
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
    // obj vertex is CounterClockwise
    GLAPI->SetFrontFace(GFrontFace::kCounterClockwise);
}

void GRaster::RefreshUI()
{
    if(cameras.size() < 1) return;

    GGameObject* mainCamera = &cameras[0];
    ui->poxXLineEdit->setText(QString("%1").arg(mainCamera->position().x()));
    ui->poxYLineEdit->setText(QString("%1").arg(mainCamera->position().y()));
    ui->poxZLineEdit->setText(QString("%1").arg(mainCamera->position().z()));
    ui->rotationXLineEdit->setText(QString("%1").arg(mainCamera->rotation().x()));
    ui->rotationYLineEdit->setText(QString("%1").arg(mainCamera->rotation().y()));
    ui->rotationZLineEdit->setText(QString("%1").arg(mainCamera->rotation().z()));
}

void GRaster::OnPreDraw()
{
}

void GRaster::DoDraw()
{
    OnPreDraw();
    timeCounter = 0;
    isRenderingCompleted = false;

    GLog::LogInfo("==> notify");
    hasDrawTask.notify_one();
}

void GRaster::OnPostDraw()
{
    CopyColorBufferToImage(colorBuffer, &middleBuffer);
    ui->canvas->setPixmap(QPixmap::fromImage(middleBuffer));
}

void GRaster::on_doDrawBtn_clicked()
{
    this->setEnabled(false);
    modalMsg->show();
    DoDraw();
}

void GRaster::on_cameraTRS_changed(const QString& text)
{
    GLog::LogInfo("trs changed!");

    if(cameras.size() < 1) return;
    GGameObject* mainCamera = &cameras[0];

    vec3f pos;
    pos.SetX(ui->poxXLineEdit->text().toFloat());
    pos.SetY(ui->poxYLineEdit->text().toFloat());
    pos.SetZ(ui->poxZLineEdit->text().toFloat());
    mainCamera->SetT(pos);
    vec3f rot;
    rot.SetX(ui->rotationXLineEdit->text().toFloat());
    rot.SetY(ui->rotationYLineEdit->text().toFloat());
    rot.SetZ(ui->rotationZLineEdit->text().toFloat());
    mainCamera->SetR(rot);
}

void GRaster::_update()
{
    if(isRenderingCompleted)
    {
        this->setEnabled(true);
        modalMsg->hide();
        OnPostDraw();
        if(timeCounter!=0)
        {
            msgLabel->setText(QString("rendering completed. cost %1 seconds!").arg(timeCounter));
            ui->statusBar->addWidget(msgLabel);
        }
    }
    else
    {
        auto msg = QString().asprintf("waited %d seconds!", timeCounter);
        modalMsg->SetMsg(msg);
        msgLabel->setText(msg);
        ui->statusBar->addWidget(msgLabel);

        timeCounter++;
    }
}

void GRaster::DoRendering()
{
    while(true)
    {
        if(isNeedExist)
        {
            break;
        }
        GLog::LogInfo("==> wait");
        std::unique_lock<std::mutex> lock(mtx);
        hasDrawTask.wait(lock);
        GLog::LogInfo("==> wait over");

        if(isNeedExist)
        {
            break;
        }

        GLAPI->Clear(GColor::black);
        for(auto& camera : cameras)
        {
            GGameObject::activeCamera = &camera;

            for(auto& model: models)
            {
                model.DrawModel(GLAPI);
            }
        }
        isRenderingCompleted = true;
    }
    GLog::LogInfo("==> exist!");
}

GModalMessage::GModalMessage(QWidget *parent)
    :QDialog(parent),msgUI(new Ui::GModalMessage)
{
    msgUI->setupUi(this);
}

void GModalMessage::SetMsg(QString msg)
{
    msgUI->msg->setText(msg);
}
