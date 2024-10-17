#include "CameraOSDSettings.h"
#include <QPainter>
#include <QKeyEvent>
#include "ValidationMessage.h"

#define CAMERAOSDSETTINGS_LEFT_MARGIN   SCALE_WIDTH(20)
#define CAMERAOSDSETTINGS_WIDTH         (BGTILE_MEDIUM_SIZE_WIDTH + (2 * CAMERAOSDSETTINGS_LEFT_MARGIN))
#define CAMERAOSDSETTINGS_HEIGHT        SCALE_HEIGHT(430)

static const QString controlLabelStrings[] = {"Date Time Overlay",
                                              "Date Time Position",
                                              "Text Overlay",
                                              "Overlay Number",
                                              "Text",
                                              "Text Position",
                                              "Camera Name Position",
                                              "Camera Status Position"};

static const QString cameraOSDList[] = {"None",
                                        "Top Left",
                                        "Top Right",
                                        "Bottom Left",
                                        "Bottom Right"};

CameraOSDSettings::CameraOSDSettings(OSD_SETTINGS_t * osdSettings,
                                     quint8 indexInPage, quint8 textOverlayMax,
                                     QWidget *parent)
    : KeyBoard(parent)
{
    this->setGeometry(0, 0, parent->width(), parent->height());

    m_osdSettings = osdSettings;
    m_indexInPage = indexInPage;
    m_textOverlayIndex = 0;
    m_textOverlayMax = textOverlayMax;

    if (m_textOverlayMax < TEXT_OVERLAY_MIN)
    {
        m_textOverlayMax = TEXT_OVERLAY_MIN;
    }
    else if (m_textOverlayMax > TEXT_OVERLAY_MAX)
    {
        m_textOverlayMax = TEXT_OVERLAY_MAX;
    }

    //Storing the TextOverlayList into QMap
    m_textOverlayList.clear();
    for (quint8 index = 0; index < m_textOverlayMax ; index++)
    {
        m_textOverlayList.insert((index), INT_TO_QSTRING(index + TEXT_OVERLAY_MIN));
    }

    m_cameraOSDPositionList.clear();
    for(quint8 index = OSD_NONE; index < MAX_OSD_POSITION; index++)
    {
        m_cameraOSDPositionList.insert (index,cameraOSDList[index]);
    }

    m_graphicOSDPositionList.clear();
    for(quint8 index = TOP_LEFT; index < MAX_OSD_POSITION; index++)
    {
        m_graphicOSDPositionList.insert ((index-1),cameraOSDList[index]);
    }

    m_background = new Rectangle((SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + ((SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - CAMERAOSDSETTINGS_WIDTH) / 2)),
                                 (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) + ((SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) - CAMERAOSDSETTINGS_HEIGHT) / 2)),
                                 CAMERAOSDSETTINGS_WIDTH,
                                 CAMERAOSDSETTINGS_HEIGHT,
                                 NORMAL_BKG_COLOR,
                                 this);

    m_closeButton = new CloseButtton((m_background->x() + m_background->width() - SCALE_WIDTH(20)),
                                     (m_background->y() + SCALE_HEIGHT(20)),
                                     this,
                                     CLOSE_BTN_TYPE_1,
                                     CAMERAOSDSETTINGS_CLOSE_BUTTON);
    m_elementList[CAMERAOSDSETTINGS_CLOSE_BUTTON] = m_closeButton;
    connect(m_closeButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCloseButtonClicked(int)));
    connect(m_closeButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_pageHeading = new Heading((m_background->x() + m_background->width() / 2),
                                (m_background->y() + SCALE_HEIGHT(20)),
                                "OSD Settings",
                                this, HEADING_TYPE_2);

    quint16 topMargin = (((CAMERAOSDSETTINGS_HEIGHT ) - (BGTILE_HEIGHT * CAMERAOSDSETTINGS_OKBUTTON)) / 2) + m_background->y();

    m_dateTimeOverlayCheckbox = new OptionSelectButton((CAMERAOSDSETTINGS_LEFT_MARGIN + m_background->x()),
                                                       (topMargin + (BGTILE_HEIGHT * CAMERAOSDSETTINGS_DATETIMEOVERLAY_CHECKBOX)),
                                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                                       BGTILE_HEIGHT,
                                                       CHECK_BUTTON_INDEX,
                                                       this,
                                                       COMMON_LAYER,
                                                       controlLabelStrings[CAMERAOSDSETTINGS_DATETIMEOVERLAY_CHECKBOX],
                                                       "", -1,
                                                       CAMERAOSDSETTINGS_DATETIMEOVERLAY_CHECKBOX);
    m_dateTimeOverlayCheckbox->changeState(m_osdSettings->dateTimeOverlay);
    m_elementList[CAMERAOSDSETTINGS_DATETIMEOVERLAY_CHECKBOX] = m_dateTimeOverlayCheckbox;
    connect(m_dateTimeOverlayCheckbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_dateTimeOverlayCheckbox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptionButtonSelected(OPTION_STATE_TYPE_e,int)));

    m_dateTimePositionDropDownbox =  new DropDown((CAMERAOSDSETTINGS_LEFT_MARGIN + m_background->x()),
                                             (topMargin + (BGTILE_HEIGHT * CAMERAOSDSETTINGS_DATETIMEPOSITION_SPINBOX)),
                                             BGTILE_MEDIUM_SIZE_WIDTH,
                                             BGTILE_HEIGHT,
                                             CAMERAOSDSETTINGS_DATETIMEPOSITION_SPINBOX,
                                             DROPDOWNBOX_SIZE_200,
                                             controlLabelStrings[CAMERAOSDSETTINGS_DATETIMEPOSITION_SPINBOX],
                                             m_graphicOSDPositionList,
                                             this, "", true, 0,
                                             COMMON_LAYER,
                                             (m_osdSettings->dateTimeOverlay == ON_STATE));

    m_dateTimePositionDropDownbox->setIndexofCurrElement(m_osdSettings->dateTimePosition-1);
    m_elementList[CAMERAOSDSETTINGS_DATETIMEPOSITION_SPINBOX] = m_dateTimePositionDropDownbox;
    connect(m_dateTimePositionDropDownbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_textOverlayCheckbox = new OptionSelectButton((CAMERAOSDSETTINGS_LEFT_MARGIN + m_background->x()),
                                                   (topMargin + (BGTILE_HEIGHT * CAMERAOSDSETTINGS_TEXTOVERLAY_CHECKBOX)),
                                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                                   BGTILE_HEIGHT,
                                                   CHECK_BUTTON_INDEX,
                                                   this,
                                                   COMMON_LAYER,
                                                   controlLabelStrings[CAMERAOSDSETTINGS_TEXTOVERLAY_CHECKBOX],
                                                   "", -1,
                                                   CAMERAOSDSETTINGS_TEXTOVERLAY_CHECKBOX);
    m_textOverlayCheckbox->changeState(m_osdSettings->textOverlay);
    m_elementList[CAMERAOSDSETTINGS_TEXTOVERLAY_CHECKBOX] = m_textOverlayCheckbox;
    connect(m_textOverlayCheckbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_textOverlayCheckbox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotOptionButtonSelected(OPTION_STATE_TYPE_e,int)));

    //create TextOverlay DropDown
    m_textOverlayDropDownbox = new DropDown((CAMERAOSDSETTINGS_LEFT_MARGIN + m_background->x()),
                                                 (topMargin + (BGTILE_HEIGHT * CAMERAOSDSETTINGS_TEXTOVERLAY_SPINBOX)),
                                                 BGTILE_MEDIUM_SIZE_WIDTH,
                                                 BGTILE_HEIGHT,
                                                 CAMERAOSDSETTINGS_TEXTOVERLAY_SPINBOX,
                                                 DROPDOWNBOX_SIZE_200,
                                                 controlLabelStrings[CAMERAOSDSETTINGS_TEXTOVERLAY_SPINBOX],
                                                 m_textOverlayList,
                                                 this, "", true, 0,
                                                 COMMON_LAYER,
                                                 (m_osdSettings->textOverlay == ON_STATE));
    m_textOverlayDropDownbox->setIndexofCurrElement(m_textOverlayIndex);
    if (m_textOverlayMax == TEXT_OVERLAY_MIN)
    {
        m_textOverlayDropDownbox->setIsEnabled(false);
    }

    m_elementList[CAMERAOSDSETTINGS_TEXTOVERLAY_SPINBOX] = m_textOverlayDropDownbox;
    connect(m_textOverlayDropDownbox,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotSpinBoxValueChanged(QString,quint32)));
    connect(m_textOverlayDropDownbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_osdTextparam = new TextboxParam();
    m_osdTextparam->maxChar = 10;
    m_osdTextparam->minChar = 1;
    m_osdTextparam->isTotalBlankStrAllow = (m_textOverlayMax == TEXT_OVERLAY_MIN) ? false : true;
    m_osdTextparam->labelStr = controlLabelStrings[CAMERAOSDSETTINGS_OSDTEXT_TEXTBOX];
    m_osdTextparam->validation = QRegExp(QString("[a-zA-Z0-9]"));
    m_osdTextbox = new TextBox((CAMERAOSDSETTINGS_LEFT_MARGIN + m_background->x()),
                               (topMargin + (BGTILE_HEIGHT * CAMERAOSDSETTINGS_OSDTEXT_TEXTBOX)),
                               BGTILE_MEDIUM_SIZE_WIDTH,
                               BGTILE_HEIGHT,
                               CAMERAOSDSETTINGS_OSDTEXT_TEXTBOX,
                               TEXTBOX_MEDIAM,
                               this,
                               m_osdTextparam,
                               COMMON_LAYER,
                               (m_osdSettings->textOverlay == ON_STATE));

    m_osdTextbox->setInputText(m_osdSettings->text[m_textOverlayIndex]);
    m_elementList[CAMERAOSDSETTINGS_OSDTEXT_TEXTBOX] = m_osdTextbox;
    connect(m_osdTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_osdTextbox,
            SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
            this,
            SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));

    m_textPositionDropDownbox = new DropDown((CAMERAOSDSETTINGS_LEFT_MARGIN + m_background->x()),
                                         (topMargin + (BGTILE_HEIGHT * CAMERAOSDSETTINGS_TEXTPOSITION_SPINBOX)),
                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         CAMERAOSDSETTINGS_TEXTPOSITION_SPINBOX,
                                         DROPDOWNBOX_SIZE_200,
                                         controlLabelStrings[CAMERAOSDSETTINGS_TEXTPOSITION_SPINBOX],
                                         m_graphicOSDPositionList,
                                         this, "", true, 0,
                                         COMMON_LAYER,
                                         (m_osdSettings->textOverlay == ON_STATE));

    m_textPositionDropDownbox->setIndexofCurrElement(m_osdSettings->textPosition[m_textOverlayIndex]-1);
    m_elementList[CAMERAOSDSETTINGS_TEXTPOSITION_SPINBOX] = m_textPositionDropDownbox;
    connect(m_textPositionDropDownbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_cameraNamePositionDownbox = new DropDown((CAMERAOSDSETTINGS_LEFT_MARGIN + m_background->x()),
                                               (topMargin + (BGTILE_HEIGHT * CAMERAOSDSETTINGS_CAMERANAMEPOSITION_SPINBOX)),
                                               BGTILE_MEDIUM_SIZE_WIDTH,
                                               BGTILE_HEIGHT,
                                               CAMERAOSDSETTINGS_CAMERANAMEPOSITION_SPINBOX,
                                               DROPDOWNBOX_SIZE_200,
                                               controlLabelStrings[CAMERAOSDSETTINGS_CAMERANAMEPOSITION_SPINBOX],
                                               m_cameraOSDPositionList,
                                               this);
    m_cameraNamePositionDownbox->setIndexofCurrElement(m_osdSettings->cameraNamePosition);
    m_elementList[CAMERAOSDSETTINGS_CAMERANAMEPOSITION_SPINBOX] = m_cameraNamePositionDownbox;
    connect(m_cameraNamePositionDownbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_cameraStatusPositionDownbox =  new DropDown((CAMERAOSDSETTINGS_LEFT_MARGIN + m_background->x()),
                                                 (topMargin + (BGTILE_HEIGHT * CAMERAOSDSETTINGS_CAMERASTATUSPOSITION_SPINBOX)),
                                                 BGTILE_MEDIUM_SIZE_WIDTH,
                                                 BGTILE_HEIGHT,
                                                 CAMERAOSDSETTINGS_CAMERASTATUSPOSITION_SPINBOX,
                                                 DROPDOWNBOX_SIZE_200,
                                                 controlLabelStrings[CAMERAOSDSETTINGS_CAMERASTATUSPOSITION_SPINBOX],
                                                 m_cameraOSDPositionList,
                                                 this);
    m_cameraStatusPositionDownbox->setIndexofCurrElement(m_osdSettings->cameraStatusPosition);
    m_elementList[CAMERAOSDSETTINGS_CAMERASTATUSPOSITION_SPINBOX] = m_cameraStatusPositionDownbox;
    connect(m_cameraStatusPositionDownbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_okButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                (m_background->x() + (m_background->width() / 2) - SCALE_WIDTH(81)),
                                (m_background->y() + m_background->height() - SCALE_HEIGHT(30)),
                                "OK", this,
                                CAMERAOSDSETTINGS_OKBUTTON);
    m_elementList[CAMERAOSDSETTINGS_OKBUTTON] = m_okButton;
    connect(m_okButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_okButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotConfigButtonClicked(int)));

    m_cancelButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                    (m_background->x() + (m_background->width() / 2) + SCALE_WIDTH(81)),
                                    (m_background->y() + m_background->height() - SCALE_HEIGHT(30)),
                                    "Cancel", this,
                                    CAMERAOSDSETTINGS_CANCELBUTTON);
    m_elementList[CAMERAOSDSETTINGS_CANCELBUTTON] = m_cancelButton;
    connect(m_cancelButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_cancelButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotConfigButtonClicked(int)));

    m_infoPage = new InfoPage(0,
                              0,
                              parent->width(),
                              parent->height(),
                              INFO_CONFIG_PAGE,
                              parentWidget());
    connect(m_infoPage,
            SIGNAL(sigInfoPageCnfgBtnClick(int)),
            this,
            SLOT(slotInfoPageBtnclick(int)));

    m_currentElement = CAMERAOSDSETTINGS_CLOSE_BUTTON;
    m_elementList[m_currentElement]->forceActiveFocus();
    this->show();
}

