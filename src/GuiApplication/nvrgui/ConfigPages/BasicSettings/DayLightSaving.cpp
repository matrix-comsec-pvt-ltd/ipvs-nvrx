#include "DayLightSaving.h"
#include "ValidationMessage.h"

#define CNFG_TO_INDEX               1
#define FIRST_ELE_XOFFSET           SCALE_WIDTH(13)
#define FIRST_ELE_YOFFSET           SCALE_HEIGHT(100)
// List of control
typedef enum
{
    DAYLIGHT_STG_ENABLE_CHECK,
    DAYLIGHT_STG_FRD_MONTH,
    DAYLIGHT_STG_FRD_WEEK,
    DAYLIGHT_STG_FRD_DAY,
    DAYLIGHT_STG_FRD_TIME,
    DAYLIGHT_STG_REV_MONTH,
    DAYLIGHT_STG_REV_WEEK,
    DAYLIGHT_STG_REV_DAY,
    DAYLIGHT_STG_REV_TIME,
    DAYLIGHT_STG_TIME_DUR,

    MAX_DAYLIGHT_STG_ELEMETS
}DAYLIGHT_STG_ELELIST_e;

// cnfg field no According to CMS comm. module
typedef enum
{
    FIELD_ENABLE_DST,
    FIELD_FRD_MONTH,
    FIELD_FRD_WEEK,
    FIELD_FRD_DAY,
    FIELD_FRD_TIME,
    FIELD_REV_MONTH,
    FIELD_REV_WEEK,
    FIELD_REV_DAY,
    FIELD_REV_TIME,
    FIELD_TIME_DUR,

    MAX_FIELD_NO
}CNFG_FIELD_NO_e;


static const QString elementLabel[MAX_DAYLIGHT_STG_ELEMETS] =
{
    "Enable DST",
    "Month",
    "Week",
    "Day",
    "Time",
    "Duration"
};

static const QString elementHeadings[] =
{
    "Forward Clock",
    "Reverse Clock",
    "Time Period"
};

static const QStringList monthList = QStringList()
      <<"January"     <<"February"    <<"March"       <<"April"
      <<"May"         <<"June"        <<"July"        <<"August"
      <<"September"   <<"October"     <<"November"    <<"December";

static const QStringList dayList = QStringList()
      <<"Sunday"       <<"Monday"     <<"Tuesday"      <<"Wednesday"
      <<"Thursday"     <<"Friday"     <<"Saturday";

static const QStringList weekList = QStringList()
       <<"First"       <<"Second"      << "Third"
       <<"Fourth"      <<"Last";

