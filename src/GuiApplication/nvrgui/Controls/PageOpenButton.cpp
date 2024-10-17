//////////////////////////////////////////////////////////////////////////
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
//   File         : DeviceClient.cpp
//   Description  :
/////////////////////////////////////////////////////////////////////////////
#include "PageOpenButton.h"
#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>

#define PAGEOPENBUTTON_FOLDER_PATH      IMAGE_PATH "PageOpenButtons/"

const QString buttonSizeFolder[MAX_PAGEOPENBUTTON_SIZE] =
{
    "Small/",
    "Medium_Back/",
    "Medium_Next/",
    "Large/",
    "ExtraLarge/",
    "UltraLarge/",
    "Large_Back/",
    "UltraLarge_ImageOverlaped/"
};
//*****************************************************************************
// PageOpenButton
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
PageOpenButton::PageOpenButton(quint32 startX,
                               quint32 startY,
                               quint32 width,
                               quint32 height,
                               quint16 controlIndex,
                               PAGEOPENBUTTON_SIZE_e buttnSize,
                               QString btnNameStr,
                               QWidget *parent,
                               QString labelStr,
                               QString suffixStr,
                               bool isBoxStartInCentre,
                               quint16 leftMarginOfLabel,
                               BGTILE_TYPE_e bgType,
                               bool isNavigationEnable,
                               TEXTLABEL_ALIGNMENT_e textAlignment,
                               quint32 leftMarginFromCenter)
    :BgTile(startX,
            startY,
            width,
            height,
            bgType,parent), NavigationControl(controlIndex, isNavigationEnable),
      btnName(btnNameStr),  label(labelStr), suffix(suffixStr), m_textAlign(textAlignment),
      isInCentre(isBoxStartInCentre), leftMargin(leftMarginOfLabel), buttonSize(buttnSize), m_leftMarginFromCenter(leftMarginFromCenter)
{
    QString tImgPath;

    if(m_isEnabled)
    {
        m_currentImageType =  IMAGE_TYPE_NORMAL;
        textColor = NORMAL_FONT_COLOR;

    }
    else
    {
        m_currentImageType =  IMAGE_TYPE_DISABLE;
        textColor = SUFFIX_FONT_COLOR;
    }

    this->setEnabled (m_isEnabled);
    this->setMouseTracking (true);

    tImgPath = PAGEOPENBUTTON_FOLDER_PATH
            + buttonSizeFolder[buttonSize] + imgTypePath[m_currentImageType];

    image = QPixmap(tImgPath);
    SCALE_IMAGE(image);

    createDefaultComponent ();
    clickeffctTimer = new QTimer(this);
    clickeffctTimer->setInterval (75);
    clickeffctTimer->setSingleShot (true);
    connect (clickeffctTimer,
             SIGNAL(timeout()),
             this,
             SLOT(slotclickeffctTimerTimeout()));

    this->show ();
}

//*****************************************************************************
// ~PageOpenButton
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
PageOpenButton::~PageOpenButton()
{
    //    delete textLabel;
    delete clickeffctTimer;
    delete btnText;
    if(label != "")
    {
        delete labelText;
    }
    if(suffix != "")
    {
        delete suffixText;
    }
}

