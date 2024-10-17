#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QWidget>
#include <QTextEdit>
#include <QTextCursor>
#include "EnumFile.h"

class TextEdit : public QTextEdit
{
    Q_OBJECT

    QTextCursor textCursor;

public:
     TextEdit(quint32 startX,
                      quint32 startY,
                      quint32 width,
                      quint32 height,
                      QWidget *parent = 0);

    int getCurrentCurrsorPosition() const;
    void setCurrentCursorPosition(int pos) ;
    void setActiveFocus();
    void navigationKeyPressed(QKeyEvent *event);
    void deleteKeyPressed(QKeyEvent *event);
    void backspaceKeyPressed(QKeyEvent *event);
    bool hasSelectedText();
    void insertText(QString str);
    void selectAllText();
    void truncateText(quint8 charCount);

    void mousePressEvent (QMouseEvent *event);
    void mouseMoveEvent (QMouseEvent *event);
    void focusInEvent (QFocusEvent *evt);
    void focusOutEvent (QFocusEvent *evt);
    void keyPressEvent (QKeyEvent *evt);

signals:
    void sigFocusChange(bool isFocusIn, bool forceFocus);

public slots:

};

#endif // TEXTEDIT_H
