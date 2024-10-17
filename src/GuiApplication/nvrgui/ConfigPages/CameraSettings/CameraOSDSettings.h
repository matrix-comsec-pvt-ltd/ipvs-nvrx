#ifndef CAMERAOSDSETTINGS_H
#define CAMERAOSDSETTINGS_H

#include "Controls/OptionSelectButton.h"
#include "Controls/TextBox.h"
#include "Controls/DropDown.h"
#include "Controls/CnfgButton.h"
#include "Controls/Closebuttton.h"
#include "Controls/Heading.h"
#include "Controls/InfoPage.h"

#include "KeyBoard.h"

#define TEXT_OVERLAY_MIN    1
#define TEXT_OVERLAY_MAX    6

typedef enum
{
    CAMERAOSDSETTINGS_DATETIMEOVERLAY_CHECKBOX,
    CAMERAOSDSETTINGS_DATETIMEPOSITION_SPINBOX,
    CAMERAOSDSETTINGS_TEXTOVERLAY_CHECKBOX,
    CAMERAOSDSETTINGS_TEXTOVERLAY_SPINBOX,
    CAMERAOSDSETTINGS_OSDTEXT_TEXTBOX,
    CAMERAOSDSETTINGS_TEXTPOSITION_SPINBOX,
    CAMERAOSDSETTINGS_CAMERANAMEPOSITION_SPINBOX,
    CAMERAOSDSETTINGS_CAMERASTATUSPOSITION_SPINBOX,
    CAMERAOSDSETTINGS_OKBUTTON,
    CAMERAOSDSETTINGS_CANCELBUTTON,
    CAMERAOSDSETTINGS_CLOSE_BUTTON,
    MAX_CAMERAOSDSETTINGS_ELEMENT

}CAMERAOSDSETTINGS_ELEMENT_e;

typedef struct
{
    OPTION_STATE_TYPE_e dateTimeOverlay;
    OSD_POSITION_e dateTimePosition;
    OPTION_STATE_TYPE_e textOverlay;
    QString text[TEXT_OVERLAY_MAX];
    OSD_POSITION_e textPosition[TEXT_OVERLAY_MAX];
    OSD_POSITION_e cameraNamePosition;
    OSD_POSITION_e cameraStatusPosition;
    CAMERA_TYPE_e cameraType;
}OSD_SETTINGS_t;

class CameraOSDSettings : public KeyBoard
{
    Q_OBJECT

private:
    Rectangle *m_background;
    CloseButtton *m_closeButton;
    Heading *m_pageHeading;
    OptionSelectButton *m_dateTimeOverlayCheckbox, *m_textOverlayCheckbox;
    DropDown *m_dateTimePositionDropDownbox;
    DropDown *m_textOverlayDropDownbox;
    TextboxParam *m_osdTextparam;
    TextBox *m_osdTextbox;
    DropDown *m_textPositionDropDownbox;
    DropDown *m_cameraNamePositionDownbox, *m_cameraStatusPositionDownbox;
    CnfgButton *m_okButton, *m_cancelButton;
    InfoPage *m_infoPage;

    OSD_SETTINGS_t *m_osdSettings;

    QMap<quint8, QString>   m_cameraOSDPositionList, m_graphicOSDPositionList, m_textOverlayList;
    quint16 m_currentElement;
    quint8 m_indexInPage;
    quint8 m_textOverlayIndex;
    quint8 m_textOverlayMax;
    NavigationControl *m_elementList[MAX_CAMERAOSDSETTINGS_ELEMENT];
public:
    CameraOSDSettings(OSD_SETTINGS_t * osdSettings,
                      quint8 indexInPage, quint8,
                      QWidget *parent = NULL);
    ~CameraOSDSettings();

    void saveConfig();

    void takeLeftKeyAction();
    void takeRightKeyAction();

    void paintEvent(QPaintEvent *);
    void showEvent(QShowEvent *event);

    //keyboard support added
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);
    virtual void insertKeyPressed(QKeyEvent *event);

signals:
    void sigDeleteObject(quint8 indexInPage);

public slots:
    void slotOptionButtonSelected(OPTION_STATE_TYPE_e currentState, int indexInPage);
    void slotUpdateCurrentElement(int indexInPage);
    void slotConfigButtonClicked(int indexInPage);
    void slotCloseButtonClicked(int);
    void slotInfoPageBtnclick(int);
    void slotTextBoxLoadInfopage(int indexInPage, INFO_MSG_TYPE_e msgType);
    void slotSpinBoxValueChanged(QString, quint32);
};

#endif // CAMERAOSDSETTINGS_H
