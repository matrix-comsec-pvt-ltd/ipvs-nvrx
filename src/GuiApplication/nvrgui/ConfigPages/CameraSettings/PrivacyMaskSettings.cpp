#include "PrivacyMaskSettings.h"
#include "ValidationMessage.h"

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>

#define MIN_WIDTH           20
#define MIN_HEIGHT          20
#define MAX_WIDTH           352 // to support 1 CIF
#define MAX_SCREEN_WIDTH    (ApplController::getWidthOfScreen())
#define MAX_SCREEN_HEIGHT   (ApplController::getHeightOfScreen())

const QString menuLabel[] = {"Clear", "Exit"};

PrivacyMaskSettings::PrivacyMaskSettings(void *data, QString deviceName,
                                         quint8 cameraIndex, CAMERA_TYPE_e cameraType,
                                         quint8 maxSupportedPrivacyMaskWindow,
                                         QWidget *parent) : QWidget(parent)
{
    QFont font = TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
    m_textWidth = QFontMetrics(font).width("4");
    m_textHeight = QFontMetrics(font).height();
    m_applController = ApplController::getInstance();
    m_currentTotalRect = 0;
    INIT_OBJ(m_payloadLib);
    m_payloadLib = new PayloadLib();
    m_currentCamera = cameraType;
    m_cameraIndex = cameraIndex;
    m_deviceName = deviceName;
    m_maxSupportedPrivacyMaskWindow = maxSupportedPrivacyMaskWindow;

    DEV_TABLE_INFO_t    devTableInfo;
    m_applController->GetDeviceInfo(m_deviceName, devTableInfo);

    if((devTableInfo.maxIpCam == 24) && (devTableInfo.maxAnalogCam == 16)
            && (devTableInfo.numOfHdd == 1))
    {
        m_widthRatio = 720;
    }
    else
    {
        m_widthRatio = 704;
    }

    if(m_currentCamera == ANALOG_CAMERA)
    {
        if(!m_applController->GetVideoStandard(deviceName, m_videoStandard))
        {
            m_videoStandard = MAX_VIDEO_STANDARD;
        }
    }
    else
    {
        m_videoStandard = PAL_VIDEO_STANDARD;
    }

    switch(m_videoStandard)
    {
    case PAL_VIDEO_STANDARD:
        m_maxHeight = 288;
        m_heightRatio = 576;
        break;

    case NTSC_VIDEO_STANDARD:
        m_maxHeight = 240;
        m_heightRatio = 480;
        break;

    default:
        break;
    }

    m_mouseLeftClick = false;
    m_mouseRightClick = false;
    m_drawRectFlag = false;
    m_privacyMaskData = (PRIVACY_MASK_DATA_t*)data;
    m_menuButtonList = NULL;
    m_hightlightedRectNumber = m_maxSupportedPrivacyMaskWindow;
    m_currentRectNumber = m_maxSupportedPrivacyMaskWindow;
    m_actionToPerform = MAX_PRIVACYMASK_MENU_LABEL_TYPE;

    m_infoPage = new InfoPage(ApplController::getXPosOfScreen(),
                              ApplController::getYPosOfScreen(),
                              ApplController::getWidthOfScreen(),
                              ApplController::getHeightOfScreen(),
                              MAX_INFO_PAGE_TYPE,
                              parent,
                              true,
                              false);
    connect(m_infoPage,
            SIGNAL(sigInfoPageCnfgBtnClick(int)),
            this,
            SLOT(slotInfoPageButtonClicked(int)));

    for(int index = 0; index < MAX_PRIVACYMASK_AREA; index++)
    {
        m_maskRectangle[index].setRect(0, 0, 0, 0);
        m_maskTextRectangle[index].setRect(0, 0, 0, 0);
    }

    setGeometryForElements();
    this->setGeometry(ApplController::getXPosOfScreen(),
                      ApplController::getYPosOfScreen(),
                      ApplController::getWidthOfScreen(),
                      ApplController::getHeightOfScreen());
    this->setEnabled(true);
    this->setMouseTracking(true);
    this->show();
}

