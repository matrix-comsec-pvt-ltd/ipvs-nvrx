#ifndef CAMERASEARCHPROCESS_H
#define CAMERASEARCHPROCESS_H

#include <QWidget>
#include "ApplController.h"
#include "EnumFile.h"

class CameraSearchProcess : public QWidget
{
    Q_OBJECT
public:
    explicit CameraSearchProcess(quint8 startx,
                                 quint8 starty,
                                 quint16 width,
                                 quint16 height,
                                 QWidget *parent = 0);

    void paintEvent (QPaintEvent *);
    
signals:
    
public slots:
    
};

#endif // CAMERASEARCHPROCESS_H
