#include "BroadbandStatus.h"

#include <QKeyEvent>
#include <QPainter>

#define BB_STATUS_PAGE_WIDTH               SCALE_WIDTH(536)
#define BB_STATUS_PAGE_HEIGHT              SCALE_HEIGHT(600)
#define BB_STATUS_PAGE_YOFFSET             SCALE_HEIGHT(180)
#define BB_STATUS_READONLY_LARGE_WIDTH     SCALE_WIDTH(290)
#define LEFT_MARGIN_FROM_CENTER            SCALE_WIDTH(60)

static const QString readOnlyLabel[BROADBAND_STATUS_MAX_ELEMENT] =
{
    "Connection Status",
    "IPv4 Address",
    "Subnet Mask",
    "Default Gateway",
    "Preferred DNS",
    "Alternate DNS",
    "IPv6 Address",
    "Prefix Length",
    "Default Gateway",
    "Preferred DNS",
    "Alternate DNS"
};

static const QString modemStatusStr[] =
{
   "No Modem",
   "Not Connected",
   "Modem connected"
};

BroadbandStatus::BroadbandStatus(QWidget *parent) : KeyBoard(parent)
{
    this->setGeometry(0, 0, parent->width(), parent->height());
    payloadLib = new PayloadLib();

    pageBgRect = new Rectangle(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) +
                               (SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - BB_STATUS_PAGE_WIDTH)/2,
                               BB_STATUS_PAGE_YOFFSET,
                               BB_STATUS_PAGE_WIDTH,
                               BB_STATUS_PAGE_HEIGHT,
                               0, BORDER_2_COLOR, NORMAL_BKG_COLOR, this, 1);

    statusHeading = new Heading(pageBgRect->x() + (pageBgRect->width()/2),
                                pageBgRect->y() + SCALE_HEIGHT(25),
                                "Status", this);

    closeBtn = new CloseButtton(pageBgRect->x() + pageBgRect->width() - SCALE_WIDTH(20),
                                pageBgRect->y() + SCALE_HEIGHT(25),
                                this);
    connect(closeBtn,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCloseButtonClick(int)));

    readOnlyElements[BROADBAND_STATUS_CONNECTION_STATUS] = new ReadOnlyElement(pageBgRect->x() + (pageBgRect->width() - BGTILE_MEDIUM_SIZE_WIDTH)/2,
                                                                               pageBgRect->y() + SCALE_HEIGHT(60),
                                                                               BGTILE_MEDIUM_SIZE_WIDTH,
                                                                               BGTILE_HEIGHT,
                                                                               SCALE_WIDTH(READONLY_LARGE_WIDTH),
                                                                               READONLY_HEIGHT,
                                                                               "", this, COMMON_LAYER,
                                                                               -1, SCALE_WIDTH(10), readOnlyLabel[BROADBAND_STATUS_CONNECTION_STATUS]);

    eleHeading[IP_ADDR_FAMILY_IPV4] = new ElementHeading(readOnlyElements[BROADBAND_STATUS_CONNECTION_STATUS]->x(),
                                                         readOnlyElements[BROADBAND_STATUS_CONNECTION_STATUS]->y() + BGTILE_HEIGHT,
                                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                                         BGTILE_HEIGHT,
                                                         "IPv4",
                                                         NO_LAYER, this, false,
                                                         SCALE_WIDTH(25), NORMAL_FONT_SIZE, true);

    for (quint8 index = BROADBAND_STATUS_IPV4_ADDRESS; index < BROADBAND_STATUS_IPV6_ADDRESS; index++)
    {

        readOnlyElements[index] = new ReadOnlyElement(eleHeading[IP_ADDR_FAMILY_IPV4]->x(),
                                                      eleHeading[IP_ADDR_FAMILY_IPV4]->y() + (index * BGTILE_HEIGHT),
                                                      BGTILE_MEDIUM_SIZE_WIDTH,
                                                      BGTILE_HEIGHT,
                                                      SCALE_WIDTH(READONLY_LARGE_WIDTH),
                                                      READONLY_HEIGHT,
                                                      "", this, COMMON_LAYER,
                                                      -1, SCALE_WIDTH(10), readOnlyLabel[index], "", "",
                                                      SCALE_FONT(10), true, NORMAL_FONT_COLOR, LEFT_MARGIN_FROM_CENTER);
    }

    eleHeading[IP_ADDR_FAMILY_IPV6] = new ElementHeading(readOnlyElements[BROADBAND_STATUS_IPV4_ALTERNATE_DNS]->x(),
                                                         readOnlyElements[BROADBAND_STATUS_IPV4_ALTERNATE_DNS]->y() + BGTILE_HEIGHT,
                                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                                         BGTILE_HEIGHT,
                                                         "IPv6",
                                                         NO_LAYER, this, false,
                                                         SCALE_WIDTH(25), NORMAL_FONT_SIZE, true);

    for (quint8 index = BROADBAND_STATUS_IPV6_ADDRESS; index < BROADBAND_STATUS_MAX_ELEMENT; index++)
    {
        readOnlyElements[index] = new ReadOnlyElement(eleHeading[IP_ADDR_FAMILY_IPV6]->x(),
                                                      eleHeading[IP_ADDR_FAMILY_IPV6]->y() +
                                                      ((index - BROADBAND_STATUS_IPV6_ADDRESS + 1) * BGTILE_HEIGHT),
                                                      BGTILE_MEDIUM_SIZE_WIDTH,
                                                      BGTILE_HEIGHT,
                                                      BB_STATUS_READONLY_LARGE_WIDTH,
                                                      READONLY_HEIGHT,
                                                      "", this, COMMON_LAYER,
                                                      -1, SCALE_WIDTH(10), readOnlyLabel[index], "", "",
                                                      SCALE_FONT(10), true, NORMAL_FONT_COLOR, LEFT_MARGIN_FROM_CENTER);
    }

    closeBtn->forceActiveFocus();
    this->show();
}
BroadbandStatus::~BroadbandStatus()
{
    delete payloadLib;
    delete pageBgRect;
    delete statusHeading;

    disconnect(closeBtn,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotCloseButtonClick(int)));
    delete closeBtn;

    for(quint8 index = 0; index < BROADBAND_STATUS_MAX_ELEMENT; index++)
    {
        delete readOnlyElements[index];
    }

    delete eleHeading[IP_ADDR_FAMILY_IPV4];
    delete eleHeading[IP_ADDR_FAMILY_IPV6];
}

