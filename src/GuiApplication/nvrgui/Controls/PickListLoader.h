#ifndef PICKLISTLOADER_H
#define PICKLISTLOADER_H

#include <QWidget>
#include <Controls/BackGround.h>
#include <Controls/Closebuttton.h>
#include <Controls/MenuButton.h>
#include "ScrollBar.h"
#include <QMap>

#define MAX_ITEM                200

class PickListLoader : public BackGround
{
    Q_OBJECT
private:
    QMap<quint8, QString> m_optionList;
    MenuButton* m_options[MAX_ITEM];
    ScrollBar* m_scrollbar;
    QString m_pickListHeading;
    int m_currentIndex, m_backgroundWidth, m_backgroundHeight, m_firstIndex, m_lastIndex;
    bool m_isScrollbarActive;
    int m_currentElement;
    quint8 m_currentKey;
    NavigationControl* m_elementList[(MAX_ITEM + 2)];
    quint8 m_effectiveSize;
    bool m_catchKey;
    bool m_isPickListInUse;

public:
    PickListLoader(QMap<quint8, QString> optionList,
                   quint8 currentKey,
                   QString heading,
                   QWidget *parent = 0,
                   bool catchKey = true,
                   bool isPickListInUse = true);
    ~PickListLoader();

    void createElements();
    void resetGeometryForMenuButtons();
    quint8 findMenubuttonIndexForSelectedKey();
    quint8 findKeyForMenubuttonIndex();
    void closePickListLoaderAction(bool isCancelClick);

    void takeUpKeyAction();
    void takeDownKeyAction();

    void escKeyPressed(QKeyEvent *event);
    void navigationKeyPressed(QKeyEvent *event);
    void tabKeyPressed(QKeyEvent *event);
    void backTab_KeyPressed(QKeyEvent *event);

    void wheelEvent (QWheelEvent * event);

signals:
    void sigValueChanged(quint8 key, QString value, bool isCancelClick);

public slots:    
    void slotCloseButtonClicked(int indexInPage);
    void slotMenuButtonClicked(int index);
    void slotScroll(int numberOfSteps);
    void slotUpdateCurrentElement(int index);
};

#endif // PICKLISTLOADER_H
