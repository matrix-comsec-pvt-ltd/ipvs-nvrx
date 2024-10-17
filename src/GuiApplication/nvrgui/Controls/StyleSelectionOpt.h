#ifndef STYLESELECTIONOPT_H
#define STYLESELECTIONOPT_H

#include "NavigationControl.h"
#include "Controls/CnfgButton.h"
#include "Controls/Rectangle.h"
#include "Controls/Heading.h"
#include "Controls/CalendarTile.h"
#include "Controls/SpinBox.h"

// List of control
typedef enum
{
    STYLE_SEL_SPINBOX,
    STYLE_SEL_OK_BTN,
    STYLE_SEL_CANCEL_BTN,

    MAX_STYLE_SEL_ELEMETS
}STYLE_SEL_ELE_e;

class StyleSelectionOpt : public KeyBoard
{
    Q_OBJECT
private:
    Rectangle *m_backGroundRect;
    Heading *m_heading;
    SpinBox *m_spinbox;
    CnfgButton *m_okBtn;
    CnfgButton *m_cancelBtn;

    NavigationControl *m_elementList[MAX_STYLE_SEL_ELEMETS];
    quint32 m_currElement;

public:
    explicit StyleSelectionOpt(QWidget *parent = 0);
    ~StyleSelectionOpt();

    void paintEvent (QPaintEvent *);
    void takeLeftKeyAction();
    void takeRightKeyAction();
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backtabKeyPressed(QKeyEvent *event);

signals:
    void sigStyleSelCnfgBtnClick(int index, quint8 selectedStyle);

public slots:
    void slotCnfgBtnButtonClick(int index);
    void slotUpadateCurrentElement(int index);
};

#endif // STYLESELECTIONOPT_H
