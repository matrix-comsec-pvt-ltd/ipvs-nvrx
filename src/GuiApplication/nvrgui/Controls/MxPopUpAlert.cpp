#include "MxPopUpAlert.h"
#include "EnumFile.h"
#include "ValidationMessage.h"

static QString popUpAlertImgSrcPath[] = {":/Images_Nvrx/PopUpAlert/Information.png"};

static QString popUpAlertCloseImgSrcPath[] = {":/Images_Nvrx/PopUpAlert/Close_Blue/"};

static QString popUpBackGroundColor[] = {"#528dc9"};

static QString popUpAlertLableStr[MAX_POP_UP_ALERT_NAME][4] = {
    //Auto cnfg cam Str
    {
        "Auto Configuration Completed",
        "Camera auto configuration process has been completed.",
        "View Status Report...",
        ""
    },
    //Auto Timezone Str
    {
        "Time Zone Detected",
        "Time Zone change has been detected.\nWould you like to auto update it now?",
        "Cancel",
        "Update"
    }
};

MxPopUpAlert::MxPopUpAlert(quint32 xParam, quint32 yParam,
                           POP_UP_ALERT_NAME_e popUpName,
                           POP_UP_ALERT_MODE_e popUpMode,
                           QWidget *parent,
                           POP_UP_ALERT_TYPE_e popUpType,
                           quint32 widthOffset,
                           quint32 heightOffset):
    QWidget(parent), m_xParamParent(xParam), m_yParamparent(yParam), m_width(0),
    m_height(0), m_widthOffset(widthOffset), m_heightOffset(heightOffset)
{
    INIT_OBJ(m_infoMsgLable);
    INIT_OBJ(m_headingLabel);
    INIT_OBJ(m_yesCtrlLable);
    INIT_OBJ(m_noCtrlLable);
    INIT_OBJ(m_backgroundRectangle);
    INIT_OBJ(m_alertImage);
    INIT_OBJ(m_closeImage);

    m_popUpNameQueue.clear();
    m_popUpModeQueue.clear();
    m_popUpTypeQueue.clear();
    m_popUpNameQueue.prepend(popUpName);
    m_popUpModeQueue.prepend(popUpMode);
    m_popUpTypeQueue.prepend(popUpType);

    createDefaultComponent();
    setPopUpMode();
    this->show ();
}

MxPopUpAlert::~MxPopUpAlert ()
{
    deleteComponent();
}

