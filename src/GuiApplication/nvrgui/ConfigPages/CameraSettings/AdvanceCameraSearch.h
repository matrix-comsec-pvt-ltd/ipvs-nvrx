#ifndef ADVANCECAMERASEARCH_H
#define ADVANCECAMERASEARCH_H

#include <QWidget>
#include "Controls/Rectangle.h"
#include "Controls/Closebuttton.h"
#include "Controls/Heading.h"
#include "Controls/CnfgButton.h"
#include "Controls/PickList.h"
#include "Controls/TextBox.h"
#include "Controls/PasswordTextbox.h"
#include "Controls/Ipv4TextBox.h"
#include "Controls/InfoPage.h"
#include "PayloadLib.h"

#include "KeyBoard.h"

#define DFLT_IP_ADDR1       "192.168.1.100"
#define DFLT_IP_ADDR2       "192.168.1.150"
#define DFLT_ADV_HTTP_PORT   "80"

typedef enum{

    ADVNC_SERCH_CAM_CLS_BUTN,
    ADVNC_SERCH_CAM_IP_RANGE1_TXTBOX,
    ADVNC_SERCH_CAM_IP_RANGE2_TXTBOX,
    ADVNC_SERCH_CAM_BRAND_PICKLIST,
    ADVNC_SERCH_CAM_HTTP_TXTBOX,
    ADVNC_SERCH_CAM_USERNAME_TXTBOX,
    ADVNC_SERCH_CAM_PASSWORD_TXTBOX,
    ADVNC_SERCH_CAM_SEARCH_BUTN,
    ADVNC_SERCH_CAM_CANCEL_BUTN,

    MAX_ADVNC_SERCH_CTRL
}ADVNC_SERCH_CTRL_e;


class AdvanceCameraSearch : public KeyBoard
{
    Q_OBJECT

public:
    explicit AdvanceCameraSearch(QString ipAddr1,
                                 QString ipAddr2,
                                 QString httpPort,
                                 PayloadLib *payloadlib,
                                 QWidget *parent = 0);
    ~AdvanceCameraSearch();

    void processDeviceResponse (DevCommParam *param, QString deviceName);

    void paintEvent (QPaintEvent *);
    void showEvent (QShowEvent *event);

    //keyboard support added
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void insertKeyPressed(QKeyEvent *event);

signals:
    void sigCreateCMDRequest(SET_COMMAND_e,quint8);
    void sigAdvanceSearchRequest(QString,QString,QString);
    void sigObjectDelete(bool);

public slots:
    void slotButtonClick(int);
    void slotValueChanged(quint8,QString,int);
    void slotUpdateCurrentElement(int);
    void slotInfoPageBtnclick(int);
    void slotIpStartDone(quint32);
    void slotIpEndDone(quint32);
    void slotIpTextLoadInfoPage(quint32);

private:

    Rectangle*          m_backGround;
    Heading*            m_heading;
    CloseButtton*       m_closeButton;

    PickList*           m_cameraBrandNamePicklist;


    TextBox*            m_usernameTextBox;
    TextboxParam*       m_usernameTextBoxParam;

    PasswordTextbox*    m_passwordTextBox;
    TextboxParam*       m_passwordTextBoxParam;

    TextBox*            m_httpPortTextBox;
    TextboxParam*       m_httpPortTextBoxParam;

    Ipv4TextBox*        m_ipAddressRangeBox1;
    Ipv4TextBox*        m_ipAddressRangeBox2;

    CnfgButton*         m_searchBtn;
    CnfgButton*         m_cancleBtn;

    NavigationControl*  m_elementlist[MAX_ADVNC_SERCH_CTRL];
    quint8              m_currElement;

    PayloadLib*         payloadLib;
    InfoPage*           infoPage;

    bool validationDone();
    void createDefaultElements();

    void takeLeftKeyAction();
    void takeRightKeyAction();
};

#endif // ADVANCECAMERASEARCH_H
