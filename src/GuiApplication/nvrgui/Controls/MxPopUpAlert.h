#ifndef POPUPALERT_H
#define POPUPALERT_H

// Qt Libraries
#include <QWidget>
#include <QMouseEvent>
#include <QMutex>

// matrix library
#include "EnumFile.h"
#include "Controls/TextLabel.h"
#include "Rectangle.h"
#include "Controls/ImageControls/Image.h"
#include "ApplController.h"

#define LEFT_MARGIN     SCALE_WIDTH(10)
#define TOP_MARGIN      SCALE_HEIGHT(10)

//enum for Alert Message Images
typedef enum
{
    AUTO_CNFG_CAM,
    AUTO_TIMEZONE_UPDATE,
    MAX_POP_UP_ALERT_NAME

}POP_UP_ALERT_NAME_e;

typedef enum
{
    INFO_POP_UP,
    MAX_POP_UP_ALERT

}POP_UP_ALERT_TYPE_e;

typedef enum
{
    POP_UP_ALRT_CTRL_CLOSE,
    POP_UP_ALRT_CTRL_NO_HYPLINK,
    POP_UP_ALRT_CTRL_YES,
    MAX_POP_UP_ALRT_CTRL
}POP_UP_ALERT_CTRL_e;

typedef enum
{
    ONLY_MSG,
    MSG_WITH_HYPERLINK,
    MSG_WITH_TWO_CTRL,
    MAX_POP_UP_ALRT_MODE
}POP_UP_ALERT_MODE_e;

class MxPopUpAlert : public QWidget
{
    Q_OBJECT

public:
    explicit MxPopUpAlert(quint32 xParam, quint32 yParam,
                          POP_UP_ALERT_NAME_e popUpName,
                          POP_UP_ALERT_MODE_e popUpMode,
                          QWidget *parent = 0,
                          POP_UP_ALERT_TYPE_e popUpType = INFO_POP_UP,
                          quint32 widthOffset = 0,
                          quint32 heightOffset = 0);
    ~MxPopUpAlert();
    void createDefaultComponent();
    void deleteComponent();
    void closePageAction(int index);
    void setPopUpMode();
    void addPopUp(POP_UP_ALERT_NAME_e popUpName,
                  POP_UP_ALERT_MODE_e popUpMode,
                  POP_UP_ALERT_TYPE_e popUpType = INFO_POP_UP);
    void changePopUp();
    POP_UP_ALERT_NAME_e getPopUpName();
    void updateGeometry(quint32 xParam, quint32 yParam);


signals:
    void sigCloseAlert(int index,bool isQueueEmpty);

public slots:
    void slotTextClicked(int index);
    void slotTextLableHover(int,bool);
    void slotImageClicked(int);

private:
    bool                m_isLineOnHoverNoBtnNeeded;
    bool                m_isClrChgOnHoverNoBtnNeeded;
    bool                m_isLineOnHoverYesBtnNeeded;
    bool                m_isClrChgOnHoverYesBtnNeeded;
    quint32             m_xParamParent;
    quint32             m_yParamparent;
    quint32             m_xParam;
    quint32             m_yParam;
    quint32             m_width;
    quint32             m_height;
    quint32             m_widthOffset;
    quint32             m_heightOffset;
    QList<POP_UP_ALERT_NAME_e> m_popUpNameQueue;
    QList<POP_UP_ALERT_MODE_e> m_popUpModeQueue;
    QList<POP_UP_ALERT_TYPE_e> m_popUpTypeQueue;
    POP_UP_ALERT_TYPE_e m_popUpType;
    POP_UP_ALERT_NAME_e m_popUpName;
    POP_UP_ALERT_MODE_e m_popUpMode;
    TextLabel*          m_headingLabel;
    TextLabel*          m_infoMsgLable;
    TextLabel*          m_yesCtrlLable;
    TextLabel*          m_noCtrlLable;
    Rectangle*          m_backgroundRectangle;
    Image*              m_alertImage;
    Image*              m_closeImage;

};

#endif // POPUPALERT_H
