#ifndef MXANALOGPRESETMENU_H
#define MXANALOGPRESETMENU_H

#include <QTime>
#include <QWidget>
#include "DataStructure.h"
#include "ApplController.h"
#include "PayloadLib.h"

#include "Controls/BackGround.h"
#include "Controls/TextBox.h"
#include "Controls/TextLabel.h"
#include "Controls/InvisibleWidgetCntrl.h"
#include "Controls/CnfgButton.h"

typedef enum{
   ANALOG_PRESET_CLS_BTN =0,
   ANALOG_PRESET_POSITION_TXTBOX,
   ANALOG_PRESET_GO_BUTTON,
   MAX_ANALOG_PRESET_CTRL
}ANALOG_PRESET_CTRL_e;


class MxAnalogPresetMenu : public BackGround
{
    Q_OBJECT
public:
    explicit MxAnalogPresetMenu(qint32 startx,
                       qint32 starty,
                       QString deviceName,
                       quint8 cameraNum,
                       QWidget *parent = 0);


    ~MxAnalogPresetMenu();

    void createCmdPayload(SET_COMMAND_e cmdType,quint8 totalFields = 0);
    void createDefaultComponent();
    void sendSetPresetGoCmd();
    void processDeviceResponse(DevCommParam *param, QString deviceName);

private:
    ApplController*             applController;
    PayloadLib*                 payloadLib;
    qint32                      m_startX;
    qint32                      m_startY;
    QString                     currentDeviceName;
    QString                     m_presetValueStr;
    quint8                      camNo;
    quint16                     m_presetValue;
    NavigationControl*          m_elementList[MAX_ANALOG_PRESET_CTRL];
    quint8                      currentButtonClick;
    quint8                      m_currentElement;
    CnfgButton*                 m_goButton;
    BgTile*                     imageTopBgTile;
    TextboxParam*               m_analogPresetParam;
    TextBox*                    m_analogPresetPosTextBox;
    TextLabel*                  m_devResponseLabel;
    MxAnalogPresetMenu*         m_analogPresetMenu;


signals:
    void sigObjectDelete();

public slots:
    void slotButtonClicked(int);
    void slotUpdateCurrentElement(int);






};

#endif // MXANALOGPRESETMENU_H
