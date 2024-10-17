#ifndef APPLICATIONMODE_H
#define APPLICATIONMODE_H

#include <QObject>
#include "EnumFile.h"

typedef enum
{
    MODE_NONE,
    IDLE_MODE,
    TOOLBAR_MODE,
    PAGE_MODE,
    PAGE_WITH_TOOLBAR_MODE,
    ASYNC_PLAYBACK_TOOLBAR_MODE
}APPLICATION_MODE_e;

class ApplicationMode : public QObject
{
    Q_OBJECT
private:
    APPLICATION_MODE_e applicationModeType;
    static ApplicationMode* applicationMode;
    explicit ApplicationMode(QObject *parent = 0);

public:
    static ApplicationMode* getApplicationModeInstance(QObject *parent);
    static void setApplicationMode(APPLICATION_MODE_e mode);
    static APPLICATION_MODE_e getApplicationMode();

private:
    void setMode(APPLICATION_MODE_e mode);
    APPLICATION_MODE_e getMode();

signals:
    void sigApplicationModeChanged();
};

#endif // APPLICATIONMODE_H
