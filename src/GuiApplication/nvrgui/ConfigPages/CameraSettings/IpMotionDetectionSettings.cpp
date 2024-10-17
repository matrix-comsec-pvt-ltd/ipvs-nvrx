#include "IpMotionDetectionSettings.h"
#include "ValidationMessage.h"

#include <QMouseEvent>
#include <QPainter>

#define MAX_COL                     44
#define WIDTH_RATIO                 704
#define HEIGHT_RATIO                576
#define MAX_SCREEN_WIDTH            (ApplController::getWidthOfScreen())
#define MAX_SCREEN_HEIGHT           (ApplController::getHeightOfScreen())
#define MAX_MOTION_EVENT_LIST       2

const QString menuLabel[] =         {"Clear", "Exit"};
const QString menuIpPointLabel[] =  {"Event", "Sensitivity", "Duration", "Clear All", "Exit"};
const QString motionEvent[] =       {"Motion", "No Motion"};

IpMotionDetectionSettings::IpMotionDetectionSettings(void *data, QString deviceName, quint8 cameraIndex, QWidget* parent)
                           : QWidget(parent), m_pointActionToPerform(MAX_IPMOTIONDETECTION_IP_POINT_MENU_LABEL_TYPE)
{
    m_applController = ApplController::getInstance();
    m_payloadLib = NULL;
    m_payloadLib = new PayloadLib();
    m_sensitivityLevelList.reserve(10);

    for(quint8 index = 0; index < 10; index++)
    {
        m_sensitivityLevelList.append(QString("%1").arg(index + 1));
    }

    QFont font = TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
    m_textWidth = QFontMetrics(font).width("4");
    m_textHeight = QFontMetrics(font).height();
    m_sensitivityTextWidth = QFontMetrics(font).width("S:10");

    m_blockWidth = ((qreal)ApplController::getWidthOfScreen() / (MAX_COL));
    m_maxRow = MAX_PAL_ROW;
    m_blockHeight = ((qreal)ApplController::getHeightOfScreen() / (m_maxRow));

    m_maxHeight = 288;
    m_heightRatio = 576;

    m_startCol = MAX_COL;
    m_startRow = m_maxRow;

    m_mouseLeftClick = false;
    m_mouseRightClick = false;
    m_menuButtonList = NULL;
    m_picklistLoader = NULL;
    m_inVisibleWidget = NULL;
    m_background = NULL;
    m_closeButton = NULL;
    m_pageHeading = NULL;
    m_NoMotionDurationTextBox = NULL;
    m_NoMotionDurationTextBoxParam = NULL;

    m_motionDetectionConfig = (MOTION_DETECTION_CONFIG_t*)data;
    m_deviceName = deviceName;
    m_cameraIndex = cameraIndex;

    m_motionSupportType = m_motionDetectionConfig->motionSupportType;
    m_isOverlapingAllowed = (m_motionDetectionConfig->motionSupportType == BLOCK_METHOD);
    m_maxMotionBlock = (m_motionDetectionConfig->motionSupportType == BLOCK_METHOD ? MAX_MOTION_BLOCK : MAX_MOTIONDETECTION_AREA);

    m_sensitivity = m_motionDetectionConfig->sensitivity;
    m_isNoMotionEvent = m_motionDetectionConfig->isNoMotionEvent;
    m_noMotionEventSupportF = m_motionDetectionConfig->noMotionEventSupportF;
    m_noMotionDuration = m_motionDetectionConfig->noMotionDuration;
    m_hightlightedRectNumber = MAX_MOTION_BLOCK;
    m_currentRectNumber = MAX_MOTION_BLOCK;
    m_currentEditRectNumber = MAX_MOTION_BLOCK;
    m_actionToPerform = MAX_IPMOTIONDETECTION_MENU_LABEL_TYPE;

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

    for(quint16 index = 0; index < MAX_MOTION_BLOCK; index++)
    {
        m_maskRectangle[index].setRect(0, 0, 0, 0);
        m_maskTextRectangle[index].setRect(0, 0, 0, 0);
        m_maskSensitivityRectangle[index].setRect(0, 0, 0, 0);
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

IpMotionDetectionSettings::~IpMotionDetectionSettings()
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

    if(m_picklistLoader != NULL)
    {
        disconnect(m_picklistLoader,
                   SIGNAL(sigValueChanged(quint8,QString,bool)),
                   this,
                   SLOT(slotValueChanged(quint8,QString,bool)));
        disconnect(m_picklistLoader,
                   SIGNAL(destroyed()),
                   this,
                   SLOT(slotPickListDestroyed()));
        delete m_picklistLoader;
    }

    m_sensitivityLevelList.clear();

    if(m_payloadLib != NULL)
    {
        DELETE_OBJ(m_payloadLib);
    }
}

void IpMotionDetectionSettings:: setGeometryForElements()
{
    for(quint16 index = 0; index < MAX_MOTION_BLOCK; index++)
    {
        m_rectInfo[index].index = index;
        m_rectInfo[index].startCol = 0;
        m_rectInfo[index].startRow = 0;
        m_rectInfo[index].endCol = 0;
        m_rectInfo[index].endRow = 0;
        m_rectInfo[index].drawFlag = false;
        m_rectInfo[index].sensitivity = 5;
        m_rectInfo[index].isindividualBlock = false;
    }

    if(m_motionSupportType == POINT_METHOD)
    {
        for(quint8 index = 0; index < MAX_MOTIONDETECTION_AREA; index++)
        {
            m_rectInfo[index].isindividualBlock = true;
            m_rectInfo[index].sensitivity = m_motionDetectionConfig->windowInfo[index].sensitivity;

            if((m_motionDetectionConfig->windowInfo[index].endCol > 0) &&
                    (m_motionDetectionConfig->windowInfo[index].endRow > 0))
            {
                m_rectInfo[index].startCol = (m_motionDetectionConfig->windowInfo[index].startCol * MAX_SCREEN_WIDTH) / WIDTH_RATIO;
                m_rectInfo[index].startRow = (m_motionDetectionConfig->windowInfo[index].startRow * MAX_SCREEN_HEIGHT) / HEIGHT_RATIO;
                m_rectInfo[index].endCol = ((m_motionDetectionConfig->windowInfo[index].endCol
                                             + m_motionDetectionConfig->windowInfo[index].startCol ) * MAX_SCREEN_WIDTH) / WIDTH_RATIO;
                m_rectInfo[index].endRow = ((m_motionDetectionConfig->windowInfo[index].endRow
                                             + m_motionDetectionConfig->windowInfo[index].startRow) * MAX_SCREEN_HEIGHT) / HEIGHT_RATIO;

                m_rectInfo[index].drawFlag = true;
            }

            createMaskRectangle(index);
        }
    }
    else
    {
        quint16 blockIndex = 0;
        for(quint8 byteIndex = 0; byteIndex < MAX_MOTION_BYTE; byteIndex++)
        {
            for(qint8 bitIndex = 7; bitIndex >= 0; bitIndex--)
            {
                if((m_motionDetectionConfig->byteInfo[byteIndex] & ((quint8)1 << bitIndex)) != 0)
                {
                    m_rectInfo[blockIndex].sensitivity = m_sensitivity;
                    m_rectInfo[blockIndex].startCol = (blockIndex % MAX_COL);
                    m_rectInfo[blockIndex].startRow = (blockIndex / MAX_COL);
                    m_rectInfo[blockIndex].endCol = (m_rectInfo[blockIndex].startCol + 1);
                    m_rectInfo[blockIndex].endRow = (m_rectInfo[blockIndex].startRow + 1);
                    m_rectInfo[blockIndex].drawFlag = true;
                }
                blockIndex++;
            }
        }

        for(quint16 index = 0; index < MAX_MOTION_BLOCK; index++)
        {
            createMaskRectangle(index);
        }
    }

    update ();
}

void IpMotionDetectionSettings::createMaskRectangle(quint16 rectIndex)
{
    quint16 width, height;
    if(m_motionSupportType == POINT_METHOD)
    {
        width = (m_rectInfo[rectIndex].endCol - m_rectInfo[rectIndex].startCol);
        height = (m_rectInfo[rectIndex].endRow - m_rectInfo[rectIndex].startRow);
    }
    else
    {
        width = (m_rectInfo[rectIndex].endCol - m_rectInfo[rectIndex].startCol) * m_blockWidth;
        height = (m_rectInfo[rectIndex].endRow - m_rectInfo[rectIndex].startRow) * m_blockHeight;
    }

    bool isVisible = ((width > 0) || (height > 0) || (m_rectInfo[rectIndex].drawFlag));

    if(isVisible)
    {
        if(width == 0)
        {
            width = m_blockWidth;
        }
        if(height == 0)
        {
            height = m_blockHeight;
        }
    }
    else
    {
        width = 0;
        height = 0;
    }

    if(width > ApplController::getWidthOfScreen())
    {
        width = ApplController::getWidthOfScreen();
    }
    if(height > ApplController::getHeightOfScreen())
    {
        height = ApplController::getHeightOfScreen();
    }

    if(m_motionSupportType == POINT_METHOD)
    {
        m_maskRectangle[rectIndex].setRect((m_rectInfo[rectIndex].startCol),
                                           (m_rectInfo[rectIndex].startRow),
                                           (width),
                                           (height));

    }
    else
    {
        m_maskRectangle[rectIndex].setRect((m_rectInfo[rectIndex].startCol * m_blockWidth),
                                           (m_rectInfo[rectIndex].startRow * m_blockHeight),
                                           (width),
                                           (height));
    }

    if(m_rectInfo[rectIndex].isindividualBlock == true)
    {
        QFont font = TextLabel::getFont (NORMAL_FONT_FAMILY,SCALE_FONT(20),true);

        QString sensitivitString = QString("S:") + QString("%1").arg(m_rectInfo[rectIndex].sensitivity);

        TextLabel::getWidthHeight (font, sensitivitString, m_sensitivityTextWidth,m_textHeight);

        m_maskTextRectangle[rectIndex].setRect((m_maskRectangle[rectIndex].x() + 2),
                                               (m_maskRectangle[rectIndex].y() + 2),
                                               m_textWidth,
                                               m_textHeight);

        m_maskSensitivityRectangle[rectIndex].setRect((m_maskRectangle[rectIndex].x() + m_maskRectangle[rectIndex].width() - m_sensitivityTextWidth - 4),
                                                      (m_maskRectangle[rectIndex].y() + 2),
                                                      m_sensitivityTextWidth,
                                                      m_textHeight);
    }
}

quint16 IpMotionDetectionSettings::findRectangleNumber()
{
    quint16 rectNumber = MAX_MOTION_BLOCK;
    for(quint16 index = 0; index < m_maxMotionBlock; index++)
    {
        if(((m_rectInfo[index].endCol - m_rectInfo[index].startCol) == 0 )
                && ((m_rectInfo[index].endRow - m_rectInfo[index].startRow) == 0)
                && (m_rectInfo[index].drawFlag == false))
        {
            rectNumber = index;
            break;
        }
    }
    return rectNumber;
}

bool IpMotionDetectionSettings::isMaskRectContainsPoint(QPoint point)
{
    bool status = false;
    for(quint16 index = 0; index < m_maxMotionBlock; index++)
    {
        if(m_maskRectangle[index].contains(point))
        {
            status = true;
            m_currentRectNumber = index;
            break;
        }
    }
    return status;
}

bool IpMotionDetectionSettings::isMaskRectContainsPoint(QPoint point, quint16 rectIndex)
{
    bool status = false;
    if(m_maskRectangle[rectIndex].contains(point))
    {
        status = true;
    }
    return status;
}

bool IpMotionDetectionSettings::isRectangleOverlapping(quint16 rectIndex)
{
    bool status = false;
    for(quint16 index = 0; index < m_maxMotionBlock; index++)
    {
        if(rectIndex != index)
        {
            if((m_maskRectangle[rectIndex].contains(m_maskRectangle[index]))
                    || (m_maskRectangle[index].contains(m_maskRectangle[rectIndex])))
            {
                status = true;
                break;
            }
            else if((m_maskRectangle[rectIndex].intersects(m_maskRectangle[index]))
                    || (m_maskRectangle[index].intersects(m_maskRectangle[rectIndex])))
            {
                status = true;
                break;
            }
        }
    }
    return status;
}

void IpMotionDetectionSettings::exitAction()
{
    m_infoPage->loadInfoPage( ValidationMessage::getValidationMessage(MAINTAIN_CHANGE), true);
}

void IpMotionDetectionSettings::clearRect()
{
    m_rectInfo[m_currentRectNumber].startCol = 0;
    m_rectInfo[m_currentRectNumber].startRow = 0;
    m_rectInfo[m_currentRectNumber].endCol = 0;
    m_rectInfo[m_currentRectNumber].endRow = 0;
    m_rectInfo[m_currentRectNumber].sensitivity = 5;
    m_rectInfo[m_currentRectNumber].drawFlag = false;
    createMaskRectangle(m_currentRectNumber);
    update ();
}

void IpMotionDetectionSettings::clearAllRect ()
{
    for(quint16 index = 0; index < MAX_MOTION_BLOCK; index++)
    {
        m_rectInfo[index].index = index;
        m_rectInfo[index].startCol = 0;
        m_rectInfo[index].startRow = 0;
        m_rectInfo[index].endCol = 0;
        m_rectInfo[index].endRow = 0;
        m_rectInfo[index].drawFlag = false;
        m_rectInfo[index].sensitivity = 5;
        createMaskRectangle(index);
    }
    m_sensitivity = 5;
    m_isNoMotionEvent = false;
    m_noMotionDuration = 5;
    update ();
}

quint8 IpMotionDetectionSettings::findRowOfPoint(QPoint point)
{
    return ceil(qreal(point.y() + 1) / m_blockHeight);
}

quint8 IpMotionDetectionSettings::findColOfPoint(QPoint point)
{
    return ceil(qreal(point.x() + 1) / m_blockWidth);
}

void IpMotionDetectionSettings::loadPickList()
{
    /* PARASOFT: Memory Deallocated in slot PickListDestroyed & CloseButtonClicked */
    m_inVisibleWidget = new QWidget(this->window());
    m_inVisibleWidget->setGeometry(QRect(0,
                                         0,
                                         this->window()->width(),
                                         this->window()->height()));
    m_inVisibleWidget->show();

    if (IPMOTIONDETECTION_IP_POINT_SENSITIVITY_LABEL == m_pointActionToPerform)
    {
        QMap<quint8, QString> sensitivityMap;
        for(quint8 index = 0; index < m_sensitivityLevelList.length(); index++)
        {
            sensitivityMap.insert(index, m_sensitivityLevelList.value(index));
        }

        m_picklistLoader = new PickListLoader(sensitivityMap,
                                              ((m_motionSupportType == POINT_METHOD) ?
                                              (m_rectInfo[m_currentEditRectNumber].sensitivity - 1)
                                              : (m_sensitivity - 1)),
                                              "Select Sensitivity",
                                              this->window());
        connect(m_picklistLoader,
                SIGNAL(sigValueChanged(quint8,QString,bool)),
                this,
                SLOT(slotValueChanged(quint8,QString,bool)));
        connect(m_picklistLoader,
                SIGNAL(destroyed()),
                this,
                SLOT(slotPickListDestroyed()));
    }
    else if(IPMOTIONDETECTION_IP_POINT_EVENT_LABEL == m_pointActionToPerform)
    {
        QMap<quint8, QString> eventTypeMap;
        for(quint8 index = 0; index < MAX_MOTION_EVENT_LIST; index++)
        {
            eventTypeMap.insert(index, motionEvent[index]);
        }
        m_picklistLoader = new PickListLoader(eventTypeMap, m_isNoMotionEvent, "Select Event", this->window());
        connect(m_picklistLoader,
                SIGNAL(sigValueChanged(quint8,QString,bool)),
                this,
                SLOT(slotValueChanged(quint8,QString,bool)));
        connect(m_picklistLoader,
                SIGNAL(destroyed()),
                this,
                SLOT(slotPickListDestroyed()));
    }
    else if(IPMOTIONDETECTION_IP_POINT_NO_MOTION_DURATION_LABEL == m_pointActionToPerform)
    {
        /* PARASOFT: Memory Deallocated in slot CloseButtonClicked */
        m_background = new Rectangle((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - SCALE_WIDTH(526)) / 2)),
                                     (ApplController::getYPosOfScreen() + ((ApplController::getHeightOfScreen() - SCALE_HEIGHT(170)) / 2)),
                                     SCALE_WIDTH(526),
                                     SCALE_HEIGHT(110),
                                     SCALE_WIDTH(RECT_RADIUS),
                                     NORMAL_BKG_COLOR,
                                     NORMAL_BKG_COLOR,
                                     this->window());

        /* PARASOFT: Memory Deallocated in slot CloseButtonClicked */
        m_pageHeading = new Heading(m_background->x() +  m_background->width()/2, m_background->y() + SCALE_HEIGHT(20), "Duration", this->window(), HEADING_TYPE_2);

        /* PARASOFT: Memory Deallocated in slot CloseButtonClicked */
        m_closeButton = new CloseButtton((m_background->x() + m_background->width() - SCALE_WIDTH(20)),
                                         (m_background->y() + SCALE_HEIGHT(20)),
                                         this->window(),
                                         CLOSE_BTN_TYPE_1,
                                         0);
        connect(m_closeButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotCloseButtonClicked(int)));

        /* PARASOFT: Memory Deallocated in slot CloseButtonClicked */
        m_NoMotionDurationTextBoxParam = new TextboxParam();
        m_NoMotionDurationTextBoxParam->labelStr = "Duration";
        m_NoMotionDurationTextBoxParam->suffixStr = "(5-3600 sec)";
        m_NoMotionDurationTextBoxParam->isNumEntry = true;
        m_NoMotionDurationTextBoxParam->minNumValue = 5;
        m_NoMotionDurationTextBoxParam->maxNumValue = 3600;
        m_NoMotionDurationTextBoxParam->maxChar = 4;
        m_NoMotionDurationTextBoxParam->validation = QRegExp(QString("[0-9]"));

        /* PARASOFT: Memory Deallocated in slot CloseButtonClicked */
        m_NoMotionDurationTextBox = new TextBox((SCALE_WIDTH(20) + m_background->x()),
                                                (SCALE_HEIGHT(40) + m_background->y()),
                                                SCALE_WIDTH(486),
                                                SCALE_HEIGHT(40),
                                                1,
                                                TEXTBOX_SMALL,
                                                this->window(),
                                                m_NoMotionDurationTextBoxParam,
                                                COMMON_LAYER,
                                                true);
        m_NoMotionDurationTextBox->setInputText(QString("%1").arg(m_noMotionDuration));
    }
}

