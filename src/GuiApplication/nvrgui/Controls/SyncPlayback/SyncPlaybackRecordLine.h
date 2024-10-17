#ifndef SYNCPLAYBACKRECORDLINE_H
#define SYNCPLAYBACKRECORDLINE_H

#include <QWidget>
#include "ApplController.h"
#include "EnumFile.h"

typedef enum
{
    HOUR_24,
    HOUR_12,
    HOUR_6,
    HOUR_1,
    MAX_HOUR_FORMAT_TYPE
}HOURS_FORMAT_TYPE_e;

class SyncPlaybackRecordLine : public QWidget
{
private:
    QList<quint16> m_recordList;
    HOURS_FORMAT_TYPE_e m_currentHourFormat;
    quint8 m_currentSlot;
public:
    SyncPlaybackRecordLine(quint16 startX,
                           quint16 startY,
                           HOURS_FORMAT_TYPE_e hourFormat = HOUR_24,
                           QWidget* parent = 0);
    ~SyncPlaybackRecordLine();

    void changeRecordList(QList<quint16> recordList);
    void changeHourFormatAndSlot(HOURS_FORMAT_TYPE_e hourFormat, quint8 currentSlot);
    void resetRecordList();
    bool ifBlockCompletelyInSlot(quint16 startingMinute, quint16 endingMinute, quint16 lowerLimit, quint16 upperLimit);
    bool ifSlotCompletelyInBlock(quint16 startingMinute, quint16 endingMinute, quint16 lowerLimit, quint16 upperLimit);
    bool ifBlockNotInSlot(quint16 startingMinute, quint16 endingMinute, quint16 lowerLimit, quint16 upperLimit);
    QList<quint16> getRecordList();

    void paintEvent(QPaintEvent *);
};

#endif // SYNCPLAYBACKRECORDLINE_H
