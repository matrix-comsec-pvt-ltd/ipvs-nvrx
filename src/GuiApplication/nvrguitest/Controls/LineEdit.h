#ifndef LINEEDIT_H
#define LINEEDIT_H
#include <QLineEdit>

class LineEdit : public QLineEdit
{
    Q_OBJECT
public:
    LineEdit(quint32 startX,
             quint32 startY,
             quint32 width,
             quint32 height,
             QWidget *parent = 0,
             quint8 index = 0);

    void mousePressEvent (QMouseEvent *event);
    void mouseMoveEvent (QMouseEvent *event);
    void setActiveFocus();
    void keyPressEvent (QKeyEvent *evt);
    void focusOutEvent (QFocusEvent *evt);
    void focusInEvent (QFocusEvent *evt);
signals:
    void sigFocusChange(quint8 index, bool isFocusIn, bool forceFocus);
    void sigMouseClickedOnLineEdit(quint8 index);
private:
    quint8 m_index;
};

#endif // LINEEDIT_H
