#ifndef SYNCPLAYBACKFILECOPYSTATUS_H
#define SYNCPLAYBACKFILECOPYSTATUS_H

#include <QWidget>
#include <Controls/Rectangle.h>
#include <Controls/TextLabel.h>

class SyncPlaybackFileCopyStatus : public QWidget
{
private:
    Rectangle *m_backgroundRectangle;
    Rectangle *m_statusRectangle;
    TextLabel *m_percentTextlabel;

    quint16 m_startX, m_startY, m_width, m_height;
    qreal m_multiplier;
    quint8 m_completedPercent;
public:
    SyncPlaybackFileCopyStatus(quint16 startX,
                               quint16 startY,
                               quint16 width,
                               quint16 height,
                               QWidget* parent);
    ~SyncPlaybackFileCopyStatus();

    void changePercentStatus(quint8 copiedPercent);
};

#endif // SYNCPLAYBACKFILECOPYSTATUS_H
