/////////////////////////////////////////////////////////////////////////////
//   MMM     MMM       AAA       TTTTTTTTTT  RRRRRR    IIIIIIII  XX   XX
//   MMMM   MMMM      AA AA          TT      RR   RR      II      XX XX
//   MM MM MM MM     AA   AA         TT      RR    RR     II       XXX
//   MM  MM   MM    AAAAAAAAA        TT      RRRRRRR      II       XXX
//   MM       MM   AA       AA       TT      RR  RR       II      XX XX
//   MM       MM  AA         AA      TT      RR   RR   IIIIIIII  XX   XX
//
//   Company      : Matrix Telecom Pvt. Ltd., Baroda, India.
//   Project      : DVR (Digital Video Recorder - TI)
//   Owner        : Tushar Rabadiya
//   File         : PageOpenButton.cpp
//   Description  :
/////////////////////////////////////////////////////////////////////////////
#include "CnfgButton.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>

//*****************************************************************************
// CnfgButton
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//
//      [Pre-condition:]
//           NONE
//      [Constraints:]
//            NONE
//*****************************************************************************
CnfgButton::CnfgButton(CNFGBUTTON_SIZE_e buttnSize,
                       int centerX,
                       int centerY,
                       QString tLabel,
                       QWidget *parent,
                       int indexInPage,
                       bool isEnabled)
    : KeyBoard(parent), NavigationControl(indexInPage, isEnabled),
       label(tLabel), fontSize(NORMAL_FONT_SIZE)
{
    isDeletionStart = false;
    QString imgPath;
    buttonSize = buttnSize;
    textLabel = NULL;
    setObjectName("CNFG_BTN");

    if(m_isEnabled)
    {
        m_currentImageType = IMAGE_TYPE_NORMAL;
        textColor = NORMAL_FONT_COLOR;
    }
    else
    {
        m_currentImageType = IMAGE_TYPE_DISABLE;
        textColor = DISABLE_FONT_COLOR;
    }

    imgPath = CNFGBUTTON_FOLDER_PATH + buttonSizeFolder[buttonSize]
            + imgTypePath[m_currentImageType];
    image = QPixmap(imgPath);
    SCALE_IMAGE(image);

    this->setGeometry(QRect((centerX - (image.width ()/2)),
                            (centerY - (image.height ()/2)),
                            image.width (),
                            image.height ()));

    this->setEnabled (m_isEnabled);
    this->setMouseTracking (true);

    clickeffectTimer = new QTimer();
    connect (clickeffectTimer,
             SIGNAL(timeout()),
             this,
             SLOT(slotclickeffectTimerTimeout()));
    clickeffectTimer->setInterval (75);
    clickeffectTimer->setSingleShot (true);

    textLabel = new TextLabel( (image.width ()/2),
                               (image.height ()/2),
                               NORMAL_FONT_SIZE,tLabel,this,
                               textColor, NORMAL_FONT_FAMILY,
                               ALIGN_CENTRE_X_CENTER_Y,
                               0, 0, (image.width () - SCALE_WIDTH(30)));

    imageRect.setRect (0, 0, image.width (), image.height ());
    this->show ();
}

//*****************************************************************************
// ~CnfgButton
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//
//      [Pre-condition:]
//           NONE
//      [Constraints:]
//            NONE
//*****************************************************************************
CnfgButton::~CnfgButton()
{
    isDeletionStart = true;
    if(clickeffectTimer->isActive ())
    {
        clickeffectTimer->stop ();
    }
    disconnect (clickeffectTimer,
                SIGNAL(timeout()),
                this,
                SLOT(slotclickeffectTimerTimeout()));
    delete clickeffectTimer;
    delete textLabel;
    textLabel = NULL;
}

//*****************************************************************************
// changeImage
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//
//      [Pre-condition:]
//           NONE
//      [Constraints:]
//            NONE
//*****************************************************************************
void CnfgButton::changeImage (IMAGE_TYPE_e type, bool isImageBoarderHiglights)
{
    QString imgPath;
    m_currentImageType = type;
    switch(m_currentImageType)
    {
    case IMAGE_TYPE_NORMAL:
        imgPath = CNFGBUTTON_FOLDER_PATH + buttonSizeFolder[buttonSize] + imgTypePath[IMAGE_TYPE_NORMAL];
        textColor = NORMAL_FONT_COLOR;
        if(textLabel != NULL)
        {
            textLabel->SetBold(false);
        }
        break;

    case IMAGE_TYPE_MOUSE_HOVER:
        if(isImageBoarderHiglights)
        {
            imgPath = CNFGBUTTON_FOLDER_PATH + buttonSizeFolder[buttonSize] + imgTypePath[IMAGE_TYPE_MOUSE_HOVER];
        }
        else
        {
            imgPath = CNFGBUTTON_FOLDER_PATH + buttonSizeFolder[buttonSize] + imgTypePath[IMAGE_TYPE_NORMAL];
        }
        textColor = HIGHLITED_FONT_COLOR;
        if(textLabel != NULL)
        {
            textLabel->SetBold(true);
        }
        break;

    case IMAGE_TYPE_CLICKED:
        imgPath = CNFGBUTTON_FOLDER_PATH + buttonSizeFolder[buttonSize] + imgTypePath[IMAGE_TYPE_CLICKED];
        textColor = HIGHLITED_FONT_COLOR;
        if(textLabel != NULL)
        {
            textLabel->SetBold(true);
        }

        break;

    case IMAGE_TYPE_DISABLE:
        imgPath = CNFGBUTTON_FOLDER_PATH + buttonSizeFolder[buttonSize] + imgTypePath[IMAGE_TYPE_DISABLE];
        textColor = DISABLE_FONT_COLOR;
        if(textLabel != NULL)
        {
            textLabel->SetBold(false);
        }
        break;

    default:
        break;
    }

    image = QPixmap(imgPath);
    SCALE_IMAGE(image);
    if(textLabel != NULL)
    {
        textLabel->changeColor(textColor);
    }
    update();
}

