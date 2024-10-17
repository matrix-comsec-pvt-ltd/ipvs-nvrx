#include "CopyToCamera.h"
#include <QKeyEvent>
#include <QPainter>

#define COPY_CAMERA_WIDTH   SCALE_WIDTH(536)
#define COPY_CAMERA_HEIGHT  SCALE_HEIGHT(600)

#define WARNING_CAM_SEL     "Note: Action should be performed only if selected event is configured in Camera Settings"

typedef enum
{
    CPY_SEL_CAM_LBL,
    CPY_ALL_CHCKBX_LBL,
    CPY_PREV_LBL,
    CPY_NXT_LBL,
    CPY_OK_LBL,
    CPY_CNCL_LBL,
    MAX_COPY_TO_CAMERA_STRINGS
}COPY_TO_CAMERA_STRINGS_e;

static const QString copyToCameraStrings[MAX_COPY_TO_CAMERA_STRINGS] =
{
    "Select Cameras",
    "All",
    "Previous",
    "Next",
    "OK",
    "Cancel"
};

CopyToCamera::CopyToCamera(QMap<quint8, QString> &cameralist,
                           CAMERA_BIT_MASK_t &cameraSelected,
                           QWidget *parent,
                           QString headingLabel,
                           quint8 indexInPage, bool footNoteNeeded) : KeyBoard(parent)
{
    for(quint8 index = 0; index < MAX_CPY_CAMERA_CONTROL; index++)
    {
        m_elementlist[index] = NULL;
    }

    elementHeadingElide = NULL;
    cameraNameList = &cameralist;
    m_index = indexInPage;
    totalCamera = cameraNameList->size ();
    totalPages = (totalCamera / MAX_CAM_ON_PAGE) + ((totalCamera % MAX_CAM_ON_PAGE) ? 1 : 0);
    currentPageNum = 0;
    nextPageSelected = false;
    camSel = &cameraSelected;
    totalCameraSelected = 0;
    footnoteLabelElide = NULL;

    this->setGeometry (0,0,parent->width (),parent->height ());

    backGround = new Rectangle((SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + SCALE_WIDTH(20) +
                               (SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - COPY_CAMERA_WIDTH)/2) ,
                               ((SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)) +
                               (SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)- COPY_CAMERA_HEIGHT)/2),
                               COPY_CAMERA_WIDTH,
                               COPY_CAMERA_HEIGHT,
                               0,
                               NORMAL_BKG_COLOR,
                               NORMAL_BKG_COLOR,
                               this);

    closeButton = new CloseButtton (backGround->x ()+backGround->width () - SCALE_WIDTH(20),
                                    backGround->y () + SCALE_HEIGHT(20),
                                    this,
                                    CLOSE_BTN_TYPE_1,
                                    CPY_CAM_CLOSE_BTN);
    m_elementlist[CPY_CAM_CLOSE_BTN] = closeButton;
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

    elementHeading = new ElementHeading(backGround->x () + SCALE_WIDTH(23),
                                        backGround->y () + SCALE_HEIGHT(50),
                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        copyToCameraStrings[CPY_SEL_CAM_LBL],
                                        UP_LAYER,
                                        this,
                                        false,
                                        SCALE_WIDTH(20), NORMAL_FONT_SIZE, true);

    allCameraChecklist = new OptionSelectButton(elementHeading->x(),
                                                (elementHeading->y() + BGTILE_HEIGHT),
                                                BGTILE_MEDIUM_SIZE_WIDTH,
                                                BGTILE_HEIGHT,
                                                CHECK_BUTTON_INDEX,
                                                copyToCameraStrings[CPY_ALL_CHCKBX_LBL],
                                                this,
                                                MIDDLE_TABLE_LAYER,
                                                SCALE_WIDTH(40),
                                                MX_OPTION_TEXT_TYPE_SUFFIX,
                                                NORMAL_FONT_SIZE,
                                                CPY_CAM_ALL_CAMERA);
    m_elementlist[CPY_CAM_ALL_CAMERA] = allCameraChecklist;
    connect (allCameraChecklist,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (allCameraChecklist,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

    for(quint8 index = 0 ; index < totalCamera; index++)
    {
        if (true == GET_CAMERA_MASK_BIT(cameraSelected, index))
        {
            isCopyToCameraEnable[index] = true;
            totalCameraSelected++;
        }
        else
        {
            isCopyToCameraEnable[index] = false;
        }
    }

    if(totalCameraSelected == (totalCamera))
    {
        allCameraChecklist->changeState (ON_STATE);
    }

    BGTILE_TYPE_e tileType = MIDDLE_TABLE_LAYER;
    for(quint8 index = 0 ; index < MAX_CAM_ON_PAGE; index++)
    {
        if ((totalCamera <= MAX_CAM_ON_PAGE) && (index == (totalCamera - 1)))
        {
            tileType = BOTTOM_TABLE_LAYER;
        }

        cameraListCheckBox[index] = new OptionSelectButton(elementHeading->x(),
                                                           (allCameraChecklist->y() + allCameraChecklist->height() + (index * BGTILE_HEIGHT)),
                                                           BGTILE_MEDIUM_SIZE_WIDTH,
                                                           BGTILE_HEIGHT,
                                                           CHECK_BUTTON_INDEX,
                                                           "",
                                                           this,
                                                           tileType,
                                                           SCALE_WIDTH(40),
                                                           MX_OPTION_TEXT_TYPE_SUFFIX,
                                                           NORMAL_FONT_SIZE,
                                                           (CPY_CAM_CAMERA_LIST + index));
        cameraListCheckBox[index]->changeState ((isCopyToCameraEnable[index] == true) ? ON_STATE : OFF_STATE);
        m_elementlist[CPY_CAM_CAMERA_LIST + index] = cameraListCheckBox[index];
        connect (cameraListCheckBox[index],
                  SIGNAL(sigUpdateCurrentElement(int)),
                  this,
                  SLOT(slotUpdateCurrentElement(int)));
        connect (cameraListCheckBox[index],
                  SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                  this,
                  SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

        cameraListLabel[index] = new TextLabel(elementHeading->x () + SCALE_WIDTH(85) ,
                                               allCameraChecklist->y () +
                                               allCameraChecklist->height () +
                                               index*BGTILE_HEIGHT + SCALE_HEIGHT(10) ,
                                               NORMAL_FONT_SIZE,
                                               (index < totalCamera) ? cameraNameList->value(index) : "",
                                               this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                               0, 0, SCALE_WIDTH(356));
    }

    prevButton = new ControlButton(PREVIOUS_BUTTON_INDEX,
                                   elementHeading->x (),
                                   cameraListCheckBox[MAX_CAM_ON_PAGE-1]->y () +
                                   cameraListCheckBox[MAX_CAM_ON_PAGE-1]->height (),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   this,
                                   DOWN_LAYER,
                                   SCALE_WIDTH(20),
                                   copyToCameraStrings[CPY_PREV_LBL],
                                   false,
                                   CPY_CAM_PREV_BTN,
                                   false);
    m_elementlist[CPY_CAM_PREV_BTN] = prevButton;
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
                                                          (SCALE_WIDTH(185) + (index*SCALE_WIDTH(40)))),
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
                                                          (CPY_CAM_PAGE_NUM_BTN + index));
        m_elementlist[(CPY_CAM_PAGE_NUM_BTN + index)] = m_PageNumberLabel[index];
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
        m_PageNumberLabel[index]->changeText (QString(" ") + QString("%1").arg (currentPageNum  + 1 + index) + QString(" "));

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
            cameraListCheckBox[index]->setVisible (false);
            cameraListCheckBox[index]->setIsEnabled (false);
            cameraListLabel[index]->setVisible (false);
            cameraListLabel[index]->setIsEnabled (false);
        }
    }

    if(totalPages < MAX_PAGE_NUMBER)
    {
        m_PageNumberLabel[0]->setOffset((m_PageNumberLabel[0]->x() + SCALE_WIDTH(20 * (MAX_PAGE_NUMBER - totalPages))), m_PageNumberLabel[0]->y());

        for(quint8 index = 1; index < MAX_PAGE_NUMBER; index++)
        {
            if (index >= totalPages)
            {
                m_PageNumberLabel[index]->setVisible (false);
                m_PageNumberLabel[index]->setIsEnabled (false);
            }
            else
            {
                m_PageNumberLabel[index]->setOffset((m_PageNumberLabel[index - 1]->x() + m_PageNumberLabel[index - 1]->width()), m_PageNumberLabel[index]->y());
            }
        }
    }

    nextButton = new ControlButton(NEXT_BUTTON_INDEX,
                                   prevButton->x () + prevButton->width () - SCALE_WIDTH(90),
                                   prevButton->y (),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   this,
                                   NO_LAYER,
                                   0,
                                   copyToCameraStrings[CPY_NXT_LBL],
                                   true,
                                   CPY_CAM_NEXT_BTN);
    m_elementlist[CPY_CAM_NEXT_BTN] = nextButton;
    connect (nextButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    connect (nextButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    if(footNoteNeeded)
    {
        elementHeadingElide = new ElementHeading(elementHeading->x (),
                                                 nextButton->y () + SCALE_HEIGHT((totalCamera <= MAX_CAM_ON_PAGE) ? 40 : 50),
                                                 BGTILE_MEDIUM_SIZE_WIDTH,
                                                 SCALE_HEIGHT(50),
                                                 "",
                                                 NO_LAYER,
                                                 this);
        QString fontColor = "#c8c8c8";
        QString fontWidth = "" + QString::number(SCALE_WIDTH(15)) +"px";
        QString styl = "ElidedLabel \
        { \
            color: %1; \
            font-size: %2; \
            font-family: %3; \
        }";

        elementHeadingElide->setStyleSheet(styl.arg(fontColor).arg(fontWidth).arg(NORMAL_FONT_FAMILY));
        footnoteLabelElide = new ElidedLabel(Multilang(WARNING_CAM_SEL), elementHeadingElide);
        footnoteLabelElide->resize(BGTILE_MEDIUM_SIZE_WIDTH, SCALE_HEIGHT(50));
        footnoteLabelElide->raise();
        footnoteLabelElide->show();
    }

    okButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                              elementHeading->x () + elementHeading->width ()/2 - SCALE_WIDTH(70),
                              (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) +
                              (COPY_CAMERA_HEIGHT + SCALE_HEIGHT(60))),
                              copyToCameraStrings[CPY_OK_LBL],
                              this,
                              CPY_CAM_OK_BTN);
    m_elementlist[CPY_CAM_OK_BTN] = okButton;
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
                                  (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) +
                                  (COPY_CAMERA_HEIGHT + SCALE_HEIGHT(60))),
                                  copyToCameraStrings[CPY_CNCL_LBL],
                                  this,
                                  CPY_CAM_CANCEL_BTN);
    m_elementlist[CPY_CAM_CANCEL_BTN] = cancelButton;
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

    currElement = CPY_CAM_ALL_CAMERA;
    m_elementlist[currElement]->forceActiveFocus ();
    this->show ();
}

