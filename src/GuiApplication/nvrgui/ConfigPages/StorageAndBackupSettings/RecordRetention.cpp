#include "RecordRetention.h"
#include "ValidationMessage.h"
#include <QPainter>
#include <QKeyEvent>

#define REC_RETETION_WIDTH  SCALE_WIDTH(536)
#define REC_RETETION_HEIGHT SCALE_HEIGHT(620)
#define REC_ELE_MARGIN SCALE_WIDTH(25)

typedef enum{

    REC_CAM_HEADING,
    REC_CAM_HEADING2,
    REC_RET_PREV_STR,
    REC_RET_NEXT_STR,
    REC_RET_OK_STR,
    REC_RET_CAN_STR,

    MAX_REC_RET_STR
}REC_RET_STR_e;


static const QString recordRetentionStrings[MAX_REC_RET_STR] ={
    "Camera Name",
    "Retain Data (in days)",
    "Previous",
    "Next",
    "OK",
    "Cancel"
};

RecordRetention::RecordRetention(QStringList &camlist,
                                 QStringList &retnDays,
                                 QWidget *parent,
                                 QString headingLabel,
                                 quint8 indexInPage) :
    KeyBoard(parent)
{
    this->setGeometry (0,0,parent->width (),parent->height ());

    m_index = indexInPage;
    recList = &retnDays ;
    totalCamera = camlist.length ();
    camList = &camlist;
    currentPageNum = 0;
    totalPages = (totalCamera < MAX_CAM_ON_PAGE) ? 1 : (totalCamera/MAX_CAM_ON_PAGE);
    nextPageSelected = false;

    backGround = new Rectangle(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + SCALE_WIDTH(20) +
                               ( SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - REC_RETETION_WIDTH)/2 ,
                               (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)
                               + (SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)- REC_RETETION_HEIGHT)/2),
                               REC_RETETION_WIDTH,
                               REC_RETETION_HEIGHT,
                               0,
                               NORMAL_BKG_COLOR,
                               NORMAL_BKG_COLOR,
                               this);

    closeButton = new CloseButtton (backGround->x ()+ backGround->width () - SCALE_WIDTH(20),
                                    backGround->y () + SCALE_HEIGHT(20),
                                    this,
                                    CLOSE_BTN_TYPE_1,
                                    REC_RETN_CLS_BTN);

    m_elementlist[REC_RETN_CLS_BTN] = closeButton;

    connect (closeButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (closeButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    heading = new Heading(backGround->x () + backGround->width ()/2,
                          backGround->y () + SCALE_HEIGHT(20),
                          headingLabel,
                          this,
                          HEADING_TYPE_2);

    elementHeading = new ElementHeading(backGround->x () +
                                        (backGround->width () - BGTILE_MEDIUM_SIZE_WIDTH)/2,
                                        backGround->y() +
                                        (totalCamera <= MAX_CAM_ON_PAGE
                                         ? (REC_RETETION_HEIGHT - (MAX_CAM_ON_PAGE + 2)*BGTILE_HEIGHT) /2
                                         : (REC_RETETION_HEIGHT - (MAX_CAM_ON_PAGE + 3)*BGTILE_HEIGHT) /2),
                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        recordRetentionStrings[REC_CAM_HEADING],
                                        TOP_LAYER,
                                        this,
                                        false,
                                        REC_ELE_MARGIN, NORMAL_FONT_SIZE);


    elementHeading1 = new ElementHeading(elementHeading->x ()+ BGTILE_MEDIUM_SIZE_WIDTH/2,
                                         elementHeading->y(),
                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT + SCALE_HEIGHT(10),
                                         recordRetentionStrings[REC_CAM_HEADING2],
                                         NO_LAYER,
                                         this,
                                         false,
                                         REC_ELE_MARGIN, NORMAL_FONT_SIZE);

    BGTILE_TYPE_e tileType = MIDDLE_TABLE_LAYER;
    for(quint8 index = 0; index < MAX_CAM_ON_PAGE ; index++)
    {
        if ((totalCamera <= MAX_CAM_ON_PAGE) && (index == (totalCamera - 1)))
        {
            tileType = BOTTOM_TABLE_LAYER;
        }

        cameraTile[index] = new BgTile(elementHeading->x (),
                                       elementHeading->y () + elementHeading->height () + index*BGTILE_HEIGHT,
                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                       BGTILE_HEIGHT,
                                       tileType,
                                       this);

        cameraNames[index] = new TextLabel(elementHeading->x () + SCALE_WIDTH(35) ,
                                           cameraTile[index]->y () + SCALE_HEIGHT(10),
                                           NORMAL_FONT_SIZE,
                                           (index < totalCamera) ? camList->at(index) : "",
                                           this);

        retainDaysTextboxParam[index] = new TextboxParam();
        retainDaysTextboxParam[index]->suffixStr = "(1-999)";
        retainDaysTextboxParam[index]->isNumEntry = true;
        retainDaysTextboxParam[index]->minNumValue = 1;
        retainDaysTextboxParam[index]->maxNumValue = 999;
        retainDaysTextboxParam[index]->validation = QRegExp(QString("[0-9]"));
        retainDaysTextboxParam[index]->maxChar = 3;
        retainDaysTextboxParam[index]->isTotalBlankStrAllow = false;

        retainDaysTextbox[index] = new TextBox (cameraTile[index]->x () + BGTILE_MEDIUM_SIZE_WIDTH/2 + SCALE_WIDTH(40),
                                                cameraTile[index]->y (),
                                                BGTILE_MEDIUM_SIZE_WIDTH,
                                                BGTILE_HEIGHT,
                                                REC_RETN_TXTBX + index,
                                                TEXTBOX_SMALL,
                                                this,
                                                retainDaysTextboxParam[index],
                                                NO_LAYER);

        retainDaysTextbox[index]->setInputText (retnDays.at (index));

        m_elementlist[REC_RETN_TXTBX + index] =  retainDaysTextbox[index];

        connect (retainDaysTextbox[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        connect (retainDaysTextbox[index],
                 SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                 this,
                 SLOT(slotLoadInfoPage(int,INFO_MSG_TYPE_e)));

    }

    prevButton = new ControlButton(PREVIOUS_BUTTON_INDEX,
                                   elementHeading->x (),
                                   cameraTile[7]->y () +
                                   cameraTile[7]->height (),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   this,
                                   DOWN_LAYER,
                                   SCALE_HEIGHT(20),
                                   recordRetentionStrings[REC_RET_PREV_STR],
                                   false,
                                   REC_PREV_BTN,
                                   false);

    m_elementlist[REC_PREV_BTN] = prevButton;

    connect (prevButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (prevButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
    {
        m_PageNumberLabel[index] = new TextWithBackground((prevButton->x () +
                                                           (SCALE_WIDTH(170) + (index*40))),
                                                          prevButton->y () + SCALE_HEIGHT(5),
                                                          NORMAL_FONT_SIZE,
                                                          "",
                                                          this,
                                                          NORMAL_FONT_COLOR,
                                                          NORMAL_FONT_FAMILY,
                                                          ALIGN_START_X_START_Y,
                                                          0,
                                                          false,
                                                          TRANSPARENTKEY_COLOR,
                                                          true,
                                                          (REC_PAGE_NUMBER_BTN + index));

        m_elementlist[(REC_PAGE_NUMBER_BTN + index)] = m_PageNumberLabel[index];

        connect(m_PageNumberLabel[index],
                SIGNAL(sigMousePressClick(QString)),
                this,
                SLOT(slotPageNumberButtonClick(QString)));

        connect(m_PageNumberLabel[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

    }

    for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
    {
        m_PageNumberLabel[index]->changeText (QString(" ") + QString("%1").arg (currentPageNum  + 1 + index) + QString(" ") );

        if((index + currentPageNum) == currentPageNum)
        {
            m_PageNumberLabel[index]->setBackGroundColor (CLICKED_BKG_COLOR);
            m_PageNumberLabel[index]->changeTextColor (HIGHLITED_FONT_COLOR);
            m_PageNumberLabel[index]->setBold (true);
            m_PageNumberLabel[index]->forceActiveFocus ();
        }
        else
        {
            m_PageNumberLabel[index]->setBackGroundColor (TRANSPARENTKEY_COLOR);
            m_PageNumberLabel[index]->changeTextColor (NORMAL_FONT_COLOR);
            m_PageNumberLabel[index]->setBold (false);
            m_PageNumberLabel[index]->deSelectControl ();
        }

        m_PageNumberLabel[index]->update ();
    }

    if (totalCamera < MAX_CAM_ON_PAGE)
    {
        for(quint8 index = totalCamera; index < MAX_CAM_ON_PAGE; index++)
        {
            cameraTile[index]->setVisible (false);
            cameraTile[index]->setEnabled (false);
            cameraNames[index]->setVisible (false);
            cameraNames[index]->setIsEnabled (false);
            retainDaysTextbox[index]->setVisible (false);
            retainDaysTextbox[index]->setIsEnabled (false);
        }
    }

    if(totalPages < MAX_PAGE_NUMBER)
    {
        for(quint8 index = totalPages; index < MAX_PAGE_NUMBER; index++)
        {
            m_PageNumberLabel[index]->setVisible (false);
            m_PageNumberLabel[index]->setIsEnabled (false);
        }
    }

    nextButton = new ControlButton(NEXT_BUTTON_INDEX,
                                   prevButton->x () +
                                   prevButton->width () - SCALE_WIDTH(90),
                                   cameraTile[7]->y () +
                                   cameraTile[7]->height (),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   this,
                                   NO_LAYER,
                                   0,
                                   recordRetentionStrings[REC_RET_NEXT_STR],
                                   true,
                                   REC_NEXT_BTN);

    m_elementlist[REC_NEXT_BTN] = nextButton;

    connect (nextButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (nextButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    okButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                              elementHeading->x () + elementHeading->width ()/2 - SCALE_WIDTH(70),
                              backGround->y () +
                              REC_RETETION_HEIGHT - SCALE_HEIGHT(50),
                              recordRetentionStrings[REC_RET_OK_STR],
                              this,
                              REC_OK_BTN);

    m_elementlist[REC_OK_BTN] = okButton;

    connect (okButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (okButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    cancelButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                  elementHeading->x () + elementHeading->width ()/2 + SCALE_WIDTH(70),
                                  backGround->y () +
                                  REC_RETETION_HEIGHT - SCALE_HEIGHT(50),
                                  recordRetentionStrings[REC_RET_CAN_STR],
                                  this,
                                  REC_CANL_BTN);

    m_elementlist[REC_CANL_BTN] = cancelButton;

    connect (cancelButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (cancelButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    if(totalCamera <= MAX_CAM_ON_PAGE)
    {
        prevButton->setVisible (false);
        prevButton->setIsEnabled (false);
        nextButton->setVisible (false);
        nextButton->setIsEnabled (false);

        for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
        {
            m_PageNumberLabel[index]->setVisible (false);
            m_PageNumberLabel[index]->setIsEnabled (false);
        }
    }

    infoPage = new InfoPage (0, 0,
                             SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH),
                             SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT),
                             INFO_CONFIG_PAGE,
                             parentWidget ());

    connect (infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageBtnclick(int)));

    currElement = REC_RETN_TXTBX ;
    m_elementlist[currElement]->forceActiveFocus ();

    this->show ();
}

RecordRetention::~RecordRetention()
{
    delete backGround;
    delete heading;
    delete elementHeading;
    delete elementHeading1;

    disconnect (closeButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));

    disconnect (closeButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete closeButton;

    for(quint8 index = 0; index < MAX_CAM_ON_PAGE ; index++)
    {
        delete cameraTile[index];
        delete cameraNames[index];

        disconnect (retainDaysTextbox[index],
                    SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                    this,
                    SLOT(slotLoadInfoPage(int,INFO_MSG_TYPE_e)));
        disconnect (retainDaysTextbox[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete retainDaysTextbox[index];
        delete retainDaysTextboxParam[index];

    }

    for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
    {
        disconnect(m_PageNumberLabel[index],
                   SIGNAL(sigMousePressClick(QString)),
                   this,
                   SLOT(slotPageNumberButtonClick(QString)));

        disconnect(m_PageNumberLabel[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        delete m_PageNumberLabel[index];
    }

    disconnect (prevButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));

    disconnect (prevButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete prevButton;

    disconnect (nextButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));

    disconnect (nextButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete nextButton;

    disconnect (okButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    disconnect (okButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete okButton;

    disconnect (cancelButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    disconnect (cancelButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete cancelButton;

    disconnect (infoPage,
                SIGNAL(sigInfoPageCnfgBtnClick(int)),
                this,
                SLOT(slotInfoPageBtnclick(int)));
    delete infoPage;

}

void RecordRetention::paintEvent (QPaintEvent *)
{
    QPainter painter(this);
    QColor color;

    color.setAlpha (0);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);

    painter.drawRoundedRect (QRect(0,
                                   0,
                                   SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT)),
                             SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));

    color.setAlpha (150);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);

    painter.drawRoundedRect (QRect(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) -SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT),
                                   SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)),
                             SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));
}

void RecordRetention::takeLeftKeyAction()
{
    do
    {
        currElement = (currElement - 1 + MAX_REC_RETN_CTRL) % MAX_REC_RETN_CTRL;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}


void RecordRetention::takeRightKeyAction()
{
    do
    {
        currElement = (currElement + 1) % MAX_REC_RETN_CTRL;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}

void RecordRetention::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    m_elementlist[currElement]->forceActiveFocus();
}

void RecordRetention::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void RecordRetention::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    currElement = REC_RETN_CLS_BTN;
    m_elementlist[currElement]->forceActiveFocus ();
}

void RecordRetention::showRecords ()
{
    quint8 fieldsOnPage;

    if( totalCamera < (MAX_CAM_ON_PAGE*(currentPageNum + 1)))
    {
        fieldsOnPage = totalCamera - ((MAX_CAM_ON_PAGE*(currentPageNum)) );
    }
    else
    {
        fieldsOnPage = MAX_CAM_ON_PAGE;
    }

    for(quint8 index = 0 ; index < fieldsOnPage; index++)
    {
        cameraNames[index]->changeText (camList->at (index + MAX_CAM_ON_PAGE*currentPageNum ));
        cameraNames[index]->update ();
        retainDaysTextbox[index]->setInputText (recList->at (index + MAX_CAM_ON_PAGE*currentPageNum ));
        if(retainDaysTextbox[index]->isHidden ())
            retainDaysTextbox[index]->setVisible (true);
    }

    for(quint8 index = fieldsOnPage ; index < MAX_CAM_ON_PAGE; index++)
    {
        cameraNames[index]->changeText ("");
        cameraNames[index]->update ();
        retainDaysTextbox[index]->setVisible (false);
    }

    if(currentPageNum != 0)
    {
        prevButton->setIsEnabled (true);
        m_elementlist[currElement]->forceActiveFocus ();
    }
    else
    {
        if( (currElement == REC_NEXT_BTN) ||
                (currElement == REC_PREV_BTN))
        {
            currElement = REC_NEXT_BTN;
            m_elementlist[currElement]->forceActiveFocus ();
        }
        prevButton->setIsEnabled (false);
    }

    if(currentPageNum == ((totalCamera/MAX_CAM_ON_PAGE)-1))
    {
        if((currElement == REC_NEXT_BTN) ||
                (currElement == REC_PREV_BTN))
        {
            currElement = REC_PREV_BTN;
            m_elementlist[currElement]->forceActiveFocus ();
        }
        nextButton->setIsEnabled (false);
    }
    else
    {
        nextButton->setIsEnabled (true);
        m_elementlist[currElement]->forceActiveFocus ();
    }

    for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
    {
        quint8 tempIndex = MAX_PAGE_NUMBER;
        quint8 tempPageNum = currentPageNum - (currentPageNum % MAX_PAGE_NUMBER);

        if(nextPageSelected == true)
        {
            if(((totalPages - currentPageNum) % MAX_PAGE_NUMBER) != 0)
            {
                if(totalPages < MAX_PAGE_NUMBER)
                {
                    tempIndex = (totalPages - ((totalPages - currentPageNum) % MAX_PAGE_NUMBER));
                }
                else
                {
                    tempIndex = (MAX_PAGE_NUMBER - ((totalPages - currentPageNum) % MAX_PAGE_NUMBER)) ;
                }
            }
            else
            {
                tempIndex = (currentPageNum%MAX_PAGE_NUMBER);
                if((currentPageNum%MAX_PAGE_NUMBER) == 0)
                {
                    tempIndex = 0;
                }

                if((tempPageNum + index) < totalPages)
                {
                    m_PageNumberLabel[index]->changeText (QString(" ") +
                                                          QString("%1").arg (tempPageNum  + 1 + index) +
                                                          QString(" ") );
                }
                else
                {
                    m_PageNumberLabel[index]->changeText ("");
                }
            }
        }
        else
        {
            tempIndex = (currentPageNum%MAX_PAGE_NUMBER);
            if((totalPages%MAX_PAGE_NUMBER) == 0)
            {
                if(((totalPages - currentPageNum - 1) % MAX_PAGE_NUMBER) != 0)
                {
                    if(totalPages < MAX_PAGE_NUMBER)
                    {
                        tempIndex = (totalPages - ((totalPages - currentPageNum) % MAX_PAGE_NUMBER));
                    }
                    else
                    {
                        tempIndex = (MAX_PAGE_NUMBER - ((totalPages - currentPageNum) % MAX_PAGE_NUMBER));

                        if((tempPageNum + index) < totalPages)
                        {
                            m_PageNumberLabel[index]->changeText (QString(" ") +
                                                                  QString("%1").arg ((tempPageNum + 1 + index)) +
                                                                  QString(" ") );
                        }
                        else
                        {
                            m_PageNumberLabel[index]->changeText ("");
                        }
                    }

                    if(((currentPageNum % MAX_PAGE_NUMBER) == 0)
                            && (tempIndex == MAX_PAGE_NUMBER))
                    {
                        tempIndex = 0;
                    }
                }
                else
                {
                    tempIndex = (currentPageNum % MAX_PAGE_NUMBER);

                    m_PageNumberLabel[index]->changeText (QString(" ") +
                                                          QString("%1").arg ((currentPageNum + 1) - (MAX_PAGE_NUMBER - index) + 1) +
                                                          QString(" ") );
                }
            }
            else
            {
                if(((totalPages - currentPageNum) % MAX_PAGE_NUMBER) != 0)
                {
                    if(totalPages < MAX_PAGE_NUMBER)
                    {
                        tempIndex = (totalPages - ((totalPages - currentPageNum) % MAX_PAGE_NUMBER));
                    }
                    else
                    {
                        tempIndex = (currentPageNum % MAX_PAGE_NUMBER);

                        if((tempPageNum + index) < totalPages)
                        {
                            m_PageNumberLabel[index]->changeText (QString(" ") +
                                                                  QString("%1").arg ((tempPageNum + 1 + index)) +
                                                                  QString(" ") );
                        }
                        else
                        {
                            m_PageNumberLabel[index]->changeText ("");
                        }
                    }

                    if(((currentPageNum % MAX_PAGE_NUMBER) == 0)
                            && (tempIndex == MAX_PAGE_NUMBER))
                    {
                        tempIndex = 0;
                    }
                }
                else
                {
                    tempIndex = (currentPageNum % MAX_PAGE_NUMBER);
                    if((tempPageNum + index) < totalPages)
                    {
                        m_PageNumberLabel[index]->changeText (QString(" ") +
                                                              QString("%1").arg ((tempPageNum + 1 + index)) +
                                                              QString(" ") );
                    }
                    else
                    {
                        m_PageNumberLabel[index]->changeText ("");
                    }
                }
            }
        }

        if((index == tempIndex) && ((tempPageNum + index) < totalPages))
        {
            m_PageNumberLabel[index]->setBackGroundColor (CLICKED_BKG_COLOR);
            m_PageNumberLabel[index]->changeTextColor (HIGHLITED_FONT_COLOR);
            m_PageNumberLabel[index]->setBold (true);
            m_PageNumberLabel[index]->forceActiveFocus ();
            currElement = (REC_PAGE_NUMBER_BTN + index);
        }
        else
        {
            m_PageNumberLabel[index]->setBackGroundColor (TRANSPARENTKEY_COLOR);
            m_PageNumberLabel[index]->changeTextColor (NORMAL_FONT_COLOR);
            m_PageNumberLabel[index]->setBold (false);
            m_PageNumberLabel[index]->deSelectControl ();
        }
        m_PageNumberLabel[index]->update ();
    }
}

bool RecordRetention::saveRecords ()
{
    bool ret=true;
    for(quint8 index = 0 ; index < MAX_CAM_ON_PAGE; index++)
    {
        if(IS_VALID_OBJ(retainDaysTextbox[index])
                && (retainDaysTextbox[index]->doneKeyValidation()))
        {
            recList->replace (index + (MAX_CAM_ON_PAGE*currentPageNum),
                              retainDaysTextbox[index]->getInputText ());
        }
        else
        {
            ret = false;
            break;
        }
    }

    return ret;
}

void RecordRetention::slotButtonClick (int index)
{
    switch(index)
    {
    case REC_OK_BTN:
    {        
        if(saveRecords())
            emit sigDeleteObject (m_index);
    }
        break;

    case REC_NEXT_BTN:
    {
        if(saveRecords())
        {

            if (currentPageNum != ((totalCamera/MAX_CAM_ON_PAGE)-1))
            {
                currentPageNum ++;
            }

            nextPageSelected = true;
            showRecords ();
        }
    }
        break;

    case REC_PREV_BTN:
    {
        if(saveRecords())
        {

            if(currentPageNum > 0)
            {
                currentPageNum --;
            }

            nextPageSelected = false;
            showRecords ();
        }
    }
        break;

    default:
        emit sigDeleteObject (m_index);
        break;
    }
}

void RecordRetention::slotUpdateCurrentElement(int index)
{
    currElement = index;
}

void RecordRetention::slotLoadInfoPage (int, INFO_MSG_TYPE_e msgtype)
{
    if(msgtype == INFO_MSG_ERROR)
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_VALUE_DEFI_RANGE));
    }
}

void RecordRetention::slotInfoPageBtnclick (int)
{
    m_elementlist[currElement]->forceActiveFocus ();
}

void RecordRetention::slotPageNumberButtonClick (QString str)
{
    currentPageNum = ((quint8)str.toUInt () - 1);
    showRecords ();
}

void RecordRetention::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void RecordRetention::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void RecordRetention::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}
