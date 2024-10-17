#ifndef NAVIGATIONCONTROL_H
#define NAVIGATIONCONTROL_H
#include <QWidget>
#include "EnumFile.h"

class NavigationControl
{
protected:
    bool m_isEnabled;
    int m_indexInPage;
    Qt::MouseButton m_leftMouseButton;
    Qt::MouseButton m_rightMouseButton;
    bool m_catchKey, m_mouseClicked, m_mouseRightClicked;
public:
    NavigationControl(int indexInPage,
                      bool isEnabled,
                      bool catchKey = true,
                      bool isRightMouseButton = false);

   virtual ~NavigationControl();

public:
    static bool m_isControlActivated;
    static void setIsControlActivate(bool flag);

    virtual bool getIsEnabled();
    virtual void setIsEnabled(bool isEnable);
    virtual int getIndexInPage();
    virtual void setIndexInPage(int indexInPage);
    virtual bool getCatchKey();
    virtual void setCatchKey(bool catchKey);

    virtual void forceActiveFocus();
    virtual void forceFocusToPage(bool);
};

#endif // NAVIGATIONCONTROL_H
