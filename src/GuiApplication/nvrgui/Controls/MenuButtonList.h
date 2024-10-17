#ifndef MENUBUTTONLIST_H
#define MENUBUTTONLIST_H

#include <QWidget>
#include "Controls/MenuButton.h"

#define MAX_MENU_POSSIBLE           50
#define BUTTON_WIDTH                SCALE_WIDTH(150)
#define BUTTON_HEIGHT               SCALE_HEIGHT(30)

class MenuButtonList : public KeyBoard
{
    Q_OBJECT
private:
    QStringList m_menuLabelList;
    bool m_catchKey;
    int m_startX, m_startY;
    MenuButton* m_menuButtons[MAX_MENU_POSSIBLE];
    quint16 m_buttonWidth;
    int m_currentElement;
    NavigationControl* m_elementList[MAX_MENU_POSSIBLE];

public:
    MenuButtonList(int xParam,
                   int yParam,
                   QStringList menuLabelList,
                   QWidget* parent = 0,
                   bool catchKey = true,
                   quint16 buttonWidth = BUTTON_WIDTH);

    ~MenuButtonList();

    void disableButton(quint8 index, bool isDisable);
    void takeUpKeyAction();
    void takeDownKeyAction();

    void forceActiveFocus();

    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);


signals:
    void sigMenuSelected(QString menuString, quint8 index);

public slots:
    void slotMenuButtonClicked(int index);
    void slotUpdateCurrentElement(int index);
};

#endif // MENUBUTTONLIST_H
