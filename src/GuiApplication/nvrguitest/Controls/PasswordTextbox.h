#ifndef PASSWORDTEXTBOX_H
#define PASSWORDTEXTBOX_H

#include "Controls/TextBox.h"
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
                    bool isNavigationEnable = true);
    ~PasswordTextbox();


private:
    QTimer *refreshTexboxTimer;
    QString showPassword;

public:
    void slotTextBoxKeyPressed(KEY_TYPE_e keyType, QString str);
    void passwordDoneValidation();
    void setInputText (QString str);

    void changeLabel(QString str);

public slots:
    void slotRefreshTimerTimeout();
};

#endif // PASSWORDTEXTBOX_H
