#include "ImageAppearenceSettings.h"
#include <QKeyEvent>

#define HEIGHT                          SCALE_HEIGHT(300)
#define APPEARENCE_LEFT_MARGIN          SCALE_WIDTH(20)
#define APPEARENCE_INNER_LEFT_MARGIN    SCALE_WIDTH(15)
#define APPEARENCE_TOP_MARGIN           SCALE_HEIGHT(50)
#define INTER_CONTROL_MARGIN            SCALE_WIDTH(17)
#define TILE_WIDTH                      SCALE_WIDTH(761)
#define LABEL_WIDTH                     SCALE_WIDTH(81)
#define MAX_VALUE                       100
#define APPEARENCE_IMAGE_PATH           ":/Images_Nvrx/AppearenceControl/"
#define BAR_IMAGE_PATH                  ":/Images_Nvrx/AppearenceControl/bar.png"

const QString labelString[MAX_FEATURE] = {"Hue", "Saturation", "Brightness", "Contrast"};
const QString configButtonString[] = {"OK",
                                      "Cancel"};

ImageAppearenceSettings::ImageAppearenceSettings(void *data,
                                                 void *configData,
                                                 quint8 cameraIndex,
                                                 QWidget* parent)
    : BackGround(((parent->width() - (TILE_WIDTH + (2 * APPEARENCE_LEFT_MARGIN))) / 2),
          (parent->height() - HEIGHT),
          (TILE_WIDTH + (2 * APPEARENCE_LEFT_MARGIN)),
          HEIGHT,
          BACKGROUND_TYPE_3,
          MAX_TOOLBAR_BUTTON,
          parent,
          true,
          "Image Appearance Settings",
          CLICKED_BKG_COLOR)
{
    for(int index = 0; index < IMAGE_APPEARENCE_PAGE_MAX_ELEMENT; index++)
    {
        m_elementList[index] = NULL;
    }

    m_cameraIndex = cameraIndex;
    m_imageAppearenceData = (IMAGE_APPEARENCE_DATA_t*)data;
    m_imageAppearenceConfigData = (IMAGE_APPEARENCE_DATA_t*)configData;
    m_value[HUE] = m_imageAppearenceData->hue;
    m_value[SATURATION] = m_imageAppearenceData->saturation;
    m_value[BRIGHTNESS] = m_imageAppearenceData->brightness;
    m_value[CONTRAST] = m_imageAppearenceData->contrast;
    m_applController = ApplController::getInstance();

    m_elementList[IMAGE_APPEARENCE_PAGE_CLOSE_BUTTON] = m_mainCloseButton;
    m_mainCloseButton->setIndexInPage(IMAGE_APPEARENCE_PAGE_CLOSE_BUTTON);
    connect(m_mainCloseButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    for(quint8 index = 0; index < MAX_FEATURE; index++)
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
        m_textboxParam[index]->textStr = QString("%1").arg(m_value[index]);

        m_valueTextbox[index] = new TextBox(APPEARENCE_LEFT_MARGIN,
                                            (APPEARENCE_TOP_MARGIN + (BGTILE_HEIGHT * index)),
                                            TILE_WIDTH,
                                            BGTILE_HEIGHT,
                                            ((2 * index) + IMAGE_APPEARENCE_PAGE_TEXTBOX),
                                            TEXTBOX_EXTRASMALL,
                                            this,
                                            m_textboxParam[index]);
        m_elementList[((2 * index) + IMAGE_APPEARENCE_PAGE_TEXTBOX)] = m_valueTextbox[index];
        connect(m_valueTextbox[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect(m_valueTextbox[index],
                SIGNAL(sigTextValueAppended(QString,int)),
                this,
                SLOT(slotTextValueAppend(QString,int)));

        m_barBackgroungImage[index] = new Image((m_valueTextbox[index]->x() + APPEARENCE_INNER_LEFT_MARGIN + LABEL_WIDTH + INTER_CONTROL_MARGIN),
                                                (APPEARENCE_TOP_MARGIN + (BGTILE_HEIGHT * index) + (BGTILE_HEIGHT / 2)),
                                                BAR_IMAGE_PATH,
                                                this,
                                                START_X_CENTER_Y,
                                                IMAGE_APPEARENCE_PAGE_MAX_ELEMENT,
                                                false,
                                                true);

        m_valueMultipler = (float)m_barBackgroungImage[index]->width()/(float)MAX_VALUE;
        int labelWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width(labelString[index]);
        m_featureLabel[index] = new TextLabel((m_barBackgroungImage[index]->x() - INTER_CONTROL_MARGIN),
                                              ((APPEARENCE_TOP_MARGIN + (BGTILE_HEIGHT * index)) + (BGTILE_HEIGHT / 2)),
                                              NORMAL_FONT_SIZE,
                                              labelString[index],
                                              this,
                                              NORMAL_FONT_COLOR,
                                              NORMAL_FONT_FAMILY,
                                              ALIGN_END_X_CENTRE_Y, 0, 0, labelWidth);

        m_sliderBar[index] = new SliderControl((m_barBackgroungImage[index]->x() - SCALE_WIDTH(7)),
                                               (m_valueTextbox[index]->y()),
                                               (m_barBackgroungImage[index]->width() + SCALE_WIDTH(14)),
                                               m_valueTextbox[index]->height(),
                                               m_barBackgroungImage[index]->width(),
                                               m_valueTextbox[index]->height(),
                                               APPEARENCE_IMAGE_PATH,
                                               (m_value[index] * m_valueMultipler),
                                               this,
                                               HORIZONTAL_SLIDER,
                                               ((2 * index) + IMAGE_APPEARENCE_PAGE_SLIDER),
                                               true,
                                               false,
                                               true,
                                               true);
        m_elementList[((2 * index) + IMAGE_APPEARENCE_PAGE_SLIDER)] = m_sliderBar[index];
        connect(m_sliderBar[index],
                SIGNAL(sigValueChanged(int,int,bool)),
                this,
                SLOT(slotValueChanged(int,int,bool)));
        connect(m_sliderBar[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    }

    m_okButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                (this->width() / 2 - SCALE_WIDTH(80)),
                                (this->height() - SCALE_HEIGHT(30)),
                                configButtonString[0],
                                this,
                                IMAGE_APPEARENCE_PAGE_OK_BUTTON,
                                true);
    m_elementList[IMAGE_APPEARENCE_PAGE_OK_BUTTON] = m_okButton;
    connect(m_okButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotConfigButtonClicked(int)));
    connect(m_okButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_cancelButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                    (m_okButton->x() + m_okButton->width() + m_okButton->width() / 2 + SCALE_WIDTH(40)),
                                    (this->height() - SCALE_HEIGHT(30)),
                                    configButtonString[1],
                                    this,
                                    IMAGE_APPEARENCE_PAGE_CANCEL_BUTTON,
                                    true);
    m_elementList[IMAGE_APPEARENCE_PAGE_CANCEL_BUTTON] = m_cancelButton;
    connect(m_cancelButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotConfigButtonClicked(int)));
    connect(m_cancelButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    m_currentElement = IMAGE_APPEARENCE_PAGE_CLOSE_BUTTON;
    m_elementList[m_currentElement]->forceActiveFocus();

    setImageParameter(m_cameraIndex,
                      m_imageAppearenceData->hue,
                      m_imageAppearenceData->saturation,
                      m_imageAppearenceData->brightness,
                      m_imageAppearenceData->contrast);
    this->show();
}

ImageAppearenceSettings::~ImageAppearenceSettings()
{
    setImageParameter(m_cameraIndex,
                      m_imageAppearenceConfigData->hue,
                      m_imageAppearenceConfigData->saturation,
                      m_imageAppearenceConfigData->brightness,
                      m_imageAppearenceConfigData->contrast);

    disconnect(m_mainCloseButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));

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

    for(quint8 index = 0; index < MAX_FEATURE; index++)
    {
        disconnect(m_sliderBar[index],
                   SIGNAL(sigValueChanged(int,int,bool)),
                   this,
                   SLOT(slotValueChanged(int,int,bool)));
        disconnect(m_sliderBar[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        delete m_sliderBar[index];

        delete m_featureLabel[index];
        delete m_barBackgroungImage[index];

        disconnect(m_valueTextbox[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_valueTextbox[index],
                   SIGNAL(sigTextValueAppended(QString,int)),
                   this,
                   SLOT(slotTextValueAppend(QString,int)));
        delete m_valueTextbox[index];
        delete m_textboxParam[index];
    }
}

void ImageAppearenceSettings::saveConfig()
{
    m_imageAppearenceData->hue = m_value[HUE];
    m_imageAppearenceData->saturation = m_value[SATURATION];
    m_imageAppearenceData->brightness = m_value[BRIGHTNESS];
    m_imageAppearenceData->contrast = m_value[CONTRAST];
}

void ImageAppearenceSettings::setImageParameter(quint8 cameraIndex,
                                                quint8 hue,
                                                quint8 saturation,
                                                quint8 brightness,
                                                quint8 contrast)
{
    Q_UNUSED(cameraIndex);
    Q_UNUSED(hue);
    Q_UNUSED(saturation);
    Q_UNUSED(brightness);
    Q_UNUSED(contrast);
}

void ImageAppearenceSettings::takeLeftKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement - 1 + IMAGE_APPEARENCE_PAGE_MAX_ELEMENT) % IMAGE_APPEARENCE_PAGE_MAX_ELEMENT;
    }while(!m_elementList[m_currentElement]->getIsEnabled());

    m_elementList[m_currentElement]->forceActiveFocus();
}

void ImageAppearenceSettings::takeRightKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement + 1) % IMAGE_APPEARENCE_PAGE_MAX_ELEMENT;
    }while(!m_elementList[m_currentElement]->getIsEnabled());

    m_elementList[m_currentElement]->forceActiveFocus();
}