PrivacyMaskSettings::~PrivacyMaskSettings()
{
    disconnect(m_infoPage,
               SIGNAL(sigInfoPageCnfgBtnClick(int)),
               this,
               SLOT(slotInfoPageButtonClicked(int)));
    delete m_infoPage;

    if(m_menuButtonList != NULL)
    {
        disconnect(m_menuButtonList,
                   SIGNAL(sigMenuSelected(QString, quint8)),
                   this,
                   SLOT(slotMenuButtonSelected(QString, quint8)));
        disconnect(m_menuButtonList,
                   SIGNAL(destroyed()),
                   this,
                   SLOT(slotMenuListDestroyed()));
        delete m_menuButtonList;
    }
    if(m_payloadLib != NULL)
    {
       DELETE_OBJ(m_payloadLib);
    }
}

void PrivacyMaskSettings::setGeometryForElements()
{
    m_currentTotalRect = 0;
    for(quint8 index = 0; index < m_maxSupportedPrivacyMaskWindow; index++)
    {
        if((m_privacyMaskData[index].width > 0)
                && (m_privacyMaskData[index].height > 0))
        {
            m_rectInfo[index].startX = (m_privacyMaskData[index].startX * MAX_SCREEN_WIDTH) / m_widthRatio;
            m_rectInfo[index].startY = (m_privacyMaskData[index].startY * MAX_SCREEN_HEIGHT) / m_heightRatio;
            m_rectInfo[index].endX = ((m_privacyMaskData[index].width + m_privacyMaskData[index].startX) * MAX_SCREEN_WIDTH) / m_widthRatio;
            m_rectInfo[index].endY = ((m_privacyMaskData[index].height + m_privacyMaskData[index].startY) * MAX_SCREEN_HEIGHT) / m_heightRatio;
            m_rectInfo[index].creatingOrder = ++m_currentTotalRect;
        }
        else
        {
            m_rectInfo[index].startX = 0;
            m_rectInfo[index].startY = 0;
            m_rectInfo[index].endX = 0;
            m_rectInfo[index].endY = 0;
            m_rectInfo[index].creatingOrder = 0;
        }
        m_rectInfo[index].index = index;
        createMaskRectangle(index);
    }
}

void PrivacyMaskSettings::createMaskRectangle(quint8 rectIndex)
{
    quint16 width, height;

    if(m_rectInfo[rectIndex].endX > ApplController::getWidthOfScreen())
    {
        m_rectInfo[rectIndex].endX = ApplController::getWidthOfScreen();
    }
    if(m_rectInfo[rectIndex].endY > ApplController::getHeightOfScreen())
    {
        m_rectInfo[rectIndex].endY = ApplController::getHeightOfScreen();
    }

    width = (m_rectInfo[rectIndex].endX - m_rectInfo[rectIndex].startX);
    height = (m_rectInfo[rectIndex].endY - m_rectInfo[rectIndex].startY);

    bool isVisible = ((width > 0) && (height > 0));

    if(!isVisible)
    {
        width = height = 0;
    }

    m_maskRectangle[rectIndex].setRect(m_rectInfo[rectIndex].startX,
                                       m_rectInfo[rectIndex].startY,
                                       width,
                                       height);

    m_maskTextRectangle[rectIndex].setRect((m_maskRectangle[rectIndex].x() + 2),
                                           (m_maskRectangle[rectIndex].y() + 2),
                                           m_textWidth,
                                           m_textHeight);
    update();
}

quint8 PrivacyMaskSettings::findRectangleNumber()
{
    quint8 rectNumber = m_maxSupportedPrivacyMaskWindow;
    for(quint8 index = 0; index < m_maxSupportedPrivacyMaskWindow; index++)
    {
        if(((m_rectInfo[index].endX - m_rectInfo[index].startX) == 0)
                && ((m_rectInfo[index].endY - m_rectInfo[index].startY) == 0))
        {
            rectNumber = index;
            break;
        }
    }
    return rectNumber;
}

