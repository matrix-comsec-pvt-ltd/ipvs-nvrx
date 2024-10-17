#ifndef HEADING1_H
#define HEADING1_H

#include <QWidget>
#include "Controls/TextLabel.h"

#define MAX_FONT_VALUE          255

typedef enum
{
    HEADING_TYPE_1 = 0,
    HEADING_TYPE_2
}HEADING_TYPE_e;


class Heading : public QWidget
{
private:
    int m_centerX, m_centerY;
    quint8 m_fontSize, m_shadowMargin;
    TextLabel * m_mainHeading;
    TextLabel * m_shadow;
    QString m_headingText;
    quint16 labelWidth ;

public:
    Heading(int centerX,
            int centerY,
            QString headingText,
            QWidget *parent = 0,
            HEADING_TYPE_e type = HEADING_TYPE_1,
            quint8 fontSize = MAX_FONT_VALUE);
    ~Heading();

    void setGeometryForElements();
    void changeHeadingText(QString text);
    void resetGeometry(int centerX, int centerY);
};

#endif // HEADING1_H
