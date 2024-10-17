#ifndef WINDOWICON_H
#define WINDOWICON_H

#include <QWidget>
#include "ApplController.h"
#include "EnumFile.h"

class WindowIcon : public QWidget
{
private:
    int m_startX, m_startY, m_alignmentOffset;
    OSD_POSITION_e m_alignmentType;
    QString m_imgSource;
    QPixmap m_image;
    QRect m_imageRect;
    WINDOW_ICON_TYPE_e m_windowIconType;

public:
    WindowIcon(OSD_POSITION_e alignmentType, int alignmentOffset, QString imgSource,
               WINDOW_ICON_TYPE_e windowIconType,QWidget *parent = 0);

    void resetGeometry(int width, int height);
    void updateImageSource(QString imgSource);
    void setGeometryForElements(int width, int height);
    void changeAlignmentType(OSD_POSITION_e alignmentType);
    void changeAlignmentOffset(int alignmentOffset);
    void setWindowIconType(WINDOW_ICON_TYPE_e windowIconType);

    void paintEvent(QPaintEvent *);
};

#endif // WINDOWICON_H
