#ifndef OSDUPDATE_H
#define OSDUPDATE_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "DeviceClient/CommandRequest/CommandRequest.h"
#include "ApplController.h"
#include "PayloadLib.h"

class OsdUpdate : public QThread
{
    Q_OBJECT
public:
    explicit OsdUpdate(quint8 cameraCount);
    // destructor function
    ~OsdUpdate ();

    void updateServerInfo();
    void updateTimeCountForOsdUpdate(quint8 count);
    void updateAllOSD(bool);

    void run();

private:
    bool                    m_isAllOSDUpdate;
    bool                    m_isFetchDateTimeForOsdUpdate;
    bool                    m_isTimeInitDone;

    quint8                  m_cameraCount;
    quint32                 m_timerCountForOsdUpdate;

    SERVER_SESSION_INFO_t   m_sesionInfo;

    QMutex                  m_OsdParamUpdateLock;
    QDateTime               m_osdUpdateDateTime;

    CommandRequest*         m_commandRequest;    
    ApplController*         m_applController;

    void updateOSDParam();
    void getDateTimeForOsdUpdate();    
};

#endif // OSDUPDATE_H