void MxPopUpAlert::createDefaultComponent()
{
    quint16 labelWidth  = 0;
    quint16 labelHeight = 0;
    quint16 finalWidth  = 0;
    quint16 finalHeight = 0;

    m_width     = 0;
    m_height    = 0;

    m_popUpType = m_popUpTypeQueue.at(0);
    m_popUpMode = m_popUpModeQueue.at(0);
    m_popUpName = m_popUpNameQueue.at(0);

    TextLabel::getWidthHeight(TextLabel::getFont(NORMAL_FONT_FAMILY, (SCALE_FONT(SUB_HEADING_FONT_SIZE) + 2)),
                              popUpAlertLableStr[m_popUpName][0],
                              labelWidth,
                              labelHeight);

    if(finalWidth < labelWidth)
    {
        finalWidth = labelWidth;
    }
    finalHeight += labelHeight ;

    TextLabel::getWidthHeight(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE),
                              popUpAlertLableStr[m_popUpName][1],
                              labelWidth,
                              labelHeight);

    if(finalWidth < labelWidth)
    {
        finalWidth = labelWidth;
    }
    finalHeight += labelHeight ;

    if(m_popUpMode != ONLY_MSG)
    {
        TextLabel::getWidthHeight(TextLabel::getFont(NORMAL_FONT_FAMILY, SCALE_FONT(SUFFIX_FONT_SIZE)),
                                  popUpAlertLableStr[m_popUpName][2] + popUpAlertLableStr[m_popUpName][3],
                                  labelWidth,
                                  labelHeight);

        if(finalWidth < labelWidth)
        {
            finalWidth = labelWidth;
        }
        finalHeight += labelHeight ;
    }

    m_height = finalHeight + m_heightOffset + SCALE_HEIGHT(45);
    m_width = finalWidth + m_widthOffset + ((4 * LEFT_MARGIN) + SCALE_WIDTH(48 + 60));

    m_xParam = m_xParamParent - m_width - SCALE_WIDTH(10);
    m_yParam = m_yParamparent - m_height;

    this->setGeometry(m_xParam,
                      m_yParam,
                      m_width,
                      m_height);

    m_backgroundRectangle = new Rectangle(0,
                                          0,
                                          this->width (),
                                          this->height (),
                                          CLICKED_BKG_COLOR,
                                          this,
                                          2,
                                          2,
                                          popUpBackGroundColor[m_popUpType]);
    m_alertImage = new Image(SCALE_WIDTH(18),
                             SCALE_HEIGHT(18),
                             popUpAlertImgSrcPath[m_popUpType],
                             this,
                             START_X_START_Y,
                             0,
                             false,
                             true,
                             true,
                             false);

    m_headingLabel = new TextLabel(SCALE_WIDTH(68),
                                   SCALE_HEIGHT(15),
                                   SCALE_FONT(SUB_HEADING_FONT_SIZE) ,
                                   popUpAlertLableStr[m_popUpName][0],
                                   this,
                                   NORMAL_FONT_COLOR,
                                   NORMAL_FONT_FAMILY,
                                   ALIGN_START_X_START_Y,
                                   0, 0, m_width);
    if(IS_VALID_OBJ(m_headingLabel))
    {
        m_headingLabel->SetBold(true);
    }

    m_infoMsgLable = new TextLabel(SCALE_WIDTH(68),
                                SCALE_HEIGHT(45),
                                NORMAL_FONT_SIZE,
                                popUpAlertLableStr[m_popUpName][1],
                                this,
                                SUFFIX_FONT_COLOR ,
                                NORMAL_FONT_FAMILY,
                                ALIGN_START_X_START_Y,
                                0, 0, m_width);

    m_noCtrlLable = new TextLabel((this->width()-SCALE_WIDTH(10)),
                                     (this->height()-SCALE_HEIGHT(15)),
                                     SCALE_FONT(SUFFIX_FONT_SIZE),
                                     popUpAlertLableStr[m_popUpName][2],
                                     this,
                                     NORMAL_FONT_COLOR ,
                                     NORMAL_FONT_FAMILY,
                                     ALIGN_END_X_END_Y,
                                     0,
                                     true,
                                     m_width,
                                     POP_UP_ALRT_CTRL_NO_HYPLINK);

    if(IS_VALID_OBJ(m_noCtrlLable))
    {
        connect(m_noCtrlLable,
                SIGNAL(sigTextClick(int)),
                this,
                SLOT(slotTextClicked(int)));
        connect(m_noCtrlLable,
                SIGNAL(sigMouseHover(int,bool)),
                this,
                SLOT(slotTextLableHover(int,bool)));
    }

    m_yesCtrlLable = new TextLabel((m_noCtrlLable->x()-SCALE_WIDTH(10)),
                                     (this->height()-SCALE_HEIGHT(15)),
                                     SCALE_FONT(SUFFIX_FONT_SIZE),
                                     popUpAlertLableStr[m_popUpName][3],
                                     this,
                                     HIGHLITED_FONT_COLOR ,
                                     NORMAL_FONT_FAMILY,
                                     ALIGN_END_X_END_Y,
                                     0,
                                     true,
                                     m_width,
                                     POP_UP_ALRT_CTRL_YES);

    if(IS_VALID_OBJ(m_yesCtrlLable))
    {
        connect(m_yesCtrlLable,
                SIGNAL(sigTextClick(int)),
                this,
                SLOT(slotTextClicked(int)));
        connect(m_yesCtrlLable,
                SIGNAL(sigMouseHover(int,bool)),
                this,
                SLOT(slotTextLableHover(int,bool)));
    }

    m_closeImage = new Image((this->width()-SCALE_WIDTH(10)),
                             SCALE_HEIGHT(10),
                             popUpAlertCloseImgSrcPath[m_popUpType],
                             this,
                             END_X_START_Y,
                             POP_UP_ALRT_CTRL_CLOSE,
                             true,
                             false,
                             true,
                             false);
    if(IS_VALID_OBJ(m_closeImage))
    {
        connect (m_closeImage,
                 SIGNAL(sigImageClicked(int)),
                 this,
                 SLOT(slotImageClicked(int)));
    }
}

void MxPopUpAlert::deleteComponent()
{
    DELETE_OBJ (m_backgroundRectangle);
    DELETE_OBJ (m_alertImage);
    DELETE_OBJ (m_headingLabel);
    DELETE_OBJ (m_infoMsgLable);

    if(IS_VALID_OBJ(m_noCtrlLable))
    {
        disconnect(m_noCtrlLable,
                   SIGNAL(sigTextClick(int)),
                   this,
                   SLOT(slotTextClicked(int)));
        disconnect(m_noCtrlLable,
                   SIGNAL(sigMouseHover(int,bool)),
                   this,
                   SLOT(slotTextLableHover(int,bool)));
        DELETE_OBJ (m_noCtrlLable);
    }

    if(IS_VALID_OBJ(m_yesCtrlLable))
    {
        disconnect(m_yesCtrlLable,
                   SIGNAL(sigTextClick(int)),
                   this,
                   SLOT(slotTextClicked(int)));
        disconnect(m_yesCtrlLable,
                   SIGNAL(sigMouseHover(int,bool)),
                   this,
                   SLOT(slotTextLableHover(int,bool)));
        DELETE_OBJ (m_yesCtrlLable);
    }


    if(IS_VALID_OBJ(m_closeImage))
    {
        disconnect (m_closeImage,
                    SIGNAL(sigImageClicked(int)),
                    this,
                    SLOT(slotImageClicked(int)));
        DELETE_OBJ (m_closeImage);
    }
}

