#include "AppearanceSetting.h"
#include "Layout/Layout.h"
#include "Controls/MessageBanner.h"
#include "ValidationMessage.h"

#include <QPainter>
#include <QKeyEvent>


#define APPEARANCE_SETTING_HEIGHT   SCALE_HEIGHT(370)
#define APPEARANCE_SETTING_WIDTH    SCALE_WIDTH(801)
#define APPEARANCE_SETTING_BGWIDTH  SCALE_WIDTH(778)

#define APPEARANCE_SETTING_HEADING      "Appearance Settings"
#define INTER_CONTROL_MARGIN            SCALE_WIDTH(15)
#define LABEL_WIDTH                     SCALE_WIDTH(100)
#define MAX_VALUE                       100
#define APPEARENCE_IMAGE_PATH           ":/Images_Nvrx/AppearenceControl/"
#define BAR_IMAGE_PATH                  ":/Images_Nvrx/AppearenceControl/bar.png"

static const QString labelString[PHYSICAL_DISPLAY_SCREEN_PARAM_MAX] = { "Brightness",
                                                                        "Contrast",
                                                                        "Saturation",
                                                                        "Hue" };


static const QString configButtonString[] = {"Default",
                                             "Refresh",
                                             "Save"};

static const QString defaultValue[PHYSICAL_DISPLAY_SCREEN_PARAM_MAX] = {
    "50", "50", "50", "50"};

static const QString appearanceSettingStrings[] = {
    "",
    "Display"
};

static const QStringList displayInterfaceList = QStringList()
<< "HDMI";

AppearanceSetting::AppearanceSetting(QWidget *parent,DISPLAY_TYPE_e currentSelectedId) :
    KeyBoard(parent), m_width(parent->width ()), m_height(parent->height ())
{
    this->setGeometry (0,0,m_width,m_height);

    createDefaultElements();

    m_currentDisplayType = (PHYSICAL_DISPLAY_TYPE_e)m_displayDropDownBox->getIndexofCurrElement ();
    getApperanceParameter();
    Q_UNUSED(currentSelectedId);
}

