#ifndef BROADBANDSTATUS_H
#define BROADBANDSTATUS_H

#include <QWidget>

#include "Controls/Rectangle.h"
#include "Controls/Heading.h"
#include "Controls/Closebuttton.h"
#include "Controls/ReadOnlyElement.h"
#include "Controls/ElementHeading.h"
#include "PayloadLib.h"
#include "DataStructure.h"

typedef enum
{
    BROADBAND_STATUS_CONNECTION_STATUS = 0,
    BROADBAND_STATUS_IPV4_ADDRESS,
    BROADBAND_STATUS_IPV4_SUBNET_MASK,
    BROADBAND_STATUS_IPV4_DEFAULT_GATEWAY,
    BROADBAND_STATUS_IPV4_PREFERRED_DNS,
    BROADBAND_STATUS_IPV4_ALTERNATE_DNS,
    BROADBAND_STATUS_IPV6_ADDRESS,
    BROADBAND_STATUS_IPV6_PREFIX_LENGTH,
    BROADBAND_STATUS_IPV6_DEFAULT_GATEWAY,
    BROADBAND_STATUS_IPV6_PREFERRED_DNS,
    BROADBAND_STATUS_IPV6_ALTERNATE_DNS,
    BROADBAND_STATUS_MAX_ELEMENT
}BROADBAND_STATUS_ELEMENTS_e;

class BroadbandStatus :public KeyBoard
{
    Q_OBJECT
private:
    Rectangle       *pageBgRect;
    Heading         *statusHeading;
    CloseButtton    *closeBtn;
    ReadOnlyElement *readOnlyElements[BROADBAND_STATUS_MAX_ELEMENT];
    PayloadLib      *payloadLib;
    ElementHeading  *eleHeading[IP_ADDR_FAMILY_MAX];

public:
    explicit BroadbandStatus(QWidget *parent = 0);
    ~BroadbandStatus();

    void createDefaultElement();
    void paintEvent (QPaintEvent *event);
    void processDeviceResponse (DevCommParam *param);
    void showEvent (QShowEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void insertKeyPressed(QKeyEvent *event);
    virtual void ctrl_S_KeyPressed(QKeyEvent *event);
    virtual void ctrl_D_KeyPressed(QKeyEvent *event);
    virtual void enterKeyPressed(QKeyEvent *event);
    virtual void navigationKeyPressed(QKeyEvent *event);

signals:
    void sigStatusPageClosed();

public slots:
    void slotCloseButtonClick(int);
};

#endif // BROADBANDSTATUS_H
