#ifndef APPEARANCESETTING_H
#define APPEARANCESETTING_H

#include <QWidget>

#include "DataStructure.h"
#include "Controls/Heading.h"
#include "Controls/Rectangle.h"
#include "Controls/Closebuttton.h"
#include "Controls/DropDown.h"

#include "Controls/TextBox.h"
#include "Controls/TextLabel.h"
#include "Controls/SliderControl.h"
#include "Controls/ImageControls/Image.h"
#include "Controls/CnfgButton.h"
#include "Controls/InfoPage.h"
#include "ApplController.h"


typedef enum
{
    APP_SET_CLOSE_BUTTON,
    APP_SET_DISP_SPINBOX,
    APP_SET_BRIGHTNESS_SLIDER,
    APP_SET_BRIGHTNESS_TEXTBOX,
    APP_SET_CONTRAST_SLIDER,
    APP_SET_CONTRAST_TEXTBOX,
    APP_SET_SATURATION_SLIDER,
    APP_SET_SATURATION_TEXTBOX,
    APP_SET_HUE_SLIDER,
    APP_SET_HUE_TEXTBOX,
    APP_SET_DEFAULT_BUTTON,
    APP_SET_REFRESH_BUTTON,
    APP_SET_SAVE_BUTTON,

    MAX_APPEARANCE_SETTINGS_CTRL
}APPEARANCE_SETTINGS_CTRL_e;


class AppearanceSetting : public KeyBoard
{
    Q_OBJECT
public:
    explicit AppearanceSetting(QWidget *parent = 0,DISPLAY_TYPE_e currentSelectedId = MAIN_DISPLAY);
    ~AppearanceSetting();

    void createDefaultElements();

    PHYSICAL_DISPLAY_TYPE_e getInterfaceIndex();
    void getApperanceParameter();
    bool saveApperanceParam();

    void setAppearnceActivity(PHYSICAL_DISPLAY_SCREEN_PARAM_TYPE_e displayParam,
                              quint32 currValueInInt);

    void displayTypeSupportedByDevice();
    void takeLeftKeyAction();
    void takeRightKeyAction();
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void functionKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    void showEvent (QShowEvent *event);
    void paintEvent (QPaintEvent *);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);
    virtual void ctrl_D_KeyPressed(QKeyEvent *event);
    virtual void ctrl_S_KeyPressed(QKeyEvent *event);

signals:
    void sigObjectDelete();

public slots:
    void slotButtonClick(int index);
    void slotValueChanged(int changedValue, int indexInPage, bool sliderMove);
    void slotUpdateCurrentElement(int index);
    void slotSpinBoxValueChanged(QString,quint32);
    void slotTextBoxValueChange(QString,int indexInPage);
    void slotInfoPageCnfgBtnClick(int index);

public:
    quint32              m_width;
    quint32              m_height;

    Rectangle*          m_backGround;
    CloseButtton*       m_closeButton;
    Heading*            m_heading;

    ApplController*     m_applController;
    DISPLAY_PARAM_t     m_displayparam;
    DEV_TABLE_INFO_t    m_devTableInfo;
    PHYSICAL_DISPLAY_TYPE_e m_currentDisplayType;

    QStringList         m_displayList;
    DropDown*           m_displayDropDownBox;

    SliderControl*      m_sliderBar[PHYSICAL_DISPLAY_SCREEN_PARAM_MAX];

    Image*              m_barBackgroungImage[PHYSICAL_DISPLAY_SCREEN_PARAM_MAX];
    float               m_valueMultipler;
    TextBox*            m_valueTextbox[PHYSICAL_DISPLAY_SCREEN_PARAM_MAX];
    TextboxParam*       m_textboxParam[PHYSICAL_DISPLAY_SCREEN_PARAM_MAX];
    TextLabel*          m_featureLabel[PHYSICAL_DISPLAY_SCREEN_PARAM_MAX];

    CnfgButton*         m_defaultButton;
    CnfgButton*         m_refreshButton;
    CnfgButton*         m_saveButton;

    NavigationControl*  m_elementList[MAX_APPEARANCE_SETTINGS_CTRL];
    quint8              m_currentElement;

    InfoPage*           m_infoPage;
    quint8              m_currentClickButton;

};

#endif // APPEARANCESETTING_H
