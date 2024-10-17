#include "StyleSelectionOpt.h"
#include <QPainter>
#include <QKeyEvent>

#define STYLE_SEL_BGRECT_WIDTH        SCALE_WIDTH(486)
#define STYLE_SEL_BGRECT_HEIGHT       SCALE_HEIGHT(200)
#define STYLE_SEL_HEADING_STRING    "Select Style"

static const QStringList styleSelList = QStringList()
        <<"1" <<"2" <<"3" << "4" <<"5";

StyleSelectionOpt::StyleSelectionOpt(QWidget *parent)
    :KeyBoard(parent)
{
    this->setGeometry (0,0, parent->width (), parent->height ());
    m_backGroundRect = new Rectangle((this->width () - STYLE_SEL_BGRECT_WIDTH)/2,
                                     SCALE_HEIGHT(DISP_SETTING_PAGE_HEADING_HEIGHT) + (this->height () - STYLE_SEL_BGRECT_HEIGHT)/2,
                                     STYLE_SEL_BGRECT_WIDTH,
                                     STYLE_SEL_BGRECT_HEIGHT,
                                     0, BORDER_2_COLOR,
                                     NORMAL_BKG_COLOR, this, 1);

    m_heading = new Heading(m_backGroundRect->x () + m_backGroundRect->width ()/2,
                            m_backGroundRect->y () + SCALE_HEIGHT(20),
                            STYLE_SEL_HEADING_STRING,
                            this, HEADING_TYPE_2);

    m_spinbox = new SpinBox(m_backGroundRect->x () + SCALE_WIDTH(10),
                            m_backGroundRect->y () + SCALE_HEIGHT(70),
                            BGTILE_MEDIUM_SIZE_WIDTH,
                            BGTILE_HEIGHT,
                            STYLE_SEL_SPINBOX,
                            SPINBOX_SIZE_90, "Style",
                            styleSelList, this, "",
                            false, 0, NO_LAYER);
    m_elementList[STYLE_SEL_SPINBOX] = m_spinbox;
    connect (m_spinbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));

    m_okBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                             m_backGroundRect->x () + m_backGroundRect->width ()/2 - SCALE_WIDTH(75),
                             m_backGroundRect->y () + m_backGroundRect->height () - SCALE_HEIGHT(50),
                             "OK", this, STYLE_SEL_OK_BTN);
    m_elementList[STYLE_SEL_OK_BTN] = m_okBtn;
    connect (m_okBtn,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));
    connect (m_okBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotCnfgBtnButtonClick(int)));

    m_cancelBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                             m_backGroundRect->x () + m_backGroundRect->width ()/2 + SCALE_WIDTH(75),
                             m_backGroundRect->y () + m_backGroundRect->height () - SCALE_HEIGHT(50),
                             "Cancel", this, STYLE_SEL_CANCEL_BTN);
    m_elementList[STYLE_SEL_CANCEL_BTN] = m_cancelBtn;
    connect (m_cancelBtn,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));
    connect (m_cancelBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotCnfgBtnButtonClick(int)));

    m_currElement = STYLE_SEL_SPINBOX;
    m_elementList[m_currElement]->forceActiveFocus ();
    this->show ();
}

StyleSelectionOpt::~StyleSelectionOpt()
{
    delete m_backGroundRect;
    delete m_heading;

    disconnect (m_spinbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));
    delete m_spinbox;

    disconnect (m_okBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotCnfgBtnButtonClick(int)));
    disconnect (m_okBtn,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));
    delete m_okBtn;

    disconnect (m_cancelBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotCnfgBtnButtonClick(int)));
    disconnect (m_cancelBtn,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));
    delete m_cancelBtn;
}

void StyleSelectionOpt::paintEvent (QPaintEvent *)
{
    QPainter painter(this);
    QColor color;

    color.setAlpha (0);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);
    painter.drawRoundedRect (QRect(0,
                                   0,
                                   SCALE_WIDTH(DISP_SETTING_PAGE_HEADING_WIDTH),
                                   SCALE_HEIGHT(DISP_SETTING_PAGE_HEADING_HEIGHT)),
                                   SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));

    color.setAlpha (150);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);
    painter.drawRoundedRect (QRect(0,
                                   SCALE_HEIGHT(DISP_SETTING_PAGE_HEADING_HEIGHT),
                                   SCALE_WIDTH(DISP_SETTING_PAGE_WIDTH),
                                   SCALE_HEIGHT(DISP_SETTING_PAGE_HEIGHT) - SCALE_HEIGHT(DISP_SETTING_PAGE_HEADING_HEIGHT)),
                                    SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));
}

void StyleSelectionOpt::takeLeftKeyAction()
{
        m_currElement = (m_currElement - 1 + MAX_STYLE_SEL_ELEMETS) % MAX_STYLE_SEL_ELEMETS;
        m_elementList[m_currElement]->forceActiveFocus();
}


void StyleSelectionOpt::takeRightKeyAction()
{
    m_currElement = (m_currElement + 1) % MAX_STYLE_SEL_ELEMETS;
    m_elementList[m_currElement]->forceActiveFocus();
}

void StyleSelectionOpt::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void StyleSelectionOpt::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    if(m_cancelBtn->hasFocus ())
    {
        emit sigStyleSelCnfgBtnClick(STYLE_SEL_CANCEL_BTN, 0);
    }
    else
    {
        m_currElement = STYLE_SEL_CANCEL_BTN;
        m_elementList[m_currElement]->forceActiveFocus ();
    }
}

void StyleSelectionOpt::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void StyleSelectionOpt::backtabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void StyleSelectionOpt::slotUpadateCurrentElement (int index)
{
    m_currElement = index;
}

void StyleSelectionOpt::slotCnfgBtnButtonClick(int index)
{
    emit sigStyleSelCnfgBtnClick (index, m_spinbox->getIndexofCurrElement ());
}
