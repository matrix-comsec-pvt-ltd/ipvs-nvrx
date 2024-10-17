#include "MessageAlert.h"
#include "EnumFile.h"
#include "ValidationMessage.h"


#define ALERT_IMAGE_SOURCE_PATH             ":/Images_Nvrx/Alert/Alert.png"
#define CLOSE_IMAGE_SOURCE_PATH             ":/Images_Nvrx/Alert/Close/"
#define RESTORE_IMAGE_SOURCE_PATH           ":/Images_Nvrx/Alert/Restore/"
#define RESTORE_POPUP_IMAGE_SOURCE_PATH     ":/Images_Nvrx/Alert/RestorePopUp/"

#define LEFT_MARGIN                 SCALE_WIDTH(10)
#define TOP_MARGIN                  SCALE_HEIGHT(10)
#define AUTO_CLOSE_POPUP_TIMER      3000         // 3 sec
#define BLINK_RESTORED_POPUP_TIMER  1000         // 1 sec

#define POPUP_BACKGROUND_COLOR      "#b38a8a"
#define POPUP_TEXT_COLOR            "#ffffff"
#define RESTORED_POPUP_TEXT_COLOR   "#fd9a9a"


typedef enum
{
    MSG_ALRT_CTRL_ALERT,
    MSG_ALRT_CTRL_CLOSE,
    MSG_ALRT_CTRL_RESTORE,
    MSG_ALRT_CTRL_POPUP,
    MAX_MSG_ALRT_CTRL
}MESSAGE_ALERT_CTRL_e;

