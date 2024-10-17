#include "AboutUs.h"
#include <QPainter>
#include <QMouseEvent>

#define ABOUT_US_WIDTH          SCALE_WIDTH(450)
#define ABOUT_US_HEIGHT         SCALE_HEIGHT(245)

AboutUs::AboutUs(QWidget *parent) :
    QWidget (parent)
{
    QString StrInfo = "";
    QString version = "";
    DEV_TABLE_INFO_t devTableInfo;

    this->setGeometry ((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - ABOUT_US_WIDTH) / 2 )),
                       (ApplController::getYPosOfScreen() + (ApplController::getHeightOfScreen() -ABOUT_US_HEIGHT) / 2 ),
                       ABOUT_US_WIDTH,
                       ABOUT_US_HEIGHT);

    applController =  ApplController::getInstance();
    applController->getFirmwareVersion(LOCAL_DEVICE_NAME, version);
    applController->GetDeviceInfo(LOCAL_DEVICE_NAME, devTableInfo);

    QString productName;

    if(deviceRespInfo.maxAnalogCameras == 0)
    {
        productName = "MATRIX SATATYA NVR";
    }
    else if (deviceRespInfo.maxIpCameras == 0)
    {
        productName = "MATRIX SATATYA DVR";
    }
    else
    {
        productName = "MATRIX SATATYA HVR";
    }

    StrInfo = QString("\n") + Multilang("Firmware Version") + ": " + version + QString("\n") +
            Multilang("Build Date and Time") + ": " + __DATE__ + " " + __TIME__ + QString("\n") +
            Multilang("Device Model") + ": " + deviceModelString[devTableInfo.productVariant] +
            QString("\n\n\n") +
            QString("Copyright Matrix Comsec")+
            QString("\n") +
            QString("www.MatrixComSec.com")+
            QString("\n\n") +
            QString("All rights are reserved.");

    backGround = new Rectangle(0,0,
                               ABOUT_US_WIDTH,
                               ABOUT_US_HEIGHT,
                               SCALE_WIDTH(RECT_RADIUS),
                               NORMAL_BKG_COLOR,
                               NORMAL_BKG_COLOR,this);

    pageHeading = new Heading(ABOUT_US_WIDTH/2, SCALE_HEIGHT(25), productName, this,
                              HEADING_TYPE_1,
                              SCALE_FONT(20));

    closeButtton = new CloseButtton((ABOUT_US_WIDTH - SCALE_WIDTH(20)),
                                    SCALE_HEIGHT(20),
                                    this);
    connect (closeButtton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    textLabel = new TextLabel(SCALE_WIDTH(25),SCALE_HEIGHT(35),NORMAL_FONT_SIZE,StrInfo,this);

    this->show();
    closeButtton->forceActiveFocus();
}

AboutUs:: ~AboutUs()
{
    DELETE_OBJ(backGround);
    DELETE_OBJ(pageHeading);
    disconnect (closeButtton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    DELETE_OBJ(closeButtton);
    DELETE_OBJ(textLabel);
}

void AboutUs::showEvent(QShowEvent * event)
{
    QWidget::showEvent(event);
    closeButtton->forceActiveFocus();
}

void AboutUs::slotButtonClick(int)
{
    emit sigClosePage(ABOUT_US_BUTTON);
}