void ImageAppearenceSettings::navigationKeyPressed(QKeyEvent * event)
{
    event->accept();
}

void ImageAppearenceSettings::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void ImageAppearenceSettings::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void ImageAppearenceSettings::slotCloseButtonClicked(int)
{
    this->deleteLater();
}

void ImageAppearenceSettings::slotValueChanged(int changedValue, int indexInPage, bool sliderMove)
{
    Q_UNUSED(sliderMove);

    m_value[indexInPage / 2] = (quint8)(qCeil((float)changedValue / m_valueMultipler));
    m_valueTextbox[indexInPage / 2]->setInputText(QString("%1").arg((quint32)(qCeil((float)changedValue / m_valueMultipler))));
    setImageParameter(m_cameraIndex,
                      m_value[HUE],
                      m_value[SATURATION],
                      m_value[BRIGHTNESS],
                      m_value[CONTRAST]);
}

void ImageAppearenceSettings::slotConfigButtonClicked(int index)
{
    switch(index)
    {
    case IMAGE_APPEARENCE_PAGE_OK_BUTTON:
        saveConfig();
        this->deleteLater();
        break;

    case IMAGE_APPEARENCE_PAGE_CANCEL_BUTTON:
        this->deleteLater();
        break;
    }
}

void ImageAppearenceSettings::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}

void ImageAppearenceSettings::slotTextValueAppend(QString string, int index)
{
    m_value[(index / 2) - 1] = string.toInt();
    m_sliderBar[(index / 2) - 1]->setCurrentValue(string.toInt() * m_valueMultipler);
    setImageParameter(m_cameraIndex,
                      m_value[HUE],
                      m_value[SATURATION],
                      m_value[BRIGHTNESS],
                      m_value[CONTRAST]);
}