MessageAlert::MessageAlert(QWidget *parent) : QWidget(parent)
{
    quint16 labelWidth = 0;
    quint16 labelHeight = 0;

    TextLabel::getWidthHeight(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE),
                              RECORDING_FAIL_MSG(LOCAL_DEVICE_NAME,10),
                              labelWidth,
                              labelHeight);

    if(IS_VALID_OBJ(parent))
    {
        this->setGeometry((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - (labelWidth + (4 * LEFT_MARGIN) + SCALE_WIDTH((48 + 30)))) / 2)),
                          (ApplController::getYPosOfScreen() + SCALE_HEIGHT(50)),
                          (labelWidth + (4 * LEFT_MARGIN) + SCALE_WIDTH((48 + 30))),
                          SCALE_HEIGHT(70));
    }

    INIT_OBJ(m_backgroundRectangle);
    INIT_OBJ(m_alertImage);
    INIT_OBJ(m_textLabel);
    INIT_OBJ(m_closeImage);
    INIT_OBJ(m_restoreImage);
    INIT_OBJ(m_restoredPopUpImage);
    INIT_OBJ(m_restoredPopUpText);
    INIT_OBJ(m_autoclearTimer);
    INIT_OBJ(m_blinkRestoredPopUpTimer);
    m_currentDisplayMode = MAXIMIZED_MSG_ALERT;
    m_restoredPopUpGeometryList.clear();

    m_backgroundRectangle = new Rectangle(0,
                                          0,
                                          this->width (),
                                          this->height (),
                                          CLICKED_BKG_COLOR,
                                          this,
                                          2,
                                          2,
                                          POPUP_BACKGROUND_COLOR);
    m_alertImage = new Image(SCALE_WIDTH(10),
                             SCALE_HEIGHT(10),
                             ALERT_IMAGE_SOURCE_PATH,
                             this,
                             START_X_START_Y,
                             MSG_ALRT_CTRL_ALERT,
                             false,
                             true,
                             true,
                             false);

    m_textLabel = new TextLabel(SCALE_WIDTH(65),
                                SCALE_HEIGHT(15),
                                NORMAL_FONT_SIZE,
                                RECORDING_FAIL_MSG(LOCAL_DEVICE_NAME,0),
                                this,
                                POPUP_TEXT_COLOR,
                                NORMAL_FONT_FAMILY,
                                ALIGN_START_X_START_Y);

    m_closeImage = new Image(this->width(),
                             SCALE_HEIGHT(10),
                             CLOSE_IMAGE_SOURCE_PATH,
                             this,
                             END_X_START_Y,
                             MSG_ALRT_CTRL_CLOSE,
                             true,
                             false,
                             true,
                             false);

    if(IS_VALID_OBJ(m_closeImage))
    {
        connect (m_closeImage,
                 SIGNAL(sigImageClicked(int)),
                 this,
                 SLOT(slotImageClicked(int)));
    }

    m_restoreImage = new Image(this->width() - (m_closeImage->width()+ SCALE_WIDTH(5)),
                               SCALE_HEIGHT(10),
                               RESTORE_IMAGE_SOURCE_PATH,
                               this,
                               END_X_START_Y,
                               MSG_ALRT_CTRL_RESTORE,
                               true,
                               false,
                               true,
                               false);

    if(IS_VALID_OBJ(m_restoreImage))
    {
        connect (m_restoreImage,
                 SIGNAL(sigImageClicked(int)),
                 this,
                 SLOT(slotImageClicked(int)));
    }

    m_restoredPopUpImage = new Image( 0,
                                      0,
                                      RESTORE_POPUP_IMAGE_SOURCE_PATH,
                                      this,
                                      START_X_START_Y,
                                      MSG_ALRT_CTRL_POPUP,
                                      true,
                                      false,
                                      true,
                                      false);

    if(IS_VALID_OBJ(m_restoredPopUpImage))
    {

        m_restoredPopUpGeometryList << (ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - m_restoredPopUpImage->width())/2))
                                    << (ApplController::getYPosOfScreen() + SCALE_HEIGHT(50))
                                    << m_restoredPopUpImage->width()
                                    << m_restoredPopUpImage->height();

        connect (m_restoredPopUpImage,
                 SIGNAL(sigImageDoubleClicked(int)),
                 this,
                 SLOT(slotImageClicked(int)));


        m_restoredPopUpText = new TextLabel( (m_restoredPopUpImage->x() + m_restoredPopUpImage->width() -SCALE_WIDTH(3)),
                                             (m_restoredPopUpImage->y() + m_restoredPopUpImage->height()-SCALE_HEIGHT(2)),
                                             SCALE_FONT(SUB_HEADING_FONT_SIZE) ,
                                             "00",
                                             this,
                                             RESTORED_POPUP_TEXT_COLOR,
                                             NORMAL_FONT_FAMILY,
                                             ALIGN_END_X_END_Y,
                                             0,
                                             false,
                                             0,
                                             MSG_ALRT_CTRL_POPUP);

        if(IS_VALID_OBJ(m_restoredPopUpText))
        {
            connect (m_restoredPopUpText,
                     SIGNAL(sigTextDoubleClicked(int)),
                     this,
                     SLOT(slotImageClicked(int)));
        }
    }

    m_autoclearTimer = new QTimer(this);
    if(IS_VALID_OBJ(m_autoclearTimer))
    {
        connect (m_autoclearTimer,
                 SIGNAL(timeout()),
                 this,
                 SLOT(slotTimeOut()));
        m_autoclearTimer->setInterval(AUTO_CLOSE_POPUP_TIMER);
        m_autoclearTimer->setSingleShot(true);
    }

    m_blinkRestoredPopUpTimer = new QTimer(this);
    if(IS_VALID_OBJ(m_blinkRestoredPopUpTimer))
    {
        connect (m_blinkRestoredPopUpTimer,
                 SIGNAL(timeout()),
                 this,
                 SLOT(slotBlinkTimeout()));
        m_blinkRestoredPopUpTimer->setInterval(BLINK_RESTORED_POPUP_TIMER);
    }

    if(IS_VALID_OBJ(m_restoredPopUpImage))
    {
        m_restoredPopUpImage->setVisible(false);
        m_restoredPopUpImage->setIsEnabled(false);
    }

    if(IS_VALID_OBJ(m_restoredPopUpText))
    {
        m_restoredPopUpText->setVisible(false);
    }

    this->show ();
}

MessageAlert::~MessageAlert ()
{
    if((IS_VALID_OBJ(m_autoclearTimer)) && (m_autoclearTimer->isActive()))
    {
        m_autoclearTimer->stop();
    }

    DELETE_OBJ(m_autoclearTimer);

    if((IS_VALID_OBJ(m_blinkRestoredPopUpTimer)) && (m_blinkRestoredPopUpTimer->isActive()))
    {
        m_blinkRestoredPopUpTimer->stop();
    }
    DELETE_OBJ(m_blinkRestoredPopUpTimer);

    DELETE_OBJ(m_backgroundRectangle);
    DELETE_OBJ(m_alertImage);
    DELETE_OBJ(m_textLabel);

    if(IS_VALID_OBJ(m_closeImage))
    {
        disconnect (m_closeImage,
                    SIGNAL(sigImageClicked(int)),
                    this,
                    SLOT(slotImageClicked(int)));
        DELETE_OBJ(m_closeImage);
    }

    if(IS_VALID_OBJ(m_restoreImage))
    {
        disconnect (m_restoreImage,
                    SIGNAL(sigImageClicked(int)),
                    this,
                    SLOT(slotImageClicked(int)));
        DELETE_OBJ(m_restoreImage);
    }

    if(IS_VALID_OBJ(m_restoredPopUpImage))
    {
        disconnect (m_restoredPopUpImage,
                    SIGNAL(sigImageDoubleClicked(int)),
                    this,
                    SLOT(slotImageClicked(int)));
        DELETE_OBJ(m_restoredPopUpImage);
    }

    if(IS_VALID_OBJ(m_restoredPopUpText))
    {
        disconnect (m_restoredPopUpText,
                    SIGNAL(sigTextDoubleClicked(int)),
                    this,
                    SLOT(slotImageClicked(int)));
        DELETE_OBJ(m_restoredPopUpText);
    }
}