//*****************************************************************************
// DayLightSaving
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
DayLightSaving::DayLightSaving(QString devName, QWidget *parent)
    :ConfigPageControl(devName, parent, MAX_DAYLIGHT_STG_ELEMETS)
{
    createDefaultComponent();
    DayLightSaving::getConfig();
}
//*****************************************************************************
// ~DayLightSaving()
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
DayLightSaving::~DayLightSaving()
{
    disconnect (enableDstCheckbox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (enableDstCheckbox,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotEnableDstClicked(OPTION_STATE_TYPE_e,int)));
    delete enableDstCheckbox;

    delete fwdClockEleHeading;
    delete revClockEleHeading;
    delete timePeriodEleHeading;

    for(quint8 index = MONTH_SPINBOX;
        index <= DAY_SPINBOX; index++)
    {
        disconnect(fwdMonthDayWeekSpinBox[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        delete fwdMonthDayWeekSpinBox[index];

        disconnect(revMonthDayWeekSpinBox[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        delete revMonthDayWeekSpinBox[index];
    }
    disconnect (fwdTimeClkSpinbox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete fwdTimeClkSpinbox;

    disconnect (revTimeClkSpinbox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete revTimeClkSpinbox;

    disconnect (timePeriodClkSpinbox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete timePeriodClkSpinbox;
}
//*****************************************************************************
// createDefaultComponent
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void DayLightSaving::createDefaultComponent()
{
    enableDstCheckbox = new OptionSelectButton(FIRST_ELE_XOFFSET,
                                               FIRST_ELE_YOFFSET,
                                               BGTILE_LARGE_SIZE_WIDTH,
                                               BGTILE_HEIGHT,
                                               CHECK_BUTTON_INDEX,
                                               elementLabel[DAYLIGHT_STG_ENABLE_CHECK],
                                               this,
                                               COMMON_LAYER,
                                               SCALE_WIDTH(10),
                                               MX_OPTION_TEXT_TYPE_SUFFIX,
                                               NORMAL_FONT_SIZE,
                                               DAYLIGHT_STG_ENABLE_CHECK);
    m_elementList[DAYLIGHT_STG_ENABLE_CHECK] = enableDstCheckbox;
    connect (enableDstCheckbox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotEnableDstClicked(OPTION_STATE_TYPE_e,int)));
    connect (enableDstCheckbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    fwdClockEleHeading = new ElementHeading(enableDstCheckbox->x (),
                                            enableDstCheckbox->y () + enableDstCheckbox->height () + SCALE_HEIGHT(6),
                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                            BGTILE_HEIGHT,
                                            elementHeadings[0],
                                            TOP_LAYER,
                                            this, false, 25, NORMAL_FONT_SIZE, true);

    QMap<quint8, QString>  monthMapList;

    for(quint8 index = 0; index < monthList.length (); index++)
    {
        monthMapList.insert (index,monthList.at (index));
    }

    fwdMonthDayWeekSpinBox[MONTH_SPINBOX] = new DropDown(fwdClockEleHeading->x (),
                                                         (fwdClockEleHeading->y () + fwdClockEleHeading->height ()),
                                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                                         BGTILE_HEIGHT,
                                                         DAYLIGHT_STG_FRD_MONTH,
                                                         DROPDOWNBOX_SIZE_200,
                                                         elementLabel[DAYLIGHT_STG_FRD_MONTH],
                                                         monthMapList,
                                                         this, "",
                                                         true, 0, MIDDLE_TABLE_LAYER, false);

    QMap<quint8, QString>  weekMapList;

    for(quint8 index = 0; index < weekList.length (); index++)
    {
        weekMapList.insert (index,weekList.at (index));
    }

    fwdMonthDayWeekSpinBox[WEEK_SPINBOX] = new DropDown(fwdMonthDayWeekSpinBox[MONTH_SPINBOX]->x (),
                                                        (fwdMonthDayWeekSpinBox[MONTH_SPINBOX]->y () +fwdMonthDayWeekSpinBox[MONTH_SPINBOX]->height ()),
                                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                                        BGTILE_HEIGHT,
                                                        DAYLIGHT_STG_FRD_WEEK,
                                                        DROPDOWNBOX_SIZE_200,
                                                        elementLabel[DAYLIGHT_STG_FRD_WEEK],
                                                        weekMapList,
                                                        this, "",
                                                        true, 0, MIDDLE_TABLE_LAYER, false);


    QMap<quint8, QString>  dayMapList;

    for(quint8 index = 0; index < dayList.length (); index++)
    {
        dayMapList.insert (index,dayList.at (index));
    }

    fwdMonthDayWeekSpinBox[DAY_SPINBOX] = new DropDown(fwdMonthDayWeekSpinBox[WEEK_SPINBOX]->x (),
                                                       (fwdMonthDayWeekSpinBox[WEEK_SPINBOX]->y () +fwdMonthDayWeekSpinBox[WEEK_SPINBOX]->height ()),
                                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                                       BGTILE_HEIGHT,
                                                       DAYLIGHT_STG_FRD_DAY,
                                                       DROPDOWNBOX_SIZE_200,
                                                       elementLabel[DAYLIGHT_STG_FRD_DAY],
                                                       dayMapList,
                                                       this, "",
                                                       true, 0, MIDDLE_TABLE_LAYER, false);

    fwdTimeClkSpinbox = new ClockSpinbox(fwdMonthDayWeekSpinBox[DAY_SPINBOX]->x (),
                                         (fwdMonthDayWeekSpinBox[DAY_SPINBOX]->y () +fwdMonthDayWeekSpinBox[DAY_SPINBOX]->height ()),
                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         DAYLIGHT_STG_FRD_TIME,
                                         CLK_SPINBOX_With_NO_SEC,
                                         elementLabel[DAYLIGHT_STG_FRD_TIME], 10,
                                         this, "", true, 0,
                                         BOTTOM_TABLE_LAYER, false);

    revClockEleHeading = new ElementHeading(fwdClockEleHeading->x () + fwdClockEleHeading->width () + SCALE_WIDTH(6),
                                            enableDstCheckbox->y () + enableDstCheckbox->height () + SCALE_HEIGHT(6),
                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                            BGTILE_HEIGHT,
                                            elementHeadings[1],
                                            TOP_LAYER,
                                            this, false, 25, NORMAL_FONT_SIZE, true);

    revMonthDayWeekSpinBox[MONTH_SPINBOX] = new DropDown(revClockEleHeading->x (),
                                                         (revClockEleHeading->y () + revClockEleHeading->height ()),
                                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                                         BGTILE_HEIGHT,
                                                         DAYLIGHT_STG_REV_MONTH,
                                                         DROPDOWNBOX_SIZE_200,
                                                         elementLabel[DAYLIGHT_STG_FRD_MONTH],
                                                         monthMapList,
                                                         this, "",
                                                         true, 0, MIDDLE_TABLE_LAYER, false);

    revMonthDayWeekSpinBox[WEEK_SPINBOX] = new DropDown(revMonthDayWeekSpinBox[MONTH_SPINBOX]->x (),
                                                        (revMonthDayWeekSpinBox[MONTH_SPINBOX]->y () +revMonthDayWeekSpinBox[MONTH_SPINBOX]->height ()),
                                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                                        BGTILE_HEIGHT,
                                                        DAYLIGHT_STG_REV_WEEK,
                                                        DROPDOWNBOX_SIZE_200,
                                                        elementLabel[DAYLIGHT_STG_FRD_WEEK],
                                                        weekMapList,
                                                        this, "",
                                                        true, 0, MIDDLE_TABLE_LAYER, false);

    revMonthDayWeekSpinBox[DAY_SPINBOX] = new DropDown(revMonthDayWeekSpinBox[WEEK_SPINBOX]->x (),
                                                       (revMonthDayWeekSpinBox[WEEK_SPINBOX]->y () +revMonthDayWeekSpinBox[WEEK_SPINBOX]->height ()),
                                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                                       BGTILE_HEIGHT,
                                                       DAYLIGHT_STG_REV_DAY,
                                                       DROPDOWNBOX_SIZE_200,
                                                       elementLabel[DAYLIGHT_STG_FRD_DAY],
                                                       dayMapList,
                                                       this, "",
                                                       true, 0, MIDDLE_TABLE_LAYER, false);

    revTimeClkSpinbox = new ClockSpinbox(revMonthDayWeekSpinBox[DAY_SPINBOX]->x (),
                                         (revMonthDayWeekSpinBox[DAY_SPINBOX]->y () +revMonthDayWeekSpinBox[DAY_SPINBOX]->height ()),
                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         DAYLIGHT_STG_REV_TIME,
                                         CLK_SPINBOX_With_NO_SEC,
                                         elementLabel[DAYLIGHT_STG_FRD_TIME], 10,
                                         this, "", true, 0,
                                         BOTTOM_TABLE_LAYER, false);

    for(quint8 index = MONTH_SPINBOX; index <= DAY_SPINBOX; index++)
    {
        m_elementList[DAYLIGHT_STG_FRD_MONTH + index] = fwdMonthDayWeekSpinBox[index];
        connect(fwdMonthDayWeekSpinBox[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

        m_elementList[DAYLIGHT_STG_REV_MONTH + index] = revMonthDayWeekSpinBox[index];
        connect(revMonthDayWeekSpinBox[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    }
    m_elementList[DAYLIGHT_STG_FRD_TIME] = fwdTimeClkSpinbox;
    connect (fwdTimeClkSpinbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    m_elementList[DAYLIGHT_STG_REV_TIME] = revTimeClkSpinbox;
    connect (revTimeClkSpinbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));


    timePeriodEleHeading = new ElementHeading(fwdTimeClkSpinbox->x (),
                                              fwdTimeClkSpinbox->y ()+ fwdTimeClkSpinbox->height () + SCALE_HEIGHT(6),
                                              BGTILE_LARGE_SIZE_WIDTH,
                                              BGTILE_HEIGHT,
                                              elementHeadings[2],
                                              TOP_LAYER, this,
                                              false, 25, NORMAL_FONT_SIZE, true);
    timePeriodClkSpinbox = new ClockSpinbox(timePeriodEleHeading->x (),
                                            timePeriodEleHeading->y () + timePeriodEleHeading->height (),
                                            BGTILE_LARGE_SIZE_WIDTH,
                                            BGTILE_HEIGHT,
                                            DAYLIGHT_STG_TIME_DUR,
                                            CLK_SPINBOX_With_NO_SEC,
                                            elementLabel[DAYLIGHT_STG_REV_MONTH], 8,
                                            this, "", true, 0,
                                            BOTTOM_TABLE_LAYER, false);
    m_elementList[DAYLIGHT_STG_TIME_DUR] = timePeriodClkSpinbox;
    connect (timePeriodClkSpinbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
}
//*****************************************************************************
// getConfig
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void DayLightSaving::getConfig()
{
    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             DST_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_FIELD_NO,
                                                             0);

    DevCommParam* param = new DevCommParam();

    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;

    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}
//*****************************************************************************
// defaultConfig
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void DayLightSaving::defaultConfig()
{
    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_DEF_CFG,
                                                             DST_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_FIELD_NO,
                                                             0);

    DevCommParam* param = new DevCommParam();

    param->msgType = MSG_DEF_CFG;
    param->payload = payloadString;

    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}
//*****************************************************************************
// saveConfig
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void DayLightSaving::saveConfig()
{
    quint32 fwdHr, revHr, fwdMin, revMin;
    QString tempStr = "";
    fwdTimeClkSpinbox->currentValue (fwdHr, fwdMin);
    revTimeClkSpinbox->currentValue (revHr, revMin);

    if((enableDstCheckbox->getCurrentState () == ON_STATE) &&
            (fwdMonthDayWeekSpinBox[MONTH_SPINBOX]->getIndexofCurrElement ()
             == revMonthDayWeekSpinBox[MONTH_SPINBOX]->getIndexofCurrElement ()) &&
            (fwdMonthDayWeekSpinBox[WEEK_SPINBOX]->getIndexofCurrElement ()
             == revMonthDayWeekSpinBox[WEEK_SPINBOX]->getIndexofCurrElement ()) &&
            (fwdMonthDayWeekSpinBox[DAY_SPINBOX]->getIndexofCurrElement ()
             == revMonthDayWeekSpinBox[DAY_SPINBOX]->getIndexofCurrElement ()) &&
            (fwdHr == revHr) &&
            (fwdMin == revMin))
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(DAY_LIGHT_DIFF_FRWD_REVRSE));
    }
    else
    {
        // fill forward parameter
        payloadLib->setCnfgArrayAtIndex (FIELD_ENABLE_DST,
                                         enableDstCheckbox->getCurrentState ());
        payloadLib->setCnfgArrayAtIndex (FIELD_FRD_MONTH,
                                         fwdMonthDayWeekSpinBox[MONTH_SPINBOX]->getIndexofCurrElement ());
        payloadLib->setCnfgArrayAtIndex (FIELD_FRD_WEEK,
                                         fwdMonthDayWeekSpinBox[WEEK_SPINBOX]->getIndexofCurrElement ());
        payloadLib->setCnfgArrayAtIndex (FIELD_FRD_DAY,
                                         fwdMonthDayWeekSpinBox[DAY_SPINBOX]->getIndexofCurrElement ());

        if(fwdHr < 10)
        {
            tempStr.append (QString("%1%2").arg (0).arg (fwdHr));
        }
        else
        {
            tempStr.append (QString("%1").arg (fwdHr));
        }

        if(fwdMin < 10)
        {
            tempStr.append (QString("%1%2").arg (0).arg (fwdMin));
        }
        else
        {
            tempStr.append (QString("%1").arg (fwdMin));
        }

        payloadLib->setCnfgArrayAtIndex (FIELD_FRD_TIME,tempStr);

        // fill reverse parameter
        payloadLib->setCnfgArrayAtIndex (FIELD_REV_MONTH,
                                         revMonthDayWeekSpinBox[MONTH_SPINBOX]->getIndexofCurrElement ());
        payloadLib->setCnfgArrayAtIndex (FIELD_REV_WEEK,
                                         revMonthDayWeekSpinBox[WEEK_SPINBOX]->getIndexofCurrElement ());
        payloadLib->setCnfgArrayAtIndex (FIELD_REV_DAY,
                                         revMonthDayWeekSpinBox[DAY_SPINBOX]->getIndexofCurrElement ());

        tempStr = "";
        if(revHr < 10)
        {
            tempStr.append (QString("%1%2").arg (0).arg (revHr));
        }
        else
        {
            tempStr.append (QString("%1").arg (revHr));
        }

        if(revMin < 10)
        {
            tempStr.append (QString("%1%2").arg (0).arg (revMin));
        }
        else
        {
            tempStr.append (QString("%1").arg (revMin));
        }
        payloadLib->setCnfgArrayAtIndex (FIELD_REV_TIME,tempStr);

        // fill Time period
        timePeriodClkSpinbox->currentValue (revHr, revMin);
        tempStr = "";
        if(revHr < 10)
        {
            tempStr.append (QString("%1%2").arg (0).arg (revHr));
        }
        else
        {
            tempStr.append (QString("%1").arg (revHr));
        }

        if(revMin < 10)
        {
            tempStr.append (QString("%1%2").arg (0).arg (revMin));
        }
        else
        {
            tempStr.append (QString("%1").arg (revMin));
        }
        payloadLib->setCnfgArrayAtIndex (FIELD_TIME_DUR,tempStr);

        //create the payload for Get Cnfg
        QString payloadString = payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                                 DST_TABLE_INDEX,
                                                                 CNFG_FRM_INDEX,
                                                                 CNFG_TO_INDEX,
                                                                 CNFG_FRM_FIELD,
                                                                 MAX_FIELD_NO,
                                                                 MAX_FIELD_NO);

        DevCommParam* param = new DevCommParam();

        param->msgType = MSG_SET_CFG;
        param->payload = payloadString;

        processBar->loadProcessBar();
        applController->processActivity(currDevName, DEVICE_COMM, param);
    }
}
//*****************************************************************************
// processDeviceResponse
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void DayLightSaving::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    OPTION_STATE_TYPE_e state;
    QString timeStr = "";
    processBar->unloadProcessBar();
    if(deviceName == currDevName)
    {
        switch(param->deviceStatus)
        {
        case CMD_SUCCESS:
            switch(param->msgType)
            {
            case MSG_GET_CFG:
                payloadLib->parsePayload(param->msgType, param->payload);
                if(payloadLib->getcnfgTableIndex () == DST_TABLE_INDEX)
                {
                    state = (OPTION_STATE_TYPE_e)(payloadLib->getCnfgArrayAtIndex (FIELD_ENABLE_DST).toUInt ());

                    enableDstCheckbox->changeState (state);
                    fwdMonthDayWeekSpinBox[MONTH_SPINBOX]->setIndexofCurrElement
                            (payloadLib->getCnfgArrayAtIndex (FIELD_FRD_MONTH).toUInt ());
                    fwdMonthDayWeekSpinBox[WEEK_SPINBOX]->setIndexofCurrElement
                            (payloadLib->getCnfgArrayAtIndex (FIELD_FRD_WEEK).toUInt ());
                    fwdMonthDayWeekSpinBox[DAY_SPINBOX]->setIndexofCurrElement
                            (payloadLib->getCnfgArrayAtIndex (FIELD_FRD_DAY).toUInt ());

                    timeStr = (payloadLib->getCnfgArrayAtIndex (FIELD_FRD_TIME).toString ());

                    fwdTimeClkSpinbox->assignValue (timeStr.mid(0, 2),
                                                    timeStr.mid (2, 2));

                    revMonthDayWeekSpinBox[MONTH_SPINBOX]->setIndexofCurrElement
                            (payloadLib->getCnfgArrayAtIndex (FIELD_REV_MONTH).toUInt ());
                    revMonthDayWeekSpinBox[WEEK_SPINBOX]->setIndexofCurrElement
                            (payloadLib->getCnfgArrayAtIndex (FIELD_REV_WEEK).toUInt ());
                    revMonthDayWeekSpinBox[DAY_SPINBOX]->setIndexofCurrElement
                            (payloadLib->getCnfgArrayAtIndex (FIELD_REV_DAY).toUInt ());

                    timeStr = (payloadLib->getCnfgArrayAtIndex (FIELD_REV_TIME).toString ());

                    revTimeClkSpinbox->assignValue (timeStr.mid(0, 2),
                                                    timeStr.mid (2, 2));

                    timeStr = (payloadLib->getCnfgArrayAtIndex (FIELD_TIME_DUR).toString ());

                    timePeriodClkSpinbox->assignValue (timeStr.mid(0, 2),
                                                       timeStr.mid (2, 2));

                    for(quint8 index = 0; index < MAX_SPINBOX; index++)
                    {
                        fwdMonthDayWeekSpinBox[index]->setIsEnabled (state);
                        revMonthDayWeekSpinBox[index]->setIsEnabled (state);
                    }
                    fwdTimeClkSpinbox->setIsEnabled (state);
                    revTimeClkSpinbox->setIsEnabled (state);
                    timePeriodClkSpinbox->setIsEnabled (state);
                }
                break;

            case MSG_SET_CFG:
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
            infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            break;
        }
    }
}
//*****************************************************************************
// slotEnableDstClicked
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void DayLightSaving::slotEnableDstClicked(OPTION_STATE_TYPE_e state, int)
{
    for(quint8 index = MONTH_SPINBOX; index <= DAY_SPINBOX; index++)
    {
        fwdMonthDayWeekSpinBox[index]->setIsEnabled (state);
        revMonthDayWeekSpinBox[index]->setIsEnabled (state);
    }
    fwdTimeClkSpinbox->setIsEnabled (state);
    revTimeClkSpinbox->setIsEnabled (state);
    timePeriodClkSpinbox->setIsEnabled (state);
}