void PrivacyMaskSettings::resizeRectangle()
{
    bool updateFlag = false;
    if((m_rectInfo[m_currentRectNumber].endX - m_rectInfo[m_currentRectNumber].startX) < MIN_WIDTH)
    {
        updateFlag = true;
        m_rectInfo[m_currentRectNumber].endX = m_rectInfo[m_currentRectNumber].startX + MIN_WIDTH;
    }
    if((m_rectInfo[m_currentRectNumber].endY - m_rectInfo[m_currentRectNumber].startY) < MIN_HEIGHT)
    {
        updateFlag = true;
        m_rectInfo[m_currentRectNumber].endY = m_rectInfo[m_currentRectNumber].startY + MIN_HEIGHT;
    }
    if(updateFlag)
    {
        createMaskRectangle(m_currentRectNumber);
    }
}

bool PrivacyMaskSettings::isMaskRectContainsPoint(QPoint point, quint8 &rectIndex)
{
    bool status = false;
    quint8 rectNumber = 0;
    quint8 currentOrder = 1;
    for(quint8 index = 0; index < m_currentTotalRect; index++)
    {
        for(quint8 rectIndex = 0; rectIndex < m_maxSupportedPrivacyMaskWindow; rectIndex++)
        {
            if((m_rectInfo[rectIndex].creatingOrder == currentOrder)
                    && (m_maskRectangle[rectIndex].contains(point)))
            {
                status = true;
                rectNumber = rectIndex;
                break;
            }
        }
        currentOrder++;
    }
    if(status == true)
    {
        rectIndex = rectNumber;
    }
    return status;
}

void PrivacyMaskSettings::exitAction()
{
    if((m_currentCamera == IP_CAMERA) || (m_currentCamera == AUTO_ADD_IP_CAMERA))
    {
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(MAINTAIN_CHANGE), true);
    }
    else
    {
        if(isMaskRectangleBeyondLimit())
        {
            m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(PRIVACY_MASK_SETTING));
        }
        else
        {
            m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(MAINTAIN_CHANGE), true);
        }
    }
}

void PrivacyMaskSettings::clearRect()
{
    for(quint8 index = 0; index < m_maxSupportedPrivacyMaskWindow; index++)
    {
        if((m_rectInfo[index].creatingOrder != 0)
                && (m_rectInfo[index].creatingOrder > m_rectInfo[m_currentRectNumber].creatingOrder))
        {
            m_rectInfo[index].creatingOrder = (m_rectInfo[index].creatingOrder - 1);
        }
    }
    m_rectInfo[m_currentRectNumber].startX = 0;
    m_rectInfo[m_currentRectNumber].startY = 0;
    m_rectInfo[m_currentRectNumber].endX = 0;
    m_rectInfo[m_currentRectNumber].endY = 0;
    m_rectInfo[m_currentRectNumber].creatingOrder = 0;
    m_currentTotalRect--;
    createMaskRectangle(m_currentRectNumber);
}

bool PrivacyMaskSettings::isMaskRectangleBeyondLimit()
{
    bool status = false;
    quint16 width = 0, height = 0;
    for(quint8 index = 0; index < m_maxSupportedPrivacyMaskWindow; index++)
    {
        width += (((m_rectInfo[index].endX - m_rectInfo[index].startX) * m_widthRatio) / MAX_SCREEN_WIDTH);
        height += (((m_rectInfo[index].endY - m_rectInfo[index].startY) * m_heightRatio) / MAX_SCREEN_HEIGHT);

        if((width > MAX_WIDTH) || (height > m_maxHeight))
        {
            status = true;
            break;
        }
    }
    return status;
}