void MessageAlert::addMessageAlert(QString deviceName, quint8 camRecFailCount)
{
    if (deviceName == "")
    {
        return;
    }

    if (camRecFailCount > MAX_CAMERAS)
    {
        camRecFailCount = MAX_CAMERAS;
    }

    m_deviceList.removeOne(deviceName);
    m_deviceList.prepend(deviceName);
    m_msgMapList[deviceName] = camRecFailCount;
    changeMessage();
}

void MessageAlert::closePageAction()
{
    if(m_deviceList.isEmpty())
    {
        return;
    }

    m_msgMapList.remove(m_deviceList.at(0));
    m_deviceList.removeFirst();

    if(!m_deviceList.isEmpty())
    {
        changeMessage();
    }
    else
    {
        emit sigCloseAlert();
    }
}

void MessageAlert::changeMessage(bool isEventOccur)
{
    if(m_deviceList.isEmpty())
    {
        return;
    }

    if(m_currentDisplayMode == RESTORED_MSG_ALERT)
    {
        this->setGeometry(m_restoredPopUpGeometryList.at(0),
                          m_restoredPopUpGeometryList.at(1),
                          m_restoredPopUpGeometryList.at(2),
                          m_restoredPopUpGeometryList.at(3));

        if(IS_VALID_OBJ(m_restoredPopUpText))
        {
            m_restoredPopUpText->changeText(QString("%1").arg(m_msgMapList.size()).rightJustified(2,'0'));
            m_restoredPopUpText->setGeometry((m_restoredPopUpImage->x() + m_restoredPopUpImage->width() - m_restoredPopUpText->width() - SCALE_WIDTH(3)),
                                                    (m_restoredPopUpImage->y() + m_restoredPopUpImage->height() - m_restoredPopUpText->height() - SCALE_HEIGHT(2)),
                                              m_restoredPopUpText->width(),
                                               m_restoredPopUpText->height());
            m_restoredPopUpText->update ();
        }
        if((IS_VALID_OBJ(m_blinkRestoredPopUpTimer)) && (!m_blinkRestoredPopUpTimer->isActive()) && (isEventOccur))
        {
            m_blinkRestoredPopUpTimer->start();
        }
    }
    else if(m_currentDisplayMode == MAXIMIZED_MSG_ALERT)
    {
        updateElements();
    }
}

void MessageAlert::changeDisplayMode(MSG_ALERT_MODE_e msgAlertMode)
{
    m_currentDisplayMode = msgAlertMode;
    bool isMessageVisible = (msgAlertMode == RESTORED_MSG_ALERT) ? false : true;

    if((IS_VALID_OBJ(m_autoclearTimer)) && (m_autoclearTimer->isActive()))
    {
        m_autoclearTimer->stop();
    }

    if((IS_VALID_OBJ(m_blinkRestoredPopUpTimer)) && (m_blinkRestoredPopUpTimer->isActive()))
    {
        m_blinkRestoredPopUpTimer->stop();
    }
    if(IS_VALID_OBJ(m_backgroundRectangle))
    {
        m_backgroundRectangle->setVisible(isMessageVisible);
    }
    if(IS_VALID_OBJ(m_alertImage))
    {
        m_alertImage->setVisible(isMessageVisible);
    }
    if(IS_VALID_OBJ(m_textLabel))
    {
        m_textLabel->setVisible(isMessageVisible);
    }
    if(IS_VALID_OBJ(m_closeImage))
    {
        m_closeImage->setVisible(isMessageVisible);
        m_closeImage->setIsEnabled(isMessageVisible);
    }
    if(IS_VALID_OBJ(m_restoreImage))
    {
        m_restoreImage->setVisible(isMessageVisible);
        m_restoreImage->setIsEnabled(isMessageVisible);
    }
    if(IS_VALID_OBJ(m_restoredPopUpImage))
    {
        m_restoredPopUpImage->setVisible(!isMessageVisible);
        m_restoredPopUpImage->setIsEnabled(!isMessageVisible);
    }

    if(IS_VALID_OBJ(m_restoredPopUpText))
    {
        m_restoredPopUpText->setVisible(!isMessageVisible);
    }

    changeMessage(false);
}