void BroadbandStatus::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QColor color;

    color.setAlpha(0);
    painter.setBrush(QBrush(color));
    painter.setPen(Qt::NoPen);

    painter.drawRect(QRect(0, 0, SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH), SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT)));
    color.setAlpha(150);
    painter.setBrush(QBrush(color));
    painter.setPen(Qt::NoPen);

    painter.drawRoundedRect(QRect(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                                  SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) -SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT),
                                  SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH), SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)),
                            SCALE_WIDTH(15), SCALE_HEIGHT(15));
    QWidget::paintEvent(event);
}

void BroadbandStatus::processDeviceResponse(DevCommParam *param)
{
    payloadLib->parseDevCmdReply(true, param->payload);
    quint32 modemStatus = payloadLib->getCnfgArrayAtIndex(0).toUInt();
    if (modemStatus >= 3)
    {
        modemStatus = 2;
    }

    readOnlyElements[0]->changeValue(modemStatusStr[modemStatus]);
    for(quint8 index = 1; index < BROADBAND_STATUS_MAX_ELEMENT; index++)
    {
        if ((index == BROADBAND_STATUS_IPV6_PREFIX_LENGTH)
                && (readOnlyElements[BROADBAND_STATUS_IPV6_ADDRESS]->getCurrentValue() == ""))
        {
            readOnlyElements[index]->changeValue("");
        }
        else
        {
            readOnlyElements[index]->changeValue(payloadLib->getCnfgArrayAtIndex(index).toString());
        }
    }
    closeBtn->forceActiveFocus();
}

void BroadbandStatus::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    closeBtn->forceActiveFocus();
}

void BroadbandStatus::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    slotCloseButtonClick(0);
}

void BroadbandStatus::slotCloseButtonClick(int)
{
    emit sigStatusPageClosed();
}

void BroadbandStatus::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
}

void BroadbandStatus::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void BroadbandStatus::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void BroadbandStatus::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void BroadbandStatus::ctrl_S_KeyPressed(QKeyEvent *event)
{
    event->accept();
}

void BroadbandStatus::ctrl_D_KeyPressed(QKeyEvent *event)
{
    event->accept();
}

void BroadbandStatus::enterKeyPressed(QKeyEvent *event)
{
    event->accept();
}
