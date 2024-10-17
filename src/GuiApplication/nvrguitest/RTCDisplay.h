#ifndef RTCDISPLAY_H
#define RTCDISPLAY_H

#include <QWidget>

class RTCDisplay
{
public:
    bool setSystemTime();
    bool SetHwClockTimeHI3536();

    explicit RTCDisplay(QWidget *parent = 0);
    ~RTCDisplay() { };

    bool getLocalTimeInBrokenTm(struct tm * localTimeInTmStruct);
    bool getLocalTimeInSec(time_t * currLocalTime);
    Q_INVOKABLE QString getLocalTime(struct tm  *tempInputTmStruct);
    Q_INVOKABLE quint8 getSntpTime();
    bool ConvertLocalTimeInSec(tm *inputTimeStruct, time_t *outputTimeInSec);

signals:
    void rxLocalTime(int hour, int minute, int second);

public slots:
    void slotButtonClick(quint8);
};

#endif // RTC_DISPLAY_H
