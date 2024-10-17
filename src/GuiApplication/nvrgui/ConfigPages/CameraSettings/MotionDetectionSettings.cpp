#include "MotionDetectionSettings.h"
#include "ValidationMessage.h"

#include <QMouseEvent>
#include <QPainter>

#define MAX_COL     43

const QString menuLabel[] =     {"Clear", "Exit"};

MotionDetectionSettings::MotionDetectionSettings(void *data,QString deiceName,
                                                 QWidget* parent) : QWidget(parent)
{
    m_sensitivityLevelList.reserve(10);

    for(quint8 index = 0; index < 10; index++)
    {
        m_sensitivityLevelList.append(QString("%1").arg(index + 1));
    }

    QFont font = TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
    m_textWidth = QFontMetrics(font).width("4");
    m_textHeight = QFontMetrics(font).height();
    m_sensitivityTextWidth = QFontMetrics(font).width("S:10");

    m_blockWidth = ceil((qreal)ApplController::getWidthOfScreen() / (MAX_COL));

    m_applController = ApplController::getInstance();

    if(!m_applController->GetVideoStandard(deiceName, m_videoStandard))
    {
        m_videoStandard = MAX_VIDEO_STANDARD;
    }

    switch(m_videoStandard)
    {
    case PAL_VIDEO_STANDARD:
        m_maxRow = MAX_PAL_ROW;
        break;
    case NTSC_VIDEO_STANDARD:
        m_maxRow = MAX_NTSC_ROW;
        break;
    default:
        break;
    }


    m_startCol = MAX_COL;
    m_startRow = m_maxRow;
    m_blockHeight = ceil((qreal)ApplController::getHeightOfScreen() / (m_maxRow));

    m_mouseLeftClick = false;
    m_mouseRightClick = false;
    m_menuButtonList = NULL;
    m_picklistLoader = NULL;
    m_inVisibleWidget = NULL;
    m_motionDetectionData = (MOTION_DETECTION_WINDOWINFO_t*)data;
    m_hightlightedRectNumber = MAX_MOTIONDETECTION_AREA;
    m_currentRectNumber = MAX_MOTIONDETECTION_AREA;
    m_currentEditRectNumber = MAX_MOTIONDETECTION_AREA;
    m_actionToPerform = MAX_MOTIONDETECTION_MENU_LABEL_TYPE;

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

    for(quint8 index = 0; index < MAX_MOTIONDETECTION_AREA; index++)
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

MotionDetectionSettings::~MotionDetectionSettings()
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
        delete m_inVisibleWidget;
    }
    m_sensitivityLevelList.clear();
}

void MotionDetectionSettings::setGeometryForElements()
{
    for(quint8 index = 0; index < MAX_MOTIONDETECTION_AREA; index++)
    {
        m_rectInfo[index].index = index;
        m_rectInfo[index].sensitivity = m_motionDetectionData[index].sensitivity;
        if((m_motionDetectionData[index].startCol != m_motionDetectionData[index].endCol)
                && (m_motionDetectionData[index].startRow != m_motionDetectionData[index].endRow))
        {
            m_rectInfo[index].startCol = m_motionDetectionData[index].startCol;
            m_rectInfo[index].startRow = m_motionDetectionData[index].startRow;
            m_rectInfo[index].endCol = m_motionDetectionData[index].endCol;
            m_rectInfo[index].endRow = m_motionDetectionData[index].endRow;
            m_rectInfo[index].drawFlag = true;
        }
        else
        {
            m_rectInfo[index].startCol = 0;
            m_rectInfo[index].startRow = 0;
            m_rectInfo[index].endCol = 0;
            m_rectInfo[index].endRow = 0;
            m_rectInfo[index].drawFlag = false;
        }
        createMaskRectangle(index);
    }
}

void MotionDetectionSettings::createMaskRectangle(quint8 rectIndex)
{
    quint16 width, height;
    width = (m_rectInfo[rectIndex].endCol - m_rectInfo[rectIndex].startCol) * m_blockWidth;
    height = (m_rectInfo[rectIndex].endRow - m_rectInfo[rectIndex].startRow) * m_blockHeight;
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

    m_maskRectangle[rectIndex].setRect((m_rectInfo[rectIndex].startCol * m_blockWidth),
                                       (m_rectInfo[rectIndex].startRow * m_blockHeight),
                                       (width),
                                       (height));
    m_maskTextRectangle[rectIndex].setRect((m_maskRectangle[rectIndex].x() + 2),
                                           (m_maskRectangle[rectIndex].y() + 2),
                                           m_textWidth,
                                           m_textHeight);

    m_maskSensitivityRectangle[rectIndex].setRect((m_maskRectangle[rectIndex].x() + m_maskRectangle[rectIndex].width() - m_sensitivityTextWidth),
                                                  (m_maskRectangle[rectIndex].y() + 2),
                                                  m_sensitivityTextWidth,
                                                  m_textHeight);
    update();
}

