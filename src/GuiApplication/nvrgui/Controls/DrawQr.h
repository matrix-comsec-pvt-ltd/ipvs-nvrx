#ifndef DRAWQR_H
#define DRAWQR_H

#include <QObject>
#include <QWidget>
#include "KeyBoard.h"
#include "ApplController.h"
#include <QPainter>
#include <QPixmap>
#include <QPaintEvent>
#include "EnumFile.h"
#include "qrencode.h"

#define QR_IMAGE_WIDTH      SCALE_WIDTH(105)
#define QR_IMAGE_HEIGHT     SCALE_HEIGHT(105)

class DrawQr: public QWidget
{
    Q_OBJECT
private:
    int m_startX, m_startY;
    bool m_isDrawFromPng;
    QRcode *qr;
    QPixmap m_qrImage;
    QString m_qrData, m_qrPath;


public:
    DrawQr(int xParam,
           int yParam,
           QString path = "",
           QWidget* parent = NULL,
           QString qrData = "",
           bool isDrawFromPng = true);

    void paintEvent(QPaintEvent *event);
    void DisplayQrCode();
    ~DrawQr();
};

#endif // DRAWQR_H
