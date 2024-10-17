#ifndef IMAGE_H
#define IMAGE_H

#include <QWidget>
#include <QEvent>
#include <QTimer>
#include "Enumfile.h"
#include "NavigationControl.h"

class Image : public QWidget, public NavigationControl
{
    Q_OBJECT
private:
    POINT_PARAM_TYPE_e m_pointParamType;
    int m_startX, m_startY, m_width, m_height, m_xParam, m_yParam;
    QString m_imgSource;
    IMAGE_TYPE_e m_currentImageType;
    QPixmap m_image;
    QRect m_imageRect;
    bool m_isCompleteImageSource;
    bool m_alreadyHover;
    bool m_isPressTimerReq;
    QTimer *mouseButtonPressTimer;
    QTimer *clickeffctTimer;

    int currentIndex;


public:
    Image(int xParam, int yParam,
          QString imageSource,
          QWidget *parent = 0,
          POINT_PARAM_TYPE_e pointparamType = START_X_START_Y,
          int indexInPage = 0,
          bool isEnabled = false,
          bool isCompleteImageSource = false,
          bool catchKey = true,
          bool isPressTimerReq = false);
    ~Image();

    void setGeometryForElements();
    void changeImage(IMAGE_TYPE_e type, bool isForceupdate = false);
    void updateImageSource(QString imgSource, bool isForceupdate = false);
    QString getImageSource();
    IMAGE_TYPE_e getImageType();
    void resetGeometry(int xParam, int yParam);
    void selectControl();
    void deSelectControl();

    void scale(int width,int height);
    void forceActiveFocus();
    void setIsEnabled(bool isEnable);

    void takeEnterKeyAction();
    void takeDoubleClickEnterKeyAction();
    void takeClickAction();
    void notifyError();
    bool getImageClickSts();
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    void keyPressEvent(QKeyEvent *event);

    bool eventFilter (QObject *, QEvent *event);

signals:
    void sigImagePressed(int indexInPage);
    void sigImageClicked(int indexInPage);
    void sigImageDoubleClicked(int indexInPage);
    void sigUpdateCurrentElement(int index);
    void sigImageMouseHover(int indexInPage, bool isImageCantain);
    void sigMouseMove(int currentValue, int index);

public slots:
    void slotmouseButtonPressTimerTimeout();
    void slotclickeffctTimerTimeout();
};

#endif // IMAGE_H
