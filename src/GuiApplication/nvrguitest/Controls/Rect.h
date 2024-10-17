#ifndef RECT_H
#define RECT_H

#include <QWidget>

class Rect : public QWidget
{
    Q_OBJECT
public:
    explicit Rect(quint16 xPos,
                  quint16 yPos,
                  quint16 width,
                  quint16 height,
                  QString bgColor,
                  QWidget *parent = 0);

    void paintEvent (QPaintEvent *);
    void changeBgColor(QString bgColor);
    QString getBgColor();

signals:
    
public slots:
    
private:
    quint16 m_xPos;
    quint16 m_yPos;
    quint16 m_width;
    quint16 m_height;
    QString m_bgColor;

};

#endif // RECT_H
