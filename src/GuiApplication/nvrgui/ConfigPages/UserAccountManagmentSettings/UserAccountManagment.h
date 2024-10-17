#ifndef USERACCOUNTMANAGMENT_H
#define USERACCOUNTMANAGMENT_H

#include <QWidget>
#include "Controls/ConfigPageControl.h"

#include "Controls/DropDown.h"
#include "Controls/ControlButton.h"
#include "Controls/ElementHeading.h"
#include "Controls/TextBox.h"
#include "Controls/PasswordTextbox.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/TableCell.h"
#include "Controls/TextWithList.h"
#include "Controls/TextWithBackground.h"
#include "Controls/TextLabel.h"

#define MAX_PAGE_NUMBER     4
#define MAX_CAMERA_ON_PAGE  8

class UserAccountManagment : public ConfigPageControl
{
    Q_OBJECT

public:
    explicit UserAccountManagment(QString devName,QWidget *parent = 0,
                         DEV_TABLE_INFO_t *tableInfo = NULL);
    ~UserAccountManagment();
    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void getConfig();
    void defaultConfig();
    void saveConfig();
    void handleInfoPageMessage(int);

public slots:
    void slotCheckBoxClicked(OPTION_STATE_TYPE_e,int);
    void slotSpinBoxValueChange(QString,quint32);
    void slotControlBtnClicked(int);
    void slotTextBoxLoadInfopage(int index, INFO_MSG_TYPE_e msgType);
    void slotPageNumberButtonClick(QString);

private:
    // private variables
    quint8                      maxCameraNum;
    quint8                      maxCameraOnPage;
    quint8                      currentGroupIndex;
    quint8                      m_devListCount;
    quint8                      networkDeviceIndex;
    quint8                      m_minPassLen;
    quint8                      totalPages;
    quint8                      clickedUserButton;
    quint8                      currentPageNum;
    quint32                     cnfgToIndex[MAX_DEVICES];
    quint32                     cnfgFromIndex[MAX_DEVICES];
    quint32                     currentUserIndex[MAX_DEVICES];
    QString                     networkDevName;
    QString                     currentLangIndex;
    QString                     currentPassword;
    QString                     userName;
    QString                     password;
    QString                     cnfrmPassword;
    QStringList                 cameraList;
    QStringList                 m_devList;
    QStringList                 deviceUserStringList[MAX_DEVICES];
    QMap<quint8, QString>       dropDownUserStringList[MAX_DEVICES];
    QMap<quint8, QString>       grpStringList;
    QMap<quint8, QString>       lanStringList;

    TextboxParam*               userListParam;
    TextWithList*               userListDropDown;
    ControlButton*              createBtn;
    ControlButton*              editBtn;
    ControlButton*              deleteBtn;
    ElementHeading*             elementHeading1;
    TextboxParam*               usernameParam;
    TextBox*                    usernameTextBox;
    OptionSelectButton*         enableUserCheckBox;
    TextboxParam*               passwordParam;
    PasswordTextbox*            passwordTextBox;
    OptionSelectButton*         multiLoginCheckBox;
    TextboxParam*               confirmpasswordParam;
    PasswordTextbox*            confirmpasswordTextBox;
    DropDown*                   groupDropDownBox;
    DropDown*                   languageDropDownBox;
    TextboxParam*               accessTimeLimitParam;
    TextBox*                    accessTimeLimitTextBox;
    OptionSelectButton*         enableSynDevicescheckBox;
    OptionSelectButton*         pushNotificationCheckBox;
    BgTile*                     bgTile_m;
    TableCell*                  tabelCellC1_H;
    TableCell*                  tabelCellC2_H;
    TableCell*                  tabelCellC3_H;
    TableCell*                  tabelCellC4_H;
    TableCell*                  tabelCellC5_H;
    TableCell*                  tabelCellC6_H;
    TableCell*                  tabelCellC7_H;
    TextLabel*                  textLabelC1_H;
    OptionSelectButton*         allMonitoringCheckBox;
    OptionSelectButton*         allPlaybackCheckBox;
    OptionSelectButton*         allPTZCheckBox;
    OptionSelectButton*         allAudioInCheckBox;
    OptionSelectButton*         allAudioOutCheckBox;
    OptionSelectButton*         allVideoCheckBox;
    TableCell*                  tabelCellC1[MAX_CAMERA_ON_PAGE];
    TextLabel*                  textLabelC1[MAX_CAMERA_ON_PAGE];
    TableCell*                  tabelCellC2[MAX_CAMERA_ON_PAGE];
    OptionSelectButton*         perCamMonitorCheckBox[MAX_CAMERA_ON_PAGE];
    TableCell*                  tabelCellC3[MAX_CAMERA_ON_PAGE];
    OptionSelectButton*         perCamPlaybackCheckBox[MAX_CAMERA_ON_PAGE];
    TableCell*                  tabelCellC4[MAX_CAMERA_ON_PAGE];
    OptionSelectButton*         perCamPTZCheckBox[MAX_CAMERA_ON_PAGE];
    TableCell*                  tabelCellC5[MAX_CAMERA_ON_PAGE];
    OptionSelectButton*         perCamAudioInCheckBox[MAX_CAMERA_ON_PAGE];
    TableCell*                  tabelCellC6[MAX_CAMERA_ON_PAGE];
    OptionSelectButton*         perCamAudioOutCheckBox[MAX_CAMERA_ON_PAGE];
    TableCell*                  tableCellC7[MAX_CAMERA_ON_PAGE];
    OptionSelectButton*         perCamVideoPopupCheckBox[MAX_CAMERA_ON_PAGE];
    TextWithBackground*         m_PageNumberLabel[MAX_PAGE_NUMBER];
    BgTile*                     bgTile_Bottom;
    ControlButton*              prevButton;
    TextLabel*                  prevLabel;
    ControlButton*              nextButton;

    bool                        userDeleteCmd;
    bool                        nextPageSelected;
    bool                        perCamMonitorCheckBoxStatus[MAX_CAMERAS];
    bool                        perCamPlaybackCheckBoxStatus[MAX_CAMERAS];
    bool                        perCamPTZCheckBoxStatus[MAX_CAMERAS];
    bool                        perCamAudioInCheckBoxStatus[MAX_CAMERAS];
    bool                        perCamAudioOutCheckBoxStatus[MAX_CAMERAS];
    bool                        perCamVideoPopupCheckBoxStatus[MAX_CAMERAS];
    bool                        userListReq[MAX_DEVICES];
    bool                        isValidationSuceess;
    bool                        isInitDone;
    bool                        ntwrkDevicesAvailable;
    bool                        syncNtwrkDevice;
    bool                        m_userChange;

    // private Fuctions
    void intialMode(quint8 currDeviceIndex);
    void createDefaultComponents();
    void getUserConfig(quint8 nwDeviceIndex);
    void showFieldData();
    void setAllCheckBox();
    void changeAllCheckboxState();
    bool setCnfgField(quint8 nwDeviceIndex,QString nwDevName);
    void getLanguage();
    void getUserList(quint8 nwDeviceIndex);
    void getUserListForNtwrkDevice(quint8 nwDeviceIndex, QString nwDevName);
};

#endif // USERACCOUNTMANAGMENT_H
