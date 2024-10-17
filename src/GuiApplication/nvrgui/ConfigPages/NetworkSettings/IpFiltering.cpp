#include "IpFiltering.h"
#include "ValidationMessage.h"

#define FIRST_ELE_TOP_MARGIN                    SCALE_HEIGHT(54)
#define FIRST_ELE_LEFT_MARGIN                   SCALE_WIDTH(13)
#define IP_FILTER_APPLY_TABLE_MAX_INDEX         1
#define IP_FILTER_SETTING_TABLE_MAX_INDEX       64
#define LEFT_MARGIN_FROM_CENTER                 SCALE_WIDTH(60)

#define HEADING_IP_RANGE            "IP Range"
#define HEADING_IP_LIST             "IP List"

// List of control
typedef enum
{
    IP_FILTER_ENABLE,
    IP_FILTER_TYPE_ALLOW,
    IP_FILTER_TYPE_DENY,
    IP_FILTER_START_IP,
    IP_FILTER_END_IP,
    IP_FILTER_ADD_BUTTON,
    IP_FILTER_LIST_SEL_ALL,
    IP_FILTER_LIST_SEL_1ST,
    IP_FILTER_LIST_SEL_8TH = IP_FILTER_LIST_SEL_ALL + MAX_LIST_IN_ONE_PAGE,
    IP_FILTER_PREV_BUTTON,
    IP_FILTER_NEXT_BUTTON,
    IP_FILTER_DELETE_BUTTON,

    MAX_IP_FILTER_ELEMETS
}IP_FILTER_ELELIST_e;

typedef enum
{
    FIELD_FILTER_ENABLE =0,
    FIELD_FILTER_TYPE,
    FIELD_START_ADDR,
    FIELD_END_ADDR,

    MAX_IP_FILTER_FIELD = 2
}IP_FILTER_FIELD_e;

const static QString eleLabels[] =
{
    "Enable IP Address Filtering",
    "Allow",
    "Deny",
    "Start IP Address",
    "End IP Address",
    "Add"
};

IpFiltering::IpFiltering(QString devName, QWidget *parent)
    :ConfigPageControl(devName, parent, MAX_IP_FILTER_ELEMETS),
     currIpFilterEnableState(0), currFilterMode(FILTER_MODE_ALLOW), totalTick(0), currPageIndex(0)
{
    createDefaultComponent();
    IpFiltering::getConfig();
}

