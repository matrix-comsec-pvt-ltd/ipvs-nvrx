#include "ZoomFeatureControl.h"
#include <QMouseEvent>
#include <QPainter>
#include <Layout/Layout.h>

#define ZOOM_ICON_IMAGE_PATH        ":Images_Nvrx/WindowIcon/ZoomFeatureIcon/"
#define ZOOM_SCALER                 (24)

const QString menuLabel[] = {"Zoom Out",
                             "Exit"};
const QString imagePath[] = {"Zoom_In.png",
                             "Zoom_Out.png"};

ZoomFeatureControl::ZoomFeatureControl(QString deviceName,
                                       quint16 windowIndex,
                                       QWidget* parent,
                                       bool isRightButtonClickAvailable,
                                       bool isMouseMoveEventToBeForwarded,
                                       quint16 height) : QWidget(parent)
{
    INIT_OBJ(m_zoomIconImage);
    if(height == 0)
    {
        this->setGeometry(ApplController::getXPosOfScreen(),
                          ApplController::getYPosOfScreen(),
                          ApplController::getWidthOfScreen(),
                          ApplController::getHeightOfScreen());
    }
    else
    {
        this->setGeometry(0, 0, parent->width(), height);
    }
    m_deviceName = deviceName;
    m_windowIndex = windowIndex;
    m_isMouseMoveEventToBeForwarded = isMouseMoveEventToBeForwarded;
    m_mouseLeftClick = false;
    m_mouseRightClick = false;
    m_zoomIconImageSource = "";
    m_menuButtonList = NULL;
    m_leftMouseButton = Qt::LeftButton;
    m_actionToPerform = MAX_ZOOM_MENU_LABEL_TYPE;
    pstartx= 0;
    pendx= this->width();
    pstarty = 0;
    pendy=this->height();
    m_scaleFactor = 1.0;
    resetCordinates ();
    if(isRightButtonClickAvailable)
    {
        m_rightMouseButton = Qt::RightButton;
    }
    else
    {
        m_rightMouseButton = Qt::NoButton;
    }
    m_zoomIconImage = new Image((this->width() - SCALE_WIDTH(5)),
                                SCALE_HEIGHT(5),
                                m_zoomIconImageSource,
                                this,
                                END_X_START_Y,
                                0,
                                false,
                                true,
                                false);
    if(IS_VALID_OBJ(m_zoomIconImage))
    {

    }

    changeState(ZOOM_STATE_OUT);
    this->setEnabled(true);
    this->setMouseTracking(true);
    this->show();
}

ZoomFeatureControl::~ZoomFeatureControl()
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
    }
    m_optionList.clear();
    delete m_zoomIconImage;
}

bool ZoomFeatureControl::SetViewArea(qint16 startX,
                                     qint16 endX,
                                     qint16 startY,
                                     qint16 endY)
{
    CROP_PARAM_t cropParam;

    cropParam.startXPos = round(startX);
    cropParam.startYPos = round(startY);
    cropParam.width = round(endX - startX);
    cropParam.height = round(endY - startY);
    pstartx = startX;
    pendx   = endX;
    pstarty = startY;
    pendy   = endY;

    if(startX> endX || startY >endY)
        return false;

    return SetCropParam(Layout::streamInfoArray[MAIN_DISPLAY][m_windowIndex].m_windowId, &cropParam, ZOOM_IN);
}

void ZoomFeatureControl::exitAction()
{
    m_scaleFactor = 1.0;
    resetCordinates ();
    CROP_PARAM_t cropParam;
    SetCropParam(Layout::streamInfoArray[MAIN_DISPLAY][m_windowIndex].m_windowId,
                 &cropParam,ZOOM_OUT);
    sigExitFromZoomFeature();
    this->deleteLater();
}

void ZoomFeatureControl::zoomOutAction()
{
    CROP_PARAM_t cropParam;

    SetCropParam(Layout::streamInfoArray[MAIN_DISPLAY][m_windowIndex].m_windowId,
                 &cropParam,ZOOM_OUT);
    m_currentZoomState = ZOOM_STATE_OUT;
    changeState(ZOOM_STATE_OUT);
    emit sigChangeLayout(ONE_X_ONE,
                         MAIN_DISPLAY,
                         Layout::streamInfoArray[MAIN_DISPLAY][m_windowIndex].m_windowId,
                         true,
                         false);
}

