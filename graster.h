#ifndef GRASTER_H
#define GRASTER_H

#include <QMainWindow>

namespace Ui {
class GRaster;
}

class GRaster : public QMainWindow
{
    Q_OBJECT

public:
    explicit GRaster(QWidget *parent = 0);
    ~GRaster();

private:
    Ui::GRaster *ui;
    QImage *framebuffer;
};

#endif // GRASTER_H
