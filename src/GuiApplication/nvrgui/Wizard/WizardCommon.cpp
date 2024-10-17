#include "WizardCommon.h"

#define START_X                 SCALE_WIDTH(30)
#define START_Y                 SCALE_WIDTH(105)
#define BGTILE_HEADING_WIDTH    SCALE_WIDTH(1080)
#define BGTILE_HEADING_HEIGHT   SCALE_HEIGHT(565)

InvisibleBackground  * WizardCommon :: m_invisibleBackGround = NULL;
QWidget *WizardCommon::m_parent = NULL;

WizardCommon::WizardCommon(QWidget *parent, WIZARD_PAGE_INDEXES_e PageIndex) :QWidget(parent)
{
    INIT_OBJ(m_bgTile);
    INIT_OBJ(m_invisibleBackGround);
    INIT_OBJ(processBar);

    m_parent = parent;
    this->setGeometry(START_X, START_Y,  (BGTILE_HEADING_WIDTH),(BGTILE_HEADING_HEIGHT));

    m_bgTile = new BgTile(0,
                          0,
                          (BGTILE_HEADING_WIDTH),
                          (BGTILE_HEADING_HEIGHT),
                          COMMON_LAYER,
                          this);

    m_CurrentPage = PageIndex;
    m_deletePage = false;
    m_getConfig = false;
}

WIZARD_PAGE_INDEXES_e WizardCommon :: getCurrPageIndex() const
{
    return m_CurrentPage;
}

bool WizardCommon ::isObjDeleteReq() const
{
    return m_deletePage;
}

WIZARD_PAGE_INDEXES_e WizardCommon :: getNextPrevPageIndex(PG_NAVG_OPT_e option)
{
    quint8 page = MAX_WIZ_PG;

    if(option == NEXT_PG)
    {
        if(m_CurrentPage < WIZ_PG_STATUS)
        {
            page = m_CurrentPage + (WIZARD_PAGE_INDEXES_e)1;
        }
    }
    else
    {
        if(m_CurrentPage > WIZ_PG_INIT)
        {
            page = m_CurrentPage - (WIZARD_PAGE_INDEXES_e)1;
        }
    }

    return (WIZARD_PAGE_INDEXES_e)page;
}

WizardCommon :: ~WizardCommon()
{
    DELETE_OBJ(m_bgTile);
}

void WizardCommon :: InfoPageImage()
{
    if(m_invisibleBackGround == NULL)
    {
       m_invisibleBackGround = new InvisibleBackground(m_parent);
    }
}

void WizardCommon :: LoadProcessBar()
{
    if(processBar == NULL)
    {
		/* PARASOFT: Memory Deallocated in Unload Process Bar */
        processBar = new ProcessBar(0, 0,
                                    WIZARD_MAIN_RECT_WIDTH,
                                    WIZARD_MAIN_RECT_HEIGHT,
                                    SCALE_WIDTH(RECT_RADIUS), m_parent);
    }
}

void WizardCommon :: UnloadInfoPageImage()
{
    DELETE_OBJ(m_invisibleBackGround);
}

void WizardCommon :: UnloadProcessBar()
{
    DELETE_OBJ(processBar);
}

void InvisibleBackground::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QColor color;

    color.setAlpha (150);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);

    painter.drawRoundedRect (QRect(0,
                                   0,
                                   WIZARD_MAIN_RECT_WIDTH,
                                   WIZARD_MAIN_RECT_HEIGHT),
                                   SCALE_WIDTH(RECT_RADIUS),
                                   SCALE_HEIGHT(RECT_RADIUS));
}

InvisibleBackground::InvisibleBackground(QWidget *parent) : QWidget(parent)
{
    this->setGeometry(0,0,  WIZARD_MAIN_RECT_WIDTH, WIZARD_MAIN_RECT_HEIGHT);
    this->show();
}

InvisibleBackground :: ~InvisibleBackground()
{
}

