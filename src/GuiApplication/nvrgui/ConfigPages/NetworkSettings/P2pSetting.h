#ifndef P2PSETTING_H
#define P2PSETTING_H

#include <QWidget>
#include <QPainter>
#include <QPainter>
#include <QPixmap>
#include <QPaintEvent>
#include <QObject>
#include <QtCore/QFileInfo>
#include <png.h>
#include "Controls/ConfigPageControl.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ElementHeading.h"
#include "Controls/ImageControls/Image.h"
#include "Controls/DrawQr.h"
#include "Elidedlabel.h"

/* Defines the total number of QR codes */
#define TOTAL_QR_ELEMENTS              3

class P2pSetting : public ConfigPageControl
{
    Q_OBJECT
private:

    OptionSelectButton*         m_enableP2p;
    OptionSelectButton*         m_p2pRelayServerFallback;
    ElementHeading*             m_statusLabel;
    ElementHeading*             m_macLabel;
    ElementHeading*             m_applLabelAndroid;
    ElementHeading*             m_applLabelIos;
    ElementHeading*             m_androidAppLabel;
    ElementHeading*             m_iosAppLabel;
    Image*                      m_statusImage;
    DrawQr*                     m_macQr;
    DrawQr*                     m_iosQr;
    DrawQr*                     m_androidQr;
    QString                     m_macAddrString, m_devName;
    QRcode*                     m_qr;

public:
    explicit P2pSetting(QString devName, QWidget *parent = 0);
    ~P2pSetting(void);

    void createDefaultComponent(void);
    void createQrComponents(void);
    void getMacAddress(void);
    void validateFileAndRegenerate(void);
    void GeneratePNG(const QRcode *qrcode, const char *outfile);
    void getP2pStatus(void);
    void createPayload(REQ_MSG_ID_e msgType);
    void sendCommand(SET_COMMAND_e cmdType, int totalfeilds = 0);

    /* Virtual function */
    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void getConfig(void);
    void defaultConfig(void);
    void saveConfig(void);
    void handleInfoPageMessage(int index);

public slots:
    void slotOptionButtonSelected(OPTION_STATE_TYPE_e currentState, int indexInPage);
};

#endif // P2PSETTING_H
