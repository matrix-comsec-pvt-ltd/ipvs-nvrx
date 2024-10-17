#ifndef STORAGEMANAGMENT_H
#define STORAGEMANAGMENT_H

#include "Controls/ConfigPageControl.h"
#include "Controls/ElementHeading.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/PageOpenButton.h"
#include "Controls/TextBox.h"
#include "DataStructure.h"

#include "ConfigPages/StorageAndBackupSettings/RecordRetention.h"

typedef enum{

    ENBL_ALRT_STOP_REC,
    ENBL_OVER_FILE,
    ENBL_CLN_OLDER_FILE,

    MAX_STR_MNG_STORG_MODE
}STR_MNG_STORG_MODE_e;

class StorageManagment : public ConfigPageControl
{
    Q_OBJECT
public:
    explicit StorageManagment(QString deviceName, QWidget *parent = 0,
                              DEV_TABLE_INFO_t *devTabInfo = NULL);
    ~StorageManagment();

    void createDefaultComponents ();
    void getCameraList();

    void fillConfigData();
    void createPayload(REQ_MSG_ID_e msgType);
    void getConfig ();
    void saveConfig ();
    void defaultConfig ();
    void setConfigFields();

    void processDeviceResponse (DevCommParam *param, QString deviceName);
signals:
    
public slots:
    void slotSelectionButtonClick(OPTION_STATE_TYPE_e,int);
    void slotButtonClick(int index);
    void slotSubOjectDelete(quint8);
    void slotLoadInfoPage(int,INFO_MSG_TYPE_e);

private:

    QStringList cameraList;

    ElementHeading*     elementHeading[2];

    OptionSelectButton* storageOptions[MAX_STR_MNG_STORG_MODE];
    TextBox*            storageTextBox;
    TextboxParam*       storageTextBoxParam;

    OptionSelectButton* recordingRetation;
    OptionSelectButton* recordingDrive;
    TextBox*            recordingTextBox;
    TextboxParam*       recordingTextBoxParam;
    OptionSelectButton* recordingRetationCameraWise;
    PageOpenButton*     recordingSelectCamera;

    OptionSelectButton* backupRetation;
    PageOpenButton*     backupSelectCamera;

    OptionSelectButton* storageAlert;
    TextBox*            storageAlertTextBox;
    TextboxParam*       storageAlertTextBoxParam;

    QStringList recordingRetationValues;
    QStringList backupRetationValues;

    RecordRetention* recordRetSelCam;
    RecordRetention* backupRetSelCam;

    
};

#endif // STORAGEMANAGMENT_H
