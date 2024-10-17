#ifndef WIZARDCAMERAINFORMATION_H
#define WIZARDCAMERAINFORMATION_H

#include <QWidget>
#include "KeyBoard.h"
#include "Controls/Rectangle.h"
#include "Controls/Closebuttton.h"
#include "Controls/Heading.h"
#include "Controls/ControlButton.h"
#include "Controls/TableCell.h"
#include "Controls/TextLabel.h"
#include "NavigationControl.h"
#include "Elidedlabel.h"
#include "Controls/ElementHeading.h"

#define MAX_CAMERA_ON_PAGE      8
#define MAX_INFO_FIELDS         5

typedef enum
{
    WIZ_CAM_INFO_CLOSE_BUTTON,
    WIZ_CAM_INFO_SET_BUTTON,
    WIZ_CAM_INFO_PREVIOUS_BUTTON = (WIZ_CAM_INFO_SET_BUTTON + MAX_CAMERA_ON_PAGE),
    WIZ_CAM_INFO_NEXT_BUTTON,
    WIZ_MAX_CAM_INFO_CTRL
}WIZ_CAM_INFO_CTRL_e;


class WizardCameraInformation : public KeyBoard
{
    Q_OBJECT
public:
    explicit WizardCameraInformation(QStringList cameraIndex,
                               QStringList cameraIp,
                               QStringList cameraName,
                               QStringList cameraState , QWidget *parent=0,
                               quint8 selIndex = 0);

    ~WizardCameraInformation();

signals:
    void sigObjectDelete(quint8);

public slots:
    void slotButtonClick(int);

private:

    // private variable
    QStringList cameraIndexList;
    QStringList cameraIpList;
    QStringList cameraNameList;
    QStringList cameraStateList;

    Rectangle*          m_backGround;
    Heading*            m_heading;
    CloseButtton*       m_closeButton;

    TableCell*          m_caminfoHeading[MAX_INFO_FIELDS];
    TextLabel*          m_caminfoHeadingStr[MAX_INFO_FIELDS];
    ElidedLabel*        m_caminfoElideStr;

    TableCell*          m_caminfoSrNum[MAX_CAMERA_ON_PAGE];
    TextLabel*          m_caminfoSrNumStr[MAX_CAMERA_ON_PAGE];

    TableCell*          m_caminfoIp[MAX_CAMERA_ON_PAGE];
    TextLabel*          m_caminfoIpStr[MAX_CAMERA_ON_PAGE];

    TableCell*          m_caminfoCamName[MAX_CAMERA_ON_PAGE];
    TextLabel*          m_caminfoCamNameStr[MAX_CAMERA_ON_PAGE];

    TableCell*          m_caminfoCamState[MAX_CAMERA_ON_PAGE];
    TextLabel*          m_caminfoCamStateStr[MAX_CAMERA_ON_PAGE];

    TableCell*          m_caminfoSet[MAX_CAMERA_ON_PAGE];
    ControlButton*      m_caminfoSetButton[MAX_CAMERA_ON_PAGE];

    ControlButton*      m_previousButton;
    ControlButton*      m_nextButton;

    quint8              m_maxListCount;
    quint8              m_currentPageNo;
    quint8              m_maximumPages;
    quint8              m_selIndex;

    TextLabel*          m_caminfoBottomString;
    ElementHeading*     elementHeading;

    // private Function
    void initilizeVariable();
    void createDefaultElements();
    void showCamInfo();
    void updateNavigationControlStatus();
};

#endif // WIZARDCAMERAINFORMATION_H