void IpMotionDetectionSettings::mouseLeftButtonPressEvent(QMouseEvent* event)
{
    bool returnFlag = false;
    for(quint16 index = 0; index < m_maxMotionBlock; index++)
    {
        if(isMaskRectContainsPoint(event->pos(), index))
        {
            if(m_maskSensitivityRectangle[index].contains(event->pos()))
            {
                m_currentEditRectNumber = index;
            }
            returnFlag = true;
        }
    }

    if(!returnFlag)
    {
        quint16 rectNumber = findRectangleNumber();

        if(rectNumber != MAX_MOTION_BLOCK)
        {
            m_currentRectNumber = rectNumber;
            m_mouseLeftClick = true;

            if(m_motionSupportType == POINT_METHOD)
            {
                m_startPoint = QPoint(event->pos());
                m_rectInfo[m_currentRectNumber].startRow = m_startPoint.y ();
                m_rectInfo[m_currentRectNumber].startCol = m_startPoint.x ();
                m_rectInfo[m_currentRectNumber].endCol = m_startCol;
                m_rectInfo[m_currentRectNumber].endRow = m_startRow;
            }
            else
            {
                m_startCol = (findColOfPoint(event->pos()) - 1);
                m_startRow = (findRowOfPoint(event->pos()) - 1);
                m_rectInfo[m_currentRectNumber].startCol = m_startCol;
                m_rectInfo[m_currentRectNumber].startRow = m_startRow;
                m_rectInfo[m_currentRectNumber].endCol = m_startCol;
                m_rectInfo[m_currentRectNumber].endRow = m_startRow;
                m_rectInfo[m_currentRectNumber].drawFlag = false;
            }
        }
    }
}

