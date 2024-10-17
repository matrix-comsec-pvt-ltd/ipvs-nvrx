#ifndef LAYOUTLIST_H
#define LAYOUTLIST_H

#include <QWidget>
#include "LayoutListButton.h"
#include "NavigationControl.h"
#include "EnumFile.h"
#include "ScrollBar.h"

typedef enum
{
    LAYOUT_LIST_WITH_SCROLL,
    LAYOUT_LIST_4X4_TYPE,
    LAYOUT_LIST_4X1_TYPE,
    MAX_LAYOUT_LIST_TYPE,

}LAYOUT_LIST_TYPE_e;


class LayoutList : public KeyBoard, public NavigationControl
{
    Q_OBJECT
private:
    quint8 m_maxLayout;
    LayoutListButton* m_layoutListButtons[MAX_LAYOUTS];
    QRect m_mainRect;
    QRect m_topBorder;
    QRect m_leftBorder;
    QRect m_rightBorder;
    QRect m_bottomBorder;

    quint32 m_startx;
    quint32 m_starty;

    LAYOUT_LIST_TYPE_e m_listType;
    quint8  m_totalRow;
    quint8  m_totalCol;
    ScrollBar *m_scrollbar;
    quint8  m_offset;

public:
    LayoutList(quint32 startX,
               quint32 startY,
               LAYOUT_LIST_TYPE_e listType,
               QWidget *parent = 0,
               quint8 maxLayout = MAX_LAYOUTS,
               int indexInPage = 0);
    ~LayoutList();

    NavigationControl* m_elementList[MAX_LAYOUTS];
    int m_currentElement;

    void setGeometryForElements();
    void changeMaxLayout(quint8 maxLayout);
    void takeUpKeyAction();
    void takeDownKeyAction();
    void takeLeftKeyAction();
    void takeRightKeyAction();
	void resetGeometry(quint32 iStartX, quint32 iStartY);

    void paintEvent(QPaintEvent *);
    void wheelEvent(QWheelEvent *event);
    virtual void navigationKeyPressed(QKeyEvent *event);
    void forceFocusToPage(bool isFirstElement);

signals:
    void sigChangeLayout(LAYOUT_TYPE_e index);
    void sigUpdateCurrentElement(int index);

public slots:
    void slotButtonClicked(int index);
    void slotUpdateCurrentElement(int index);
    void slotScroll(int offset);
};

#endif // LAYOUTLIST_H
