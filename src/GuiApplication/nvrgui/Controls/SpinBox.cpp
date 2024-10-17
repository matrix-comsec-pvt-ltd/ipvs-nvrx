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

#include "SpinBox.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>

#define SPINBOX_IMAGE_PATH              IMAGE_PATH "SpinBox/"
#define SPINBOX_UP_ARROW_PATH           IMAGE_PATH "SpinBox/Up_Arrow/"
#define SPINBOX_DOWN_ARROW_PATH         IMAGE_PATH "SpinBox/Down_Arrow/"

static const QString buttonSizeFolder[MAX_SPINBOX_SIZE] =
{
    "Spinbox_78/",
    "Spinbox_90/",
};
//*****************************************************************************
// SpinBox
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
SpinBox :: SpinBox(quint32 startX,
                   quint32 startY,
                   quint32 width,
                   quint32 height,
                   quint16 controlIndex,
                   SPINBOX_SIZE_e butnSize,
                   QString labelStr,
                   QStringList listStr,
                   QWidget* parent,
                   QString suffixStr,
                   bool isBoxStartInCentre,
                   quint16 leftMarginOfLabel,
                   BGTILE_TYPE_e bgType,
                   bool isNavigationEnable)
    :BgTile(startX,
            startY,
            width,
            height,
            bgType,
            parent) ,NavigationControl(controlIndex, isNavigationEnable), isCentre(isBoxStartInCentre),
      leftMargin(leftMarginOfLabel), label(labelStr),  suffix(suffixStr), valueList(listStr),
       buttonSize(butnSize), currListNo(0), isMouseClick(false)
{
    if(valueList.isEmpty ())
    {
        valueList.append ("");
    }

    maxListNo = valueList.length ();

    if(m_isEnabled)
    {
        m_currentImageType = IMAGE_TYPE_NORMAL;
    }
    else
    {
        m_currentImageType = IMAGE_TYPE_DISABLE;
    }

    imgPath = SPINBOX_IMAGE_PATH + buttonSizeFolder[buttonSize] +
            imgTypePath[m_currentImageType];

    image = QPixmap(imgPath);
    SCALE_IMAGE(image);

    imgUpperScrollPath = SPINBOX_UP_ARROW_PATH + imgTypePath[m_currentImageType];
    imgUpperScroll = QPixmap(imgUpperScrollPath);
    SCALE_IMAGE(imgUpperScroll);

    imgDownScrollPath = SPINBOX_DOWN_ARROW_PATH + imgTypePath[m_currentImageType];
    imgDownScroll = QPixmap(imgDownScrollPath);
    SCALE_IMAGE(imgDownScroll);

    this->setMouseTracking (true);
    this->setEnabled (m_isEnabled);

    //        changeImage (SPINBOX_MAIN, IMAGE_TYPE_NORMAL);
    //        changeImage (SPINBOX_UPPER_SCROLL, IMAGE_TYPE_NORMAL);
    //        changeImage (SPINBOX_DOWN_SCROLL, IMAGE_TYPE_NORMAL);

    createDefaultComponent();

    if(!m_isEnabled)
    {
        listText->changeColor(DISABLE_FONT_COLOR);
    }

    this->show ();
}
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
SpinBox :: ~SpinBox()
{
    valueList.clear();
    if(label != "")
    {
        delete labelText;
    }
    if(suffix != "")
    {
        delete suffixText;
    }
    delete listText;
}

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
void SpinBox::createDefaultComponent()
{
    quint16 labelWidth = 0, suffixWidth = 0, labelHeight = 0, strHeight = 0;
    qint8 verticalOffset = 0;
    QFont labelFont, suffixFont;
    quint16 textHeight = 0;

    if(label != "")
    {
        labelFont = TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
        labelWidth = QFontMetrics(labelFont).width(label);
        labelHeight = QFontMetrics(labelFont).height();
        labelWidth += SCALE_WIDTH(10);
    }

    if(suffix != "")
    {
        suffixFont = TextLabel::getFont (NORMAL_FONT_FAMILY, SCALE_FONT(SUFFIX_FONT_SIZE));
        suffixWidth = QFontMetrics(suffixFont).width (suffix);
        strHeight = QFontMetrics(suffixFont).height ();
        suffixWidth += SCALE_WIDTH(10);
    }

    switch(m_bgTileType)
    {
    case NO_LAYER:
        m_width = image.width() + labelWidth + suffixWidth;
        this->setGeometry(m_startX, m_startY, m_width, m_height);
        leftMargin = 0;
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

    if(isCentre == true)
    {
        imageRect.setRect((this->width() / 2),
                          (((this->height() - image.height()) / 2) + verticalOffset),
                          image.width(),
                          image.height());

        if(label != "")
        {
            labelText = new TextLabel((imageRect.x() - labelWidth),
                                      ((this->height() - labelHeight) / 2) + verticalOffset,
                                      NORMAL_FONT_SIZE,
                                      label,
                                      this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                      0, 0, labelWidth, 0, 0, Qt::AlignRight);
        }
        if(suffix != "")
        {
            suffixText = new TextLabel((imageRect.x() + imageRect.width () + SCALE_WIDTH(10)),
                                       (((this->height() - strHeight ) / 2) + verticalOffset),
                                       SCALE_FONT(SUFFIX_FONT_SIZE),
                                       suffix,
                                       this,
                                       SUFFIX_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                       0, 0, suffixWidth);
        }
    }
    else
    {
        if(label != "")
        {
            labelText = new TextLabel(leftMargin,
                                      (((this->height () - labelHeight) / 2) + verticalOffset),
                                      NORMAL_FONT_SIZE,
                                      label,
                                      this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                      0, 0, labelWidth, 0, 0, Qt::AlignRight);
        }

        imageRect.setRect ((leftMargin + labelWidth),
                           (((this->height() - image.height()) / 2) + verticalOffset),
                           image.width(),
                           image.height());

        if(suffix != "")
        {
            suffixText = new TextLabel((leftMargin + labelWidth + imageRect.width() + SCALE_WIDTH(10)),
                                       (((this->height () - strHeight) / 2) + verticalOffset),
                                       SCALE_FONT(SUFFIX_FONT_SIZE),
                                       suffix,
                                       this,
                                       SUFFIX_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                       0, 0, suffixWidth);
        }
    }

    labelFont = TextLabel::getFont (NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
    textHeight = QFontMetrics(labelFont).height ();

    listText = new TextLabel (imageRect.x () + SCALE_WIDTH(15),
                              (imageRect.y () + ((imageRect.height () - textHeight)/2)),
                              NORMAL_FONT_SIZE, valueList.at (0), this);

    upperScrollRect.setRect ((imageRect.x () + imageRect.width () - imgUpperScroll.width ()- SCALE_WIDTH(9)),
                             imageRect.y () + SCALE_HEIGHT(4),
                             imgUpperScroll.width (),
                             imgUpperScroll.height ());

    downScrollRect.setRect ((imageRect.x () + imageRect.width () - imgDownScroll.width () - SCALE_WIDTH(9)),
                            (imageRect.y () + (imageRect.height ()/ 2)),
                            imgDownScroll.width (),
                            imgDownScroll.height ());
}

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
void SpinBox::changeImage (SPINBOX_IMAGE_TYPE_e index, IMAGE_TYPE_e imgType)
{
    switch(index)
    {
    case SPINBOX_MAIN:
        imgPath = SPINBOX_IMAGE_PATH + buttonSizeFolder[buttonSize] + imgTypePath[imgType];
        image = QPixmap(imgPath);
        SCALE_IMAGE(image);
        break;

    case SPINBOX_UPPER_SCROLL:
        imgUpperScrollPath = SPINBOX_UP_ARROW_PATH + imgTypePath[imgType];
        imgUpperScroll = QPixmap(imgUpperScrollPath);
        SCALE_IMAGE(imgUpperScroll);
        break;

    case SPINBOX_DOWN_SCROLL:
        imgDownScrollPath =  SPINBOX_DOWN_ARROW_PATH + imgTypePath[imgType];
        imgDownScroll = QPixmap(imgDownScrollPath);
        SCALE_IMAGE(imgDownScroll);
        break;

    default:
        break;
    }
    update();
}

void SpinBox:: updateWithPrevItem()
{
    quint8 previousListNo = currListNo;
    currListNo = ((currListNo - 1 + maxListNo) % maxListNo);
    changeImage (SPINBOX_UPPER_SCROLL, IMAGE_TYPE_MOUSE_HOVER);
    changeImage (SPINBOX_DOWN_SCROLL, IMAGE_TYPE_NORMAL);

    if(previousListNo != currListNo)
    {
        listText->changeText (valueList.at (currListNo));
        update ();
        emit sigValueChanged (valueList.at (currListNo), m_indexInPage);
    }
}

void SpinBox:: updateWithNextItem()
{
    quint8 previousListNo = currListNo;
    currListNo = ((currListNo + 1) % maxListNo);
    changeImage (SPINBOX_DOWN_SCROLL, IMAGE_TYPE_MOUSE_HOVER);
    changeImage (SPINBOX_UPPER_SCROLL, IMAGE_TYPE_NORMAL);

    if(previousListNo != currListNo)
    {
        listText->changeText (valueList.at (currListNo));
        update ();
        emit sigValueChanged (valueList.at (currListNo), m_indexInPage);
    }
}

void SpinBox::changeSuffixString(QString str)
{
    if(suffix != "")
    {
        suffixText->changeText (str);
    }
}

void SpinBox::setCurrValue(QString val)
{
    currListNo = valueList.indexOf (val);
    listText->changeText (valueList.at (currListNo));
    update ();
}

QString SpinBox::getCurrValue()
{
    return valueList.at(currListNo);
}

void SpinBox::setIndexofCurrElement(quint8 index)
{
    currListNo = index;
    listText->changeText(valueList.at(currListNo));
    update();
}

quint8 SpinBox::getIndexofCurrElement()
{
    return currListNo;
}

void SpinBox::changeTextAtIndex(quint8 index, QString newVal)
{
    if(index < maxListNo)
    {
        valueList.replace (index, newVal);
        if(index == currListNo)
        {
            listText->changeText (valueList.at (currListNo));
            listText->update ();
        }
    }
}

void SpinBox::appendInList(QString str)
{
    valueList.append(str);
    maxListNo = valueList.length ();
}

void SpinBox::setNewList(QStringList list, quint8 newSelectedIndex)
{
    if(valueList != list)
    {
        valueList = list;

        if(valueList.isEmpty ())
        {
            valueList.append ("");
        }

        maxListNo = valueList.length ();
    }

    if(newSelectedIndex < maxListNo)
    {
        currListNo = newSelectedIndex;
    }
    else
    {
        currListNo = 0;
    }
    setCurrValue (valueList.at (currListNo));
}

void SpinBox::selectControl()
{
    if(m_isEnabled == true)
    {
        changeImage(SPINBOX_MAIN, IMAGE_TYPE_MOUSE_HOVER);
    }
}

void SpinBox::deSelectControl()
{
    if(m_isEnabled == true)
    {
        changeImage(SPINBOX_MAIN, IMAGE_TYPE_NORMAL);
        changeImage (SPINBOX_UPPER_SCROLL, IMAGE_TYPE_NORMAL);
        changeImage (SPINBOX_DOWN_SCROLL, IMAGE_TYPE_NORMAL);
    }
    else
    {
        changeImage(SPINBOX_MAIN, IMAGE_TYPE_DISABLE);
        changeImage (SPINBOX_UPPER_SCROLL, IMAGE_TYPE_DISABLE);
        changeImage (SPINBOX_DOWN_SCROLL, IMAGE_TYPE_DISABLE);
    }
}

void SpinBox::forceActiveFocus()
{
    this->setFocus();
}

void SpinBox::setIsEnabled(bool isEnable)
{
    if(m_isEnabled != isEnable)
    {
        m_isEnabled = isEnable;
        this->setEnabled (m_isEnabled);

        if(isEnable == true)
        {
            changeImage(SPINBOX_MAIN, IMAGE_TYPE_NORMAL);
            changeImage(SPINBOX_UPPER_SCROLL, IMAGE_TYPE_NORMAL);
            changeImage(SPINBOX_DOWN_SCROLL, IMAGE_TYPE_NORMAL);
            listText->changeColor (NORMAL_FONT_COLOR);
            listText->update ();
        }
        else
        {
            changeImage(SPINBOX_MAIN, IMAGE_TYPE_DISABLE);
            changeImage(SPINBOX_UPPER_SCROLL, IMAGE_TYPE_DISABLE);
            changeImage(SPINBOX_DOWN_SCROLL, IMAGE_TYPE_DISABLE);
            listText->changeColor (DISABLE_FONT_COLOR);
            listText->update ();
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
void SpinBox :: paintEvent (QPaintEvent *event)
{
    QWidget::paintEvent(event);
    BgTile::paintEvent (event);

    QPainter painter(this);
    painter.drawPixmap (imageRect, image);
    painter.drawPixmap (upperScrollRect, imgUpperScroll);
    painter.drawPixmap (downScrollRect, imgDownScroll);
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
void SpinBox::mouseMoveEvent(QMouseEvent *event)
{
    if((imageRect.contains(event->pos()))
            && (m_isControlActivated))
    {
        if(this->hasFocus())
        {
            if(upperScrollRect.contains(event->pos()))
            {
                changeImage (SPINBOX_UPPER_SCROLL, IMAGE_TYPE_MOUSE_HOVER);
                changeImage (SPINBOX_DOWN_SCROLL, IMAGE_TYPE_NORMAL);
            }
            else if(downScrollRect.contains(event->pos()))
            {
                changeImage (SPINBOX_DOWN_SCROLL, IMAGE_TYPE_MOUSE_HOVER);
                changeImage (SPINBOX_UPPER_SCROLL, IMAGE_TYPE_NORMAL);
            }
            else
            {
                changeImage(SPINBOX_MAIN, IMAGE_TYPE_MOUSE_HOVER);
                changeImage (SPINBOX_UPPER_SCROLL, IMAGE_TYPE_NORMAL);
                changeImage (SPINBOX_DOWN_SCROLL, IMAGE_TYPE_NORMAL);
            }
        }
        else
        {
            forceActiveFocus();
            emit sigUpdateCurrentElement(m_indexInPage);
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
void SpinBox :: mousePressEvent (QMouseEvent *event)
{
    if((imageRect.contains(event->pos()))
            && (event->button() == m_leftMouseButton))
    {
        if(!this->hasFocus())
        {
            forceActiveFocus();
            emit sigUpdateCurrentElement(m_indexInPage);
        }
        if(upperScrollRect.contains (event->pos ()))
        {
            m_mouseClicked = true;
        }
        else if(downScrollRect.contains (event->pos ()))
        {
            m_mouseClicked = true;
        }
    }
}

void SpinBox::mouseReleaseEvent(QMouseEvent *event)
{
    if((imageRect.contains(event->pos()))
            && (event->button() == m_leftMouseButton)
            && (m_mouseClicked))
    {
        if(upperScrollRect.contains(event->pos ()))
        {
            updateWithPrevItem() ;
        }
        else if(downScrollRect.contains(event->pos ()))
        {
            updateWithNextItem();
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
void SpinBox ::wheelEvent (QWheelEvent *event)
{
    if(imageRect.contains(event->pos()))
    {
        quint8 previousListNo = currListNo;
        if(event->delta() < 0)
        {
            currListNo = ((currListNo + 1) % maxListNo);
        }
        else if(event->delta() > 0)
        {
            currListNo = ((currListNo - 1 + maxListNo) % maxListNo);
        }

        if(previousListNo != currListNo)
        {
            listText->changeText (valueList.at (currListNo));
            update ();
            emit sigValueChanged (valueList.at (currListNo), m_indexInPage);
        }
    }
}

void SpinBox::focusInEvent(QFocusEvent *)
{
    selectControl();
}

void SpinBox::focusOutEvent(QFocusEvent *)
{
    deSelectControl();
}

void SpinBox::navigationKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        switch(event->key())
        {
        case Qt::Key_Up:
            event->accept();
            updateWithPrevItem();
            changeImage(SPINBOX_UPPER_SCROLL, IMAGE_TYPE_NORMAL);
            break;

        case Qt::Key_Down:
            event->accept();
            updateWithNextItem();
            changeImage(SPINBOX_DOWN_SCROLL, IMAGE_TYPE_NORMAL);
            break;

        default:
            event->accept();
            break;
        }
    }
}
