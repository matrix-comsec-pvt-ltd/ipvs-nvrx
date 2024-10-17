#ifndef CLOSEBUTTTON_H
#define CLOSEBUTTTON_H

#include <QWidget>
#include "Enumfile.h"
#include "NavigationControl.h"

#define CLOSE_BTN_PATH          IMAGE_PATH"/ControlButtons/CloseButtons/"

static QString closeBtnSubFolder[6] ={ "ButtonType_1/",
                                       "ButtonType_2/",
                                       "ButtonType_3/",
                                       "ButtonType_4/",
                                       "ButtonType_5/",
                                       "ButtonType_6/"};

typedef enum
{
    CLOSE_BTN_TYPE_1,
    CLOSE_BTN_TYPE_2,
    CLOSE_BTN_TYPE_3,
    CLOSE_BTN_TYPE_4,
    CLOSE_BTN_TYPE_5,
    CLOSE_BTN_TYPE_6
}type_e;

class CloseButtton: public QWidget, public NavigationControl
{
    Q_OBJECT

private:
    bool m_onlyHoverImg;
    QString m_imageSource;
    type_e m_buttonType;
    QRect m_imageRect;
    IMAGE_TYPE_e m_imageType;
    int m_centerX, m_centerY;
    QPixmap m_image;

public:
    CloseButtton(int centerX,
                 int centerY,
                 QWidget* parent = 0,
                 type_e buttonType = CLOSE_BTN_TYPE_1,
                 int indexInPage = 0,
                 bool isEnabled = true,
                 bool catchKey = true);

    CloseButtton(int endX,
                 int startY,
                 int topMargin,
                 int rightMargin,
                 QWidget* parent,
                 type_e buttonType = CLOSE_BTN_TYPE_1,
                 int indexInPage = 0,
                 bool isEnabled = true,
                 bool catchKey = true,
                 bool onlyHoverImage = false);

    void changeImage(IMAGE_TYPE_e type);
    void changeImageState(type_e buttonType);
    void resetGeometry(int centerX, int centerY);
    void selectControl();
    void deSelectControl();

    void forceActiveFocus();

    void takeEnterKeyAction();

    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent * event);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    void keyPressEvent(QKeyEvent *event);

signals:
    void sigButtonClick(int indexInPage);
    void sigUpdateCurrentElement(int index);
};

#endif // CLOSEBUTTTON_H