void ZoomFeatureControl::changeState(ZOOM_STATE_TYPE_e zoomState)
{
    m_currentZoomState = zoomState;
    m_zoomIconImageSource = ZOOM_ICON_IMAGE_PATH + imagePath[m_currentZoomState];
    m_zoomIconImage->updateImageSource(m_zoomIconImageSource);
}

void ZoomFeatureControl::mouseLeftButtonPressEvent(QMouseEvent* event)
{
    m_mouseLeftClick = true;
    m_startPoint = QPoint(event->pos());
}

void ZoomFeatureControl::mouseRightButtonPressEvent(QMouseEvent*)
{
    m_mouseRightClick = true;
}

void ZoomFeatureControl::mouseLeftButtonReleaseEvent(QMouseEvent*)
{
    if(m_mouseLeftClick)
    {
        changeState(ZOOM_STATE_IN);
        updateCordinates ();
    }
}

void ZoomFeatureControl::mouseRightButtonReleaseEvent(QMouseEvent* event)
{
    m_optionList << menuLabel[ZOOM_EXIT_LABEL];

    QPoint menu_pos(event->pos().x(),event->pos().y());

    if(menu_pos.x()+BUTTON_WIDTH >this->width())
        menu_pos.setX(event->pos().x()-BUTTON_WIDTH);
    if(menu_pos.y()+BUTTON_HEIGHT>this->height())
        menu_pos.setY(event->pos().y()-BUTTON_HEIGHT);

    m_menuButtonList = new MenuButtonList((menu_pos.x()),
                                          (menu_pos.y()),
                                          m_optionList,
                                          this,
                                          false);
    connect(m_menuButtonList,
            SIGNAL(sigMenuSelected(QString, quint8)),
            this,
            SLOT(slotMenuButtonSelected(QString, quint8)));
    connect(m_menuButtonList,
            SIGNAL(destroyed()),
            this,
            SLOT(slotMenuListDestroyed()));
    m_mouseRightClick = false;
}

void ZoomFeatureControl::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPen pen = painter.pen();
    pen.setWidth(4);
    pen.setBrush(QBrush(QColor(HIGHLITED_FONT_COLOR)));

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(pen);
}

void ZoomFeatureControl::mousePressEvent(QMouseEvent * event)
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
        m_optionList.clear();
    }

    if(event->button() == m_leftMouseButton)
    {

        if(m_currentZoomState != ZOOM_STATE_OUT)
        {
            m_prevstrt = m_startPoint;
            m_startPoint = QPoint(event->pos());
            m_mouseLeftClick = true;
            QApplication::setOverrideCursor(Qt::OpenHandCursor);
        }
    }

    else if(event->button() == m_rightMouseButton)
    {
        mouseRightButtonPressEvent(event);
    }
}

void ZoomFeatureControl::mouseReleaseEvent(QMouseEvent * event)
{
    m_refrancept = QPoint(event->pos());
    if(event->button() == m_leftMouseButton)
    {
        if((m_refrancept.x () != m_startPoint.x ())&&(m_refrancept.y () != m_startPoint.y ()))
        {
            m_mouseLeftClick = false;
            QApplication::setOverrideCursor(Qt::ArrowCursor);
        }
        else
        {
            m_startPoint=m_prevstrt;
            m_mouseLeftClick = false;
            QApplication::setOverrideCursor(Qt::ArrowCursor);
        }
    }
    else if(event->button() == m_rightMouseButton)
    {
        mouseRightButtonReleaseEvent(event);
    }
}

void ZoomFeatureControl::mouseMoveEvent(QMouseEvent * event)
{
    if(m_mouseLeftClick)
    {
        static quint8 mouseCount = 0;
        mouseCount++;
        if(mouseCount > 2)
        {
            m_endPoint = QPoint(event->pos());
            mouseLeftButtonReleaseEvent(event);
            m_startPoint = m_endPoint;
            mouseCount = 0;
        }


    }
    if(m_isMouseMoveEventToBeForwarded)
    {
        QWidget::mouseMoveEvent(event);
    }
}


void ZoomFeatureControl::slotMenuButtonSelected(QString menuLabelString, quint8)
{
    disconnect(m_menuButtonList,
               SIGNAL(sigMenuSelected(QString, quint8)),
               this,
               SLOT(slotMenuButtonSelected(QString, quint8)));
    if(menuLabelString == menuLabel[ZOOM_OUT_LABEL])
    {
        m_actionToPerform = ZOOM_OUT_LABEL;
    }
    else if(menuLabelString == menuLabel[ZOOM_EXIT_LABEL])
    {
        m_actionToPerform = ZOOM_EXIT_LABEL;
    }
}