void IpFiltering::createDefaultComponent ()
{
    enableIpFiltercheckbox = new OptionSelectButton(FIRST_ELE_LEFT_MARGIN,
                                                    FIRST_ELE_TOP_MARGIN,
                                                    BGTILE_MEDIUM_SIZE_WIDTH,
                                                    BGTILE_HEIGHT,
                                                    CHECK_BUTTON_INDEX,
                                                    eleLabels[IP_FILTER_ENABLE],
                                                    this,
                                                    TOP_LAYER,
                                                    SCALE_HEIGHT(10),
                                                    MX_OPTION_TEXT_TYPE_SUFFIX,
                                                    NORMAL_FONT_SIZE,
                                                    IP_FILTER_ENABLE, true, NORMAL_FONT_COLOR, true);
    m_elementList[IP_FILTER_ENABLE] = enableIpFiltercheckbox;
    connect(enableIpFiltercheckbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (enableIpFiltercheckbox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotAllOptionSelBtnClicked(OPTION_STATE_TYPE_e,int)));

    filterTypecheckbox[FILTER_MODE_ALLOW] = new OptionSelectButton(enableIpFiltercheckbox->x (),
                                                                   enableIpFiltercheckbox->y () + enableIpFiltercheckbox->height (),
                                                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                                                   BGTILE_HEIGHT,
                                                                   RADIO_BUTTON_INDEX,
                                                                   this,
                                                                   BOTTOM_TABLE_LAYER,
                                                                   "Filtering Type",
                                                                   eleLabels[IP_FILTER_TYPE_ALLOW],
                                                                   -1,
                                                                   IP_FILTER_TYPE_ALLOW,
                                                                   true,
                                                                   NORMAL_FONT_SIZE,
                                                                   NORMAL_FONT_COLOR);

    filterTypecheckbox[FILTER_MODE_DENY] = new OptionSelectButton(filterTypecheckbox[FILTER_MODE_ALLOW]->x () + SCALE_WIDTH(340),
                                                                  filterTypecheckbox[FILTER_MODE_ALLOW]->y (),
                                                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                                                  BGTILE_HEIGHT,
                                                                  RADIO_BUTTON_INDEX,
                                                                  eleLabels[IP_FILTER_TYPE_DENY],
                                                                  this,
                                                                  NO_LAYER,
                                                                  -1,
                                                                  MX_OPTION_TEXT_TYPE_SUFFIX,
                                                                  NORMAL_FONT_SIZE,
                                                                  IP_FILTER_TYPE_DENY);

    for(quint8 index = 0; index <= FILTER_MODE_DENY; index++)
    {
        m_elementList[IP_FILTER_TYPE_ALLOW + index] = filterTypecheckbox[index];
        connect(filterTypecheckbox[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect (filterTypecheckbox[index],
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotAllOptionSelBtnClicked(OPTION_STATE_TYPE_e,int)));
    }

    ipRangeEleHeading = new ElementHeading(filterTypecheckbox[FILTER_MODE_ALLOW]->x (),
                                           filterTypecheckbox[FILTER_MODE_ALLOW]->y () +filterTypecheckbox[FILTER_MODE_ALLOW]->height () + SCALE_HEIGHT(6),
                                           BGTILE_MEDIUM_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           HEADING_IP_RANGE,
                                           TOP_LAYER,
                                           this, false, SCALE_WIDTH(20), NORMAL_FONT_SIZE, true);

    startIpAddrBox = new IpTextBox(ipRangeEleHeading->x (),
                                   ipRangeEleHeading->y () + ipRangeEleHeading->height (),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   IP_FILTER_START_IP,
                                   eleLabels[IP_FILTER_START_IP],
                                   IP_ADDR_TYPE_IPV4_AND_IPV6,
                                   this, MIDDLE_TABLE_LAYER,
                                   true, 0, true,
                                   IP_FIELD_TYPE_IPV6_ADDR,
                                   IP_TEXTBOX_ULTRALARGE,
                                   LEFT_MARGIN_FROM_CENTER);

    m_elementList[IP_FILTER_START_IP] = startIpAddrBox;
    connect(startIpAddrBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));


    endIpAddrBox = new IpTextBox(startIpAddrBox->x (),
                                 startIpAddrBox->y () + startIpAddrBox->height (),
                                 BGTILE_MEDIUM_SIZE_WIDTH,
                                 BGTILE_HEIGHT,
                                 IP_FILTER_END_IP,
                                 eleLabels[IP_FILTER_END_IP],
                                 IP_ADDR_TYPE_IPV4_AND_IPV6,
                                 this, MIDDLE_TABLE_LAYER,
                                 true, 0, true,
                                 IP_FIELD_TYPE_IPV6_ADDR,
                                 IP_TEXTBOX_ULTRALARGE,
                                 LEFT_MARGIN_FROM_CENTER);
    m_elementList[IP_FILTER_END_IP] = endIpAddrBox;
    connect(endIpAddrBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    addBtn = new ControlButton(ADD_BUTTON_INDEX,
                               endIpAddrBox->x (),
                               endIpAddrBox->y () + endIpAddrBox->height (),
                               BGTILE_MEDIUM_SIZE_WIDTH,
                               BGTILE_HEIGHT,
                               this, DOWN_LAYER, SCALE_WIDTH(380),
                               eleLabels[IP_FILTER_ADD_BUTTON],
                               true, IP_FILTER_ADD_BUTTON);
    m_elementList[IP_FILTER_ADD_BUTTON] = addBtn;
    connect(addBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (addBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotControlBtnClick(int)));

    ipListEleHeading = new ElementHeading(enableIpFiltercheckbox->x () + enableIpFiltercheckbox->width () + SCALE_WIDTH(6),
                                          enableIpFiltercheckbox->y (),
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT, HEADING_IP_LIST, TOP_LAYER,
                                          this, false, SCALE_WIDTH(20), NORMAL_FONT_SIZE, true);

    allAddrSelBtn = new OptionSelectButton(ipListEleHeading->x (),
                                           ipListEleHeading->y () + ipListEleHeading->height (),
                                           BGTILE_MEDIUM_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           CHECK_BUTTON_INDEX,
                                           this, MIDDLE_TABLE_LAYER,
                                           "", "", SCALE_WIDTH(50), IP_FILTER_LIST_SEL_ALL);
    m_elementList[IP_FILTER_LIST_SEL_ALL] = allAddrSelBtn;
    connect(allAddrSelBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (allAddrSelBtn,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotAllOptionSelBtnClicked(OPTION_STATE_TYPE_e,int)));

    allSelTextlabel = new TextLabel(allAddrSelBtn->x () + SCALE_WIDTH(90),
                                    (allAddrSelBtn->y () + (allAddrSelBtn->height ()/2)),
                                    NORMAL_FONT_SIZE,
                                    "",
                                    this,
                                    NORMAL_FONT_COLOR,
                                    NORMAL_FONT_FAMILY,
                                    ALIGN_START_X_CENTRE_Y);

    for(quint8 index = 0; index < MAX_LIST_IN_ONE_PAGE; index++)
    {
        ipListOptionSelBtn[index] = new OptionSelectButton(allAddrSelBtn->x (),
                                                           allAddrSelBtn->y () + allAddrSelBtn->height () +
                                                           (index * BGTILE_HEIGHT),
                                                           BGTILE_MEDIUM_SIZE_WIDTH,
                                                           BGTILE_HEIGHT,
                                                           CHECK_BUTTON_INDEX,
                                                           this, MIDDLE_TABLE_LAYER,
                                                           "", "", SCALE_WIDTH(50),
                                                           (IP_FILTER_LIST_SEL_1ST+ index));

        m_elementList[IP_FILTER_LIST_SEL_1ST+ index] = ipListOptionSelBtn[index];
        connect(ipListOptionSelBtn[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        connect (ipListOptionSelBtn[index],
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotAllOptionSelBtnClicked(OPTION_STATE_TYPE_e,int)));

        ipListTextlabel[index] = new TextLabel(ipListOptionSelBtn[index]->x () + SCALE_WIDTH(90),
                                               ipListOptionSelBtn[index]->y () + (ipListOptionSelBtn[index]->height ()/2),
                                               NORMAL_FONT_SIZE,
                                               "",
                                               this,
                                               NORMAL_FONT_COLOR,
                                               NORMAL_FONT_FAMILY,
                                               ALIGN_START_X_CENTRE_Y, 0, false, 370);
    }

    prevBtn = new ControlButton(PREVIOUS_BUTTON_INDEX,
                                ipListOptionSelBtn[MAX_LIST_IN_ONE_PAGE-1]->x (),
                                ipListOptionSelBtn[MAX_LIST_IN_ONE_PAGE-1]->y () + ipListOptionSelBtn[MAX_LIST_IN_ONE_PAGE-1]->height (),
                                BGTILE_MEDIUM_SIZE_WIDTH,
                                BGTILE_HEIGHT,
                                this, DOWN_LAYER,
                                SCALE_WIDTH(20),
                                "Previous",
                                true,
                                IP_FILTER_PREV_BUTTON,
                                false);
    m_elementList[IP_FILTER_PREV_BUTTON] = prevBtn;
    connect(prevBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (prevBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotControlBtnClick(int)));

    nextBtn = new ControlButton(NEXT_BUTTON_INDEX,
                                prevBtn->x () + SCALE_WIDTH(170),
                                prevBtn->y (),
                                BGTILE_MEDIUM_SIZE_WIDTH,
                                BGTILE_HEIGHT, this, NO_LAYER,
                                0, "Next", true, IP_FILTER_NEXT_BUTTON);
    m_elementList[IP_FILTER_NEXT_BUTTON] = nextBtn;
    connect(nextBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (nextBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotControlBtnClick(int)));

    deleteBtn = new ControlButton(DELETE_BUTTON_INDEX,
                                  prevBtn->x () + SCALE_WIDTH(380),
                                  prevBtn->y (),
                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                  BGTILE_HEIGHT, this, NO_LAYER, 0,
                                  "Delete", true, IP_FILTER_DELETE_BUTTON);
    m_elementList[IP_FILTER_DELETE_BUTTON] = deleteBtn;
    connect(deleteBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (deleteBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotControlBtnClick(int)));
}

IpFiltering::~IpFiltering()
{
    disconnect (enableIpFiltercheckbox,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotAllOptionSelBtnClicked(OPTION_STATE_TYPE_e,int)));
    disconnect(enableIpFiltercheckbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete enableIpFiltercheckbox;

    for(quint8 index = 0; index <= FILTER_MODE_DENY; index++)
    {
        disconnect (filterTypecheckbox[index],
                    SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                    this,
                    SLOT(slotAllOptionSelBtnClicked(OPTION_STATE_TYPE_e,int)));
        disconnect(filterTypecheckbox[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        delete filterTypecheckbox[index];
    }

    delete ipRangeEleHeading;

    disconnect(startIpAddrBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete startIpAddrBox;

    disconnect(endIpAddrBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete endIpAddrBox;

    disconnect (addBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotControlBtnClick(int)));
    disconnect(addBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete addBtn;

    delete ipListEleHeading;

    disconnect (allAddrSelBtn,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotAllOptionSelBtnClicked(OPTION_STATE_TYPE_e,int)));
    disconnect(allAddrSelBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete allAddrSelBtn;

    delete allSelTextlabel;

    for(quint8 index = 0; index < MAX_LIST_IN_ONE_PAGE; index++)
    {
        disconnect (ipListOptionSelBtn[index],
                    SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                    this,
                    SLOT(slotAllOptionSelBtnClicked(OPTION_STATE_TYPE_e,int)));
        disconnect(ipListOptionSelBtn[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        delete ipListOptionSelBtn[index];

        delete ipListTextlabel[index];
    }

    disconnect (prevBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotControlBtnClick(int)));
    disconnect(prevBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete prevBtn;

    disconnect (nextBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotControlBtnClick(int)));
    disconnect(nextBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete nextBtn;

    disconnect (deleteBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotControlBtnClick(int)));
    disconnect(deleteBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete deleteBtn;
}

void IpFiltering::getConfig ()
{
    currPageIndex = 0;
    startIpAddrBox->setIpaddress ("");
    endIpAddrBox->setIpaddress ("");
    allAddrSelBtn->changeState (OFF_STATE);
    for(quint8 index = 0; index < IP_FILTER_SETTING_TABLE_MAX_INDEX; index++)
    {
        ipListSelect[index] = false;
    }
    createPayload (MSG_GET_CFG);
}

void IpFiltering::defaultConfig()
{
    createPayload(MSG_DEF_CFG);
}

void IpFiltering::saveConfig ()
{
    QStringList tempList;
    quint8 row;
    if((currIpFilterEnableState == true) && (!ipAddrList.length ()))
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(IP_FILTER_ENABLE_MODE));
        getConfig();
        return;
    }
    else
    {
        payloadLib->setCnfgArrayAtIndex(FIELD_FILTER_ENABLE,
                                        ((currIpFilterEnableState == true)?1:0));

        payloadLib->setCnfgArrayAtIndex(FIELD_FILTER_TYPE,
                                        currFilterMode);

        for(row = 0 ; row < ipAddrList.length (); row++)
        {
            tempList = ipAddrList.at (row).split (" - ");
            payloadLib->setCnfgArrayAtIndex(FIELD_START_ADDR + (2 * row),
                                            tempList.at (0));

            payloadLib->setCnfgArrayAtIndex(FIELD_END_ADDR + (2 * row),
                                            tempList.at (1));
        }

        for(quint8 index = row; index < (IP_FILTER_SETTING_TABLE_MAX_INDEX);
            index++)
        {
            payloadLib->setCnfgArrayAtIndex(FIELD_START_ADDR + (2 * index),
                                            "");

            payloadLib->setCnfgArrayAtIndex(FIELD_END_ADDR + (2 * index),
                                            "");
        }
    }
    processBar->loadProcessBar();
    createPayload(MSG_SET_CFG);
}

void IpFiltering::createPayload (REQ_MSG_ID_e requestType)
{
    QString payloadString = payloadLib->createDevCnfgPayload(requestType,
                                                             IP_FILTER_APPLY_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             IP_FILTER_APPLY_TABLE_MAX_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_IP_FILTER_FIELD,
                                                             MAX_IP_FILTER_FIELD);

    payloadString = payloadLib->createDevCnfgPayload(requestType,
                                                     IP_FILTER_TABLE_INDEX,
                                                     CNFG_FRM_INDEX,
                                                     IP_FILTER_SETTING_TABLE_MAX_INDEX,
                                                     CNFG_FRM_FIELD,
                                                     MAX_IP_FILTER_FIELD,
                                                     (MAX_IP_FILTER_FIELD * IP_FILTER_SETTING_TABLE_MAX_INDEX),
                                                     payloadString,
                                                     MAX_IP_FILTER_FIELD);
    DevCommParam* param = new DevCommParam();

    param->msgType = requestType;
    param->payload = payloadString;

    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}


void IpFiltering::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    QString tempStrIp1, totalStrAppend;
    if(deviceName == currDevName)
    {
        switch(param->deviceStatus)
        {
        case CMD_SUCCESS:
            switch(param->msgType)
            {
            case MSG_GET_CFG:
                payloadLib->parsePayload(param->msgType, param->payload);
                if((payloadLib->getcnfgTableIndex (0) == IP_FILTER_APPLY_TABLE_INDEX) &&
                        (payloadLib->getcnfgTableIndex (1) == IP_FILTER_TABLE_INDEX))
                {
                    currIpFilterEnableState = (bool)
                            (payloadLib->getCnfgArrayAtIndex (FIELD_FILTER_ENABLE).toUInt ());
                    currFilterMode = (FILTER_MODE_e)
                            (payloadLib->getCnfgArrayAtIndex (FIELD_FILTER_TYPE).toUInt ());

                    enableIpFiltercheckbox->changeState ((currIpFilterEnableState == true)?ON_STATE:
                                                                                           OFF_STATE);

                    if(currFilterMode == FILTER_MODE_ALLOW)
                    {
                        filterTypecheckbox[FILTER_MODE_ALLOW]->changeState (ON_STATE);
                        filterTypecheckbox[FILTER_MODE_DENY]->changeState (OFF_STATE);
                    }
                    else
                    {
                        filterTypecheckbox[FILTER_MODE_ALLOW]->changeState (OFF_STATE);
                        filterTypecheckbox[FILTER_MODE_DENY]->changeState (ON_STATE);
                    }

                    ipAddrList.clear ();

                    for(quint8 row = 0 ; row < IP_FILTER_SETTING_TABLE_MAX_INDEX; row++)
                    {
                        tempStrIp1 = payloadLib->getCnfgArrayAtIndex (FIELD_START_ADDR + (2 * row)).toString ();
                        if(tempStrIp1 != "")
                        {
                            totalStrAppend = tempStrIp1 + " - " +
                                    payloadLib->getCnfgArrayAtIndex (FIELD_END_ADDR + (2 * row)).toString ();
                            ipAddrList.append (totalStrAppend);
                        }
                    }
                    setAllEleEnableDisable();

                    if(ipAddrList.length () >= IP_FILTER_SETTING_TABLE_MAX_INDEX)
                    {
                        addBtn->setIsEnabled (false);
                    }
                    showTableList ();
                }
                processBar->unloadProcessBar ();
                break;

            case MSG_SET_CFG:
                // unload processing icon
                processBar->unloadProcessBar ();
                //load info page with msg
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                break;

            case MSG_DEF_CFG:
                getConfig();
                break;

            default:
                break;
            }
            break;

        default:
            processBar->unloadProcessBar ();
            infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            break;
        }
        update ();
    }
}

void IpFiltering::setAllEleEnableDisable()
{
    for(quint8 index = 0; index < MAX_FILTER_MODE; index++)
    {
        filterTypecheckbox[index]->setIsEnabled (currIpFilterEnableState);
    }
    startIpAddrBox->setIsEnabled (currIpFilterEnableState);
    endIpAddrBox->setIsEnabled (currIpFilterEnableState);
    addBtn->setIsEnabled (currIpFilterEnableState);
}

void IpFiltering::showTableList()
{
    quint8 ipRangeIndex;
    quint8 listLength = ipAddrList.length ();
    for(quint8 index =0;  index < MAX_LIST_IN_ONE_PAGE; index++)
    {
        ipListOptionSelBtn[index]->setIsEnabled (false);
        ipListTextlabel[index]->changeText ("");
        ipListOptionSelBtn[index]->changeState (OFF_STATE);
    }

    deleteBtn->setIsEnabled (false);
    if(!listLength)
    {
        allAddrSelBtn->setIsEnabled (false);
        allSelTextlabel->changeText ("");

        prevBtn->setIsEnabled (false);
        nextBtn->setIsEnabled (false);
    }
    else
    {
        allAddrSelBtn->setIsEnabled (true);
        allSelTextlabel->changeText ("All");

        for(ipRangeIndex = 0; ipRangeIndex < (IP_FILTER_SETTING_TABLE_MAX_INDEX); ipRangeIndex++)
        {
            if(ipListSelect[ipRangeIndex] == true)
            {
                deleteBtn->setIsEnabled (true);
                break;
            }
        }

        if(!nextBtn->hasFocus ())
        {
            nextBtn->setIsEnabled (false);
        }
        if(!prevBtn->hasFocus ())
        {
            prevBtn->setIsEnabled (false);
        }

        if(listLength <= (MAX_LIST_IN_ONE_PAGE *(currPageIndex + 1)))
        {
            for(quint8 index = (MAX_LIST_IN_ONE_PAGE * currPageIndex); index < listLength; index++)
            {
                ipRangeIndex = index - (MAX_LIST_IN_ONE_PAGE * currPageIndex);
                ipListOptionSelBtn[ipRangeIndex]->setIsEnabled (true);
                ipListTextlabel[ipRangeIndex]->changeText (ipAddrList.at (index));
                if(ipListSelect[index] == true)
                {
                    ipListOptionSelBtn[ipRangeIndex]->changeState (ON_STATE);
                }
            }

            if(currPageIndex != 0)
            {
                if(!prevBtn->hasFocus ())
                {
                    prevBtn->setIsEnabled (true);
                }
            }

            if(nextBtn->hasFocus ())
            {
                m_currentElement = IP_FILTER_PREV_BUTTON;
                m_elementList[m_currentElement]->forceActiveFocus ();
            }
            nextBtn->setIsEnabled (false);
        }
        else
        {
            for(quint8 index = (MAX_LIST_IN_ONE_PAGE * currPageIndex);
                index < (MAX_LIST_IN_ONE_PAGE *(currPageIndex + 1)); index++)
            {
                ipRangeIndex = index - (MAX_LIST_IN_ONE_PAGE * currPageIndex);
                ipListOptionSelBtn[ipRangeIndex]->setIsEnabled (true);
                ipListTextlabel[ipRangeIndex]->changeText (ipAddrList.at (index));
                if(ipListSelect[index] == true)
                {
                    ipListOptionSelBtn[ipRangeIndex]->changeState (ON_STATE);
                }
            }

            if(currPageIndex != 7)
            {
                if(!nextBtn->hasFocus ())
                {
                    nextBtn->setIsEnabled (true);
                }
            }

            if(currPageIndex > 0)
            {
                if(!prevBtn->hasFocus ())
                {
                    prevBtn->setIsEnabled (true);
                }
            }
            else
            {
                if(prevBtn->hasFocus ())
                {
                    m_currentElement = IP_FILTER_NEXT_BUTTON;
                    m_elementList[m_currentElement]->forceActiveFocus ();
                }
                prevBtn->setIsEnabled (false);
            }
        }
    }
    update ();
}

void IpFiltering::slotAllOptionSelBtnClicked(OPTION_STATE_TYPE_e state,int index)
{
    if(index == IP_FILTER_ENABLE)
    {
        currIpFilterEnableState = state;
        setAllEleEnableDisable();
    }
    else if(index == IP_FILTER_TYPE_ALLOW)
    {
        if(state == ON_STATE)
        {
            currFilterMode = FILTER_MODE_ALLOW;
            filterTypecheckbox[FILTER_MODE_DENY]->changeState (OFF_STATE);
        }
    }
    else if(index == IP_FILTER_TYPE_DENY)
    {
        if(state == ON_STATE)
        {
            currFilterMode = FILTER_MODE_DENY;
            filterTypecheckbox[FILTER_MODE_ALLOW]->changeState (OFF_STATE);
        }
    }
    else if(index == IP_FILTER_LIST_SEL_ALL)
    {
        for(quint8 index = 0; index < ipAddrList.length (); index++)
        {
            ipListSelect[index] = state;
        }
        showTableList ();
    }
    else
    {
        if(state == ON_STATE)
        {
            ipListSelect[(currPageIndex*MAX_LIST_IN_ONE_PAGE) + (index - IP_FILTER_LIST_SEL_1ST)] = true;

            totalTick++;
            if(totalTick == ipAddrList.length ())
            {
                allAddrSelBtn->changeState (ON_STATE);
            }
            deleteBtn->setIsEnabled (true);
        }
        else
        {
            ipListSelect[(currPageIndex*MAX_LIST_IN_ONE_PAGE) + (index - IP_FILTER_LIST_SEL_1ST)] = false;
            totalTick--;
            allAddrSelBtn->changeState (OFF_STATE);
            if(totalTick == 0)
            {
                deleteBtn->setIsEnabled (false);
            }
        }
    }
}

void IpFiltering::slotControlBtnClick(int index)
{
    bool isMatchDone = true;
    QString startIpString, endIpString;

    switch(index)
    {
        case IP_FILTER_ADD_BUTTON:
            startIpAddrBox->getIpaddress(startIpString);
            endIpAddrBox->getIpaddress(endIpString);

            if(startIpString == "")
            {
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(IP_FILTER_START_OR_START_END_IP_ADDR));
            }
            else
            {
                if (endIpString == "")
                {
                    endIpString = startIpString;
                    endIpAddrBox->setIpaddress (endIpString);
                }

                do
                {
                    /* Range validation for ipv4 & ipv6 address */
                    if (QHostAddress(startIpString).protocol() == QHostAddress(endIpString).protocol())
                    {

                        if ((startIpString == endIpString) || (*startIpAddrBox < *endIpAddrBox))
                        {
                            startIpString += " - " + endIpString;
                            isMatchDone = ipAddrList.contains (startIpString);
                        }
                        else
                        {
                            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(IP_FILTER_START_IP_LESS_THAN_END_IP));
                            break;
                        }
                    }
                    else
                    {
                        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(IP_FILTER_ENT_SAME_IP_VERSION));
                        break;
                    }

                    if(isMatchDone == false)
                    {
                        if((ipAddrList.length () + 1) > (MAX_LIST_IN_ONE_PAGE * (currPageIndex + 1)))
                        {
                            currPageIndex++;
                            if(currPageIndex < MAX_LIST_IN_ONE_PAGE)
                            {
                                ipAddrList.append (startIpString);
                            }
                        }
                        else
                        {
                            ipAddrList.append (startIpString);
                        }

                        if(ipAddrList.length () >= IP_FILTER_SETTING_TABLE_MAX_INDEX)
                        {
                            addBtn->setIsEnabled (false);
                        }

                        showTableList ();
                    }
                    else
                    {
                        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(IP_FILTER_RANGE));
                    }

                } while(0);
            }

            startIpString = "";
            endIpString = "";
            startIpAddrBox->setIpaddress (startIpString);
            endIpAddrBox->setIpaddress (endIpString);
            break;

        case IP_FILTER_PREV_BUTTON:
            if(currPageIndex)
            {
                currPageIndex--;
            }
            showTableList ();
            break;

        case IP_FILTER_NEXT_BUTTON:
            if(currPageIndex < (MAX_LIST_IN_ONE_PAGE - 1))
            {
                currPageIndex++;
            }
            showTableList ();
            break;


        case IP_FILTER_DELETE_BUTTON:
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(IP_FILTER_DELETE_IP_RANGE), true);
            break;

        default:
             break;
    }
}

