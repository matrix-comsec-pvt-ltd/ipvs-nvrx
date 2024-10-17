#ifndef DROPDOWNLIST_H
#define DROPDOWNLIST_H

#include <QWidget>
#include <QMap>
#include <QPointer>
#include <Controls/BackGround.h>
#include <Controls/Closebuttton.h>
#include <Controls/MenuButton.h>
#include "ScrollBar.h"

#define MAX_LIST_ITEM                150
#define MAX_LIST_ITEM_ON_PAGE        8

class DropDownList : public BackGround
{
    Q_OBJECT

public:
    explicit DropDownList(quint16 startx,
                          quint16 starty,
                          quint16 width,
                          QMap<quint8, QString> optionList,
                          quint8 currentKey,
                          QWidget *parent = 0,
                          quint8 maxElemetOnList = MAX_LIST_ITEM_ON_PAGE);

    ~DropDownList();

    void createElements();
    void resetGeometryForMenuButtons();
    quint8 findMenubuttonIndexForSelectedKey();
    quint8 findKeyForMenubuttonIndex();
    void closeDropListAction();

    void takeUpKeyAction();
    void takeDownKeyAction();

    void setFocusToPage();

    void isMouseClikOnMove(bool, QPoint pos);

    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void enterKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    void wheelEvent (QWheelEvent * event);
    virtual void backTab_KeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void insertKeyPressed(QKeyEvent *event);
    virtual void ctrl_S_KeyPressed(QKeyEvent *event);
    virtual void ctrl_D_KeyPressed(QKeyEvent *event);

signals:
    void sigValueChanged(quint8 key, QString value);

public slots:
    void slotOptionClicked(int index);
    void slotScroll(int numberOfSteps);
    void slotUpdateCurrentElement(int index);

private:

    QString                 m_pickListHeading;
    quint8                  m_currentKey;
    quint8                  m_effectiveSize;
    quint16                 m_tempstartX,m_tempstartY;
    quint16                 m_currentIndex;
    quint16                 m_backgroundWidth;
    quint16                 m_backgroundHeight;
    quint16                 m_firstIndex, m_lastIndex;
    quint16                 m_currentElement;
    quint16                 m_tempwidth;

    bool                    m_isScrollbarActive;
    bool                    m_catchKey;
    bool                    m_isPickListInUse;
    bool                    m_isUsedForTextList;
    
    QMap<quint8, QString>   m_optionList;
    QPointer<ScrollBar>     m_scrollbar;
    QPointer<MenuButton>    m_options[MAX_LIST_ITEM];
    NavigationControl*      m_elementList[(MAX_LIST_ITEM + 2)];

    quint8                  m_maxElemetOnList;
};

#endif // DROPDOWNLIST_H