void IpMotionDetectionSettings::mouseRightButtonPressEvent(QMouseEvent* event)
{
    m_mouseRightClick = true;
    m_rightClickPoint = QPoint(event->pos());
}

void IpMotionDetectionSettings::mouseLeftButtonReleaseEvent(QMouseEvent*)
{
    if(m_currentRectNumber != MAX_MOTION_BLOCK)
    {
        if(!m_rectInfo[m_currentRectNumber].drawFlag)
        {
            clearRect();
        }
        else
        {
            if((!m_isOverlapingAllowed)
                    && (isRectangleOverlapping(m_currentRectNumber)))
            {
                clearRect();
            }
        }

        if(m_motionSupportType == BLOCK_METHOD)
        {
            quint16 temRect = 0;

            // at top most rectangle number is zero so value of rectinfo
            // reinitilize again with new value so loop conditions also modified
            // so have to take temp values intead of direct use
            quint16 startRow = m_rectInfo[m_currentRectNumber].startRow;
            quint16 endRow = m_rectInfo[m_currentRectNumber].endRow;
            quint16 startCol = m_rectInfo[m_currentRectNumber].startCol;
            quint16 endCol = m_rectInfo[m_currentRectNumber].endCol;

            for(quint16 row = startRow; row < endRow; row++)
            {
                for(quint16 col = startCol; col < endCol; col++)
                {
                    temRect = col + (row * MAX_COL);
                    m_rectInfo[temRect].sensitivity = m_sensitivity;
                    m_rectInfo[temRect].startCol = col;
                    m_rectInfo[temRect].startRow = row;
                    m_rectInfo[temRect].endCol = (m_rectInfo[temRect].startCol + 1);
                    m_rectInfo[temRect].endRow = (m_rectInfo[temRect].startRow + 1);
                    m_rectInfo[temRect].drawFlag = true;

                    createMaskRectangle(temRect);
                }
            }
            update ();
        }

        m_mouseLeftClick = false;
        m_currentRectNumber = MAX_MOTION_BLOCK;
        m_startCol = MAX_COL;
        m_startRow = m_maxRow;
    }

    if(m_currentEditRectNumber != MAX_MOTION_BLOCK)
    {
        loadPickList();
    }
}

