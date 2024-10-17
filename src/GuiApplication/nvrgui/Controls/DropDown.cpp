/////////////////////////////////////////////////////////////////////////////
//   MMM     MMM       AAA       TTTTTTTTTT  RRRRRR    IIIIIIII  XX   XX
//   MMMM   MMMM      AA AA          TT      RR   RR      II      XX XX
//   MM MM MM MM     AA   AA         TT      RR    RR     II       XXX
//   MM  MM   MM    AAAAAAAAA        TT      RRRRRRR      II       XXX
//   MM       MM   AA       AA       TT      RR  RR       II      XX XX
//   MM       MM  AA         AA      TT      RR   RR   IIIIIIII  XX   XX
//
//   Company      : Matrix Telecom Pvt. Ltd., Baroda, India.
//   Project      : Satatya Products - TI
//   Owner        : Vandit Maniar
//   File         : DropDown.cpp
//   Description  :
/////////////////////////////////////////////////////////////////////////////
#include "DropDown.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>

#define DROPDOWNBOX_IMAGE_PATH              IMAGE_PATH "Dropdown/"
#define DROPDOWNBOX_DOWN_ARROW_PATH         IMAGE_PATH "Dropdown/Down_Arrow/"
#define BUTTON_HEIGHT                       SCALE_HEIGHT(30)

static const QString buttonSizeFolder[MAX_DROPDOWNBOX_SIZE] =
{
    "Dropdown_78/",
    "Dropdown_90/",
    "Dropdown_114/",
    "Dropdown_200/",
    "Dropdown_225/",
    "Dropdown_320/",
    "Dropdown_405/",
    "Dropdown_755/"
};

DropDown::DropDown(quint32 startX, quint32 startY,
                   quint32 width, quint32 height,
                   quint16 controlIndex,
                   DROPDOWNBOX_SIZE_e butnSize,
                   QString labelStr,
                   QMap<quint8, QString> listStr,
                   QWidget* parent,
                   QString suffixStr,
                   bool isBoxStartInCentre,
                   quint16 leftMarginOfLabel,
                   BGTILE_TYPE_e bgType,
                   bool isNavigationEnable,
                   quint8 maxElemetOnList,
                   bool isBold,
                   bool isDropUpList,
                   quint16 leftMarginOfControl,
                   quint32 leftMarginFromCenter)
    : // intializing Base Class
      BgTile(startX,startY,width,height,bgType,parent),
      NavigationControl(controlIndex, isNavigationEnable),
      m_leftMarginOfLabel(leftMarginOfLabel), m_label(labelStr), m_suffix(suffixStr),
      m_valueList(listStr), m_currListNo(0),m_maxElemetOnList(maxElemetOnList)
{
    // intializing variable
    m_isCentre = isBoxStartInCentre;
    m_buttonSize =  butnSize;
    m_mouseClicked = false;
    m_dropDownList = NULL;
    m_inVisibleWidget = NULL;
    m_isDropUpList = isDropUpList;
    m_leftMarginOfControl = leftMarginOfControl;
    m_leftMarginFromCenter = leftMarginFromCenter;

    if(m_valueList.isEmpty ())
    {
        m_valueList.insert (0,"");
    }

    m_maxListNo = m_valueList.size ();
    m_currentImageType = (m_isEnabled) ? IMAGE_TYPE_NORMAL : IMAGE_TYPE_DISABLE;

    // assign ImagePath
    m_imgPath = DROPDOWNBOX_IMAGE_PATH + buttonSizeFolder[m_buttonSize] + imgTypePath[m_currentImageType];
    m_image = QPixmap(m_imgPath);
    SCALE_IMAGE(m_image);

    // assign DownImagePath
    m_imgDownScrollPath = DROPDOWNBOX_DOWN_ARROW_PATH + imgTypePath[m_currentImageType];
    m_imgDownScroll = QPixmap(m_imgDownScrollPath);
    SCALE_IMAGE(m_imgDownScroll);

    createDefaultComponent(isBold);

    this->setMouseTracking (true);
    this->setEnabled (m_isEnabled);
    this->show ();
}

