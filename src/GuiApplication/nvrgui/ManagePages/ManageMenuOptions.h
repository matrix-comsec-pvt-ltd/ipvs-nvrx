#ifndef MANAGEMENUOPTIONS_H
#define MANAGEMENUOPTIONS_H

#include <QWidget>
#include "ApplController.h"
#include "DataStructure.h"
#include "PayloadLib.h"
#include "NavigationControl.h"
#include "Controls/InfoPage.h"
#include "Controls/ProcessBar.h"
#include "ValidationMessage.h"
#include "KeyBoard.h"

#define MANAGE_PAGE_RIGHT_PANEL_STARTX                     ( MANAGE_LEFT_PANEL_WIDTH + SCALE_WIDTH(INNER_LEFT_MARGIN)  )
#define MANAGE_PAGE_RIGHT_PANEL_STARTY                     ( SCALE_HEIGHT(INNER_TOP_MARGIN) + SCALE_HEIGHT(OUTER_TOP_MARGIN)  + SCALE_HEIGHT(30 ))
#define MANAGE_PAGE_RIGHT_PANEL_WIDTH                      ( MANAGE_RIGHT_PANEL_WIDTH - SCALE_WIDTH(INNER_RIGHT_MARGIN) - SCALE_WIDTH(INNER_LEFT_MARGIN))
#define MANAGE_PAGE_RIGHT_PANEL_HEIGHT                     ( MANAGE_RIGHT_PANEL_HEIGHT - SCALE_HEIGHT(INNER_BOTTOM_MARGIN)  - SCALE_HEIGHT(INNER_TOP_MARGIN))
#define MANAGE_PAGE_RIGHT_PANEL_HEIGHT_WITHOUT_CNFGBUTTON  ( MANAGE_PAGE_RIGHT_PANEL_HEIGHT - SCALE_HEIGHT(55) )

class ManageMenuOptions :public KeyBoard, public NavigationControl
{
    Q_OBJECT

protected:
    InfoPage *infoPage;
    ProcessBar *processBar;

    QString m_currentDeviceName;
    ApplController *m_applController;
    PayloadLib *m_payloadLib;

    qint32 m_currentElement;
    NavigationControl* m_elementList[255];
    quint16 m_maxElements;

public:
    ManageMenuOptions(QString deviceName,
                      QWidget* parent,
                      quint16 maxElements);
    ~ManageMenuOptions();

    void virtual processDeviceResponse(DevCommParam *param, QString devName) = 0;
    void virtual updateStatus(QString devName, qint8 status, qint8 index, quint8, quint8);
    void virtual handleInfoPageMessage(int buttonIndex);

    void takeLeftKeyAction();
    void takeRightKeyAction();

    void forceFocusToPage(bool isFirstElement);

    void navigationKeyPressed(QKeyEvent *);
    void showEvent(QShowEvent *event);
    void tabKeyPressed(QKeyEvent *event);
    void backTab_KeyPressed(QKeyEvent *event);
//    void escKeyPressed(QKeyEvent *event);


signals:
    void sigSubHeadingChange(bool);
    void sigLanguageCfgChanged(QString str);
    void sigFocusToOtherElement(bool isPrevoiusElement);

public slots:
    void slotInfoPageBtnclick(int);
    void slotUpdateCurrentElement(int index);

};

#endif // MANAGEMENUOPTIONS_H