void IpMotionDetectionSettings::mouseRightButtonReleaseEvent(QMouseEvent* event)
{
    if(m_motionSupportType == POINT_METHOD)
    {
        if(isMaskRectContainsPoint(event->pos()))
        {
            m_menuButtonList = new MenuButtonList(m_rightClickPoint.x(),
                                                  m_rightClickPoint.y(),
                                                  (QStringList() << menuLabel[IPMOTIONDETECTION_CLEAR_LABEL] << menuLabel[IPMOTIONDETECTION_EXIT_LABEL]),
                                                  this,
                                                  false,
                                                  SCALE_WIDTH(70));
        }
        else
        {
            m_menuButtonList = new MenuButtonList((this->width() / 2),
                                                  (this->height() / 2),
                                                  (QStringList() << menuLabel[IPMOTIONDETECTION_EXIT_LABEL]),
                                                  this,
                                                  false,
                                                  SCALE_WIDTH(70));
        }
    }
    else
    {
        if (true == m_noMotionEventSupportF)
        {
            m_menuButtonList = new MenuButtonList(m_rightClickPoint.x(),
                                                  m_rightClickPoint.y(),
                                                  (QStringList() << menuIpPointLabel[IPMOTIONDETECTION_IP_POINT_EVENT_LABEL]
                                                   << menuIpPointLabel[IPMOTIONDETECTION_IP_POINT_SENSITIVITY_LABEL]
                                                   << menuIpPointLabel[IPMOTIONDETECTION_IP_POINT_NO_MOTION_DURATION_LABEL]
                                                   << menuIpPointLabel[IPMOTIONDETECTION_IP_POINT_CLEAR_ALL_LABEL]
                                                   << menuIpPointLabel[IPMOTIONDETECTION_IP_POINT_EXIT_LABEL]),
                                                  this,
                                                  false,
                                                  SCALE_WIDTH(120));

            /* Enable duration button if no motion is selected else disable it */
            m_menuButtonList->disableButton(IPMOTIONDETECTION_IP_POINT_NO_MOTION_DURATION_LABEL, !m_isNoMotionEvent);
        }
        else
        {
            m_menuButtonList = new MenuButtonList(m_rightClickPoint.x(),
                                                  m_rightClickPoint.y(),
                                                  (QStringList() << menuIpPointLabel[IPMOTIONDETECTION_IP_POINT_SENSITIVITY_LABEL]
                                                   << menuIpPointLabel[IPMOTIONDETECTION_IP_POINT_CLEAR_ALL_LABEL]
                                                   << menuIpPointLabel[IPMOTIONDETECTION_IP_POINT_EXIT_LABEL]),
                                                  this,
                                                  false,
                                                  SCALE_WIDTH(120));
        }
    }
    connect(m_menuButtonList,
            SIGNAL(sigMenuSelected(QString, quint8)),
            this,
            SLOT(slotMenuButtonSelected(QString, quint8)));
    connect(m_menuButtonList,
            SIGNAL(destroyed()),
            this,
            SLOT(slotMenuListDestroyed()));
    m_mouseRightClick = false;
    m_rightClickPoint = QPoint(0, 0);
}