CameraOSDSettings::~CameraOSDSettings()
{
    delete m_background;

    disconnect(m_closeButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotCloseButtonClicked(int)));
    disconnect(m_closeButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_closeButton;

    delete m_pageHeading;

    disconnect(m_dateTimeOverlayCheckbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_dateTimeOverlayCheckbox,
               SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
               this,
               SLOT(slotOptionButtonSelected(OPTION_STATE_TYPE_e,int)));
    delete m_dateTimeOverlayCheckbox;

    disconnect(m_dateTimePositionDropDownbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_dateTimePositionDropDownbox;

    disconnect(m_textOverlayCheckbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_textOverlayCheckbox,
               SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
               this,
               SLOT(slotOptionButtonSelected(OPTION_STATE_TYPE_e,int)));
    delete m_textOverlayCheckbox;

    disconnect(m_textOverlayDropDownbox,
               SIGNAL(sigValueChanged(QString,quint32)),
               this,
               SLOT(slotSpinBoxValueChanged(QString,quint32)));
    disconnect(m_textOverlayDropDownbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
     delete m_textOverlayDropDownbox;

    disconnect(m_osdTextbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_osdTextbox,
               SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
               this,
               SLOT(slotTextBoxLoadInfopage(int,INFO_MSG_TYPE_e)));
    delete m_osdTextbox;
    delete m_osdTextparam;

    disconnect(m_textPositionDropDownbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_textPositionDropDownbox;

    disconnect(m_cameraNamePositionDownbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_cameraNamePositionDownbox;

    disconnect(m_cameraStatusPositionDownbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_cameraStatusPositionDownbox;

    disconnect(m_okButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_okButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotConfigButtonClicked(int)));
    delete m_okButton;

    disconnect(m_cancelButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(m_cancelButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotConfigButtonClicked(int)));
    delete m_cancelButton;

    disconnect(m_infoPage,
               SIGNAL(sigInfoPageCnfgBtnClick(int)),
               this,
               SLOT(slotInfoPageBtnclick(int)));
    delete m_infoPage;
}

void CameraOSDSettings::saveConfig()
{
    if((m_cameraNamePositionDownbox->getCurrValue() == m_cameraStatusPositionDownbox->getCurrValue())
            && (m_cameraNamePositionDownbox->getCurrValue() != cameraOSDList[OSD_NONE]))
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(CAM_OSD_SETTING_CANNOT_SET_BOTH_OSD_VALUE));
    }
    else if((m_textOverlayCheckbox->getCurrentState() == ON_STATE)
            && (m_osdTextbox->getInputText() == "") && (m_textOverlayMax == TEXT_OVERLAY_MIN))
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(CAM_OSD_SETTING_ENT_TEXT));
    }
    else
    {
        m_osdSettings->dateTimeOverlay = m_dateTimeOverlayCheckbox->getCurrentState();
        m_osdSettings->dateTimePosition = (OSD_POSITION_e)(m_dateTimePositionDropDownbox->getIndexofCurrElement() + 1);
        m_osdSettings->textOverlay = m_textOverlayCheckbox->getCurrentState();
        m_osdSettings->text[m_textOverlayIndex] = m_osdTextbox->getInputText();
        m_osdSettings->textPosition[m_textOverlayIndex] = (OSD_POSITION_e)(m_textPositionDropDownbox->getIndexofCurrElement() + 1);
        m_osdSettings->cameraNamePosition = (OSD_POSITION_e)m_cameraNamePositionDownbox->getIndexofCurrElement();
        m_osdSettings->cameraStatusPosition = (OSD_POSITION_e)m_cameraStatusPositionDownbox->getIndexofCurrElement();
        emit sigDeleteObject(m_indexInPage);
    }
}

