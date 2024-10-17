#ifndef EVENTACTIONPTZPOS_H
#define EVENTACTIONPTZPOS_H

#include <QWidget>

#include "Controls/Rectangle.h"
#include "Controls/Closebuttton.h"
#include "Controls/Heading.h"
#include "Controls/CnfgButton.h"
#include "Controls/DropDown.h"
#include "ApplController.h"
#include "PayloadLib.h"
#include "Controls/ProcessBar.h"

typedef enum{

    EVNT_PTZ_CLS_CTRL,
    EVNT_PTZ_CAM_SPINBOX_CTRL,
    EVNT_PTZ_PRE_SPINBOX_CTRL,
    EVNT_PTZ_OK_CTRL,
    EVNT_PTZ_CANCEL_CTRL,

    MAX_EVNT_PTZ_NOTIFY_CTRL
}EVNT_PTZ_NOTIFY_CTRL_e;


class EventActionPtzPos : public KeyBoard
{
    Q_OBJECT

    ApplController* applController;
    PayloadLib* payloadLib;
    ProcessBar* processBar;

    QString currDevName;
    Rectangle* backGround;
    CloseButtton* closeButton;
    Heading* heading;

    DropDown* cameraNameDropDownBox;
    DropDown* presetPosDropDownBox;
    QMap<quint8, QString>  positionList;

    CnfgButton* okButton;
    CnfgButton* cancelButton;

    NavigationControl* m_elementlist[MAX_EVNT_PTZ_NOTIFY_CTRL];
    quint8 currElement;
    quint8 m_index;
    quint32 frmIndex;
    quint8 currentCameraIndex;
    quint8 m_maxCam;
    quint32 *camNumber;
    quint32 *presetPosition;
    QMap<quint8, QString>  cameraList;

public:
    explicit EventActionPtzPos(quint8 index,
                               QString devName,
                               quint32 &camNum,
                               quint32 &prePosition,
                               quint8 totalCam,
                               QWidget *parent = 0);
    ~EventActionPtzPos();

    void paintEvent (QPaintEvent *event);

    void processDeviceResponse(QMap<quint8, QString> &strList);
    void createPayload(REQ_MSG_ID_e msgType );
    void getConfig ();
    void fillCameraList();

    void takeLeftKeyAction();
    void takeRightKeyAction();
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
    void slotUpdateCurrentElement(int index);
    void slotSpinBoxValueChanged(QString,quint32);
};

#endif // EVENTACTIONPTZPOS_H
