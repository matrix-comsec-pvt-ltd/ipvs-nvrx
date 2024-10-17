#include "DrawQr.h"

#define QR_PAINT_MARGIN_WIDTH       SCALE_WIDTH(8)
#define QR_PAINT_MARGIN_HEIGHT      SCALE_HEIGHT(8)

#define QR_INTERNAL_IMAGE_WIDTH     (QR_IMAGE_WIDTH - QR_PAINT_MARGIN_WIDTH)
#define QR_INTERNAL_IMAGE_HEIGHT    (QR_IMAGE_HEIGHT - QR_PAINT_MARGIN_HEIGHT)

DrawQr::DrawQr(int xParam,
               int yParam,
               QString path,
               QWidget* parent,
               QString qrData,
               bool isDrawFromPng): QWidget(parent)
{
    qr = NULL;
    m_startX = xParam;
    m_startY = yParam;
    m_qrPath = path;
    m_qrData = qrData;
    m_isDrawFromPng = isDrawFromPng;

    DisplayQrCode();

    if(m_isDrawFromPng == true)
    {
        this->setGeometry(QRect(m_startX,m_startY,m_qrImage.width(),m_qrImage.height()));       
    }
    else
    {
        this->setGeometry(QRect(m_startX,m_startY,QR_IMAGE_WIDTH,QR_IMAGE_HEIGHT));
    }
    this->show();
}

void DrawQr::DisplayQrCode()
{
    if(m_isDrawFromPng == true)
    {
        m_qrImage = QPixmap((m_qrPath));

        m_qrImage = m_qrImage.scaled(QR_IMAGE_WIDTH, QR_IMAGE_HEIGHT, Qt::KeepAspectRatio);
    }
    else
    {         
        qr = QRcode_encodeString(m_qrData.toLocal8Bit().constData(),1,QR_ECLEVEL_L,QR_MODE_8,1);
    }
    update();
}

void DrawQr::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    if(m_isDrawFromPng == true)
    {
        /* The Qr code cordinates are given here according to which it will be drawn */
        painter.drawPixmap(0,0, m_qrImage);
    }
    else
    {
        /* This logic draws an background white rectangle of size same as the QR width and height
         * as well as draws inner rectangles of size according to innerblock size when and where
         * the data array has a non zero value
         */
        if(NULL!=qr)
        {
            QColor fg("black");
            QColor bg("white");
            painter.setBrush(bg);
            painter.setPen(Qt::NoPen);
            painter.drawRect(0,0,QR_IMAGE_WIDTH,QR_IMAGE_HEIGHT);
            painter.setBrush(fg);
            const int main_width=qr->width>0?qr->width:1;
            const double internal_width=QR_INTERNAL_IMAGE_WIDTH;
            const double internal_height=QR_INTERNAL_IMAGE_HEIGHT;
            const double aspectRatio=internal_width/internal_height;

            const double innerBlockSize=((aspectRatio>1.0)?internal_height:internal_width)/main_width;
            for(int row=0;row<main_width;row++)
            {
                const int rowIndex=row * main_width;
                for(int col=0;col<main_width;col++)
                {
                    const int colIndex=rowIndex+col;
                    const unsigned char qrDataElement=qr->data[colIndex];
                    if(qrDataElement & 0x01)
                    {
                        const double rectStartX=col*innerBlockSize, rectStartY=row*innerBlockSize;
                        /* Here margin added so that we can have white border surrounded to the qr blocks */
                        QRectF r(rectStartX + QR_PAINT_MARGIN_WIDTH/2,
                                 rectStartY + QR_PAINT_MARGIN_HEIGHT/2,
                                 innerBlockSize, innerBlockSize);
                        painter.drawRects(&r,1);
                    }
                }
            }
        }
    }
}

DrawQr::~DrawQr()
{
    if(NULL!=qr)
    {
        QRcode_free(qr);
    }
}