DropDown::~DropDown ()
{
    unloadDropList();
    m_valueList.clear();
    if(m_label != "")
    {
        delete m_labelText;
    }
    if(m_suffix != "")
    {
        delete m_suffixText;
    }
    delete m_listText;
}

void DropDown::createDefaultComponent(bool isBold)
{
    quint16 labelWidth = 0, suffixWidth = 0, labelHeight = 0, strHeight = 0, translatedlabelWidth = 0;
    quint16 textHeight = 0;
    qint8 verticalOffset = 0;
    QFont labelFont, suffixFont;

    if(m_label != "")
    {
        labelFont = TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
        translatedlabelWidth = QFontMetrics(labelFont).width(QApplication::translate(QT_TRANSLATE_STR, m_label.toUtf8().constData()));
        labelWidth = QFontMetrics(labelFont).width(m_label);
        labelHeight = QFontMetrics(labelFont).height();
        labelWidth += SCALE_WIDTH(10);
    }

    if(m_suffix != "")
    {
        suffixFont = TextLabel::getFont (NORMAL_FONT_FAMILY, SCALE_FONT(SUFFIX_FONT_SIZE));
        suffixWidth = QFontMetrics(suffixFont).width (m_suffix);
        strHeight = QFontMetrics(suffixFont).height ();
        suffixWidth += SCALE_WIDTH(10);
    }

    switch(m_bgTileType)
    {
    case NO_LAYER:
        m_width = m_image.width() + labelWidth + suffixWidth;
        this->setGeometry(m_startX, m_startY, m_width, m_height);
        m_leftMarginOfLabel = 0;
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

    if(m_isCentre == true)
    {
        m_imageRect.setRect((this->width() / 2) - m_leftMarginFromCenter,
                            (((this->height() - m_image.height()) / 2) + verticalOffset),
                            m_image.width(),
                            m_image.height());

        if(m_label != "")
        {
            labelWidth = (translatedlabelWidth > ((this->width()/2) - SCALE_WIDTH(20)))? ((this->width()/2) - SCALE_WIDTH(20)) : translatedlabelWidth;
            m_labelText = new TextLabel((m_imageRect.x() - labelWidth - SCALE_WIDTH(10)),
                                        ((this->height() - labelHeight) / 2) + verticalOffset,
                                        NORMAL_FONT_SIZE,
                                        m_label,
                                        this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                        0, 0, labelWidth, 0, 0, Qt::AlignRight);
            m_labelText->SetBold(isBold);
        }
        if(m_suffix != "")
        {
            m_suffixText = new TextLabel((m_imageRect.x() + m_imageRect.width () + SCALE_WIDTH(10)),
                                         (((this->height() - strHeight ) / 2) + verticalOffset),
                                         SCALE_FONT(SUFFIX_FONT_SIZE),
                                         m_suffix,
                                         this,
                                         SUFFIX_FONT_COLOR);
            m_suffixText->SetBold(isBold);
        }
    }
    else
    {
        if(m_label != "")
        {
            translatedlabelWidth = (translatedlabelWidth > ((m_leftMarginOfLabel + labelWidth))) ? ((m_leftMarginOfLabel + labelWidth) - SCALE_WIDTH(16)) : (translatedlabelWidth);
            m_labelText = new TextLabel(abs((abs(translatedlabelWidth - (m_leftMarginOfLabel + labelWidth))) - SCALE_WIDTH(m_leftMarginOfControl)),
                                        (((this->height () - labelHeight) / 2) + verticalOffset),
                                        NORMAL_FONT_SIZE,
                                        m_label,
                                        this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                        0, 0, translatedlabelWidth);
            m_labelText->SetBold(isBold);
        }

        m_imageRect.setRect ((m_leftMarginOfLabel + labelWidth),
                             (((this->height() - m_image.height()) / 2) + verticalOffset),
                             m_image.width(),
                             m_image.height());

        if(m_suffix != "")
        {
            m_suffixText = new TextLabel((m_leftMarginOfLabel + labelWidth + m_imageRect.width() + SCALE_WIDTH(10)),
                                         (((this->height () - strHeight) / 2) + verticalOffset),
                                         SCALE_FONT(SUFFIX_FONT_SIZE),
                                         m_suffix,
                                         this,
                                         SUFFIX_FONT_COLOR);
            m_suffixText->SetBold(isBold);
        }
    }

    labelFont = TextLabel::getFont (NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
    textHeight = QFontMetrics(labelFont).height ();

    m_listText = new TextLabel ((m_imageRect.x () + SCALE_WIDTH(15)),
                                (m_imageRect.y () + ((m_imageRect.height () - textHeight)/2)),
                                NORMAL_FONT_SIZE,
                                m_valueList.value (0),
                                this,
                                ((m_isEnabled) ?  NORMAL_FONT_COLOR : DISABLE_FONT_COLOR),
                                NORMAL_FONT_FAMILY,
                                ALIGN_START_X_START_Y,
                                0,
                                false,
                                (m_imageRect.width () - m_imgDownScroll.width () - SCALE_WIDTH(33)));

    m_downScrollRect.setRect ((m_imageRect.x () + m_imageRect.width () - m_imgDownScroll.width () - SCALE_WIDTH(9)),
                              (m_imageRect.y () + (m_imageRect.height () - m_imgDownScroll.height ())/2),
                              m_imgDownScroll.width (),
                              m_imgDownScroll.height ());
}

void DropDown::changeImage (DROPDOWNBOX_IMAGE_TYPE_e index, IMAGE_TYPE_e imgType)
{
    switch(index)
    {
    case DROPDOWNBOX_MAIN:
        m_imgPath = DROPDOWNBOX_IMAGE_PATH + buttonSizeFolder[m_buttonSize] + imgTypePath[imgType];
        m_image = QPixmap(m_imgPath);
        SCALE_IMAGE(m_image);
        break;

    case DROPDOWNBOX_DOWN_SCROLL:
        m_imgDownScrollPath =  DROPDOWNBOX_DOWN_ARROW_PATH + imgTypePath[imgType];
        m_imgDownScroll = QPixmap(m_imgDownScrollPath);
        SCALE_IMAGE(m_image);
        break;

    default:
        break;
    }
    //    update();
}

void DropDown::setCurrValue(QString val)
{
    //    m_currListNo = m_valueList.find (val);
    m_currListNo = m_valueList.key(val);
    m_listText->changeText (m_valueList.value (m_currListNo), (m_imageRect.width () - m_imgDownScroll.width () - SCALE_WIDTH(33)));
    update ();
}

QString DropDown::getCurrValue() const
{
    return m_valueList.value(m_currListNo);
}

void DropDown::setIndexofCurrElement(quint8 index)
{
    m_currListNo = index;
    m_listText->changeText(m_valueList.value (m_currListNo), (m_imageRect.width () - m_imgDownScroll.width () - SCALE_WIDTH(33)));
    update();
}

quint8 DropDown::getIndexofCurrElement() const
{
    return m_currListNo;
}

void DropDown::changeTextAtIndex(quint8 index, QString newVal)
{
    if(index < m_maxListNo)
    {
        m_valueList.insert (index, newVal);
        if(index == m_currListNo)
        {
            m_listText->changeText (m_valueList.value (m_currListNo), (m_imageRect.width () - m_imgDownScroll.width () - SCALE_WIDTH(33)));
            m_listText->update ();
        }
    }
}

void DropDown::appendInList(QString str)
{
    m_valueList.insert (m_maxListNo,str);
    m_maxListNo = m_valueList.size ();
}

void DropDown::setNewList(QMap<quint8, QString> list, quint8 newSelectedIndex)
{
    if(m_valueList != list)
    {
        m_valueList = list;

        if(m_valueList.isEmpty ())
        {
            m_valueList.insert (0,"");
        }

        m_maxListNo = m_valueList.size ();
    }

    if(newSelectedIndex < m_maxListNo)
    {
        m_currListNo = newSelectedIndex;
    }
    else
    {
        m_currListNo = 0;
    }
    setCurrValue (m_valueList.value (m_currListNo));
}

void DropDown::setNewStringList (QStringList stringList, quint8 newSelectedIndex)
{
    QMap<quint8, QString> list;

    if(stringList.empty ())
    {
        list.insert (0,"");
    }
    else
    {
        for(quint16 index = 0; index < stringList.length (); index++)
        {
            list.insert (index,stringList.at (index));
        }
    }

    setNewList (list,newSelectedIndex);
}

void DropDown::selectControl()
{
    if(m_isEnabled == true)
    {
        changeImage(DROPDOWNBOX_MAIN, IMAGE_TYPE_MOUSE_HOVER);
        changeImage(DROPDOWNBOX_DOWN_SCROLL, IMAGE_TYPE_MOUSE_HOVER);
        update ();
    }
}

void DropDown::deSelectControl()
{
    if(m_isEnabled == true)
    {
        changeImage(DROPDOWNBOX_MAIN, IMAGE_TYPE_NORMAL);
        changeImage(DROPDOWNBOX_DOWN_SCROLL, IMAGE_TYPE_NORMAL);
        update ();
    }
}

void DropDown::forceActiveFocus()
{
    this->setFocus();
}

void DropDown::setIsEnabled(bool isEnable)
{
    if(m_isEnabled != isEnable)
    {
        m_isEnabled = isEnable;
        this->setEnabled (m_isEnabled);

        if(isEnable == true)
        {
            changeImage(DROPDOWNBOX_MAIN, IMAGE_TYPE_NORMAL);
            changeImage(DROPDOWNBOX_DOWN_SCROLL, IMAGE_TYPE_NORMAL);
            m_listText->changeColor (NORMAL_FONT_COLOR);
        }
        else
        {
            changeImage(DROPDOWNBOX_MAIN, IMAGE_TYPE_DISABLE);
            changeImage(DROPDOWNBOX_DOWN_SCROLL, IMAGE_TYPE_DISABLE);
            m_listText->changeColor (DISABLE_FONT_COLOR);
        }

        update ();
    }
}

void DropDown::loadDropList ()
{
    if((m_dropDownList == NULL) && (m_valueList.size () > 1))
    {
        m_inVisibleWidget = new InvisibleWidgetCntrl(parentWidget ());
        m_inVisibleWidget->setGeometry(QRect(0, 0, parentWidget ()->width(), parentWidget ()->height()));

        connect (m_inVisibleWidget,
                 SIGNAL(sigMouseClick()),
                 this,
                 SLOT(slotUnloadDropList()));

        m_inVisibleWidget->show();

        qint32 listStartY;
        if (m_isDropUpList)
        {
            /* Prepare list above the drop list button */
            listStartY = this->y() - (BUTTON_HEIGHT * ((m_maxListNo > m_maxElemetOnList) ? m_maxElemetOnList : m_maxListNo)) + SCALE_HEIGHT(1);
        }
        else
        {
            /* Prepare list below the drop list button */
            listStartY = this->y() + m_imageRect.y() + m_imageRect.height();
        }

        m_dropDownList = new DropDownList(this->x() + m_imageRect.x() + SCALE_WIDTH(5), listStartY,
                                          (m_imageRect.width() - m_imgDownScroll.width() - SCALE_WIDTH(5)),
                                          m_valueList, m_currListNo, parentWidget(), m_maxElemetOnList);

        connect(m_dropDownList,
                SIGNAL(destroyed()),
                this,
                SLOT(slotDropListDestroyed()));
        connect(m_dropDownList,
                SIGNAL(sigValueChanged(quint8,QString)),
                this,
                SLOT(slotValueChanged(quint8,QString)));

    }
}

void DropDown::unloadDropList ()
{
    if(m_dropDownList != NULL)
    {
        disconnect (m_inVisibleWidget,
                    SIGNAL(sigMouseClick()),
                    this,
                    SLOT(slotUnloadDropList()));
        delete m_inVisibleWidget;

        disconnect(m_dropDownList,
                   SIGNAL(destroyed()),
                   this,
                   SLOT(slotDropListDestroyed()));
        disconnect(m_dropDownList,
                   SIGNAL(sigValueChanged(quint8,QString)),
                   this,
                   SLOT(slotValueChanged(quint8,QString)));
        delete m_dropDownList;
        m_dropDownList = NULL;
    }
}

void DropDown :: paintEvent (QPaintEvent *event)
{
    QWidget::paintEvent(event);
    BgTile::paintEvent (event);

    QPainter painter(this);
    painter.drawPixmap (m_imageRect, m_image);
    painter.drawPixmap (m_downScrollRect, m_imgDownScroll);
}

void DropDown::mouseMoveEvent(QMouseEvent *event)
{
    if((m_imageRect.contains(event->pos()))
            && (m_isControlActivated))
    {
        if(this->hasFocus())
        {
            if(m_downScrollRect.contains(event->pos()))
            {
                changeImage(DROPDOWNBOX_MAIN, IMAGE_TYPE_MOUSE_HOVER);
                changeImage(DROPDOWNBOX_DOWN_SCROLL, IMAGE_TYPE_MOUSE_HOVER);
            }
            else
            {
                changeImage(DROPDOWNBOX_MAIN, IMAGE_TYPE_MOUSE_HOVER);
                changeImage(DROPDOWNBOX_DOWN_SCROLL, IMAGE_TYPE_NORMAL);
            }
        }
        else
        {
            forceActiveFocus();
            emit sigUpdateCurrentElement(m_indexInPage);
        }
    }
}

void DropDown :: mousePressEvent (QMouseEvent *event)
{
    if((m_imageRect.contains(event->pos()))
            && (event->button() == m_leftMouseButton))
    {
        if(!this->hasFocus())
        {
            forceActiveFocus();
            emit sigUpdateCurrentElement(m_indexInPage);
        }
        m_mouseClicked = true;
    }
}

void DropDown::mouseReleaseEvent(QMouseEvent *event)
{
    if((m_imageRect.contains(event->pos()))
            && (event->button() == m_leftMouseButton)
            && (m_mouseClicked))
    {
        if(m_dropDownList == NULL)
        {
            loadDropList ();
        }
        else
        {
            unloadDropList ();
        }
        m_mouseClicked = false;
    }
}

void DropDown::focusInEvent(QFocusEvent *)
{
    selectControl();
}

void DropDown::focusOutEvent(QFocusEvent *)
{
    deSelectControl();
}

void DropDown::navigationKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Up:
        event->accept();
        break;

    case Qt::Key_Down:
        event->accept();
        changeImage(DROPDOWNBOX_DOWN_SCROLL, IMAGE_TYPE_NORMAL);
        break;

    default:
//        event->accept();
        QWidget::keyPressEvent(event);
        break;
    }
}

void DropDown::enterKeyPressed(QKeyEvent *event)
{
    event->accept ();
    loadDropList ();
}

void DropDown::slotDropListDestroyed ()
{
    disconnect(m_dropDownList,
               SIGNAL(destroyed()),
               this,
               SLOT(slotDropListDestroyed()));
    m_dropDownList = NULL;

    disconnect (m_inVisibleWidget,
                SIGNAL(sigMouseClick()),
                this,
                SLOT(slotUnloadDropList()));
    delete m_inVisibleWidget;
    forceActiveFocus();
    emit sigDropDownListDestroyed();
}

void DropDown::slotValueChanged(quint8 key, QString str)
{
    disconnect(m_dropDownList,
               SIGNAL(sigValueChanged(quint8,QString)),
               this,
               SLOT(slotValueChanged(quint8,QString)));

    if(m_currListNo !=  key)
    {
        m_currListNo = key;
        m_listText->changeText (str, (m_imageRect.width () - m_imgDownScroll.width () - SCALE_WIDTH(33)));
        emit sigValueChanged(str,m_indexInPage);
    }
}

void DropDown::slotUnloadDropList ()
{
    unloadDropList ();
    forceActiveFocus ();
}
