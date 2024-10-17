#ifndef IMAGEAPPEARENCESETTINGS_H
#define IMAGEAPPEARENCESETTINGS_H

#include "Controls/BackGround.h"
#include "DataStructure.h"
#include "Controls/TextBox.h"
#include "Controls/TextLabel.h"
#include "Controls/SliderControl.h"
#include "Controls/ImageControls/Image.h"
#include "Controls/CnfgButton.h"
#include "ApplController.h"

typedef enum
{
    HUE,
    SATURATION,
    BRIGHTNESS,
    CONTRAST,
    MAX_FEATURE
}APPEARENCE_FEATURE_TYPE_e;

typedef enum
{
    IMAGE_APPEARENCE_PAGE_CLOSE_BUTTON,
    IMAGE_APPEARENCE_PAGE_SLIDER,
    IMAGE_APPEARENCE_PAGE_TEXTBOX,
    IMAGE_APPEARENCE_PAGE_OK_BUTTON = 9,
    IMAGE_APPEARENCE_PAGE_CANCEL_BUTTON,
    IMAGE_APPEARENCE_PAGE_MAX_ELEMENT
}IMAGE_APPEARENCE_PAGE_ELEMENT_TYPE_e;

class ImageAppearenceSettings : public BackGround
{
    Q_OBJECT
private:
    IMAGE_APPEARENCE_DATA_t *m_imageAppearenceData;
    IMAGE_APPEARENCE_DATA_t *m_imageAppearenceConfigData;
    SliderControl* m_sliderBar[MAX_FEATURE];
    Image* m_barBackgroungImage[MAX_FEATURE];
    TextBox* m_valueTextbox[MAX_FEATURE];
    CnfgButton* m_okButton;
    CnfgButton* m_cancelButton;
    TextboxParam* m_textboxParam[MAX_FEATURE];
    TextLabel* m_featureLabel[MAX_FEATURE];
    ApplController* m_applController;
    quint8 m_value[MAX_FEATURE];
    NavigationControl* m_elementList[IMAGE_APPEARENCE_PAGE_MAX_ELEMENT];
    int m_currentElement;
    quint8 m_cameraIndex;
    float m_valueMultipler;
public:
    ImageAppearenceSettings(void *data, void *configData, quint8 cameraIndex, QWidget* parent = 0);
    ~ImageAppearenceSettings();

    void saveConfig();

    void static setImageParameter(quint8 cameraIndex, quint8 hue, quint8 saturation, quint8 brightness, quint8 contrast);

    void takeLeftKeyAction();
    void takeRightKeyAction();

    //keyboard support added
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);

public slots:
    void slotCloseButtonClicked(int indexInPage);
    void slotValueChanged(int changedValue, int indexInPage, bool sliderMove);
    void slotConfigButtonClicked(int index);
    void slotUpdateCurrentElement(int index);
    void slotTextValueAppend(QString str, int index);
};

#endif // IMAGEAPPEARENCESETTINGS_H