void ZoomFeatureControl::slotMenuListDestroyed()
{
    disconnect(m_menuButtonList,
               SIGNAL(destroyed()),
               this,
               SLOT(slotMenuListDestroyed()));
    if(m_actionToPerform == ZOOM_OUT_LABEL)
    {
        zoomOutAction();
    }
    else if(m_actionToPerform == ZOOM_EXIT_LABEL)
    {
        exitAction();
    }
    m_actionToPerform = MAX_ZOOM_MENU_LABEL_TYPE;
    m_menuButtonList = NULL;
    m_optionList.clear();
    this->setFocus();
}

void ZoomFeatureControl::wheelEvent(QWheelEvent *event)
{
    quint64 pointUpdate   = 0;
    quint64 y_pointUpdate = 0;

    float scaleF  = m_scaleFactor;
    m_startX=pstartx;
    m_endX=pendx;
    m_startY=pstarty;
    m_endY=pendy;

    if(event->delta() < 0)
    {
        if(m_scaleFactor > 1.0)
        {
            pointUpdate = (33/m_scaleFactor)*((float)this->width()/1000.00);
            y_pointUpdate = pointUpdate*((float)this->height()/(float)this->width());

            m_scaleFactor -= 0.1;

            m_startX -= (pointUpdate);
            m_startY -= (y_pointUpdate);

            m_endX += (pointUpdate);
            m_endY += (y_pointUpdate);

            if(m_startX < 0)
            {
                m_endX -= (m_startX);
                m_startX = 0;
            }

            if(m_startY < 0)
            {
                m_endY -= (m_startY);
                m_startY = 0;
            }

            if ( m_endX > this->width())
            {
                m_startX -= (m_endX - this->width());
                m_endX = this->width();
            }

            if(m_endY > this->height())
            {
                m_startY -= (m_endY-this->height());
                m_endY = this->height();
            }

            if ( m_scaleFactor == 1)
                resetCordinates();
        }
    }
    else if(event->delta() > 0)
    {
        if(m_scaleFactor < 3.8)
        {
            m_scaleFactor += 0.1;
            pointUpdate = (33/m_scaleFactor)*((float)this->width()/1000.00);
            y_pointUpdate = pointUpdate*((float)this->height()/(float)this->width());

            m_startX += (pointUpdate);
            m_startY +=  (y_pointUpdate);

            m_endX -= (pointUpdate);
            m_endY -=  (y_pointUpdate);
        }
    }


    if(m_scaleFactor < 1.0)
    {
        m_scaleFactor = 1.0;
    }

    if(scaleF != m_scaleFactor)
    {
        if((m_startX == 0) && (m_endX == this->width())
                && (m_startY == 0)  && (m_endY == this->height()))
        {
            m_scaleFactor = 1.0;
        }

        if((m_scaleFactor <= 1.0) )
        {
            resetCordinates ();
        }

        if(m_scaleFactor > 1.0)
        {
            changeState(ZOOM_STATE_IN);
        }
        else
        {
            changeState(ZOOM_STATE_OUT);
        }
        SetViewArea(m_startX,m_endX,m_startY,m_endY);

    }

}

void ZoomFeatureControl::resetCordinates()
{
    m_startX    = 0;
    m_endX      = this->width();
    m_startY    = 0;
    m_endY      = this->height();
    m_endPoint  = QPoint(0,0);
    m_startPoint= QPoint(0,0);
}


void ZoomFeatureControl::updateCordinates()
{
    if((m_startPoint != m_endPoint) && (m_scaleFactor > 1.0))
    {
        float  x_offset  = m_startPoint.x()-m_endPoint.x();
        float  y_offset  = m_startPoint.y()-m_endPoint.y();

        if( (pendx+x_offset) >this->width() )
        {
            x_offset = (this->width() - pendx);
        }
        else if( (pstartx+x_offset) < 0)    //to prevent stretch
        {
            x_offset = 0 - pstartx;
        }


        if( (pendy + y_offset) > this->height() )
        {
            y_offset = (this->height() - pendy);
        }
        else if( (pstarty+y_offset) < 0)
        {
            y_offset=0 -pstarty;
        }

        SetViewArea(pstartx+x_offset, pendx+x_offset,pstarty+y_offset,pendy+y_offset);
    }
}