AppearanceSetting::~AppearanceSetting ()
{
    // If configuration not saved and video pop up comes,
    // then revert the  applied changes.
    getApperanceParameter();

    if(IS_VALID_OBJ(m_backGround))
    {
        DELETE_OBJ(m_backGround);
    }

    if(IS_VALID_OBJ(m_closeButton))
    {
        disconnect (m_closeButton,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));
        disconnect (m_closeButton,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ( m_closeButton);
    }

    if(IS_VALID_OBJ(m_heading))
    {
        DELETE_OBJ(m_heading);
    }

    if(IS_VALID_OBJ(m_displayDropDownBox))
    {
        disconnect (m_displayDropDownBox,
                    SIGNAL(sigValueChanged(QString,quint32)),
                    this,
                    SLOT(slotSpinBoxValueChanged(QString,quint32)));
        disconnect(m_displayDropDownBox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ (m_displayDropDownBox);
    }
    m_displayList.clear ();

    for(quint8 index = 0; index < PHYSICAL_DISPLAY_SCREEN_PARAM_MAX; index++)
    {
        if(IS_VALID_OBJ(m_valueTextbox[index]))
        {
            disconnect(m_valueTextbox[index],
                       SIGNAL(sigUpdateCurrentElement(int)),
                       this,
                       SLOT(slotUpdateCurrentElement(int)));
            disconnect (m_valueTextbox[index],
                        SIGNAL(sigTextValueAppended(QString,int)),
                        this,
                        SLOT(slotTextBoxValueChange(QString,int)));
            DELETE_OBJ (m_valueTextbox[index]);
            DELETE_OBJ (m_textboxParam[index]);

            DELETE_OBJ (m_barBackgroungImage[index]);

            DELETE_OBJ (m_featureLabel[index]);

            disconnect(m_sliderBar[index],
                       SIGNAL(sigValueChanged(int,int,bool)),
                       this,
                       SLOT(slotValueChanged(int,int,bool)));
            disconnect(m_sliderBar[index],
                       SIGNAL(sigUpdateCurrentElement(int)),
                       this,
                       SLOT(slotUpdateCurrentElement(int)));
            DELETE_OBJ (m_sliderBar[index]);
        }
    }

    if(IS_VALID_OBJ(m_refreshButton))
    {
        disconnect(m_refreshButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        disconnect(m_refreshButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_refreshButton);
    }

    if(IS_VALID_OBJ(m_defaultButton))
    {
        disconnect(m_defaultButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        disconnect(m_defaultButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_defaultButton);
    }

    if(IS_VALID_OBJ(m_saveButton))
    {
        disconnect(m_saveButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        disconnect(m_saveButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_saveButton);
     }

    if(IS_VALID_OBJ(m_infoPage))
    {
        disconnect (m_infoPage,
                    SIGNAL(sigInfoPageCnfgBtnClick(int)),
                    this,
                    SLOT(slotInfoPageCnfgBtnClick(int)));
        DELETE_OBJ(m_infoPage);
    }
}

void AppearanceSetting::createDefaultElements()
{
    m_applController = ApplController::getInstance ();

    m_backGround = new Rectangle((m_width - APPEARANCE_SETTING_WIDTH)/2 ,
                                 (m_height - APPEARANCE_SETTING_HEIGHT)/2,
                                 APPEARANCE_SETTING_WIDTH,
                                 APPEARANCE_SETTING_HEIGHT,
                                 0,
                                 BORDER_2_COLOR,
                                 NORMAL_BKG_COLOR,
                                 this,
                                 SCALE_WIDTH(2));

    m_closeButton = new CloseButtton (m_backGround->x ()+m_backGround->width () - SCALE_WIDTH(20),
                                      m_backGround->y () + SCALE_HEIGHT(20),
                                      this,
                                      CLOSE_BTN_TYPE_1,
                                      APP_SET_CLOSE_BUTTON);

    m_elementList[APP_SET_CLOSE_BUTTON] = m_closeButton;

    connect (m_closeButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (m_closeButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    m_heading = new Heading(m_backGround->x () + m_backGround->width ()/2,
                            m_backGround->y () + SCALE_HEIGHT(20),
                            APPEARANCE_SETTING_HEADING,
                            this,
                            HEADING_TYPE_2);

    displayTypeSupportedByDevice();

    QMap<quint8, QString>  displayList;

    for(quint8 index = 0; index < m_displayList.length (); index++)
    {
        displayList.insert (index,m_displayList.at (index));
    }

    m_displayDropDownBox =  new DropDown(m_backGround->x () + (APPEARANCE_SETTING_WIDTH -
                                                               APPEARANCE_SETTING_BGWIDTH)/2,
                                         m_backGround->y () + 2*BGTILE_HEIGHT,
                                         APPEARANCE_SETTING_BGWIDTH,
                                         BGTILE_HEIGHT,
                                         APP_SET_DISP_SPINBOX,
                                         DROPDOWNBOX_SIZE_200,
                                         appearanceSettingStrings[APP_SET_DISP_SPINBOX],
                                         displayList,
                                         this,
                                         "",false,
                                         SCALE_WIDTH(20));

    m_elementList[APP_SET_DISP_SPINBOX] = m_displayDropDownBox;

    connect (m_displayDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinBoxValueChanged(QString,quint32)));
    connect(m_displayDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    for(quint8 index = 0; index < PHYSICAL_DISPLAY_SCREEN_PARAM_MAX; index++)
    {
        m_textboxParam[index] = new TextboxParam();
        m_textboxParam[index]->suffixStr = "(0-100)";
        m_textboxParam[index]->minNumValue = 0;
        m_textboxParam[index]->maxNumValue = MAX_VALUE;
        m_textboxParam[index]->maxChar = 3;
        m_textboxParam[index]->isNumEntry = true;
        m_textboxParam[index]->validation = QRegExp("[0-9]");
        m_textboxParam[index]->isCentre = false;
        m_textboxParam[index]->leftMargin = SCALE_WIDTH(630);
        m_textboxParam[index]->textStr = QString("%1").arg(defaultValue[index]);

        m_valueTextbox[index] = new TextBox(m_displayDropDownBox->x (),
                                            m_displayDropDownBox->y () + SCALE_HEIGHT(5) +
                                            m_displayDropDownBox->height ()+ (BGTILE_HEIGHT * index),
                                            APPEARANCE_SETTING_BGWIDTH,
                                            BGTILE_HEIGHT,
                                            APP_SET_BRIGHTNESS_TEXTBOX + (2*index),
                                            TEXTBOX_EXTRASMALL,
                                            this,
                                            m_textboxParam[index]);

        m_elementList[((2 * index) + APP_SET_BRIGHTNESS_TEXTBOX)] = m_valueTextbox[index];

        connect(m_valueTextbox[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect (m_valueTextbox[index],
                 SIGNAL(sigTextValueAppended(QString,int)),
                 this,
                 SLOT(slotTextBoxValueChange(QString,int)));

       m_barBackgroungImage[index] = new Image((m_valueTextbox[index]->x() + LABEL_WIDTH + INTER_CONTROL_MARGIN),
                                                (m_valueTextbox[index]->y() + (BGTILE_HEIGHT / 2)),
                                                BAR_IMAGE_PATH,
                                                this,
                                                START_X_CENTER_Y,
                                                0,
                                                false,
                                                true);

       m_valueMultipler = (float)m_barBackgroungImage[index]->width()/(float)MAX_VALUE;
       m_featureLabel[index] = new TextLabel((m_barBackgroungImage[index]->x() - INTER_CONTROL_MARGIN),
                                              (m_valueTextbox[index]->y() + (BGTILE_HEIGHT / 2)),
                                              NORMAL_FONT_SIZE,
                                              labelString[index],
                                              this,
                                              NORMAL_FONT_COLOR,
                                              NORMAL_FONT_FAMILY,
                                              ALIGN_END_X_CENTRE_Y);

        m_sliderBar[index] = new SliderControl((m_barBackgroungImage[index]->x() - SCALE_WIDTH(7)),
                                               m_barBackgroungImage[index]->y(),
                                               (m_barBackgroungImage[index]->width() + SCALE_WIDTH(14)),
                                               m_barBackgroungImage[index]->height(),
                                               m_barBackgroungImage[index]->width(),
                                               m_valueTextbox[index]->height(),
                                               APPEARENCE_IMAGE_PATH,
                                               defaultValue[index].toUInt ()*m_valueMultipler,
                                               this,
                                               HORIZONTAL_SLIDER,
                                               APP_SET_BRIGHTNESS_SLIDER + (2*index),
                                               true,
                                               false,
                                               true,
                                               true);

        m_elementList[((2 * index) + APP_SET_BRIGHTNESS_SLIDER)] = m_sliderBar[index];

        connect(m_sliderBar[index],
                SIGNAL(sigValueChanged(int,int,bool)),
                this,
                SLOT(slotValueChanged(int,int,bool)));
        connect(m_sliderBar[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    }

    m_refreshButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                     m_backGround->x () + (m_backGround->width() / 2),
                                     m_backGround->y () + (m_backGround->height() - SCALE_HEIGHT(30)),
                                     configButtonString[1],
                                     this,
                                     APP_SET_REFRESH_BUTTON,
                                     true);

    m_elementList[APP_SET_REFRESH_BUTTON] = m_refreshButton;

    connect(m_refreshButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));
    connect(m_refreshButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_defaultButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                     (m_refreshButton->x() - (m_refreshButton->width() / 2)),
                                     m_backGround->y() + (m_backGround->height() - SCALE_HEIGHT(30)),
                                     configButtonString[0],
                                     this,
                                     APP_SET_DEFAULT_BUTTON,
                                     true);

    m_elementList[APP_SET_DEFAULT_BUTTON] = m_defaultButton;

    connect(m_defaultButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));
    connect(m_defaultButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_saveButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                  (m_refreshButton->x() + (m_refreshButton->width() / 2) + m_refreshButton->width()),
                                  m_backGround->y() + (m_backGround->height() - SCALE_HEIGHT(30)),
                                  configButtonString[2],
                                  this,
                                  APP_SET_SAVE_BUTTON,
                                  true);

    m_elementList[APP_SET_SAVE_BUTTON] = m_saveButton;

    connect(m_saveButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));
    connect(m_saveButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_infoPage = new InfoPage(0, 0,
                              SCALE_WIDTH(DISP_SETTING_PAGE_WIDTH),
                              SCALE_HEIGHT(DISP_SETTING_PAGE_HEIGHT),
                              INFO_LIVEVIEW,
                              this);
    connect (m_infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageCnfgBtnClick(int)));

    m_currentClickButton = MAX_APPEARANCE_SETTINGS_CTRL;
    m_currentElement = APP_SET_DISP_SPINBOX;
    m_elementList[m_currentElement]->forceActiveFocus();

    this->show ();
}

void AppearanceSetting::displayTypeSupportedByDevice ()
{
    m_displayList.clear ();

    if(deviceRespInfo.noOfVGA)
    {
        m_displayList.append ("VGA");
    }

    if(deviceRespInfo.hdmi1)
    {
        m_displayList.append ("HDMI");
    }

    if(deviceRespInfo.CVBSMain)
    {
        m_displayList.append ("CVBS");
    }

    if(deviceRespInfo.hdmi2)
    {
        m_displayList.append ("HDMI 2");
    }

    if(deviceRespInfo.CVBSSpot)
    {
        m_displayList.append ("CVBS");
    }
}

void AppearanceSetting::paintEvent (QPaintEvent *)
{
    QPainter painter(this);
    QColor color;

    color.setAlpha (150);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);

    painter.drawRoundedRect (QRect(0,0,
                                   SCALE_WIDTH(DISP_SETTING_PAGE_HEADING_WIDTH),
                                   SCALE_HEIGHT(DISP_SETTING_PAGE_HEADING_HEIGHT)),
                             SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));
}

void AppearanceSetting::takeLeftKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement - 1 + MAX_APPEARANCE_SETTINGS_CTRL) % MAX_APPEARANCE_SETTINGS_CTRL;
    }while(!m_elementList[m_currentElement]->getIsEnabled());

    m_elementList[m_currentElement]->forceActiveFocus();
}


void AppearanceSetting::takeRightKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement + 1) % MAX_APPEARANCE_SETTINGS_CTRL;
    }while(!m_elementList[m_currentElement]->getIsEnabled());

    m_elementList[m_currentElement]->forceActiveFocus();
}

