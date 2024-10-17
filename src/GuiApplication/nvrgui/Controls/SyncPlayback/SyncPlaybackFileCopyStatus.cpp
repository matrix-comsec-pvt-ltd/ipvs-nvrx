#include "SyncPlaybackFileCopyStatus.h"

#define BORDER_WIDTH        1

SyncPlaybackFileCopyStatus::SyncPlaybackFileCopyStatus(quint16 startX,
                                                       quint16 startY,
                                                       quint16 width,
                                                       quint16 height,
                                                       QWidget* parent) : QWidget(parent),
    m_startX(startX), m_startY(startY), m_width(width), m_height(height), m_completedPercent(0)
{
    m_multiplier = (qreal)(m_width - (2 * BORDER_WIDTH)) / 100;
    this->setGeometry(startX, startY, width, height);

    m_backgroundRectangle = new Rectangle(0, 0,
                                          width,
                                          height,
                                          NORMAL_BKG_COLOR,
                                          this,
                                          0, BORDER_WIDTH,
                                          BORDER_2_COLOR);
    m_statusRectangle = new Rectangle(BORDER_WIDTH,
                                      BORDER_WIDTH,
                                      qRound(m_completedPercent * m_multiplier),
                                      (height - (2 * BORDER_WIDTH)),
                                      HIGHLITED_FONT_COLOR,
                                      this);
    m_percentTextlabel = new TextLabel((width / 2),
                                       (height / 2),
                                       NORMAL_FONT_SIZE,
                                       QString("%1").arg(m_completedPercent) + "%",
                                       this,
                                       BORDER_2_COLOR,
                                       NORMAL_FONT_FAMILY,
                                       ALIGN_CENTRE_X_CENTER_Y);
    m_percentTextlabel->SetBold(true);
    this->show();
}

SyncPlaybackFileCopyStatus::~SyncPlaybackFileCopyStatus()
{
    delete m_percentTextlabel;
    delete m_statusRectangle;
    delete m_backgroundRectangle;
}

void SyncPlaybackFileCopyStatus::changePercentStatus(quint8 copiedPercent)
{
    m_completedPercent = copiedPercent;
    m_percentTextlabel->changeText(QString("%1").arg(m_completedPercent) + "%");
    m_percentTextlabel->update();
    m_statusRectangle->resetGeometry(qRound(m_completedPercent * m_multiplier));
}
