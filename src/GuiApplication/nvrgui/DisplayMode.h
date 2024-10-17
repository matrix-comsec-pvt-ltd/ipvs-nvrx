#ifndef DISPLAYMODE_H
#define DISPLAYMODE_H

#include <QWidget>
#include "Controls/LayoutList.h"
#include "DataStructure.h"
#include "KeyBoard.h"

class DisplayMode : public KeyBoard
{
    Q_OBJECT
private:
    LayoutList* m_layoutList;
public:
    explicit DisplayMode(QWidget *parent = 0);
    ~DisplayMode();

    virtual void escKeyPressed(QKeyEvent *event);

signals:
    void sigClosePage(TOOLBAR_BUTTON_TYPE_e buttonIndex);
    void sigApplyNewLayout(DISPLAY_TYPE_e displayType, DISPLAY_CONFIG_t displayConfig, STYLE_TYPE_e styleNo);
    void sigToolbarStyleChnageNotify(STYLE_TYPE_e);

public slots:
    void slotChangeLayout(LAYOUT_TYPE_e index);
};

#endif // DISPLAYMODE_H