quint8 MotionDetectionSettings::findRectangleNumber()
{
    quint8 rectNumber = MAX_MOTIONDETECTION_AREA;
    for(quint8 index = 0; index < MAX_MOTIONDETECTION_AREA; index++)
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

bool MotionDetectionSettings::isMaskRectContainsPoint(QPoint point)
{
    bool status = false;
    for(quint8 index = 0; index < MAX_MOTIONDETECTION_AREA; index++)
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

bool MotionDetectionSettings::isMaskRectContainsPoint(QPoint point, quint8 rectIndex)
{
    bool status = false;
    if(m_maskRectangle[rectIndex].contains(point))
    {
        status = true;
    }
    return status;
}

bool MotionDetectionSettings::isRectangleOverlapping(quint8 rectIndex)
{
    bool status = false;
    for(quint8 index = 0; index < MAX_MOTIONDETECTION_AREA; index++)
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

void MotionDetectionSettings::exitAction()
{
    m_infoPage->loadInfoPage( ValidationMessage::getValidationMessage(SAVE_CHANGES), true);
}

void MotionDetectionSettings::clearRect()
{
    m_rectInfo[m_currentRectNumber].startCol = 0;
    m_rectInfo[m_currentRectNumber].startRow = 0;
    m_rectInfo[m_currentRectNumber].endCol = 0;
    m_rectInfo[m_currentRectNumber].endRow = 0;
    m_rectInfo[m_currentRectNumber].sensitivity = 5;
    m_rectInfo[m_currentRectNumber].drawFlag = false;
    createMaskRectangle(m_currentRectNumber);
}

quint8 MotionDetectionSettings::findRowOfPoint(QPoint point)
{
    return ceil(qreal(point.y() + 1) / m_blockHeight);
}

quint8 MotionDetectionSettings::findColOfPoint(QPoint point)
{
    return ceil(qreal(point.x() + 1) / m_blockWidth);
}

void MotionDetectionSettings::loadPickList()
{
    m_inVisibleWidget = new QWidget(this->window());
    m_inVisibleWidget->setGeometry(QRect(0,
                                         0,
                                         this->window()->width(),
                                         this->window()->height()));
    m_inVisibleWidget->show();

    QMap<quint8, QString> sensitivityMap;
    for(quint8 index = 0; index < m_sensitivityLevelList.length(); index++)
    {
        sensitivityMap.insert(index, m_sensitivityLevelList.value(index));
    }
    m_picklistLoader = new PickListLoader(sensitivityMap,
                                          (m_rectInfo[m_currentEditRectNumber].sensitivity - 1),
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

void MotionDetectionSettings::mouseLeftButtonPressEvent(QMouseEvent* event)
{
    bool returnFlag = false;
    for(quint8 index = 0; index < MAX_MOTIONDETECTION_AREA; index++)
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
        quint8 rectNumber = findRectangleNumber();
        if(rectNumber != MAX_MOTIONDETECTION_AREA)
        {
            m_currentRectNumber = rectNumber;
            m_mouseLeftClick = true;
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

void MotionDetectionSettings::mouseRightButtonPressEvent(QMouseEvent* event)
{
    m_mouseRightClick = true;
    m_rightClickPoint = QPoint(event->pos());
}

void MotionDetectionSettings::mouseLeftButtonReleaseEvent(QMouseEvent*)
{
    if(m_currentRectNumber != MAX_MOTIONDETECTION_AREA)
    {
        if(!m_rectInfo[m_currentRectNumber].drawFlag)
        {
            clearRect();
        }
        else
        {
            if(isRectangleOverlapping(m_currentRectNumber))
            {
                clearRect();
            }
        }
        m_mouseLeftClick = false;
        m_currentRectNumber = MAX_MOTIONDETECTION_AREA;
        m_startCol = MAX_COL;
        m_startRow = m_maxRow;
    }

    if(m_currentEditRectNumber != MAX_MOTIONDETECTION_AREA)
    {
        loadPickList();
    }
}

void MotionDetectionSettings::mouseRightButtonReleaseEvent(QMouseEvent* event)
{
    if(isMaskRectContainsPoint(event->pos()))
    {
        m_menuButtonList = new MenuButtonList(m_rightClickPoint.x(),
                                              m_rightClickPoint.y(),
                                              (QStringList() << menuLabel[MOTIONDETECTION_CLEAR_LABEL] << menuLabel[MOTIONDETECTION_EXIT_LABEL]),
                                              this,
                                              false,
                                              SCALE_WIDTH(70));
    }
    else
    {
        m_menuButtonList = new MenuButtonList((this->width() / 2),
                                              (this->height() / 2),
                                              (QStringList() << menuLabel[MOTIONDETECTION_EXIT_LABEL]),
                                              this,
                                              false,
                                              SCALE_WIDTH(70));
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

void MotionDetectionSettings::saveConfig()
{
    for(quint8 index = 0; index < MAX_MOTIONDETECTION_AREA; index++)
    {
        if(m_rectInfo[index].drawFlag)
        {
            m_motionDetectionData[index].startRow = m_rectInfo[index].startRow;
            m_motionDetectionData[index].startCol = m_rectInfo[index].startCol;
            m_motionDetectionData[index].endRow = (m_rectInfo[index].endRow);
            m_motionDetectionData[index].endCol = (m_rectInfo[index].endCol);
            m_motionDetectionData[index].sensitivity = m_rectInfo[index].sensitivity;
        }
        else
        {
            m_motionDetectionData[index].startRow = 0;
            m_motionDetectionData[index].startCol = 0;
            m_motionDetectionData[index].endRow = 0;
            m_motionDetectionData[index].endCol = 0;
            m_motionDetectionData[index].sensitivity = 5;

        }
    }
}

void MotionDetectionSettings::forceActiveFocus()
{
    this->setFocus();
}

void MotionDetectionSettings::paintEvent(QPaintEvent * event)
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

    for(quint8 index = 0; index < MAX_MOTIONDETECTION_AREA; index++)
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

        if((m_maskRectangle[index].width() > 0) && (m_maskRectangle[index].height() > 0))
        {
            QColor color;
            color.setAlpha (0);
            painter.setBrush(QBrush(color));
            painter.drawRect(m_maskRectangle[index]);

            QString textString = QString("%1").arg(m_rectInfo[index].index + 1);
            QString sensitivitString = QString("S:") + QString("%1").arg(m_rectInfo[index].sensitivity);
            painter.setPen(QColor(HIGHLITED_FONT_COLOR));
            painter.drawText(m_maskTextRectangle[index], textString);
            painter.drawText(m_maskSensitivityRectangle[index], sensitivitString);
        }
    }
}

void MotionDetectionSettings::mousePressEvent(QMouseEvent * event)
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

void MotionDetectionSettings::mouseReleaseEvent(QMouseEvent * event)
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

void MotionDetectionSettings::mouseMoveEvent(QMouseEvent * event)
{
    if(m_mouseLeftClick)
    {
        m_rectInfo[m_currentRectNumber].drawFlag = true;
        quint8 pointCol = findColOfPoint(event->pos());
        quint8 pointRow = findRowOfPoint(event->pos());
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
        createMaskRectangle(m_currentRectNumber);
    }
    else
    {
        quint8 rectIndex = MAX_MOTIONDETECTION_AREA;
        for(quint8 index = 0; index < MAX_MOTIONDETECTION_AREA; index++)
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
    }
}

void MotionDetectionSettings::slotMenuButtonSelected(QString menuLabelString, quint8)
{
    disconnect(m_menuButtonList,
               SIGNAL(sigMenuSelected(QString, quint8)),
               this,
               SLOT(slotMenuButtonSelected(QString, quint8)));
    if(menuLabelString == menuLabel[MOTIONDETECTION_CLEAR_LABEL])
    {
        m_actionToPerform = MOTIONDETECTION_CLEAR_LABEL;
    }
    else if(menuLabelString == menuLabel[MOTIONDETECTION_EXIT_LABEL])
    {
        m_actionToPerform = MOTIONDETECTION_EXIT_LABEL;
    }
}

void MotionDetectionSettings::slotMenuListDestroyed()
{
    disconnect(m_menuButtonList,
               SIGNAL(destroyed()),
               this,
               SLOT(slotMenuListDestroyed()));
    if(m_actionToPerform == MOTIONDETECTION_CLEAR_LABEL)
    {
        clearRect();
    }
    else if(m_actionToPerform == MOTIONDETECTION_EXIT_LABEL)
    {
        m_infoPage->loadInfoPage( ValidationMessage::getValidationMessage(SAVE_CHANGES), true);
    }
    m_actionToPerform = MAX_MOTIONDETECTION_MENU_LABEL_TYPE;
    forceActiveFocus();
    m_menuButtonList = NULL;
}

void MotionDetectionSettings::slotInfoPageButtonClicked(int index)
{
    forceActiveFocus();
    if(index == INFO_OK_BTN)
    {
        saveConfig();
    }
    this->deleteLater();
}

void MotionDetectionSettings::slotValueChanged(quint8, QString value, bool)
{
    forceActiveFocus();
    disconnect(m_picklistLoader,
               SIGNAL(sigValueChanged(quint8,QString,bool)),
               this,
               SLOT(slotValueChanged(quint8,QString,bool)));

    m_rectInfo[m_currentEditRectNumber].sensitivity = value.toInt();
    m_currentEditRectNumber = MAX_MOTIONDETECTION_AREA;
}

void MotionDetectionSettings::slotPickListDestroyed()
{
    disconnect(m_picklistLoader,
               SIGNAL(destroyed()),
               this,
               SLOT(slotPickListDestroyed()));
    m_picklistLoader = NULL;
    delete m_inVisibleWidget;
}
