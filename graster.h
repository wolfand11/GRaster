#ifndef GRASTER_H
#define GRASTER_H

#include <QMainWindow>
#include <QDialog>
#include <QTimer>
#include <QImage>
#include <QLabel>
#include "ggameobject.h"
#include "ggraphiclibapi.h"
#include <thread>
#include <condition_variable>

namespace Ui {
class GRaster;
}

class GModalMessage;
class GRaster : public QMainWindow
{
    Q_OBJECT

public:
    explicit GRaster(QWidget *parent = 0);
    ~GRaster();

protected:
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void closeEvent(QCloseEvent* event) override;
    void CreateScene();
    void SetupGRaster();
    void RefreshUI();
    void OnPreDraw();
    void DoDraw();
    void OnPostDraw();

private slots:
    void on_doDrawBtn_clicked();
    void on_cameraTRS_changed(const QString& text);
    void _update();

private:
    Ui::GRaster *ui;
    QLabel* msgLabel;
    GModalMessage *modalMsg;
    QTimer updateTimer;
    QImage middleBuffer;
    GGraphicLibAPI* GLAPI;

    std::vector<GGameObject> cameras;
    std::vector<GGameObject> models;

    int timeCounter;
    bool isNeedExist;
    bool isRenderingCompleted;
    void DoRendering();
    std::mutex mtx;
    std::condition_variable hasDrawTask;
    std::thread renderingThread;
};


namespace Ui {
class GModalMessage;
}
class GModalMessage : public QDialog
{
public:
    GModalMessage(QWidget* parent);
    void SetMsg(QString msg);

private:
    Ui::GModalMessage *msgUI;
};

#endif // GRASTER_H
