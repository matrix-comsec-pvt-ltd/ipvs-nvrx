#ifndef PASSWORDTEXTBOX_H
#define PASSWORDTEXTBOX_H

#include "Controls/TextBox.h"
#include <QWidget>
#include <QKeyEvent>
#include <QTimer>

class PasswordTextbox: public TextBox
{
    Q_OBJECT
public:
    PasswordTextbox(quint32 startX,
                    quint32 startY,
                    quint32 width,
                    quint32 height,
                    quint16 controlIndex,
                    TEXTBOX_SIZE_e size,
                    QWidget* parent = 0,
                    TextboxParam *textBoxParam = 0,
                    BGTILE_TYPE_e bgType = COMMON_LAYER,
                    bool isNavigationEnable = true,
                    quint32 leftMarginFromCenter = 0);
    ~PasswordTextbox();


private:
    QTimer *refreshTexboxTimer;
    QString showPassword;

public:
    void slotTextBoxKeyPressed(KEY_TYPE_e keyType, QString str);
    bool passwordDoneValidation();
    void setInputText (QString str);

    void changeLabel(QString str);

    void asciiCharKeyPressed(QKeyEvent *event);
    void backspaceKeyPressed(QKeyEvent *event);
    void deleteKeyPressed(QKeyEvent *event);
    void navigationKeyPressed(QKeyEvent *event);

    void ctrl_C_KeyPressed(QKeyEvent *event);
    void ctrl_V_KeyPressed(QKeyEvent *event);
    void ctrl_X_KeyPressed(QKeyEvent *event);
    void ctrl_Y_KeyPressed(QKeyEvent *event);
    void ctrl_Z_KeyPressed(QKeyEvent *event);

public slots:
    void slotRefreshTimerTimeout();
};

#endif // PASSWORDTEXTBOX_H
