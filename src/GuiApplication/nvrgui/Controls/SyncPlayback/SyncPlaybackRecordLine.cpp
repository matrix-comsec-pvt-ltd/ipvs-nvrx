#include "SyncPlaybackRecordLine.h"
#include <QPaintEvent>
#include <QPainter>

#define HEIGHT      SCALE_HEIGHT(2)
#define WIDTH       1500

const QString recordingColor[] = {"#c39405",       //Schedule
                                  "#0c822d",      //Manual
                                  "#528dc9",     //Cosec
                                  "#990303"     //Alarm
                                 };

const quint8 minuteValueInPixel[MAX_HOUR_FORMAT_TYPE] = {1, 2, 4, 24};

SyncPlaybackRecordLine::SyncPlaybackRecordLine(quint16 startX,
                                               quint16 startY,
                                               HOURS_FORMAT_TYPE_e hourFormat,
                                               QWidget* parent) : QWidget(parent),
                        m_currentHourFormat(hourFormat), m_currentSlot(0)
{
    m_recordList.append(0);
    m_recordList.append(0);
    m_recordList.append(SCALE_WIDTH(WIDTH));
    this->setGeometry(startX, startY, SCALE_WIDTH(WIDTH), HEIGHT);
}

SyncPlaybackRecordLine::~SyncPlaybackRecordLine()
{
    m_recordList.clear();
}

void SyncPlaybackRecordLine::changeRecordList(QList<quint16> recordList)
{
    m_recordList = recordList;
    update();
}

void SyncPlaybackRecordLine::changeHourFormatAndSlot(HOURS_FORMAT_TYPE_e hourFormat,
                                                     quint8 currentSlot)
{
    if((hourFormat != m_currentHourFormat)
            || (m_currentSlot != currentSlot))
    {
        m_currentHourFormat = hourFormat;
        m_currentSlot = currentSlot;
        update();
    }
}

void SyncPlaybackRecordLine::resetRecordList()
{
    m_recordList.clear();
    m_recordList.append(0);
    m_recordList.append(0);
    m_recordList.append(SCALE_WIDTH(WIDTH));
    update();
}

bool SyncPlaybackRecordLine::ifBlockCompletelyInSlot(quint16 startingMinute, quint16 endingMinute, quint16 lowerLimit, quint16 upperLimit)
{
    return ((startingMinute >= lowerLimit) && (endingMinute <= upperLimit));
}

bool SyncPlaybackRecordLine::ifSlotCompletelyInBlock(quint16 startingMinute, quint16 endingMinute, quint16 lowerLimit, quint16 upperLimit)
{
    return ((startingMinute <= lowerLimit) && (endingMinute >= upperLimit));
}

bool SyncPlaybackRecordLine::ifBlockNotInSlot(quint16 startingMinute, quint16 endingMinute, quint16 lowerLimit, quint16 upperLimit)
{
    return ((endingMinute < lowerLimit) || (startingMinute > upperLimit));
}

QList<quint16> SyncPlaybackRecordLine::getRecordList()
{
    return m_recordList;
}

void SyncPlaybackRecordLine::paintEvent(QPaintEvent * event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing, true);

    quint16 minuteLowerLimit = (1440 / minuteValueInPixel[m_currentHourFormat]) * m_currentSlot;
    quint16 minuteUpperLimit = ((1440 / minuteValueInPixel[m_currentHourFormat]) * (m_currentSlot + 1)) - 1;

    /* here the for loop has its variable divided by 3 as the list stores 3 values for each cameras; these values are colour its start time and its end time */
    for(quint16 index = 0; index < m_recordList.length() / 3; index++)
    {
        quint16 color = m_recordList.at(index * 3);
        quint16 startingMinute = ((m_recordList.at(index * 3 + 1)));
        quint16 endingMinute = ((m_recordList.at(index * 3 + 2)));

        if(!ifBlockNotInSlot(startingMinute, endingMinute, minuteLowerLimit, minuteUpperLimit))
        {
            if(ifBlockCompletelyInSlot(startingMinute, endingMinute, minuteLowerLimit, minuteUpperLimit))
            {
                startingMinute = (startingMinute - minuteLowerLimit);
                endingMinute = (endingMinute - minuteLowerLimit);
            }
            else if(ifSlotCompletelyInBlock(startingMinute, endingMinute, minuteLowerLimit, minuteUpperLimit))
            {
                startingMinute = 0;
                endingMinute = (minuteUpperLimit - minuteLowerLimit);
            }
            else
            {
                if(startingMinute <= minuteLowerLimit)
                {
                    startingMinute = 0;
                    endingMinute = (endingMinute - minuteLowerLimit);
                }
                if(endingMinute >= minuteUpperLimit)
                {
                    endingMinute = minuteUpperLimit;
                    startingMinute = (startingMinute - minuteLowerLimit);
                }
            }
            qreal recStarting;
            qreal blockWidth = (endingMinute - startingMinute + 1) * (qreal)((qreal)minuteValueInPixel[m_currentHourFormat]* (qreal)((qreal)WIDTH/(qreal)1440));
            recStarting = (qreal)startingMinute * (qreal)((qreal)minuteValueInPixel[m_currentHourFormat] * (qreal)((qreal)WIDTH/(qreal)1440));
            if(color != 0)
            {
                painter.setBrush(QBrush(QColor(recordingColor[color - 1])));
                painter.drawRect(QRectF((qreal)((qreal)(recStarting * (qreal)SCREEN_WIDTH)/(qreal)1920),
                                 (qreal)0,
                                 (qreal)(((qreal)(blockWidth * SCREEN_WIDTH))/(qreal)1920),
                                 (qreal)HEIGHT));
            }
        }
    }
}