void PrivacyMaskSettings::mouseLeftButtonPressEvent(QMouseEvent* event)
{
    quint8 rectNumber = findRectangleNumber();
    if(rectNumber != m_maxSupportedPrivacyMaskWindow)
    {
        m_currentRectNumber = rectNumber;
        m_currentTotalRect++;
        m_mouseLeftClick = true;
        m_startPoint = QPoint(event->pos());
        m_rectInfo[m_currentRectNumber].creatingOrder = m_currentTotalRect;
    }
}

void PrivacyMaskSettings::mouseRightButtonPressEvent(QMouseEvent* event)
{
    m_mouseRightClick = true;
    m_rightClickPoint = QPoint(event->pos());
}

void PrivacyMaskSettings::mouseLeftButtonReleaseEvent(QMouseEvent*)
{
    if(m_currentRectNumber != m_maxSupportedPrivacyMaskWindow)
    {
        if(!m_drawRectFlag)
        {
            clearRect();
        }
        else
        {
            resizeRectangle();
        }
        m_mouseLeftClick = false;
        m_currentRectNumber = m_maxSupportedPrivacyMaskWindow;
        m_drawRectFlag = false;
        m_startPoint = QPoint(0, 0);
    }
}

void PrivacyMaskSettings::mouseRightButtonReleaseEvent(QMouseEvent* event)
{
    if(isMaskRectContainsPoint(event->pos(), m_currentRectNumber))
    {
        m_menuButtonList = new MenuButtonList(m_rightClickPoint.x(),
                                              m_rightClickPoint.y(),
                                              (QStringList() << menuLabel[PRIVACYMASK_CLEAR_LABEL] << menuLabel[PRIVACYMASK_EXIT_LABEL]),
                                              this,
                                              false,
                                              SCALE_WIDTH(70));
    }
    else
    {
        m_menuButtonList = new MenuButtonList((this->width() / 2),
                                              (this->height() / 2),
                                              (QStringList() << menuLabel[PRIVACYMASK_EXIT_LABEL]),
                                              this,
                                              false,
                                              SCALE_WIDTH(70));
    }
    connect(m_menuButtonList,
            SIGNAL(sigMenuSelected(QString, quint8)),
            this,
            SLOT(slotMenuButtonSelected(QString, quint8)));
    connect(m_menuButtonList,
            SIGNAL(sigMenuSelected(QString,quint8)),
            this,
            SLOT(slotMenuListDestroyed()));
    m_mouseRightClick = false;
    m_rightClickPoint = QPoint(0, 0);
}

void PrivacyMaskSettings::saveConfig()
{
    for(quint8 index = 0; index < m_maxSupportedPrivacyMaskWindow; index++)
    {
        m_privacyMaskData[index].startX = (m_rectInfo[index].startX * m_widthRatio) / MAX_SCREEN_WIDTH;
        m_privacyMaskData[index].startY = (m_rectInfo[index].startY * m_heightRatio) / MAX_SCREEN_HEIGHT;
        m_privacyMaskData[index].width = ((m_rectInfo[index].endX - m_rectInfo[index].startX) * m_widthRatio) / MAX_SCREEN_WIDTH;
        m_privacyMaskData[index].height = ((m_rectInfo[index].endY - m_rectInfo[index].startY) * m_heightRatio) / MAX_SCREEN_HEIGHT;
    }

    if((m_currentCamera == IP_CAMERA) || (m_currentCamera == AUTO_ADD_IP_CAMERA))
    {
        m_payloadLib->setCnfgArrayAtIndex(0,m_cameraIndex);
        m_payloadLib->setCnfgArrayAtIndex(1,m_maxSupportedPrivacyMaskWindow);
        for(quint8 index = 0, fieldIndex = 0; index < m_maxSupportedPrivacyMaskWindow; index++)
        {
            m_payloadLib->setCnfgArrayAtIndex((2 + ((index * 4) + fieldIndex++)), m_privacyMaskData[index].startX);
            m_payloadLib->setCnfgArrayAtIndex((2 + ((index * 4) + fieldIndex++)), m_privacyMaskData[index].startY);
            m_payloadLib->setCnfgArrayAtIndex((2 + ((index * 4) + fieldIndex++)), m_privacyMaskData[index].width);
            m_payloadLib->setCnfgArrayAtIndex((2 + ((index * 4) + fieldIndex)),   m_privacyMaskData[index].height);
            fieldIndex = 0;
        }

        QString payloadString = m_payloadLib->createDevCmdPayload(2 + (m_maxSupportedPrivacyMaskWindow*4));
        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CMD;
        param->cmdType = SET_PRIVACY_MASK_WINDOW;
        param->payload = payloadString;
        m_applController->processActivity(m_deviceName, DEVICE_COMM, param);
        emit sigLoadProcessBar();
    }
}

