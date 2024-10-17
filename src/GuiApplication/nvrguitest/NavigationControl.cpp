#include "NavigationControl.h"

bool NavigationControl::m_isControlActivated = true;

NavigationControl::NavigationControl(int indexInPage,
                                     bool isEnabled,
                                     bool catchKey,
                                     bool isRightMouseButton)
{
    m_indexInPage = indexInPage;
    m_isEnabled = isEnabled;
    m_catchKey = catchKey;
    m_mouseClicked = false;
    m_mouseRightClicked = false;
    m_leftMouseButton = Qt::LeftButton;
    if(isRightMouseButton)
    {
        m_rightMouseButton = Qt::RightButton;
    }
    else
    {
        m_rightMouseButton = Qt::NoButton;
    }
}

void NavigationControl::setIsControlActivate(bool flag)
{
    m_isControlActivated = flag;
}

bool NavigationControl::getIsEnabled()
{
    return m_isEnabled;
}

void NavigationControl::setIsEnabled(bool isEnable)
{
    m_isEnabled = isEnable;
}

int NavigationControl::getIndexInPage()
{
    return m_indexInPage;
}

void NavigationControl::setIndexInPage(int indexInPage)
{
    m_indexInPage = indexInPage;
}

bool NavigationControl::getCatchKey()
{
    return m_catchKey;
}

void NavigationControl::setCatchKey(bool catchKey)
{
    m_catchKey = catchKey;
}

void NavigationControl::forceActiveFocus()
{

}

void NavigationControl::forceFocusToPage(bool)
{

}