void IpMotionDetectionSettings::saveConfig()
{
    if(m_motionSupportType == POINT_METHOD)
    {
        m_payloadLib->setCnfgArrayAtIndex(0, m_cameraIndex);
        m_payloadLib->setCnfgArrayAtIndex(1, POINT_METHOD);

        for(quint8 index = 0, fieldIndex = 0; index < m_maxMotionBlock; index++)
        {
            if(m_rectInfo[index].drawFlag == true)
            {
                m_payloadLib->setCnfgArrayAtIndex((2 + (index * 5) + fieldIndex++),
                                                  ((m_rectInfo[index].startCol* WIDTH_RATIO) / MAX_SCREEN_WIDTH));
                m_payloadLib->setCnfgArrayAtIndex((2 + (index * 5) + fieldIndex++),
                                                  ((m_rectInfo[index].startRow* HEIGHT_RATIO) / MAX_SCREEN_HEIGHT));

                m_payloadLib->setCnfgArrayAtIndex((2 + (index * 5) + fieldIndex++),
                                                  ((m_rectInfo[index].endCol - m_rectInfo[index].startCol) * WIDTH_RATIO) / MAX_SCREEN_WIDTH);

                m_payloadLib->setCnfgArrayAtIndex((2 + (index * 5) + fieldIndex++),
                                                  ((m_rectInfo[index].endRow - m_rectInfo[index].startRow) * HEIGHT_RATIO) / MAX_SCREEN_HEIGHT);

                m_payloadLib->setCnfgArrayAtIndex((2 + (index * 5) + fieldIndex),
                                                  m_rectInfo[index].sensitivity);
            }
            else
            {
                m_payloadLib->setCnfgArrayAtIndex((2 + (index * 5) + fieldIndex++), 0);
                m_payloadLib->setCnfgArrayAtIndex((2 + (index * 5) + fieldIndex++), 0);
                m_payloadLib->setCnfgArrayAtIndex((2 + (index * 5) + fieldIndex++), 0);
                m_payloadLib->setCnfgArrayAtIndex((2 + (index * 5) + fieldIndex++), 0);
                m_payloadLib->setCnfgArrayAtIndex((2 + (index * 5) + fieldIndex), 5);
            }
            fieldIndex = 0;
        }

        QString payloadString = m_payloadLib->createDevCmdPayload(22);
        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CMD;
        param->cmdType = SET_MOTION_WINDOW;
        param->payload = payloadString;
        m_applController->processActivity(m_deviceName, DEVICE_COMM, param);
    }
    else
    {
        for(quint16 byteIndex = 0; byteIndex < MAX_MOTION_BLOCK; byteIndex++)
        {
            m_rectInfo[byteIndex].drawFlag = false;
        }

        for(quint16 byteIndex = 0; byteIndex < MAX_MOTION_BLOCK; byteIndex++)
        {
            for(quint16 col = m_rectInfo[byteIndex].startCol; col < m_rectInfo[byteIndex].endCol; col++)
            {
                for(quint16 row = m_rectInfo[byteIndex].startRow; row < m_rectInfo[byteIndex].endRow; row++)
                {
                    m_rectInfo[((row*MAX_COL) + col)].drawFlag = true;
                }
            }
        }

        quint16 blockIndex = 0;
        QString byteValueInHex = "";

        for(quint8 byteIndex = 0; byteIndex < MAX_MOTION_BYTE; byteIndex++)
        {
            quint8 byteValue = 0;

            for(qint8 bitIndex = 7; bitIndex >= 0; bitIndex--)
            {
                if(m_rectInfo[blockIndex++].drawFlag == true)
                {
                    byteValue |= ((quint8)1 << bitIndex);
                }
            }

            QString temp = QString("%1").arg (byteValue,1,16);
            if(temp.length () == 1)
            {
                temp = temp.insert (0,"0");
            }

            byteValueInHex.append (temp);
        }

        m_payloadLib->setCnfgArrayAtIndex(0, m_cameraIndex);
        m_payloadLib->setCnfgArrayAtIndex(1, BLOCK_METHOD);
        m_payloadLib->setCnfgArrayAtIndex(2, m_sensitivity);
        m_payloadLib->setCnfgArrayAtIndex(3, m_noMotionEventSupportF);
        m_payloadLib->setCnfgArrayAtIndex(4, m_isNoMotionEvent);
        m_payloadLib->setCnfgArrayAtIndex(5, m_noMotionDuration);
        m_payloadLib->setCnfgArrayAtIndex(6, byteValueInHex);

        QString payloadString = m_payloadLib->createDevCmdPayload(7);

        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CMD;
        param->cmdType = SET_MOTION_WINDOW;
        param->payload = payloadString;

        m_applController->processActivity(m_deviceName, DEVICE_COMM, param);
    }
    emit sigLoadProcessBar();
}