CopyToCamera::~CopyToCamera ()
{
    delete backGround;

    disconnect (closeButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    disconnect (closeButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete closeButton;

    delete heading;
    delete elementHeading;

    disconnect (allCameraChecklist,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (allCameraChecklist,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
    delete allCameraChecklist;

    for(quint8 index = 0 ; index < MAX_CAM_ON_PAGE; index++)
    {
        disconnect (cameraListCheckBox[index],
                     SIGNAL(sigUpdateCurrentElement(int)),
                     this,
                     SLOT(slotUpdateCurrentElement(int)));
        disconnect (cameraListCheckBox[index],
                     SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                     this,
                     SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
        delete cameraListCheckBox[index];
        delete cameraListLabel[index];
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

    DELETE_OBJ(footnoteLabelElide);
    DELETE_OBJ(elementHeadingElide);
}

void CopyToCamera::paintEvent (QPaintEvent *)
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

void CopyToCamera::takeLeftKeyAction()
{
    do
    {
        currElement = (currElement - 1 + MAX_CPY_CAMERA_CONTROL) % MAX_CPY_CAMERA_CONTROL;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}


void CopyToCamera::takeRightKeyAction()
{
    do
    {
        currElement = (currElement + 1) % MAX_CPY_CAMERA_CONTROL;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}

void CopyToCamera::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    if(m_elementlist[currElement] != NULL)
    {
        m_elementlist[currElement]->forceActiveFocus ();
    }
}

void CopyToCamera::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void CopyToCamera::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void CopyToCamera::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void CopyToCamera::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    currElement = CPY_CAM_CLOSE_BTN;
    m_elementlist[currElement]->forceActiveFocus ();
}

void CopyToCamera ::setAllCheckBoxState (OPTION_STATE_TYPE_e state)
{
    for(quint8 index = 0; index < MAX_CAM_ON_PAGE; index++)
    {
        cameraListCheckBox[index]->changeState (state);
    }
}

void CopyToCamera::showCameraList ()
{
    quint8 fieldsOnPage;

    if(totalCamera < (MAX_CAM_ON_PAGE*(currentPageNum + 1)))
    {
        fieldsOnPage = totalCamera - ((MAX_CAM_ON_PAGE*(currentPageNum)));
    }
    else
    {
        fieldsOnPage = MAX_CAM_ON_PAGE;
    }

    for(quint8 index = 0 ; index < fieldsOnPage; index++)
    {
        cameraListLabel[index]->changeText (cameraNameList->value (index + MAX_CAM_ON_PAGE*currentPageNum));
        cameraListLabel[index]->update ();
        cameraListCheckBox[index]->changeState (isCopyToCameraEnable[index + MAX_CAM_ON_PAGE*currentPageNum] == true ? ON_STATE :OFF_STATE);
        if(cameraListCheckBox[index]->isHidden())
        {
            cameraListCheckBox[index]->setVisible(true);
            cameraListCheckBox[index]->setIsEnabled(true);
        }
    }

    for(quint8 index = fieldsOnPage ; index < MAX_CAM_ON_PAGE; index++)
    {
        cameraListLabel[index]->changeText ("");
        cameraListLabel[index]->update();
        cameraListCheckBox[index]->setVisible(false);
        cameraListCheckBox[index]->setIsEnabled(false);
    }

    if(currentPageNum != 0)
    {
        prevButton->setIsEnabled (true);
        m_elementlist[currElement]->forceActiveFocus ();
    }
    else
    {
        if((currElement == CPY_CAM_NEXT_BTN) || (currElement == CPY_CAM_PREV_BTN))
        {
            currElement = CPY_CAM_NEXT_BTN;
            m_elementlist[currElement]->forceActiveFocus();
        }
        prevButton->setIsEnabled (false);
    }

    if(currentPageNum == (totalPages-1))
    {
        if((currElement == CPY_CAM_NEXT_BTN) || (currElement == CPY_CAM_PREV_BTN))
        {
            currElement = CPY_CAM_PREV_BTN;
            m_elementlist[currElement]->forceActiveFocus ();
        }
        nextButton->setIsEnabled (false);
    }
    else
    {
        nextButton->setIsEnabled (true);
        m_elementlist[currElement]->forceActiveFocus ();
    }

    prevButton->resetGeometry(prevButton->x(), (cameraListCheckBox[0]->y() + SCALE_HEIGHT(fieldsOnPage * 40)), prevButton->width(), prevButton->height());
    nextButton->resetGeometry(nextButton->x(), prevButton->y(), nextButton->width(), nextButton->height());

    for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
    {
        /* Calculate pagelable index (Possible values: 0, 1, 2, 3) */
        quint8 tempIndex = currentPageNum % MAX_PAGE_NUMBER;

        /* Derive first page index from currently visible page numbers (Possible values: 0, 4, 8, 12, ...) */
        quint8 pageOffset = currentPageNum - (currentPageNum % MAX_PAGE_NUMBER);

        if(nextPageSelected == true)
        {
            /* Check if we need to change page number text */
            if((pageOffset + index) < totalPages)
            {
                m_PageNumberLabel[index]->changeText (QString(" ") + QString("%1").arg(pageOffset + 1 + index) + QString(" "));
            }
            else
            {
                m_PageNumberLabel[index]->changeText ("");
            }
        }
        else
        {
           /* Check if we need to change page number text */
           if(((currentPageNum + 1) % MAX_PAGE_NUMBER) == 0)
           {
               m_PageNumberLabel[index]->changeText (QString(" ") + QString("%1").arg ((currentPageNum + 1) - (MAX_PAGE_NUMBER - index) + 1) + QString(" "));
           }
        }

        if((index == tempIndex) && ((pageOffset + index) < totalPages))
        {
            m_PageNumberLabel[index]->setBackGroundColor (CLICKED_BKG_COLOR);
            m_PageNumberLabel[index]->changeTextColor (HIGHLITED_FONT_COLOR);
            m_PageNumberLabel[index]->setBold (true);
            m_PageNumberLabel[index]->forceActiveFocus ();
            currElement = (CPY_CAM_PAGE_NUM_BTN + index);
        }
        else
        {
            m_PageNumberLabel[index]->setBackGroundColor (TRANSPARENTKEY_COLOR);
            m_PageNumberLabel[index]->changeTextColor (NORMAL_FONT_COLOR);
            m_PageNumberLabel[index]->setBold (false);
            m_PageNumberLabel[index]->deSelectControl ();
        }

        m_PageNumberLabel[index]->setOffset(m_PageNumberLabel[index]->x(), (cameraListCheckBox[0]->y() + SCALE_HEIGHT(fieldsOnPage * 40) + SCALE_HEIGHT(5)));
        m_PageNumberLabel[index]->update ();
    }
}

void CopyToCamera::slotButtonClick (int index)
{
    switch(index)
    {
        case CPY_CAM_NEXT_BTN:
        {
            if (currentPageNum != (totalPages-1))
            {
                currentPageNum ++;
            }
            nextPageSelected = true;
            showCameraList();
        }
        break;

        case CPY_CAM_PREV_BTN:
        {
            if(currentPageNum > 0)
            {
                currentPageNum --;
            }
            nextPageSelected = false;
            showCameraList();
        }
        break;

        case CPY_CAM_OK_BTN:
        {
            CAMERA_BIT_MASK_t  cameraSelected = *camSel;
            for(quint8 index = 0; index < totalCamera; index++)
            {
                if(isCopyToCameraEnable[index])
                {
                    SET_CAMERA_MASK_BIT(cameraSelected, index);
                }
                else
                {
                    CLR_CAMERA_MASK_BIT(cameraSelected, index);
                }
            }

            *camSel = cameraSelected;
            emit sigDeleteObject (m_index);
        }
        break;

        case CPY_CAM_CLOSE_BTN:
        case CPY_CAM_CANCEL_BTN:
        {
            emit sigDeleteObject (m_index);
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void CopyToCamera::slotOptionButtonClicked (OPTION_STATE_TYPE_e state, int index)
{
    if(index == CPY_CAM_ALL_CAMERA)
    {
        allCameraChecklist->changeState (state);

        for(quint8 index = 0; index < MAX_CAM_ON_PAGE; index++)
        {
            cameraListCheckBox[index]->changeState (state);
        }
        for (quint8 index1 = 0 ; index1 < totalCamera ; index1++)
        {
            isCopyToCameraEnable[index1] = (state == ON_STATE ? true : false);
        }
    }
    else
    {
        isCopyToCameraEnable[index + (MAX_CAM_ON_PAGE*currentPageNum) - 2] = (state == ON_STATE ? true : false);

        quint8 selectedCameras = 0;
        for(quint8 index = 0; index < totalCamera; index++)
        {
            if(isCopyToCameraEnable[index] == true)
            {
                selectedCameras++;
            }
        }

        allCameraChecklist->changeState ((selectedCameras == totalCamera) ? ON_STATE : OFF_STATE);
    }
}

void CopyToCamera::slotUpdateCurrentElement(int index)
{
    currElement = index;
}

void CopyToCamera::slotPageNumberButtonClick (QString str)
{
    currentPageNum = ((quint8)str.toUInt () - 1);
    showCameraList ();
}

void CopyToCamera::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}
