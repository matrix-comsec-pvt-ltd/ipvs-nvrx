#ifndef INVISIBLEWIDGETCNTRL_H
#define INVISIBLEWIDGETCNTRL_H

#include <QWidget>

class InvisibleWidgetCntrl : public QWidget
{
    Q_OBJECT
public:
    explicit InvisibleWidgetCntrl(QWidget *parent = 0);

    void mousePressEvent (QMouseEvent *);
    void mouseMoveEvent (QMouseEvent *);
    void mouseReleaseEvent (QMouseEvent *);
    void mouseDoubleClickEvent (QMouseEvent *);

signals:
    void sigMouseClick();

public slots:
    
};

#endif // INVISIBLEWIDGETCNTRL_H
