#ifndef TOOLTIP_H
#define TOOLTIP_H
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

#include <QWidget>
#include <EnumFile.h>
#include "Controls/TextLabel.h"
#include "Controls/Rectangle.h"

class ToolTip : public QWidget
{
private:
    QString m_elementLabel;
    Rectangle* m_backgroundRectangle;
    TextLabel* m_textLabel;
    POINT_PARAM_TYPE_e m_paramType;
    quint32 m_startX, m_startY;
    quint8  m_fontSize;
    int translatedlabelWidth;

public:
    ToolTip(quint32 xParam,
            quint32 yParam,
            QString label,
            QWidget* parent = 0,
            POINT_PARAM_TYPE_e pointParamType = CENTER_X_CENTER_Y);
    ~ToolTip();

    void setWholeGeometry(quint32 startX, quint32 startY);
    void resetGeometry(quint32 startX, quint32 startY);
    void textChange(QString text);
    QString getTooltipText();
    void showHideTooltip(bool isShow);    
    void setFontSize(quint8 fontSize);
};


#endif // TOOLTIP_H
