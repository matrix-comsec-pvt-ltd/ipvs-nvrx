#include "MxPopUpAlert.h"

QString popUpAlertImgSrcPath[] = {IMAGE_PATH"/PopUpAlert/Information.png"};

QString popUpAlertCloseImgSrcPath[] = {IMAGE_PATH"/PopUpAlert/Close_Blue/"};

QString popUpBackGroundColor[] = {"#528dc9"};

QString popUpAlertLableStr[MAX_POP_UP_ALERT_NAME][4][10] = {
    //Test Completed
    {
        {"Test Completed Successfully"},
        {"Report generated and saved on FTP Drive."},
    },
    //Test Failed
    {
        {"Test Failed"},
        {"LED test failed",
         "Audio IN test failed",
         "Audio OUT test failed",
         "HDMI Audio test failed",
         "HDMI Video test failed",
         "RTC test failed",
         "Ethernet test failed",
         "Sensor IN and Alarm OUT test failed",
         "HDD test failed",
         "USB test failed",},
    }
};

MxPopUpAlert::MxPopUpAlert(quint32 xParam, quint32 yParam,
                           POP_UP_ALERT_NAME_e popUpName,
                           POP_UP_ALERT_MODE_e popUpMode,
                           QWidget *parent, quint8 advDetailIndex,
                           POP_UP_ALERT_TYPE_e popUpType,
                           quint32 widthOffset,
                           quint32 heightOffset):
    QWidget(parent)
{
    INIT_OBJ(m_infoMsgLable);
    INIT_OBJ(m_headingLabel);
    INIT_OBJ(m_yesCtrlLable);
    INIT_OBJ(m_noCtrlLable);
    INIT_OBJ(m_backgroundRectangle);
    INIT_OBJ(m_alertImage);
    INIT_OBJ(m_closeImage);

    m_width     = 0;
    m_height    = 0;
    m_xParamParent = xParam;
    m_yParamparent = yParam;
    m_widthOffset  = widthOffset;
    m_heightOffset = heightOffset;
    m_advDetailIndex = advDetailIndex;

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
    QString headingFontColor = NORMAL_FONT_COLOR;
    QString infoFontColor = SUFFIX_FONT_COLOR;

    m_width     = 0;
    m_height    = 0;

    m_popUpType = m_popUpTypeQueue.at(0);
    m_popUpMode = m_popUpModeQueue.at(0);
    m_popUpName = m_popUpNameQueue.at(0);

    if(m_popUpName == TEST_FAIL)
    {
        headingFontColor = RED_COLOR;
        infoFontColor = RED_COLOR;
    }


    TextLabel::getWidthHeight(TextLabel::getFont(NORMAL_FONT_FAMILY, (NORMAL_FONT_SIZE)),
                              popUpAlertLableStr[m_popUpName][0][0],
                              labelWidth,
                              labelHeight);

    if(finalWidth < labelWidth)
    {
        finalWidth = labelWidth;
    }
    finalHeight += labelHeight ;

    TextLabel::getWidthHeight(TextLabel::getFont(NORMAL_FONT_FAMILY, SMALL_SUFFIX_FONT_SIZE),
                              popUpAlertLableStr[m_popUpName][1][m_advDetailIndex],
                              labelWidth,
                              labelHeight);

    if(finalWidth < labelWidth)
    {
        finalWidth = labelWidth;
    }
    finalHeight += labelHeight ;

    if(m_popUpMode != ONLY_MSG)
    {
        TextLabel::getWidthHeight(TextLabel::getFont(NORMAL_FONT_FAMILY, SUFFIX_FONT_SIZE),
                                  popUpAlertLableStr[m_popUpName][2][0] + popUpAlertLableStr[m_popUpName][3][0],
                                  labelWidth,
                                  labelHeight);

        if(finalWidth < labelWidth)
        {
            finalWidth = labelWidth;
        }
        finalHeight += labelHeight ;
    }

    m_height = finalHeight + m_heightOffset + 35;
    m_width = finalWidth + m_widthOffset + ((4 * LEFT_MARGIN) + 48);

    m_xParam = m_xParamParent - m_width - 10;
    m_yParam = m_yParamparent - m_height - 10;

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
    m_alertImage = new Image(13,
                             13,
                             popUpAlertImgSrcPath[m_popUpType],
                             this,
                             START_X_START_Y,
                             0,
                             false,
                             true,
                             true,
                             false);

    m_headingLabel = new TextLabel(63,
                                   12,
                                   NORMAL_FONT_SIZE ,
                                   popUpAlertLableStr[m_popUpName][0][0],
                                   this,
                                   headingFontColor,
                                   NORMAL_FONT_FAMILY,
                                   ALIGN_START_X_START_Y);
    if(IS_VALID_OBJ(m_headingLabel))
    {
        m_headingLabel->SetBold(true);
    }

    m_infoMsgLable = new TextLabel(63,
                                40,
                                SMALL_SUFFIX_FONT_SIZE,
                                popUpAlertLableStr[m_popUpName][1][m_advDetailIndex],
                                this,
                                infoFontColor ,
                                NORMAL_FONT_FAMILY,
                                ALIGN_START_X_START_Y);

    m_noCtrlLable = new TextLabel((this->width()-10),
                                     (this->height()-15),
                                     SUFFIX_FONT_SIZE,
                                     popUpAlertLableStr[m_popUpName][2][0],
                                     this,
                                     NORMAL_FONT_COLOR ,
                                     NORMAL_FONT_FAMILY,
                                     ALIGN_END_X_END_Y,
                                     0,
                                     true,
                                     0,
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

    m_yesCtrlLable = new TextLabel((m_noCtrlLable->x()-10),
                                     (this->height()-15),
                                     SUFFIX_FONT_SIZE,
                                     popUpAlertLableStr[m_popUpName][3][0],
                                     this,
                                     HIGHLITED_FONT_COLOR ,
                                     NORMAL_FONT_FAMILY,
                                     ALIGN_END_X_END_Y,
                                     0,
                                     true,
                                     0,
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

    m_closeImage = new Image((this->width()-10),
                             10,
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

        m_isLineOnHoverNoBtnNeeded = false;
        m_isLineOnHoverYesBtnNeeded = false;
        m_isClrChgOnHoverNoBtnNeeded = true;
        m_isClrChgOnHoverYesBtnNeeded = false;
        m_yesCtrlLable->hide();
        m_noCtrlLable->setUnderline(true);
        m_noCtrlLable->changeColor(HIGHLITED_FONT_COLOR);
    }
        break;

    case MSG_WITH_TWO_CTRL:
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
    m_yesCtrlLable->repaint();
    m_noCtrlLable->repaint();
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
    this->repaint();
}

POP_UP_ALERT_NAME_e MxPopUpAlert::getPopUpName()
{
        return m_popUpName;
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
            m_noCtrlLable->repaint();
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
            m_yesCtrlLable->repaint();
        }
    }
        break;
    default:
        break;
    }
}