void MessageAlert::updateElements()
{
    ApplController *applController = ApplController::getInstance();
    QString str = RECORDING_FAIL_MSG(applController->GetDispDeviceName(m_deviceList.at(0)), m_msgMapList[m_deviceList.at(0)]);
    quint16 labelWidth = 0;
    quint16 labelHeight = 0;

    TextLabel::getWidthHeight(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE),
                              str,
                              labelWidth,
                              labelHeight);

    this->setGeometry((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - (labelWidth + (4 * LEFT_MARGIN) + SCALE_WIDTH((48 + 30)))) / 2)),
                      (ApplController::getYPosOfScreen() + SCALE_HEIGHT(50)),
                      (labelWidth + (4 * LEFT_MARGIN) + SCALE_WIDTH((48 + 30))),
                      SCALE_HEIGHT(70));

    if(IS_VALID_OBJ(m_backgroundRectangle))
    {
        m_backgroundRectangle->resetGeometry(0,0,this->width (),this->height ());
    }

    if(IS_VALID_OBJ(m_alertImage))
    {
        m_alertImage->setGeometry (SCALE_WIDTH(10), SCALE_HEIGHT(10), SCALE_WIDTH(48), SCALE_HEIGHT(48));
    }

    if(IS_VALID_OBJ(m_closeImage))
    {
        m_closeImage->setGeometry ((this->width ()- SCALE_WIDTH(30)), SCALE_HEIGHT(10), SCALE_WIDTH(20), SCALE_HEIGHT(20));
        m_closeImage->update();
    }

    if(IS_VALID_OBJ(m_restoreImage))
    {
        m_restoreImage->setGeometry ((this->width() - SCALE_WIDTH(50)),
                                     m_restoreImage->y(),
                                     m_restoreImage->width(),
                                     m_restoreImage->height());
    }

    if(IS_VALID_OBJ(m_textLabel))
    {
        m_textLabel->changeText(str);
        m_textLabel->update();
    }

    bool autoCloseFlag = false;

    if (applController->GetAutoCloseRecFailAlertFlag(m_deviceList.at(0),autoCloseFlag))
    {
        if((IS_VALID_OBJ(m_autoclearTimer)))
        {
            if(m_autoclearTimer->isActive())
            {
                m_autoclearTimer->stop();
            }

            if(autoCloseFlag)
            {
                m_autoclearTimer->start();
            }
        }
    }
}

void MessageAlert::getMessageList(QMap<QString, quint8> &msgMap, QStringList &deviceList)
{
    if(IS_VALID_OBJ(m_autoclearTimer) && m_autoclearTimer->isActive())
    {
        m_autoclearTimer->stop();
    }
    if(IS_VALID_OBJ(m_blinkRestoredPopUpTimer) && m_blinkRestoredPopUpTimer->isActive())
    {
        m_blinkRestoredPopUpTimer->stop();
    }

    msgMap = m_msgMapList;
    deviceList = m_deviceList;
}

void MessageAlert::setMessageList(QMap<QString, quint8> msgMap, QStringList deviceList)
{
    m_msgMapList = msgMap;
    m_deviceList = deviceList;
}

MSG_ALERT_MODE_e MessageAlert::getMessageAlertMode()
{
    return m_currentDisplayMode;
}

void MessageAlert::slotTimeOut()
{
    closePageAction();
}

void MessageAlert::slotBlinkTimeout()
{
    IMAGE_TYPE_e imgType = IMAGE_TYPE_NORMAL;

    if(IS_VALID_OBJ(m_restoredPopUpImage))
    {
        if(m_restoredPopUpImage->getImageType() == IMAGE_TYPE_NORMAL)
        {
            imgType = IMAGE_TYPE_MOUSE_HOVER;
        }

        m_restoredPopUpImage->changeImage(imgType,true);
    }
}

void MessageAlert::slotImageClicked(int index)
{
    switch(index)
    {
    case MSG_ALRT_CTRL_CLOSE:
        closePageAction();
        break;

    case MSG_ALRT_CTRL_RESTORE:
        changeDisplayMode(RESTORED_MSG_ALERT);
        break;

    case MSG_ALRT_CTRL_POPUP:
        changeDisplayMode(MAXIMIZED_MSG_ALERT);
        break;

    default:
        break;
    }
}
