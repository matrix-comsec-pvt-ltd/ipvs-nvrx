#ifndef DAYLIGHTSAVING_H
#define DAYLIGHTSAVING_H

#include "Controls/ConfigPageControl.h"
#include "Controls/DropDown.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/Clockspinbox.h"
#include "Controls/ElementHeading.h"

typedef enum
{
    MONTH_SPINBOX,
    WEEK_SPINBOX,
    DAY_SPINBOX,

    MAX_SPINBOX
}MONTH_WEEK_DAY_SPINBOXES_e;

class DayLightSaving : public ConfigPageControl
{
    Q_OBJECT
private:
    OptionSelectButton *enableDstCheckbox;
    DropDown *fwdMonthDayWeekSpinBox[MAX_SPINBOX];
    ClockSpinbox *fwdTimeClkSpinbox;

    DropDown *revMonthDayWeekSpinBox[MAX_SPINBOX];
    ClockSpinbox *revTimeClkSpinbox;

    ElementHeading *fwdClockEleHeading;
    ElementHeading *revClockEleHeading;
    ElementHeading *timePeriodEleHeading;
    ClockSpinbox *timePeriodClkSpinbox;

public:
    explicit DayLightSaving(QString devName,
                   QWidget *parent = 0);

    ~DayLightSaving();

    void createDefaultComponent();
    void processDeviceResponse(DevCommParam *param, QString deviceName);

    void getConfig();
    void defaultConfig();
    void saveConfig();

public slots:
    void slotEnableDstClicked(OPTION_STATE_TYPE_e state,int index);
};

#endif // DAYLIGHTSAVING_H