IMAGE_TYPE_e CnfgButton::getCurrentImageType ()
{
    return m_currentImageType;
}

void CnfgButton :: changeColor(QString textcolor)
{
    textColor = textcolor ;
    textLabel->changeColor (textColor);
}

void CnfgButton:: changeText(QString str)
{
    label = str;
    textLabel->changeText(label, (image.width () - SCALE_WIDTH(30)));
    textLabel->repaint();
}

QString CnfgButton::getText()
{
    return label;
}

void CnfgButton::resetGeometry (quint32 startX,
                                quint32 startY)
{
    this->setGeometry(QRect((startX - (image.width ()/2)),
                            (startY - (image.height ()/2)),
                            image.width (),
                            image.height ()));

    imageRect.setRect(0, 0, image.width (), image.height ());
}

void CnfgButton::selectControl()
{
    if(m_currentImageType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
    }
}

void CnfgButton::deSelectControl()
{
    if(m_isEnabled == true)
    {
        changeImage(IMAGE_TYPE_NORMAL);
    }
    else
    {
        changeImage(IMAGE_TYPE_DISABLE);
    }
}

void CnfgButton::takeEnterKeyAction()
{
    if((!isDeletionStart) && (!clickeffectTimer->isActive()))
    {
        changeImage(IMAGE_TYPE_CLICKED);
        clickeffectTimer->start();
    }
}

void CnfgButton::forceActiveFocus()
{
    if(!isDeletionStart)
    {
        this->setFocus();
    }
}

void CnfgButton::setIsEnabled(bool isEnable)
{
    if(m_isEnabled != isEnable)
    {
        NavigationControl::setIsEnabled(isEnable);
        m_isEnabled = isEnable;
        this->setEnabled(m_isEnabled);
        if(isEnable == true)
        {
            changeImage(IMAGE_TYPE_NORMAL);
        }
        else
        {
            changeImage(IMAGE_TYPE_DISABLE);
        }
    }
}

//*****************************************************************************
// paintEvent
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//
//      [Pre-condition:]
//           NONE
//      [Constraints:]
//            NONE
//*****************************************************************************
void CnfgButton :: paintEvent (QPaintEvent *event)
{
    if(!isDeletionStart)
    {
        QWidget::paintEvent(event);
        QPainter painter(this);
        painter.drawPixmap(imageRect, image);
    }
    event->accept ();
}

//*****************************************************************************
// mousePressEvent
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//
//      [Pre-condition:]
//           NONE
//      [Constraints:]
//            NONE
//*****************************************************************************
void CnfgButton::mousePressEvent(QMouseEvent * event)
{
    if((event->button() == m_leftMouseButton)
            && (!isDeletionStart))
    {
        lastClickPoint = event->pos();
        m_mouseClicked = true;
        if(!this->hasFocus())
        {
            forceActiveFocus();
            emit sigUpdateCurrentElement(m_indexInPage);
        }
    }
    event->accept ();
}


//*****************************************************************************
// mouseReleaseEvent
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//
//      [Pre-condition:]
//           NONE
//      [Constraints:]
//            NONE
//*****************************************************************************
void CnfgButton::mouseReleaseEvent(QMouseEvent * event)
{
    if((m_mouseClicked == true)
            && (imageRect.contains(event->pos()))
            //            && (lastClickPoint == event->pos())
            && (event->button() == m_leftMouseButton)
            && (!isDeletionStart))
    {
        takeEnterKeyAction();
    }
    m_mouseClicked = false;
    event->accept ();
}

//*****************************************************************************
// mouseMoveEvent
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void CnfgButton::mouseMoveEvent(QMouseEvent *event)
{
    if(imageRect.contains(event->pos())
            && (m_isControlActivated)
            && (!isDeletionStart))
    {
        if(this->hasFocus())
        {
            selectControl();
        }
        else
        {
            forceActiveFocus();
            emit sigUpdateCurrentElement(m_indexInPage);
        }
    }
    event->accept ();
}

void CnfgButton::focusInEvent(QFocusEvent *)
{
    if(!isDeletionStart)
    {
        selectControl();
    }
}

void CnfgButton::focusOutEvent(QFocusEvent *)
{
    if(!isDeletionStart)
    {
        deSelectControl();
    }
}


void CnfgButton::enterKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        event->accept();
        takeEnterKeyAction();
    }
}

void CnfgButton::slotclickeffectTimerTimeout ()
{
    if(this->hasFocus())
    {
        selectControl();
    }
    emit sigButtonClick(m_indexInPage);
}
