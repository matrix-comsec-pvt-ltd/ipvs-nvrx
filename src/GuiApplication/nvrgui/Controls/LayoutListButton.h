#ifndef LAYOUTLISTBUTTON_H
#define LAYOUTLISTBUTTON_H

#define LAYOUTLIST_BUTTON_IMG_PATH ":/Images_Nvrx/Layouts/"

#include <QWidget>
#include "EnumFile.h"
#include "KeyBoard.h"
#include "NavigationControl.h"
#include <QTimer>

#define LAYOUT_LIST_BUTTON_WIDTH        SCALE_WIDTH(120)
#define LAYOUT_LIST_BUTTON_HEIGHT       SCALE_HEIGHT(47)

const QString layoutListImgPath [MAX_LAYOUTS] = {"1X1/",
                                                 "2X2/",
                                                 "1+5/",
                                                 "3+4/",
                                                 "1+7/",
                                                 "3X3/",
                                                 "2+8/",
                                                 "1+9/",
                                                 "1+12/",
                                                 "1C+12/",
                                                 "4+9/",
                                                 "2+12/",
                                                 "4X4/",
                                                 "5X5/",
                                                 "6X6/",
                                                 "8X8/",
                                                };

class LayoutListButton : public KeyBoard, public NavigationControl
{
    Q_OBJECT
private :
    QString m_imageSource;
    int m_index;
    IMAGE_TYPE_e m_currentImageType;
    QPixmap m_image;
    QRect m_imageRect;
    QTimer* m_clickEffectTimer;
    quint8  m_totalRow;
    quint8  m_totalCol;
    quint8  m_actualRow;
    quint8  m_actualCol;

public:
    LayoutListButton(int index, quint8 totalRow, quint8 totalCol, QWidget *parent = 0);
    ~LayoutListButton();

    void setGeometryForElements();
    void changeButtonImage(IMAGE_TYPE_e type);
    void selectControl();
    void deSelectControl();    
    void changeButtonIndex(int index);

    void takeEnterKeyAction();

    void forceActiveFocus();

    //virtual functions inherited from QWidget
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    virtual void enterKeyPressed(QKeyEvent *event);
	void setSyncPbButtonGeometry(quint8 iRowValue, quint8 iColValue);

signals:
    void sigButtonClicked(int index);
    void sigUpdateCurrentElement(int index);

public slots:
    void slotClickEffectTimerTimeout();
};

#endif // LAYOUTLISTBUTTON_H