void IpMotionDetectionSettings::forceActiveFocus()
{
    this->setFocus();
}

void IpMotionDetectionSettings::paintEvent(QPaintEvent * event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#000000"));

    for(quint8 index = 0; index < m_maxRow; index++)
    {
        painter.drawRect(0, ((index + 1) * m_blockHeight), this->width(), 2);
    }
    for(quint8 index = 0; index < MAX_COL; index++)
    {
        painter.drawRect(((index + 1) * m_blockWidth), 0, 2, this->height());
    }

    QPen pen = painter.pen();
    pen.setStyle(Qt::SolidLine);
    pen.setCapStyle(Qt::SquareCap);
    pen.setJoinStyle(Qt::MiterJoin);

    for(quint16 index = 0; index < m_maxMotionBlock; index++)
    {
        if(m_motionSupportType == POINT_METHOD)
        {
            if(index == m_hightlightedRectNumber)
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
        }
        else
        {
            pen.setWidth(4);
            pen.setBrush(QBrush(QColor(RED_COLOR)));
            painter.setPen(pen);
        }

        if((m_maskRectangle[index].width() > 0) && (m_maskRectangle[index].height() > 0))
        {
            QColor color;
            color.setAlpha (0);
            QPen pen = painter.pen();
            painter.setBrush(QBrush(color));
            painter.drawRect(m_maskRectangle[index]);
            pen.setWidth(5);
            painter.setPen(pen);
            painter.setPen(QColor(HIGHLITED_FONT_COLOR));

            if(m_motionSupportType == POINT_METHOD)
            {
                QFont font = TextLabel::getFont (NORMAL_FONT_FAMILY,20,true);
                painter.setFont(font);

                QString textString = QString("%1").arg(m_rectInfo[index].index + 1);
                QString sensitivitString = QString("S:") + QString("%1").arg(m_rectInfo[index].sensitivity);

                painter.drawText(m_maskTextRectangle[index], textString);
                painter.drawText(m_maskSensitivityRectangle[index], sensitivitString);
            }
        }
    }
}

