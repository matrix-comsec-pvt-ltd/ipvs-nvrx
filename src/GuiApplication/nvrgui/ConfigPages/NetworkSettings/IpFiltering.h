#ifndef IPFILTERING_H
#define IPFILTERING_H

#include "Controls/ConfigPageControl.h"
#include "Controls/IpTextBox.h"
#include "Controls/ControlButton.h"
#include "Controls/ElementHeading.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/TextLabel.h"
#include <QHostAddress>
#include <arpa/inet.h>

#define MAX_LIST_IN_ONE_PAGE        8

typedef enum
{
    FILTER_MODE_ALLOW,
    FILTER_MODE_DENY,

    MAX_FILTER_MODE
}FILTER_MODE_e;

class IpFiltering :public ConfigPageControl
{
    Q_OBJECT
private:
    bool                currIpFilterEnableState;
    bool                ipListSelect[MAX_LIST_IN_ONE_PAGE *MAX_LIST_IN_ONE_PAGE];
    FILTER_MODE_e       currFilterMode;
    quint8              totalTick;
    quint8              currPageIndex;

    OptionSelectButton  *enableIpFiltercheckbox;
    OptionSelectButton  *filterTypecheckbox[MAX_FILTER_MODE];

    IpTextBox           *startIpAddrBox;
    IpTextBox           *endIpAddrBox;
    ElementHeading  	*ipRangeEleHeading;
    ElementHeading      *ipListEleHeading;

    ControlButton       *addBtn;
    ControlButton       *prevBtn;
    ControlButton       *nextBtn;
    ControlButton       *deleteBtn;

    OptionSelectButton  *allAddrSelBtn;
    TextLabel           *allSelTextlabel;

    OptionSelectButton  *ipListOptionSelBtn[MAX_LIST_IN_ONE_PAGE];
    TextLabel           *ipListTextlabel[MAX_LIST_IN_ONE_PAGE];

    QStringList         ipAddrList;

public:
    explicit IpFiltering(QString devName, QWidget *parent = 0);
    ~IpFiltering();

    void createDefaultComponent();
    void getConfig ();
    void saveConfig ();
    void defaultConfig();
    void createPayload(REQ_MSG_ID_e requestType);
    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void handleInfoPageMessage(int index = INFO_OK_BTN);

    void setAllEleEnableDisable();
    void deleteBtnAction();
    void showTableList(void);

public slots:
    void slotAllOptionSelBtnClicked(OPTION_STATE_TYPE_e state,int index);
    void slotControlBtnClick(int index);
};

#endif // IPFILTERING_H
