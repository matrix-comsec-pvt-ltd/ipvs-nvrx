#ifndef PRESETTOUR_H
#define PRESETTOUR_H

#include "Controls/ConfigPageControl.h"
#include "Controls/SpinBox.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/PageOpenButton.h"
#include "Controls/ElementHeading.h"
#include "Controls/TextBox.h"
#include "Controls/TableCell.h"
#include "Controls/ControlButton.h"
#include "DataStructure.h"
#include "Controls/PickList.h"
#include "Controls/DropDown.h"
#include "Controls/TextLabel.h"

#include "ConfigPages/CameraSettings/PTZSchd.h"

//#define MAX_PRESET_POS                  30
#define MAX_PRESET_PER_CAM              5
#define MAX_AUTO_PRE_TOUR_END_FIELD     83
#define MAX_TOUR_SCHD_END_FEILDS        8
#define MAX_MANUAL_PRESET_END_FEILDS    3

#define MAX_TABLE_CELL                  12
#define MAX_TABLE_ELEMENTS              10
#define MAX_PRESET_POS_SELECT           40
#define MAX_PRESET_TOUR_WEEKDAYS        7

class PresetTour : public ConfigPageControl
{
    Q_OBJECT
public:
    explicit PresetTour(QString deviceName,
                        QWidget* parent = 0,
                        DEV_TABLE_INFO_t *devTabInfo = NULL);
    ~PresetTour();

    void createDefaultComponents();
    void getCameraList();
    void getAllTourName();
    void getConfigOfPresetTour ();

    void createPayload(REQ_MSG_ID_e msgType );
    void getConfig();
    void getConfig1();
    void defaultConfig();
    void saveConfig();
    void saveConfigFeilds ();
    void fillPresetPosition(quint8 pageNumber = 0);
    void sendCommand(SET_COMMAND_e cmdType, quint8 totalfeilds);

    void processDeviceResponse (DevCommParam *param, QString deviceName);
    bool isUserChangeConfig();
    void handleInfoPageMessage(int index);

signals:
    
public slots:
    void slotButtonClick(int);
    void slotSubObjectDel();
    void slotDropDownValueChange(QString,quint32);

private:

    DropDown*               cameraListDropDownBox;
    QMap<quint8, QString>   cameraList;
    OptionSelectButton*     overrideCheckBox;

    OptionSelectButton*     overrideTourByControlCheckBox;

    DropDown*               manualPresetTourDropDownBox;
    QMap<quint8, QString>   manualPresetTourList;
    PageOpenButton*         weeklySchedule;

    ElementHeading*         elementHeading;
    DropDown*               tourNumDropDownBox;
    QMap<quint8, QString>   tourNumList;
    TextboxParam*           tourNameParam;
    TextBox*                tourNameTextbox;
    TextboxParam*           pauseTextBoxParam;
    TextBox*                pauseTextBox;
    DropDown*               viewOrderDropDownBox;

    BgTile*                 middleTitle[MAX_PRESET_TOUR_WEEKDAYS + 1];

    TableCell*          numbers[MAX_TABLE_CELL];
    TextLabel*          numberLabel[MAX_TABLE_CELL];
    TableCell*          presetPosition[MAX_TABLE_CELL];
    PickList*           presetPick[MAX_TABLE_ELEMENTS];
    QMap<quint8, QString> presetPositionList;
    TextLabel*          presetPositionLabel[2];
    TableCell*          viewTime[MAX_TABLE_CELL];
    TextLabel*          viewTimeLabel[2];
    TextboxParam*       viewTimeTextBoxParam[MAX_TABLE_ELEMENTS];
    TextBox*            viewTimeTextBox[MAX_TABLE_ELEMENTS];

    ControlButton*      prevButton;
    ControlButton*      nextButton;

    PTZSchd*            ptzSchd;

    QStringList         currentPresetViewTime;
    quint8              currentPresetPositionIndex[MAX_PRESET_POS_SELECT];

    bool isNextPageSelect;
    bool isPTZsupport;
    bool reqTourName;
    quint8 currentCameraIndex;
    quint16 cnfg1FrmIndx;
    quint16 cnfg1ToIndx;
    quint16 cnfg2FrmIndx;
    quint16 cnfg2ToIndx;
    quint16 cnfg3FrmIndx;
    quint16 cnfg3ToIndx;
    quint16 cnfg4FrmIndx;
    quint16 cnfg4ToIndx;

    bool entireDaySchdSelect[MAX_PRESET_TOUR_WEEKDAYS];
    quint8 ptzTour[MAX_PRESET_POS];
    QStringList startTime1List;
    QStringList startTime2List;
    QStringList stopTime1List;
    QStringList stopTime2List;

    quint8 currentPageNo;
    quint8 previousPageNo;

    bool isCnfgTourReq;
    quint8        m_currentDropBoxChangeType;
    quint8        m_currentTourNumber;
};

#endif // PRESETTOUR_H