void AppearanceSetting::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    m_elementList[m_currentElement]->forceActiveFocus();
}

void AppearanceSetting::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void AppearanceSetting::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currentElement = APP_SET_CLOSE_BUTTON;
    m_elementList[APP_SET_CLOSE_BUTTON]->forceActiveFocus ();
}

void AppearanceSetting::functionKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_F5:
        event->accept();
        m_currentElement = APP_SET_REFRESH_BUTTON;
        m_elementList[m_currentElement]->forceActiveFocus();
        m_refreshButton->takeEnterKeyAction ();
        break;

    default:
        event->accept();
        break;
    }
}

void AppearanceSetting::getApperanceParameter()
{
    if(m_applController->readDisplayParameters(getInterfaceIndex(), m_displayparam))
    {
        m_valueTextbox[PHYSICAL_DISPLAY_BRIGHTNESS]->setInputText
                (QString("%1").arg(m_displayparam.brighteness));

        m_sliderBar[PHYSICAL_DISPLAY_BRIGHTNESS]->setCurrentValue
                ((m_displayparam.brighteness*m_valueMultipler));

        m_valueTextbox[PHYSICAL_DISPLAY_CONTRAST]->setInputText
                (QString("%1").arg(m_displayparam.contrast));

        m_sliderBar[PHYSICAL_DISPLAY_CONTRAST]->setCurrentValue
                ((m_displayparam.contrast*m_valueMultipler));

        m_valueTextbox[PHYSICAL_DISPLAY_SATURATION]->setInputText
                (QString("%1").arg(m_displayparam.saturation));

        m_sliderBar[PHYSICAL_DISPLAY_SATURATION]->setCurrentValue
                ((m_displayparam.saturation*m_valueMultipler));

        m_valueTextbox[PHYSICAL_DISPLAY_HUE]->setInputText
                (QString("%1").arg(m_displayparam.hue));

        m_sliderBar[PHYSICAL_DISPLAY_HUE]->setCurrentValue
                ((m_displayparam.hue*m_valueMultipler));
    }

    for(quint8 index = 0; index < PHYSICAL_DISPLAY_SCREEN_PARAM_MAX; index++)
    {
        setAppearnceActivity ((PHYSICAL_DISPLAY_SCREEN_PARAM_TYPE_e)index,
                              m_valueTextbox[index]->getInputText ().toUInt ());

    }
}