void IpMotionDetectionSettings::mousePressEvent(QMouseEvent * event)
{
    bool isMenuEnabled = false;
    if(m_menuButtonList != NULL)
    {
        isMenuEnabled = true;
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

    if (event->button() == Qt::LeftButton)
    {
        /* Ignore left click if menu was enabled */
        if (false == isMenuEnabled)
        {
            mouseLeftButtonPressEvent(event);
        }
    }
    else if (event->button() == Qt::RightButton)
    {
        mouseRightButtonPressEvent(event);
    }
}

void IpMotionDetectionSettings::mouseReleaseEvent(QMouseEvent * event)
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

void IpMotionDetectionSettings::mouseMoveEvent(QMouseEvent * event)
{
    qint16 diffX, diffY;
    if(m_mouseLeftClick)
    {
        m_rectInfo[m_currentRectNumber].drawFlag = true;
        if(m_motionSupportType == POINT_METHOD)
        {
            diffX = event->x() - m_startPoint.x();
            diffY = event->y() - m_startPoint.y();

            if((abs(diffX) >= 20) && (abs(diffY) >= 20))
            {
                m_rectInfo[m_currentRectNumber].startRow = m_startPoint.y();
                m_rectInfo[m_currentRectNumber].startCol = m_startPoint.x();
            }

            if(m_rectInfo[m_currentRectNumber].startRow > event->y())
            {
                m_rectInfo[m_currentRectNumber].endRow = m_startPoint.y();
                m_rectInfo[m_currentRectNumber].startRow = event->y();
            }
            else if(m_rectInfo[m_currentRectNumber].startRow < event->y())
            {
                m_rectInfo[m_currentRectNumber].endRow = event->y();
            }

            if(m_rectInfo[m_currentRectNumber].startCol > event->x())
            {
                m_rectInfo[m_currentRectNumber].endCol = m_startPoint.x();
                m_rectInfo[m_currentRectNumber].startCol = event->x();
            }
            else if(m_rectInfo[m_currentRectNumber].startCol < event->x())
            {
                m_rectInfo[m_currentRectNumber].endCol = event->x();
            }
        }
        else
        {
            quint8 pointCol = findColOfPoint(event->pos());
            quint8 pointRow = findRowOfPoint(event->pos());

            if(pointCol > MAX_COL)
            {
                pointCol = MAX_COL;
            }
            if(pointRow > MAX_PAL_ROW)
            {
                pointRow = MAX_PAL_ROW;
            }

            if(m_startCol < pointCol)
            {
                m_rectInfo[m_currentRectNumber].endCol = pointCol;
                m_rectInfo[m_currentRectNumber].startCol = m_startCol;
            }
            else if(m_startCol >= pointCol)
            {
                m_rectInfo[m_currentRectNumber].startCol = (pointCol - 1);
            }
            if(m_startRow < pointRow)
            {
                m_rectInfo[m_currentRectNumber].endRow = pointRow;
                m_rectInfo[m_currentRectNumber].startRow = m_startRow;
            }
            else if(m_startRow >= pointRow)
            {
                m_rectInfo[m_currentRectNumber].startRow = (pointRow - 1);
            }
        }

        createMaskRectangle(m_currentRectNumber);
        update();
    }
    else if(m_motionSupportType == POINT_METHOD)
    {
        quint16 rectIndex = MAX_MOTION_BLOCK;
        for(quint16 index = 0; index < m_maxMotionBlock; index++)
        {
            if(isMaskRectContainsPoint(event->pos(), index))
            {
                rectIndex = index;
                break;
            }
        }

        if((rectIndex != m_hightlightedRectNumber))
        {
            quint8 oldHighlighted = m_hightlightedRectNumber;
            m_hightlightedRectNumber = rectIndex;
            if(m_hightlightedRectNumber != MAX_MOTIONDETECTION_AREA)
            {
                QRect rect((m_maskRectangle[m_hightlightedRectNumber].x() - 5),
                           (m_maskRectangle[m_hightlightedRectNumber].y() - 5),
                           (m_maskRectangle[m_hightlightedRectNumber].width() + 10),
                           (m_maskRectangle[m_hightlightedRectNumber].height() + 10));
                update(rect);
            }
            if(oldHighlighted != MAX_MOTIONDETECTION_AREA)
            {
                QRect rect((m_maskRectangle[oldHighlighted].x() - 5),
                           (m_maskRectangle[oldHighlighted].y() - 5),
                           (m_maskRectangle[oldHighlighted].width() + 10),
                           (m_maskRectangle[oldHighlighted].height() + 10));
                update(rect);
            }
        }
        update();
    }
}

void IpMotionDetectionSettings::slotMenuButtonSelected(QString menuLabelString, quint8)
{
    disconnect(m_menuButtonList,
               SIGNAL(sigMenuSelected(QString, quint8)),
               this,
               SLOT(slotMenuButtonSelected(QString, quint8)));

    if(m_motionSupportType == POINT_METHOD)
    {
        if(menuLabelString == menuLabel[IPMOTIONDETECTION_CLEAR_LABEL])
        {
            m_actionToPerform = IPMOTIONDETECTION_CLEAR_LABEL;
        }
        else if(menuLabelString == menuLabel[IPMOTIONDETECTION_EXIT_LABEL])
        {
            m_actionToPerform = IPMOTIONDETECTION_EXIT_LABEL;
        }
    }
    else
    {
        if(menuLabelString == menuIpPointLabel[IPMOTIONDETECTION_IP_POINT_SENSITIVITY_LABEL])
        {
            m_pointActionToPerform = IPMOTIONDETECTION_IP_POINT_SENSITIVITY_LABEL;
        }
        else if(menuLabelString == menuIpPointLabel[IPMOTIONDETECTION_IP_POINT_EVENT_LABEL])
        {
            m_pointActionToPerform = IPMOTIONDETECTION_IP_POINT_EVENT_LABEL;
        }
        else if(menuLabelString == menuIpPointLabel[IPMOTIONDETECTION_IP_POINT_NO_MOTION_DURATION_LABEL])
        {
            m_pointActionToPerform = IPMOTIONDETECTION_IP_POINT_NO_MOTION_DURATION_LABEL;
        }
        else if(menuLabelString == menuIpPointLabel[IPMOTIONDETECTION_IP_POINT_CLEAR_ALL_LABEL])
        {
            m_pointActionToPerform = IPMOTIONDETECTION_IP_POINT_CLEAR_ALL_LABEL;
        }
        else if(menuLabelString == menuIpPointLabel[IPMOTIONDETECTION_IP_POINT_EXIT_LABEL])
        {
            m_pointActionToPerform = IPMOTIONDETECTION_IP_POINT_EXIT_LABEL;
        }
    }
}

void IpMotionDetectionSettings::slotMenuListDestroyed()
{
    disconnect(m_menuButtonList,
               SIGNAL(destroyed()),
               this,
               SLOT(slotMenuListDestroyed()));

    if(m_motionSupportType == POINT_METHOD)
    {
        if(m_actionToPerform == IPMOTIONDETECTION_CLEAR_LABEL)
        {
            clearRect();
        }
        else if(m_actionToPerform == IPMOTIONDETECTION_EXIT_LABEL)
        {
            m_infoPage->loadInfoPage( ValidationMessage::getValidationMessage(MAINTAIN_CHANGE), true);
        }
    }
    else
    {
        if ((m_pointActionToPerform == IPMOTIONDETECTION_IP_POINT_SENSITIVITY_LABEL)
            || (m_pointActionToPerform == IPMOTIONDETECTION_IP_POINT_EVENT_LABEL)
            || (m_pointActionToPerform == IPMOTIONDETECTION_IP_POINT_NO_MOTION_DURATION_LABEL))
        {
            loadPickList();
        }
        else if(m_pointActionToPerform == IPMOTIONDETECTION_IP_POINT_CLEAR_ALL_LABEL)
        {
            clearAllRect ();
        }
        else if(m_pointActionToPerform == IPMOTIONDETECTION_IP_POINT_EXIT_LABEL)
        {
            m_infoPage->loadInfoPage( ValidationMessage::getValidationMessage(MAINTAIN_CHANGE), true);
        }
    }

    m_actionToPerform = MAX_IPMOTIONDETECTION_MENU_LABEL_TYPE;
    forceActiveFocus();
    m_menuButtonList = NULL;
}

void IpMotionDetectionSettings::slotInfoPageButtonClicked(int index)
{
    forceActiveFocus();
    if(index == INFO_OK_BTN)
    {
        saveConfig();
    }
    this->deleteLater();
}

void IpMotionDetectionSettings::slotValueChanged(quint8 key, QString value, bool)
{
    forceActiveFocus();
    disconnect(m_picklistLoader,
            SIGNAL(sigValueChanged(quint8,QString,bool)),
            this,
            SLOT(slotValueChanged(quint8,QString,bool)));
    if(m_motionSupportType == POINT_METHOD)
    {
        m_rectInfo[m_currentEditRectNumber].sensitivity = value.toInt();
    }
    else
    {
        if (m_pointActionToPerform == IPMOTIONDETECTION_IP_POINT_SENSITIVITY_LABEL)
        {
            m_sensitivity = value.toInt ();
        }
        if (m_pointActionToPerform == IPMOTIONDETECTION_IP_POINT_EVENT_LABEL)
        {
            m_isNoMotionEvent = key;
        }
    }
    m_currentEditRectNumber = MAX_MOTION_BLOCK;
}

void IpMotionDetectionSettings::slotPickListDestroyed()
{        
    disconnect(m_picklistLoader,
               SIGNAL(destroyed()),
               this,
               SLOT(slotPickListDestroyed()));
    m_picklistLoader = NULL;
    DELETE_OBJ(m_inVisibleWidget);
}

void IpMotionDetectionSettings::slotCloseButtonClicked(int)
{
    m_noMotionDuration = m_NoMotionDurationTextBox->getInputText().toInt();
    disconnect(m_closeButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotCloseButtonClicked(int)));
    DELETE_OBJ(m_closeButton);
    DELETE_OBJ(m_pageHeading);
    DELETE_OBJ(m_NoMotionDurationTextBox);
    DELETE_OBJ(m_background);
    DELETE_OBJ(m_NoMotionDurationTextBoxParam);
    DELETE_OBJ(m_inVisibleWidget);
}
