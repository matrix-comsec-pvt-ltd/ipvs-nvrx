#ifndef SYSTEMDETAIL_H
#define SYSTEMDETAIL_H

#include <QWidget>
#include "EnumFile.h"
#include "HealthStatus.h"
#include "AdvanceDetails.h"

class SystemDetail : public QWidget
{
    Q_OBJECT
public:
    explicit SystemDetail(QWidget *parent = 0);
    ~SystemDetail();

    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void updateDeviceList(void);
private:
    HealthStatus *m_healthStatus;
    AdvanceDetails *m_advanceDetails;

signals:
    void sigClosePage(TOOLBAR_BUTTON_TYPE_e btnType);

public slots:
    void slotCloseButtonClicked(TOOLBAR_BUTTON_TYPE_e btnType);
    void slotShowHealthStatus(QString devName, bool isCamFieldToShow);
    void slotShowAdvanceDetail(QString devName, MENU_TAB_e tabToShow);
};

#endif // SYSTEMDETAIL_H