void CameraOSDSettings::takeLeftKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement - 1 + MAX_CAMERAOSDSETTINGS_ELEMENT) % MAX_CAMERAOSDSETTINGS_ELEMENT;
    }while(!m_elementList[m_currentElement]->getIsEnabled());

    m_elementList[m_currentElement]->forceActiveFocus();
}

void CameraOSDSettings::takeRightKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement + 1) % MAX_CAMERAOSDSETTINGS_ELEMENT;
    }while(!m_elementList[m_currentElement]->getIsEnabled());

    m_elementList[m_currentElement]->forceActiveFocus();
}

void CameraOSDSettings::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QColor color;
    QRect mainRect(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                   SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT),
                   SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH),
                   SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT));
    color.setAlpha(150);
    painter.setBrush(QBrush(color));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(mainRect, SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));
}

void CameraOSDSettings::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void CameraOSDSettings::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void CameraOSDSettings::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void CameraOSDSettings::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currentElement = CAMERAOSDSETTINGS_CLOSE_BUTTON;
    m_elementList[m_currentElement]->forceActiveFocus();
}

void CameraOSDSettings::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if(m_elementList[m_currentElement] != NULL)
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void CameraOSDSettings::slotOptionButtonSelected(OPTION_STATE_TYPE_e currentState, int indexInPage)
{
    bool isEnable = (currentState == ON_STATE ? true : false);
    switch(indexInPage)
    {
    case CAMERAOSDSETTINGS_DATETIMEOVERLAY_CHECKBOX:
        m_dateTimePositionDropDownbox->setIsEnabled(isEnable);
        break;

    case CAMERAOSDSETTINGS_TEXTOVERLAY_CHECKBOX:
        if (m_textOverlayMax > TEXT_OVERLAY_MIN)
        {
            m_textOverlayDropDownbox->setIsEnabled(isEnable);
        }       
        m_osdTextbox->setIsEnabled(isEnable);
        m_textPositionDropDownbox->setIsEnabled(isEnable);
        break;
    }
}