void IpFiltering::deleteBtnAction()
{
    QStringList tempDeleteCopy;

    for(quint8 index = 0; index < ipAddrList.length (); index++)
    {
        if(ipListSelect[index] == false)
        {
            tempDeleteCopy.append (ipAddrList.at (index));
        }
        else
        {
            ipListSelect[index] = false;
        }
    }

    ipAddrList = tempDeleteCopy;
    totalTick = 0;

    allAddrSelBtn->changeState (OFF_STATE);

    if(!ipAddrList.length ())
    {
        currPageIndex = 0;
    }
    else
    {
        while(ipAddrList.length () <= (MAX_LIST_IN_ONE_PAGE * (currPageIndex)))
        {
            currPageIndex--;
        }
    }

    if(!addBtn->isEnabled ())
    {
        addBtn->setIsEnabled (true);
    }
    m_currentElement = IP_FILTER_ADD_BUTTON;
    m_elementList[m_currentElement]->forceActiveFocus ();

    showTableList ();
}

void IpFiltering::handleInfoPageMessage(int index)
{
    if(index == INFO_OK_BTN)
    {
        if(infoPage->getText () == ValidationMessage::getValidationMessage(IP_FILTER_DELETE_IP_RANGE))
        {
            deleteBtnAction();
        }
    }
}
