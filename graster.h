#ifndef GRASTER_H
#define GRASTER_H

#include <QMainWindow>
#include <QImage>
#include "ggameobject.h"
#include "ggraphiclibapi.h"

namespace Ui {
class GRaster;
}

class GRaster : public QMainWindow
{
    Q_OBJECT

public:
    explicit GRaster(QWidget *parent = 0);
    ~GRaster();

protected:
    virtual void resizeEvent(QResizeEvent* event) override;
    void CreateScene();
    void SetupGRaster();
    void OnDraw();
    void OnPostDraw();

private:
    Ui::GRaster *ui;
    QImage middleBuffer;
    GGraphicLibAPI* GLAPI;
    GGameObject camera;
    GGameObject light;

    std::vector<GGameObject> cameras;
    std::vector<GGameObject> models;
};

#endif // GRASTER_H