void AppearanceSetting::setAppearnceActivity(PHYSICAL_DISPLAY_SCREEN_PARAM_TYPE_e displayParam,
                                             quint32 currValueInInt)
{
    m_applController->setDisplayParameters(getInterfaceIndex(), displayParam, currValueInInt);
}

PHYSICAL_DISPLAY_TYPE_e AppearanceSetting::getInterfaceIndex()
{   
    return (PHYSICAL_DISPLAY_TYPE_e)displayInterfaceList.indexOf (m_displayDropDownBox->getCurrValue ());
}

bool AppearanceSetting::saveApperanceParam()
{
    bool status = false;

    m_displayparam.brighteness = m_valueTextbox[PHYSICAL_DISPLAY_BRIGHTNESS]->getInputText ().toUInt ();
    m_displayparam.contrast = m_valueTextbox[PHYSICAL_DISPLAY_CONTRAST]->getInputText ().toUInt ();
    m_displayparam.saturation = m_valueTextbox[PHYSICAL_DISPLAY_SATURATION]->getInputText ().toUInt ();
    m_displayparam.hue = m_valueTextbox[PHYSICAL_DISPLAY_HUE]->getInputText ().toUInt ();    

    if(m_applController->writeDisplayParameters(getInterfaceIndex(), m_displayparam))
    {
        status = true;
    }
    return status;
}