void CameraOSDSettings::slotUpdateCurrentElement(int indexInPage)
{
    m_currentElement = indexInPage;
}

void CameraOSDSettings::slotConfigButtonClicked(int indexInPage)
{
    switch(indexInPage)
    {
    case CAMERAOSDSETTINGS_OKBUTTON:
        saveConfig();
        break;

    case CAMERAOSDSETTINGS_CANCELBUTTON:
        emit sigDeleteObject(m_indexInPage);
        break;
    }
}

void CameraOSDSettings::slotCloseButtonClicked(int)
{
    emit sigDeleteObject(m_indexInPage);
}

void CameraOSDSettings::slotInfoPageBtnclick(int)
{
    m_elementList[m_currentElement]->forceActiveFocus();
}

void CameraOSDSettings::slotTextBoxLoadInfopage(int, INFO_MSG_TYPE_e msgType)
{
    if(msgType == INFO_MSG_ERROR)
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(CAM_OSD_SETTING_ENT_TEXT));
    }
}

void CameraOSDSettings::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void CameraOSDSettings::slotSpinBoxValueChanged(QString, quint32 index)
{
    switch(index)
    {
        case CAMERAOSDSETTINGS_TEXTOVERLAY_SPINBOX:
        {
            quint8 selectedIndex = m_textOverlayDropDownbox->getIndexofCurrElement();
            if (selectedIndex != m_textOverlayIndex)
            {
                m_osdSettings->text[m_textOverlayIndex] = m_osdTextbox->getInputText();
                m_osdSettings->textPosition[m_textOverlayIndex] = (OSD_POSITION_e)(m_textPositionDropDownbox->getIndexofCurrElement() + 1);
                m_textOverlayIndex = selectedIndex;
                m_osdTextbox->setInputText(m_osdSettings->text[m_textOverlayIndex]);
                m_textPositionDropDownbox->setIndexofCurrElement(m_osdSettings->textPosition[m_textOverlayIndex]-1);
            }
        }
        break;

        default:
            break;
    }

}
