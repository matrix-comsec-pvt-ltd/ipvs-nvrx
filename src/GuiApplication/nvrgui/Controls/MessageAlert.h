#ifndef MESSAGEALERT_H
#define MESSAGEALERT_H

// Qt Libraries
#include <QWidget>
#include <QMouseEvent>
#include <QTimer>

// matrix library
#include "EnumFile.h"
#include "Controls/TextLabel.h"
#include "Rectangle.h"
#include "Controls/ImageControls/Image.h"
#include "ApplController.h"

typedef enum
{
    RESTORED_MSG_ALERT,
    MAXIMIZED_MSG_ALERT,
    MAX_MSG_ALERT_MODE
}MSG_ALERT_MODE_e;

class MessageAlert : public QWidget
{
    Q_OBJECT

public:
    explicit MessageAlert(QWidget *parent = 0);
    ~MessageAlert();

    void addMessageAlert(QString deviceName,quint8 camRecFailCount);
    void closePageAction();
    void getMessageList(QMap<QString,quint8>& msgMap,QStringList& deviceList);
    void setMessageList(QMap<QString,quint8> msgMap,QStringList deviceList);
    MSG_ALERT_MODE_e getMessageAlertMode();
    void changeDisplayMode(MSG_ALERT_MODE_e msgAlertMode);

signals:
    void sigCloseAlert();

public slots:
    void slotImageClicked(int);
    void slotTimeOut();
    void slotBlinkTimeout();

private:

    QList<quint16>      m_restoredPopUpGeometryList;
    QTimer*             m_autoclearTimer;
    QTimer*             m_blinkRestoredPopUpTimer;


    TextLabel*          m_textLabel;
    Rectangle*          m_backgroundRectangle;
    Image*              m_alertImage;
    Image*              m_closeImage;
    Image*              m_restoreImage;
    Image*              m_restoredPopUpImage;
    TextLabel*          m_restoredPopUpText;
    MSG_ALERT_MODE_e    m_currentDisplayMode;

    QStringList     m_deviceList;

    QMap<QString,quint8> m_msgMapList;

    void changeMessage(bool isEventOccur = true);
    void updateElements();
};

#endif // MESSAGEALERT_H