void PrivacyMaskSettings::forceActiveFocus()
{
    this->setFocus();
}

void PrivacyMaskSettings::paintEvent(QPaintEvent * event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen = painter.pen();
    pen.setCapStyle(Qt::SquareCap);
    pen.setJoinStyle(Qt::MiterJoin);
    quint8 currentOrder = 1;

    for(quint8 index = 0; index < m_currentTotalRect; index++)
    {
        for(quint8 rectIndex = 0; rectIndex < m_maxSupportedPrivacyMaskWindow; rectIndex++)
        {
            if(m_rectInfo[rectIndex].creatingOrder == currentOrder)
            {
                if(rectIndex == m_hightlightedRectNumber)
                {
                    pen.setWidth(5);
                    pen.setBrush(QBrush(QColor(HIGHLITED_FONT_COLOR)));

                }
                else
                {
                    pen.setWidth(4);
                    pen.setBrush(QBrush(QColor(RED_COLOR)));
                }
                painter.setPen(pen);

                if((m_maskRectangle[rectIndex].width() > 0) && (m_maskRectangle[rectIndex].height() > 0))
                {
                    QColor color;
                    color.setAlpha (0);
                    painter.setBrush(QBrush(color));
                    painter.drawRect(m_maskRectangle[rectIndex]);

                    QString textString = QString("%1").arg(m_rectInfo[rectIndex].index + 1);
                    painter.setPen(QColor(HIGHLITED_FONT_COLOR));
                    painter.drawText(m_maskTextRectangle[rectIndex], textString);
                }
                currentOrder++;
                break;
            }
        }
    }
}

void PrivacyMaskSettings::mousePressEvent(QMouseEvent * event)
{
    if(m_menuButtonList != NULL)
    {
        disconnect(m_menuButtonList,
                   SIGNAL(sigMenuSelected(QString, quint8)),
                   this,
                   SLOT(slotMenuButtonSelected(QString, quint8)));
        disconnect(m_menuButtonList,
                   SIGNAL(destroyed()),
                   this,
                   SLOT(slotMenuListDestroyed()));
        delete m_menuButtonList;
        m_menuButtonList = NULL;
    }
    if(event->button() == Qt::LeftButton)
    {
        mouseLeftButtonPressEvent(event);
    }
    else if(event->button() == Qt::RightButton)
    {
        mouseRightButtonPressEvent(event);
    }
}

void PrivacyMaskSettings::mouseReleaseEvent(QMouseEvent * event)
{
    if(event->button() == Qt::LeftButton)
    {
        mouseLeftButtonReleaseEvent(event);
    }
    else if(event->button() == Qt::RightButton)
    {
        mouseRightButtonReleaseEvent(event);
    }
}

