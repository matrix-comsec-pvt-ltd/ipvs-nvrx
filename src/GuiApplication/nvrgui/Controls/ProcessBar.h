#ifndef PROCESSBAR_H
#define PROCESSBAR_H

#include <QWidget>
#include "ApplController.h"
#include "EnumFile.h"

class ProcessBar : public QWidget
{
private:
     int roundRadius;
     QRect mainRect;
     QRect imageRect;
     QPixmap image;

public:
     ProcessBar(int xParam,
                int yParam,
                int width,
                int height,
                int radius,
                QWidget *parent = 0);
     void unloadProcessBar();
     void loadProcessBar();
     bool isLoadedProcessBar();
     void paintEvent (QPaintEvent *event);
};

#endif // PROCESSBAR_H
