#include "graster.h"
#include "ui_graster.h"

GRaster::GRaster(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GRaster)
{
    ui->setupUi(this);
    framebuffer = new QImage(500,500,QImage::Format_ARGB32);
}

GRaster::~GRaster()
{
    delete ui;
    delete framebuffer;
}
