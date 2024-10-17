//////////////////////////////////////////////////////////////////////////
//   MMM     MMM       AAA       TTTTTTTTTT  RRRRRR    IIIIIIII  XX   XX
//   MMMM   MMMM      AA AA          TT      RR   RR      II      XX XX
//   MM MM MM MM     AA   AA         TT      RR    RR     II       XXX
//   MM  MM   MM    AAAAAAAAA        TT      RRRRRRR      II       XXX
//   MM       MM   AA       AA       TT      RR  RR       II      XX XX
//   MM       MM  AA         AA      TT      RR   RR   IIIIIIII  XX   XX
//
//   Company      : Matrix Telecom Pvt. Ltd., Baroda, India.
//   Project      : DVR (Digital Video Recorder - TI)
//   Owner        : Tushar Rabadiya
//   File         : DeviceClient.cpp
//   Description  :
/////////////////////////////////////////////////////////////////////////////
#include "ToolTip.h"

#define TEXT_X_OFFSET               SCALE_WIDTH(3)
#define TEXT_Y_OFFSET               SCALE_HEIGHT(4)
#define TOOLTIP_BACKGROUND_COLOR       "#161616"
#define TOOLTIP_RADIUS              SCALE_WIDTH(5)
#define TOOLTIP_BORDER_WIDTH        SCALE_WIDTH(2)

ToolTip :: ToolTip (quint32 xParam,
                    quint32 yParam,
                    QString label,
                    QWidget *parent,
                    POINT_PARAM_TYPE_e pointParamType)
      :QWidget (parent),  m_elementLabel(label), m_paramType(pointParamType),
        m_fontSize(NORMAL_FONT_SIZE)
{
    setWholeGeometry(xParam, yParam);
    m_backgroundRectangle = new Rectangle(0, 0,
                                          this->width(),
                                          this->height(),
                                          TOOLTIP_BACKGROUND_COLOR,
                                          this,
                                          TOOLTIP_RADIUS,
                                          TOOLTIP_BORDER_WIDTH,
                                          DISABLE_FONT_COLOR);
    translatedlabelWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width(QApplication::translate(QT_TRANSLATE_STR, m_elementLabel.toUtf8().constData()));
    m_textLabel = new TextLabel((this->width() / 2),
                                (this->height() / 2),
                                m_fontSize,
                                m_elementLabel,
                                this,
                                HIGHLITED_FONT_COLOR,
                                NORMAL_FONT_FAMILY,
                                ALIGN_CENTRE_X_CENTER_Y, 0, 0, translatedlabelWidth);
}

ToolTip::~ToolTip()
{
    delete m_backgroundRectangle;
    delete m_textLabel;
}

void ToolTip::setWholeGeometry(quint32 startX, quint32 startY)
{
    quint16 labelWidth = 0;
    quint16 labelHeight = 0;
    TextLabel::getWidthHeight(TextLabel::getFont(NORMAL_FONT_FAMILY, m_fontSize),
                              QApplication::translate(QT_TRANSLATE_STR, m_elementLabel.toUtf8().constData()),
                              labelWidth,
                              labelHeight);
    quint16 totalWidth = labelWidth + (2 * TEXT_X_OFFSET);
    quint16 totalHeight = labelHeight + (2 * TEXT_Y_OFFSET);
    switch(m_paramType)
    {
    case START_X_START_Y:
        m_startX = startX;
        m_startY = startY;
        break;

    case CENTER_X_END_Y:
        m_startX = (startX - (totalWidth / 2));
        m_startY = (startY - totalHeight);
        break;

    case CENTER_X_CENTER_Y:
        m_startX = (startX - (totalWidth / 2));
        m_startY = (startY - (totalHeight / 2));
        break;

    case START_X_CENTER_Y:
        m_startX = startX;
        m_startY = (startY - (totalHeight / 2));
        break;

    case START_X_END_Y:
        m_startX = startX;
        m_startY = (startY - totalHeight);
        break;

    case END_X_START_Y:
        m_startX = (startX - totalWidth);
        m_startY = startY;
        break;

    case CENTER_X_START_Y:
        m_startX = (startX - (totalWidth / 2));
        m_startY = startY;
        break;

    default:
        break;
    }
    this->setGeometry(m_startX,
                      m_startY,
                      (totalWidth + SCALE_WIDTH(6)),
                      (totalHeight + SCALE_HEIGHT(2)));
}

void ToolTip::resetGeometry(quint32 startX, quint32 startY)
{
    setWholeGeometry(startX, startY);
    m_textLabel->setOffset((this->width() / 2),
                           (this->height() / 2));
    m_backgroundRectangle->resetGeometry(0,
                                         0,
                                         this->width(),
                                         this->height());
}

void ToolTip::textChange(QString text)
{
    m_elementLabel = text;
    translatedlabelWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width(QApplication::translate(QT_TRANSLATE_STR, m_elementLabel.toUtf8().constData()));
    m_textLabel->changeText(text, translatedlabelWidth);
    m_textLabel->update ();
    resetGeometry(m_startX, m_startY);
}

QString ToolTip::getTooltipText()
{
    return m_elementLabel;
}

void ToolTip::showHideTooltip(bool isShow)
{
    if(isShow)
    {
       resetGeometry(m_startX, m_startY);
    }
    else
    {
        this->setGeometry(m_startX,
                          m_startY,
                          0,
                          0);
    }
    update();
}

void ToolTip::setFontSize(quint8 fontSize)
{
    m_fontSize = fontSize;
    m_textLabel->setFontSize(m_fontSize);
    m_textLabel->update ();
    resetGeometry(m_startX, m_startY);
}


