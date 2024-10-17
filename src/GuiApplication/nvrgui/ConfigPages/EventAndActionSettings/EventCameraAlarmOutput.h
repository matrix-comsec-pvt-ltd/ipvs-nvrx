#ifndef EVENTCAMERAALARMOUTPUT_H
#define EVENTCAMERAALARMOUTPUT_H

#include <QWidget>

#include "Controls/SpinBox.h"
#include "Controls/Heading.h"
#include "Controls/Rectangle.h"
#include "Controls/Closebuttton.h"
#include "Controls/ElementHeading.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/CnfgButton.h"
#include "Controls/DropDown.h"
#include "Controls/TextLabel.h"

typedef enum{

    EVENT_CAM_ALAM_CLS,
    EVENT_CAM_SPINBOX,
    EVENT_CAM_ALAM_ALL_CHECKBOX,
    EVENT_CAM_ALAM1_CHECKBOX,
    EVENT_CAM_ALAM2_CHECKBOX,
    EVENT_CAM_ALAM3_CHECKBOX,
    EVENT_CAM_OK_BTN,
    EVENT_CAM_CANCEL_BTN,

    MAX_EVENT_CAM_ALAM_CTRL
}EVENT_CAM_ALAM_CTRL_e;

class EventCameraAlarmOutput : public KeyBoard
{
    Q_OBJECT
public:
    explicit EventCameraAlarmOutput(quint8 index,
                                    QMap<quint8, QString> cameraList,
                                    bool* camAlarmStatus,
                                    quint8 &camIndex,
                                    QWidget *parent=0);
    ~EventCameraAlarmOutput();

    void paintEvent (QPaintEvent *);

    void showEvent (QShowEvent *event);
    void navigationKeyPressed(QKeyEvent *event);
    void backTab_KeyPressed(QKeyEvent *event);
    void tabKeyPressed(QKeyEvent *event);
    void escKeyPressed(QKeyEvent *event);
    void insertKeyPressed(QKeyEvent *event);

signals:
    void sigDeleteObject(quint8);
    
public slots:
    void slotButtonClick(int);
    void slotOptionsClicked(OPTION_STATE_TYPE_e, int tIndexInPage);
    void slotSpinboxValueChange(QString,quint32);
    void slotUpdateCurrentElement(int index);

private:

    quint8*             camSelected;
    quint8              indexInPage;
    Rectangle*          backGround;
    CloseButtton*       closeButton;
    Heading*            heading;
    ElementHeading*     elementHeading;
    CnfgButton*         okButton;
    CnfgButton*         cancelButton;

    DropDown*           cameraNameDropDownBox;
    BgTile*             numberTile;
    TextLabel*          camAlaramCheckBoxNumber[MAX_CAM_ALARM + 1];
    OptionSelectButton* camAlaramCheckBox[MAX_CAM_ALARM + 1];
    bool*               cameraAlarmStatus;

    NavigationControl* m_elementlist[MAX_EVENT_CAM_ALAM_CTRL];
    quint8 currElement;
    
    void assignValues();
    void resetValues();
    void takeLeftKeyAction();
    void takeRightKeyAction();

};

#endif // EVENTCAMERAALARMOUTPUT_H
