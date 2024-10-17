#ifndef AUTOADDCAMERA_H
#define AUTOADDCAMERA_H

#include <QWidget>
#include "Controls/TextBox.h"
#include "Controls/ElementHeading.h"
#include "Controls/CnfgButton.h"
#include "Controls/ConfigPageControl.h"
#include "Controls/PasswordTextbox.h"
#include "Controls/Rectangle.h"
#include "Controls/Closebuttton.h"
#include "Controls/Heading.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/InfoPage.h"
#include "ConfigField.h"

#include "KeyBoard.h"

typedef enum
{
    AUTO_ADD_CLOSE_BUTTON,
    AUTO_ADD_CAM_TCP_PORT,
    AUTO_ADD_CAM_POLL_DURATION,
    AUTO_ADD_CAM_POLL_INTERVAL,
    AUTO_ADD_CAM_OK_BUTTON,
    AUTO_ADD_CAM_CANCEL_BUTTON,
    MAX_AUTO_ADD_CAM_ELEMETS
}AUTO_ADD_CAM_ELEMENT_LIST_e;

typedef enum
{
    FIELD_AUTO_ADD_TCP_PORT,
    FIELD_AUTO_ADD_POLL_DURATION,
    FIELD_AUTO_ADD_POLL_INTERVAL,
    MAX_FIELD_AUTO_ADD_CAM

}AUTO_ADD_CAM_FIELD_LIST_e;

class AutoAddCamera : public KeyBoard
{
    Q_OBJECT

private:
    Heading         *pageHeading;
    Rectangle       *backGround;
    CloseButtton    *closeButtton;

    TextBox         *tcpPortTextBox;
    TextboxParam    *tcpPortTextBoxParam;

    TextBox         *pollDurationTextBox;
    TextboxParam    *pollDurationTextBoxParam;

    TextBox         *pollIntervalTextBox;
    TextboxParam    *pollIntervalTextBoxParam;


    CnfgButton*         cancleBtn;
    CnfgButton*         okBtn;

    NavigationControl*  elementlist[MAX_AUTO_ADD_CAM_ELEMETS];
    quint8              m_currElement;
    InfoPage            *infoPage;
    QStringList         *autoAddCamList;

public:
    explicit    AutoAddCamera(QStringList *elementList,QWidget *parent = 0);

    ~AutoAddCamera();

    void createDefaultElements();

    void navigationKeyPressed(QKeyEvent *event);
    void backTab_KeyPressed(QKeyEvent *event);
    void tabKeyPressed(QKeyEvent *event);
    void escKeyPressed(QKeyEvent *event);
    void insertKeyPressed(QKeyEvent *event);
    void takeLeftKeyAction();
    void takeRightKeyAction();
    void paintEvent (QPaintEvent *);

    void fillRecords();
    void setRecords();
public slots:
    void slotInfoPageBtnclick(int);
    void slotUpdateCurrentElement(int index);
    void slotButtonClick(int index);
    void slotTextBoxLoadInfopage(int index, INFO_MSG_TYPE_e msgType);

signals:
    void sigObjectDelete();

};

#endif // AUTOADDCAMERA_H