void AppearanceSetting::slotButtonClick(int index)
{
    m_currentClickButton = index;
    switch(index)
    {
    case APP_SET_CLOSE_BUTTON:
        getApperanceParameter();
        emit sigObjectDelete();
        break;

    case APP_SET_DEFAULT_BUTTON:
        m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(APPEARENCE_SETTING_DEFAULT_MSG),true);
        break;

    case APP_SET_SAVE_BUTTON:
        if(saveApperanceParam() == true)
        {
            MessageBanner::addMessageInBanner (ValidationMessage::getValidationMessage(SETTING_SAVE_SUCCESS));
        }
        else
        {
            m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ERROR_SAVE_MSG));
        }
        break;

    case APP_SET_REFRESH_BUTTON:
    {
        getApperanceParameter();
    }
    break;
    default:
        break;
    }
}

void AppearanceSetting::slotSpinBoxValueChanged(QString str, quint32)
{
    m_currentDisplayType = (PHYSICAL_DISPLAY_TYPE_e)m_displayDropDownBox->getIndexofCurrElement ();
    getApperanceParameter();
    Q_UNUSED(str);
}

void AppearanceSetting::slotValueChanged(int changedValue, int indexInPage, bool sliderMove)
{
    Q_UNUSED(sliderMove);
    quint8 cntrlIndex =  (indexInPage - APP_SET_BRIGHTNESS_SLIDER)/2  ;
    quint32 tempChangedValue = (quint32)(qCeil((float)changedValue / m_valueMultipler));
    m_valueTextbox[cntrlIndex]->setInputText(QString("%1").arg(
                                                 (tempChangedValue)));

    setAppearnceActivity ((PHYSICAL_DISPLAY_SCREEN_PARAM_TYPE_e)cntrlIndex,
                          (tempChangedValue));
}

void AppearanceSetting::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}

void AppearanceSetting::slotTextBoxValueChange (QString currValue, int indexInPage)
{
    quint8 cntrlIndex =  (indexInPage - APP_SET_BRIGHTNESS_SLIDER)/2;
    quint32 currValueInInt = currValue.toUInt ();
    m_sliderBar[cntrlIndex]->setCurrentValue ((currValueInInt*m_valueMultipler));

    setAppearnceActivity ((PHYSICAL_DISPLAY_SCREEN_PARAM_TYPE_e)cntrlIndex,
                          currValueInInt);
}

void AppearanceSetting::slotInfoPageCnfgBtnClick(int index)
{
    if((index == INFO_OK_BTN) && (m_currentClickButton == APP_SET_DEFAULT_BUTTON))
    {
        for( quint8 index = 0; index < PHYSICAL_DISPLAY_SCREEN_PARAM_MAX; index++)
        {
            m_valueTextbox[index]->setInputText(QString("%1").arg(defaultValue[index]));
            m_sliderBar[index]->setCurrentValue ((defaultValue[index]).toUInt ()*m_valueMultipler);
        }
        if (saveApperanceParam () == false)
        {
            m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(APPEARENCE_SETTING_ERROR_DEFAULT_MSG));
            getApperanceParameter();
        }
        else
        {
            for(quint8 index = 0; index < PHYSICAL_DISPLAY_SCREEN_PARAM_MAX; index++)
            {
                setAppearnceActivity ((PHYSICAL_DISPLAY_SCREEN_PARAM_TYPE_e)index,
                                      m_valueTextbox[index]->getInputText ().toUInt ());

            }
        }
    }
    m_currentClickButton = MAX_APPEARANCE_SETTINGS_CTRL;
    m_elementList[m_currentElement]->forceActiveFocus();
}

void AppearanceSetting::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void AppearanceSetting::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void AppearanceSetting::ctrl_D_KeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currentElement = APP_SET_DEFAULT_BUTTON;
    m_elementList[m_currentElement]->forceActiveFocus();
    m_defaultButton->takeEnterKeyAction ();
}

void AppearanceSetting::ctrl_S_KeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currentElement = APP_SET_SAVE_BUTTON;
    m_elementList[m_currentElement]->forceActiveFocus();
    m_saveButton->takeEnterKeyAction ();
}
