#include "DisplayMode.h"
#include "ApplicationMode.h"
#include <QKeyEvent>
#include "Layout/Layout.h"

DisplayMode::DisplayMode(QWidget *parent)
    : KeyBoard(parent)
{
    ApplicationMode::setApplicationMode(PAGE_WITH_TOOLBAR_MODE);

    m_layoutList = new LayoutList(0, 0, LAYOUT_LIST_4X4_TYPE, this);
    connect(m_layoutList,
            SIGNAL(sigChangeLayout(LAYOUT_TYPE_e)),
            this,
            SLOT(slotChangeLayout(LAYOUT_TYPE_e)));

    m_layoutList->forceFocusToPage(true);

    quint8 leftMargin = (TOOLBAR_BUTTON_WIDTH / 2);
    this->setGeometry((ApplController::getXPosOfScreen() + (TOOLBAR_BUTTON_WIDTH * DISPLAY_MODE_BUTTON) + leftMargin),
                      (ApplController::getYPosOfScreen() + ApplController::getHeightOfScreen() - TOOLBAR_BUTTON_HEIGHT - SCALE_HEIGHT(10) - m_layoutList->height()),
                      m_layoutList->width(),
                      m_layoutList->height());
    this->show();
}

DisplayMode::~DisplayMode()
{
    disconnect(m_layoutList,
               SIGNAL(sigChangeLayout(LAYOUT_TYPE_e)),
               this,
               SLOT(slotChangeLayout(LAYOUT_TYPE_e)));
    delete m_layoutList;
}

void DisplayMode::escKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Escape:
//    case Qt::Key_F4:
        event->accept();
        emit sigClosePage(DISPLAY_MODE_BUTTON);
        break;

    default:
        event->accept();
        break;
    }
}

void DisplayMode::slotChangeLayout(LAYOUT_TYPE_e index)
{
    if(Layout::currentDisplayConfig[MAIN_DISPLAY].layoutId != index)
    {
        DISPLAY_CONFIG_t displayConfig;
        memcpy((void*)&displayConfig,
               (void*)&Layout::currentDisplayConfig[MAIN_DISPLAY],
               sizeof(DISPLAY_CONFIG_t));
        displayConfig.layoutId = (LAYOUT_TYPE_e) (index);
        emit sigApplyNewLayout(MAIN_DISPLAY, displayConfig, MAX_STYLE_TYPE);
        emit sigToolbarStyleChnageNotify(MAX_STYLE_TYPE);
    }
    emit sigClosePage(DISPLAY_MODE_BUTTON);
}