void PrivacyMaskSettings::mouseMoveEvent(QMouseEvent * event)
{
    qint16 diffX, diffY;    

    if(m_mouseLeftClick)
    {
        int eventX = event->x();
        int eventY = event->y();

        if(eventX < 0)
        {
            eventX = 0;
        }
        if(eventY < 0)
        {
            eventY = 0;
        }

        diffX = eventX - m_startPoint.x();
        diffY = eventY - m_startPoint.y();

        if((abs(diffX) >= 20) && (abs(diffY) >= 20) && (!m_drawRectFlag))
        {
            m_drawRectFlag = true;
            m_rectInfo[m_currentRectNumber].startX = m_startPoint.x();
            m_rectInfo[m_currentRectNumber].startY = m_startPoint.y();
        }
        if(m_drawRectFlag)
        {
            if(m_rectInfo[m_currentRectNumber].startX > eventX)
            {
                m_rectInfo[m_currentRectNumber].endX = m_startPoint.x();
                m_rectInfo[m_currentRectNumber].startX = eventX;
            }
            else if(m_rectInfo[m_currentRectNumber].startX < eventX)
            {
                m_rectInfo[m_currentRectNumber].endX = eventX;
            }
            if(m_rectInfo[m_currentRectNumber].startY > eventY)
            {
                m_rectInfo[m_currentRectNumber].endY = m_startPoint.y();
                m_rectInfo[m_currentRectNumber].startY = eventY;
            }
            else if(m_rectInfo[m_currentRectNumber].startY < eventY)
            {
                m_rectInfo[m_currentRectNumber].endY = eventY;
            }
            createMaskRectangle(m_currentRectNumber);
        }
    }
    else
    {
        quint8 rectIndex = m_maxSupportedPrivacyMaskWindow;
        isMaskRectContainsPoint(event->pos(), rectIndex);
        if((rectIndex != m_hightlightedRectNumber))
        {
            quint8 oldHighlighted = m_hightlightedRectNumber;
            m_hightlightedRectNumber = rectIndex;
            if(m_hightlightedRectNumber != m_maxSupportedPrivacyMaskWindow)
            {
                QRect rect((m_maskRectangle[m_hightlightedRectNumber].x() - 5),
                           (m_maskRectangle[m_hightlightedRectNumber].y() - 5),
                           (m_maskRectangle[m_hightlightedRectNumber].width() + 10),
                           (m_maskRectangle[m_hightlightedRectNumber].height() + 10));
                update(rect);
            }
            if(oldHighlighted != m_maxSupportedPrivacyMaskWindow)
            {
                QRect rect((m_maskRectangle[oldHighlighted].x() - 5),
                           (m_maskRectangle[oldHighlighted].y() - 5),
                           (m_maskRectangle[oldHighlighted].width() + 10),
                           (m_maskRectangle[oldHighlighted].height() + 10));
                update(rect);
            }
        }
    }
}

void PrivacyMaskSettings::slotMenuButtonSelected(QString menulabel, quint8)
{
    disconnect(m_menuButtonList,
               SIGNAL(sigMenuSelected(QString, quint8)),
               this,
               SLOT(slotMenuButtonSelected(QString, quint8)));
    if(menulabel == menuLabel[PRIVACYMASK_CLEAR_LABEL])
    {
        m_actionToPerform = PRIVACYMASK_CLEAR_LABEL;
    }
    else if(menulabel == menuLabel[PRIVACYMASK_EXIT_LABEL])
    {
        m_actionToPerform = PRIVACYMASK_EXIT_LABEL;
    }
}

void PrivacyMaskSettings::slotMenuListDestroyed()
{
    disconnect(m_menuButtonList,
               SIGNAL(destroyed()),
               this,
               SLOT(slotMenuListDestroyed()));
    if(m_actionToPerform == PRIVACYMASK_CLEAR_LABEL)
    {
        clearRect();
    }
    else if(m_actionToPerform == PRIVACYMASK_EXIT_LABEL)
    {
        exitAction();
    }
    m_actionToPerform = MAX_PRIVACYMASK_MENU_LABEL_TYPE;
    forceActiveFocus();
    m_menuButtonList = NULL;
}

void PrivacyMaskSettings::slotInfoPageButtonClicked(int index)
{
    forceActiveFocus();
    if(m_infoPage->getText() == (ValidationMessage::getValidationMessage(MAINTAIN_CHANGE)))
    {
        if(index == INFO_OK_BTN)
        {
            saveConfig();
        }
        this->deleteLater();
    }
}