void MxPopUpAlert::closePageAction(int index)
{
    m_popUpNameQueue.removeFirst();
    m_popUpModeQueue.removeFirst();
    m_popUpTypeQueue.removeFirst();
    if(m_popUpNameQueue.isEmpty() ||
            m_popUpModeQueue.isEmpty() ||
            m_popUpTypeQueue.isEmpty())
    {
        emit sigCloseAlert(index,true);
    }
    else
    {
        emit sigCloseAlert(index,false);
        changePopUp();
    }
}

void MxPopUpAlert::setPopUpMode()
{
    switch (m_popUpMode) {
    case ONLY_MSG:
    {
        m_isLineOnHoverNoBtnNeeded = false;
        m_isLineOnHoverYesBtnNeeded = false;
        m_isClrChgOnHoverNoBtnNeeded = false;
        m_isClrChgOnHoverYesBtnNeeded = false;
        m_yesCtrlLable->hide();
        m_noCtrlLable->hide();
    }
        break;

    case MSG_WITH_HYPERLINK:
    {
        switch (m_popUpName)
        {
        case AUTO_CNFG_CAM:
        {
            m_isLineOnHoverNoBtnNeeded = false;
            m_isLineOnHoverYesBtnNeeded = false;
            m_isClrChgOnHoverNoBtnNeeded = true;
            m_isClrChgOnHoverYesBtnNeeded = false;
            m_yesCtrlLable->hide();
            m_noCtrlLable->setUnderline(true);
            m_noCtrlLable->changeColor(HIGHLITED_FONT_COLOR);

         }
            break;
        default:
            break;
        }
    }
        break;

    case MSG_WITH_TWO_CTRL:
    {
        switch (m_popUpName)
        {
        case AUTO_TIMEZONE_UPDATE:
        {
            m_isLineOnHoverNoBtnNeeded = false;
            m_isLineOnHoverYesBtnNeeded = true;
            m_isClrChgOnHoverNoBtnNeeded = true;
            m_isClrChgOnHoverYesBtnNeeded = false;
            m_yesCtrlLable->setUnderline(false);
            m_noCtrlLable->setUnderline(false);
            m_yesCtrlLable->changeColor(HIGHLITED_FONT_COLOR);
            m_noCtrlLable->changeColor(SUFFIX_FONT_COLOR);
        }
            break;
        default:
            break;
        }
    }
        break;

    default:
        break;
    }
    m_yesCtrlLable->update();
    m_noCtrlLable->update();
}

void MxPopUpAlert::addPopUp(POP_UP_ALERT_NAME_e popUpName,
                            POP_UP_ALERT_MODE_e popUpMode,
                            POP_UP_ALERT_TYPE_e popUpType)
{
    if(m_popUpNameQueue.contains(popUpName))
    {
        int index = m_popUpNameQueue.indexOf(popUpName);
        m_popUpNameQueue.removeAt(index);
        m_popUpModeQueue.removeAt(index);
        m_popUpTypeQueue.removeAt(index);
    }
    m_popUpNameQueue.prepend(popUpName);
    m_popUpModeQueue.prepend(popUpMode);
    m_popUpTypeQueue.prepend(popUpType);
    changePopUp();
}

void MxPopUpAlert::changePopUp()
{
    deleteComponent();
    createDefaultComponent();
    setPopUpMode();
    this->update();
}

POP_UP_ALERT_NAME_e MxPopUpAlert::getPopUpName()
{
    return m_popUpName;
}

void MxPopUpAlert::updateGeometry(quint32 xParam, quint32 yParam)
{
    m_xParamParent = xParam;
    m_yParamparent = yParam;
    changePopUp();
}

void MxPopUpAlert::slotTextClicked(int index)
{
//    if(index == POP_UP_ALRT_CTRL_NO_HYPLINK)
//    {
        closePageAction(index);
//    }
}

void MxPopUpAlert::slotImageClicked(int index)
{
    switch(index)
    {
    case POP_UP_ALRT_CTRL_CLOSE:
    {
        closePageAction(index);
    }
        break;

    default:
        break;
    }
}

void MxPopUpAlert::slotTextLableHover(int index, bool isHoverIn)
{
    switch (index)
    {
    case POP_UP_ALRT_CTRL_NO_HYPLINK:
    {
        if(IS_VALID_OBJ(m_noCtrlLable))
        {
            if(m_isLineOnHoverNoBtnNeeded)
            {
                m_noCtrlLable->setUnderline(isHoverIn);
            }
            if(m_isClrChgOnHoverNoBtnNeeded)
            {
                m_noCtrlLable->changeColor((isHoverIn == true )? NORMAL_FONT_COLOR : SUFFIX_FONT_COLOR);
            }
            m_noCtrlLable->update();
        }

    }
        break;
    case POP_UP_ALRT_CTRL_YES:
    {
        if(IS_VALID_OBJ(m_yesCtrlLable))
        {
            if(m_isLineOnHoverYesBtnNeeded)
            {
                m_yesCtrlLable->setUnderline(isHoverIn);
            }
            if(m_isClrChgOnHoverYesBtnNeeded)
            {
                m_yesCtrlLable->changeColor((isHoverIn == true )? MOUSE_HOWER_COLOR : HIGHLITED_FONT_COLOR);
            }
            m_yesCtrlLable->update();
        }
    }
        break;
    default:
        break;
    }
}
