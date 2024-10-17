#ifndef COSECVIDEOPOPUPUSER_H
#define COSECVIDEOPOPUPUSER_H

#include <QWidget>
#include "ApplController.h"
#include "DataStructure.h"
#include "PayloadLib.h"

#include "Controls/ReadOnlyElement.h"
#include "Controls/Heading.h"
#include "Controls/Rectangle.h"
#include "Controls/Closebuttton.h"

#define MAX_USER_DETAILS_FIELD 5

class CosecVideoPopupUser : public QWidget
{
    Q_OBJECT
public:
    explicit CosecVideoPopupUser(QWidget *parent = 0);
    ~CosecVideoPopupUser();

    void requestCosecPopUp();
    void processDeviceResponse(DevCommParam *param, QString deviceName);

signals:
    void sigObjectDelete();

public slots:
    void slotButtonClick(int);

private:

    Rectangle*          backGround;
    CloseButtton*       closeButton;
    Heading*            heading;
    Heading*            eventCodeLabel;
    ReadOnlyElement*    userDetails[MAX_USER_DETAILS_FIELD];

    ApplController*     applController;
    PayloadLib*         payloadLib;
};

#endif // COSECVIDEOPOPUPUSER_H