void PageOpenButton::createDefaultComponent()
{
    quint16 labelWidth = 0,suffixWidth = 0, labelHeight = 0, strHeight = 0, translatedlabelWidth=0;
    QFont labelFont, suffixFont;
    quint16 width = 0;
    qint8 verticalOffset = 0;    

    if(label != "")
    {
        labelFont = TextLabel::getFont (NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
        translatedlabelWidth = QFontMetrics(labelFont).width (QApplication::translate(QT_TRANSLATE_STR, label.toUtf8().constData()));
        labelWidth = QFontMetrics(labelFont).width (label);
        labelHeight = QFontMetrics(labelFont).height ();
        width += SCALE_WIDTH(10);
    }

    if(suffix != "")
    {
        suffixFont = TextLabel::getFont (NORMAL_FONT_FAMILY, SCALE_FONT(SUFFIX_FONT_SIZE));
        suffixWidth = QFontMetrics(suffixFont).width (suffix);
        strHeight = QFontMetrics(suffixFont).height ();
        width += SCALE_WIDTH(10);
    }

    width += image.width () + labelWidth + suffixWidth;

    switch(m_bgTileType)
    {
    case NO_LAYER:
        m_width = width;
        this->setGeometry(m_startX, m_startY, m_width, m_height);
        break;

    case TOP_TABLE_LAYER:
        verticalOffset = (TOP_MARGIN / 2);
        break;

    case BOTTOM_TABLE_LAYER:
        verticalOffset = -(TOP_MARGIN / 2);
        break;

    default:
        break;
    }

    if(isInCentre == true)
    {
        if(label != "")
        {
            labelWidth = (translatedlabelWidth > ((getWidth()/2) - SCALE_WIDTH(20)))? ((getWidth()/2) - SCALE_WIDTH(20)) : translatedlabelWidth;
            labelText = new TextLabel(((this->width ()/2) - SCALE_WIDTH(10) - labelWidth) - m_leftMarginFromCenter,
                                      (this->height () - labelHeight)/2 + verticalOffset,
                                      NORMAL_FONT_SIZE, label,
                                      this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                      0, 0, labelWidth, 0, 0, Qt::AlignRight);
        }
        if(suffix !="")
        {
            suffixText = new TextLabel((this->width ()/2) + image.width () + SCALE_WIDTH(10) - m_leftMarginFromCenter,
                                       (this->height () - strHeight)/2 + verticalOffset,
                                       SCALE_FONT(SUFFIX_FONT_SIZE), suffix,
                                       this,
                                       SUFFIX_FONT_COLOR);
        }
        imageRect.setRect (this->width ()/2 - m_leftMarginFromCenter,
                           (this->height () - image.height ())/2 + verticalOffset,
                           image.width (),
                           image.height ());

    }
    else
    {
        if(label != "")
        {
            translatedlabelWidth = (translatedlabelWidth > ((leftMargin + labelWidth) - SCALE_WIDTH(17))) ? ((leftMargin + labelWidth) - SCALE_WIDTH(17)) : (translatedlabelWidth);
            labelText = new TextLabel(abs((abs(translatedlabelWidth - (leftMargin + labelWidth))) - SCALE_WIDTH(5)),
                                      (this->height () - labelHeight)/2 + verticalOffset,
                                      NORMAL_FONT_SIZE, label,
                                      this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                      0, 0, translatedlabelWidth);
            labelWidth += SCALE_WIDTH(10);
        }
        imageRect.setRect (leftMargin+labelWidth,
                           (this->height () - image.height ())/2 + verticalOffset,
                           image.width (),
                           image.height ());

        if(suffix != "")
        {
            suffixText = new TextLabel(leftMargin + labelWidth+  image.width () + SCALE_WIDTH(10),
                                       (this->height () - strHeight)/2 + verticalOffset,
                                       SCALE_FONT(SUFFIX_FONT_SIZE), suffix,
                                       this,
                                       SUFFIX_FONT_COLOR);
        }
    }

    labelFont = TextLabel::getFont (NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);

    if(buttonSize == PAGEOPENBUTTON_MEDIAM_BACK)
    {
        labelWidth = imageRect.x () + SCALE_WIDTH(30);
    }
    else if(buttonSize == PAGEOPENBUTTON_LARGE_BACK)
    {
        switch(m_textAlign)
        {
        case ALIGN_START_X_CENTRE_Y:
            labelWidth = imageRect.x () + SCALE_WIDTH(25);
            break;

        case ALIGN_END_X_CENTRE_Y:
            labelWidth = imageRect.x () + imageRect.width () - SCALE_WIDTH(25);
            break;

        case ALIGN_CENTRE_X_CENTER_Y:
            labelWidth = imageRect.x () + (imageRect.width ())/2;
            break;
        default:
            break;
        }
    }
    else
    {
        switch(m_textAlign)
        {
        case ALIGN_START_X_CENTRE_Y:
            labelWidth = imageRect.x () + SCALE_WIDTH(8);
            break;

        case ALIGN_END_X_CENTRE_Y:
            labelWidth = imageRect.x () + imageRect.width () - SCALE_WIDTH(25);
            break;

        case ALIGN_CENTRE_X_CENTER_Y:
            labelWidth = imageRect.x () + (imageRect.width ())/2;
            break;
        default:
            break;
        }
    }
    width =0;
    if(buttonSize == PAGEOPENBUTTON_ULTRALARGE_IMGOVERLAPED)
    {
        width = (imageRect.width() - SCALE_WIDTH(80));
    }
    else
    {
        width = (imageRect.width() - SCALE_WIDTH(25));
    }

    btnText = new TextLabel(labelWidth,
                            (imageRect.y () + (imageRect.height () / 2)),
                            NORMAL_FONT_SIZE,btnName,this,
                            textColor, NORMAL_FONT_FAMILY,
                            m_textAlign, 0, 0, width);
}

//*****************************************************************************
// ChangeImage
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
void PageOpenButton::changeImage (IMAGE_TYPE_e type)
{
    m_currentImageType = type;
    QString tImgPath = PAGEOPENBUTTON_FOLDER_PATH
            + buttonSizeFolder[buttonSize] + imgTypePath[m_currentImageType];

    switch(m_currentImageType)
    {
    case IMAGE_TYPE_NORMAL:
        textColor = NORMAL_FONT_COLOR;
        break;

    case IMAGE_TYPE_MOUSE_HOVER:
        textColor = HIGHLITED_FONT_COLOR;
        break;

    case IMAGE_TYPE_CLICKED:
        textColor = HIGHLITED_FONT_COLOR;
        break;

    case IMAGE_TYPE_DISABLE:
        textColor = SUFFIX_FONT_COLOR;
        break;

    default:
        break;
    }

    image = QPixmap(tImgPath);
    SCALE_IMAGE(image);
    btnText->changeColor (textColor);

    update ();
}

//*****************************************************************************
// resetGeometry
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
void PageOpenButton::resetGeometry (quint32 startX,
                                    quint32 startY)
{
    BgTile::resetGeometry (startX, startY, m_width, m_height);
}

void PageOpenButton::selectControl()
{
    if(m_currentImageType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
    }
}

void PageOpenButton::deSelectControl()
{
    if(m_isEnabled)
    {
        changeImage(IMAGE_TYPE_NORMAL);
    }
    else
    {
        changeImage(IMAGE_TYPE_DISABLE);
    }
}

void PageOpenButton::forceActiveFocus()
{
    this->setFocus();
}

void PageOpenButton::setIsEnabled(bool isEnable)
{
    if(m_isEnabled != isEnable)
    {
        m_isEnabled = isEnable;
        this->setEnabled (m_isEnabled);

        if(isEnable == true)
        {
            changeImage(IMAGE_TYPE_NORMAL);
        }
        else
        {
            changeImage(IMAGE_TYPE_DISABLE);
        }
        update();
    }
}

void PageOpenButton::takeEnterKeyAction()
{
    if(!clickeffctTimer->isActive())
    {
        changeImage(IMAGE_TYPE_CLICKED);
        clickeffctTimer->start();
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
void PageOpenButton :: paintEvent (QPaintEvent *event)
{
    QWidget::paintEvent(event);
    BgTile::paintEvent (event);
    QPainter painter(this);
    painter.drawPixmap ( imageRect, image);
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
void PageOpenButton::mousePressEvent(QMouseEvent * event)
{
    if((imageRect.contains(event->pos()))
            && (event->button() == m_leftMouseButton))
    {
        m_mouseClicked = true;
        lastClickPoint = event->pos();
        if(!this->hasFocus())
        {
            forceActiveFocus();
            emit sigUpdateCurrentElement(m_indexInPage);
        }
    }
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
void PageOpenButton::mouseReleaseEvent(QMouseEvent * event)
{
    if((m_mouseClicked == true)
            && (lastClickPoint == event->pos())
            && (event->button() == m_leftMouseButton))
    {
        takeEnterKeyAction();
    }
    m_mouseClicked = false;
}

void PageOpenButton::mouseMoveEvent (QMouseEvent *event)
{
    if((imageRect.contains(event->pos()))
            && (m_isControlActivated))
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
}

void PageOpenButton::focusInEvent(QFocusEvent *)
{
    selectControl();
}

void PageOpenButton::focusOutEvent(QFocusEvent *)
{
    deSelectControl();
}

void PageOpenButton::enterKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        event->accept();
        takeEnterKeyAction();
    }
}

void PageOpenButton::slotclickeffctTimerTimeout()
{
    if(this->hasFocus())
    {
        selectControl();
    }
    emit sigButtonClick(m_indexInPage);
}
