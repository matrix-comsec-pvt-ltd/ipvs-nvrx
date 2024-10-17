#include "PickList.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QPainter>

PickList::PickList(int startX,
                   int startY,
                   int tileWidth,
                   int tileHeight,
                   int width,
                   int height,
                   QString label,
                   QMap<quint8, QString> optionList,
                   quint8 selectedKey,
                   QString heading,
                   QWidget *parent,
                   BGTILE_TYPE_e tileType,
                   int pixelAlign,
                   int indexInPage,
                   bool isEnabled,
                   bool isOuterBorderNeeded,
                   bool isPickListLoadOnResponse,
                   quint32 leftMarginFromCenter)
    : BgTile(startX, startY, tileWidth, tileHeight, tileType, parent),
      NavigationControl(indexInPage, isEnabled), m_pickListLoader(NULL), m_label(label), m_optionList(optionList),
      m_pixelAlign(pixelAlign), m_rectWidth(width), m_rectHeight(height), m_pickListHeading(heading),
      m_isOuterBorderNeeded(isOuterBorderNeeded), m_newSelectedKey(selectedKey),
      m_selectedKey(selectedKey), m_isPickListLoadOnResponse(isPickListLoadOnResponse),
      m_loaderDeleteOnCancelClick(false), m_currentValue(m_optionList.value(m_selectedKey)),
      m_leftMarginFromCenter(leftMarginFromCenter)
{
    changeImage((m_isEnabled) ? IMAGE_TYPE_NORMAL : IMAGE_TYPE_DISABLE);
    setGeometryForElements();

    this->setEnabled(m_isEnabled);
    this->setMouseTracking(true);
    this->show();
    m_inVisibleWidget = NULL;
}

PickList::~PickList()
{
    deletePicklistLoader ();
    m_optionList.clear();
    delete m_readOnlyElement;
    delete m_textLabel;
}

void PickList::setGeometryForElements()
{
    int translatedlabelWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width(QApplication::translate(QT_TRANSLATE_STR, m_label.toUtf8().constData()));
    int textLabelWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width(m_label);
    int textLabelHeight = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).height();
    int verticalOffSet = 0;

    switch(m_bgTileType)
    {
    case NO_LAYER:
        m_width = textLabelWidth + SCALE_WIDTH(10) + m_rectWidth + SCALE_WIDTH(10) + m_picklistIcon.width();
        this->setGeometry(m_startX, m_startY, m_width, m_height);
        m_pixelAlign = 0;
        break;

    case TOP_TABLE_LAYER:
        if(m_pixelAlign != -1)
        {
            m_pixelAlign += LEFT_MARGIN;
        }
        //fall through
    case TOP_LAYER:
        verticalOffSet = (TOP_MARGIN / 2);
        break;

    case BOTTOM_TABLE_LAYER:
        if(m_pixelAlign != -1)
        {
            m_pixelAlign += LEFT_MARGIN;
        }
        //fall through
    case BOTTOM_LAYER:
        verticalOffSet = -(TOP_MARGIN / 2);
        break;

    case MIDDLE_TABLE_LAYER:
        if(m_pixelAlign != -1)
        {
            m_pixelAlign += LEFT_MARGIN;
        }
        break;

    default:
        break;
    }

    if(m_pixelAlign == -1) // align center
    {
        m_readOnlyElement = new ReadOnlyElement((m_width / 2) - m_leftMarginFromCenter,
                                                (((m_height - m_rectHeight) / 2) + verticalOffSet),
                                                m_rectWidth,
                                                m_rectHeight,
                                                m_rectWidth,
                                                m_rectHeight,
                                                m_optionList.value(m_selectedKey),
                                                this,
                                                NO_LAYER,
                                                -1,
                                                SCALE_WIDTH(10),
                                                "", "",
                                                SUFFIX_FONT_COLOR,
                                                SCALE_FONT(10),
                                                m_isOuterBorderNeeded,
                                                (m_isEnabled) ? NORMAL_FONT_COLOR : DISABLE_FONT_COLOR);
        textLabelWidth = (translatedlabelWidth > ((getWidth()/2) - SCALE_WIDTH(20)))? ((getWidth()/2) - SCALE_WIDTH(20)) : translatedlabelWidth;
        m_textLabel = new TextLabel((m_readOnlyElement->x() - SCALE_WIDTH(10) - textLabelWidth),
                                    (((m_height - textLabelHeight) / 2) + verticalOffSet),
                                    NORMAL_FONT_SIZE,
                                    m_label,
                                    this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                    0, 0, textLabelWidth, 0, 0, Qt::AlignRight);

        m_imageRect.setRect((m_readOnlyElement->x() + m_readOnlyElement->width() + SCALE_WIDTH(10)),
                            (((m_height - m_picklistIcon.height()) / 2) + verticalOffSet),
                            m_picklistIcon.width(),
                            m_picklistIcon.height());
    }
    else
    {
        translatedlabelWidth = (translatedlabelWidth > ((m_pixelAlign + textLabelWidth))) ? ((m_pixelAlign + textLabelWidth) - SCALE_WIDTH(17)) : (translatedlabelWidth);
        m_textLabel = new TextLabel(abs((abs(translatedlabelWidth - (m_pixelAlign + textLabelWidth))) - SCALE_WIDTH(5)),
                                    (((m_height - textLabelHeight) / 2) + verticalOffSet),
                                    NORMAL_FONT_SIZE,
                                    m_label,
                                    this,
                                    NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                    0, 0, translatedlabelWidth, 0, 0);

        m_readOnlyElement = new ReadOnlyElement((m_pixelAlign + textLabelWidth + SCALE_WIDTH(10)),
                                                (((m_height - m_rectHeight) / 2) + verticalOffSet),
                                                m_rectWidth,
                                                m_rectHeight,
                                                m_rectWidth,
                                                m_rectHeight,
                                                m_optionList.value(m_selectedKey),
                                                this,
                                                NO_LAYER,
                                                -1,
                                                SCALE_WIDTH(10),
                                                "", "",
                                                SUFFIX_FONT_COLOR,
                                                m_isOuterBorderNeeded,
                                                (m_isEnabled) ? NORMAL_FONT_COLOR : DISABLE_FONT_COLOR);

        m_imageRect.setRect((m_pixelAlign + textLabelWidth + m_rectWidth + SCALE_WIDTH(20)),
                            (((m_height - m_picklistIcon.height()) / 2) + verticalOffSet),
                            m_picklistIcon.width(),
                            m_picklistIcon.height());
    }
}

void PickList::changeOptionList(QMap<quint8, QString> optionList, quint32 selectedKey, bool updateCurrentValueFlag)
{
    m_optionList.clear();
    m_optionList = optionList;
    if(updateCurrentValueFlag)
    {
        m_currentValue = m_optionList.value(selectedKey);
        changeValue(selectedKey);
    }
}

void PickList::deletePicklistLoader ()
{
    if(m_pickListLoader != NULL)
    {
        disconnect(m_pickListLoader,
                   SIGNAL(destroyed()),
                   this,
                   SLOT(slotPickListDestroyed()));
        disconnect(m_pickListLoader,
                   SIGNAL(sigValueChanged(quint8,QString,bool)),
                   this,
                   SLOT(slotValueChanged(quint8,QString,bool)));
        delete m_pickListLoader;
        m_pickListLoader = NULL;

        disconnect (m_inVisibleWidget,
                    SIGNAL(sigMouseClick()),
                    this,
                    SLOT(slotDeletePickList()));
        delete m_inVisibleWidget;
    }
}

void PickList::changeImage(IMAGE_TYPE_e imageType)
{
    m_imageType = imageType;
    m_imageSource = QString(PICKLIST_IMAGE_PATH) + imgTypePath[m_imageType];
    m_picklistIcon = QPixmap(m_imageSource);
    SCALE_IMAGE(m_picklistIcon);
}

void PickList::changeValue(int selectedKey)
{
    m_selectedKey = selectedKey;
    m_readOnlyElement->changeValue(m_optionList.value(m_selectedKey));
}

quint8 PickList::getCurrentValue() const
{
    return m_selectedKey;
}

QString PickList::getCurrentPickStr() const
{
    return m_currentValue;
}

void PickList::selectControl()
{
    if(m_imageType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
        update();
    }
}

void PickList::deSelectControl()
{
    changeImage((m_isEnabled) ? IMAGE_TYPE_NORMAL : IMAGE_TYPE_DISABLE);
    update();
}

void PickList::loadPickList()
{
    if(m_pickListLoader == NULL)
    {
        m_inVisibleWidget = new InvisibleWidgetCntrl(this->window());
        m_inVisibleWidget->setGeometry(QRect(0,
                                             0,
                                             this->window()->width(),
                                             this->window()->height()));
        m_inVisibleWidget->show();

        connect (m_inVisibleWidget,
                 SIGNAL(sigMouseClick()),
                 this,
                 SLOT(slotDeletePickList()));

        m_pickListLoader = new PickListLoader(m_optionList,
                                              m_selectedKey,
                                              m_pickListHeading,
                                              this->window(),
                                              true,
                                              (m_currentValue != ""));
        connect(m_pickListLoader,
                SIGNAL(destroyed()),
                this,
                SLOT(slotPickListDestroyed()));
        connect(m_pickListLoader,
                SIGNAL(sigValueChanged(quint8,QString,bool)),
                this,
                SLOT(slotValueChanged(quint8,QString,bool)));

        emit sigPicklistLoad(m_indexInPage);
    }
}

void PickList::loadPickListOnResponse(QMap<quint8, QString> optionList)
{
    m_selectedKey = optionList.key(m_currentValue);
    changeOptionList(optionList, m_selectedKey, false);
    loadPickList();
}

void PickList::loadPickListOnResponse(QMap<quint8, QString> optionList, qint32 selectKey)
{
    quint8 tmpSelectedKey = m_selectedKey;

    m_selectedKey = selectKey;
    changeOptionList(optionList, selectKey, false);
    loadPickList();
    m_selectedKey = tmpSelectedKey;
}

void PickList::takeEnterKeyAction()
{
    if(m_isPickListLoadOnResponse)
    {
        emit sigButtonClick(m_indexInPage);
    }
    else
    {
        loadPickList();
    }
}

void PickList::setValue(QString str)
{
    m_currentValue = str;
    m_readOnlyElement->changeValue(str);
}

void PickList::forceActiveFocus()
{
    this->setFocus();
}

void PickList::setIsEnabled(bool isEnable)
{
    if(m_isEnabled == isEnable)
    {
        return;
    }

    m_isEnabled = isEnable;
    this->setEnabled(m_isEnabled);

    if(isEnable == true)
    {
        changeImage(IMAGE_TYPE_NORMAL);
        m_readOnlyElement->changeTextValueColor(NORMAL_FONT_COLOR);
    }
    else
    {
        changeImage(IMAGE_TYPE_DISABLE);
        m_readOnlyElement->changeTextValueColor(DISABLE_FONT_COLOR);
    }
    update();
}

void PickList::paintEvent(QPaintEvent * event)
{
    QWidget::paintEvent(event);
    BgTile::paintEvent(event);
    QPainter painter(this);
    painter.drawPixmap(m_imageRect, m_picklistIcon);
}

void PickList::mouseMoveEvent(QMouseEvent * event)
{
    if((m_imageRect.contains(event->pos()))
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

void PickList::mousePressEvent(QMouseEvent * event)
{
    if((m_imageRect.contains(event->pos()))
            && (event->button() == m_leftMouseButton))
    {
        m_mouseClicked = true;
        if(!this->hasFocus())
        {
            forceActiveFocus();
            emit sigUpdateCurrentElement(m_indexInPage);
        }
    }
}

void PickList::mouseReleaseEvent(QMouseEvent * event)
{
    if((m_imageRect.contains(event->pos()))
            && (m_mouseClicked)
            && (event->button() == m_leftMouseButton))
    {
        takeEnterKeyAction();
    }
    m_mouseClicked = false;
}

void PickList::focusInEvent(QFocusEvent *)
{
    selectControl();
    if(m_rectWidth==0)
    sigShowHideToolTip(STYLE_SELECT_BUTTON,true);

}

void PickList::focusOutEvent(QFocusEvent *)
{
    deSelectControl();
    if(m_rectWidth==0)
    sigShowHideToolTip(STYLE_SELECT_BUTTON,false);

}

void PickList::enterKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        event->accept();
        takeEnterKeyAction();
    }
}

void PickList::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent (event);
    deletePicklistLoader ();
}

void PickList::showEvent(QShowEvent *event)
{
    QWidget::showEvent (event);
    if(m_pickListLoader != NULL)
    {
        m_inVisibleWidget->setVisible(true);
        m_pickListLoader->setVisible(true);
    }
}

void PickList::slotValueChanged(quint8 key, QString, bool isCancleClick)
{
    disconnect(m_pickListLoader,
               SIGNAL(sigValueChanged(quint8,QString,bool)),
               this,
               SLOT(slotValueChanged(quint8,QString,bool)));

    if(isCancleClick == true)
    {
        m_loaderDeleteOnCancelClick = true;
    }
    else
    {
        m_newSelectedKey = key;
    }
}

void PickList::slotPickListDestroyed()
{
    disconnect(m_pickListLoader,
               SIGNAL(destroyed()),
               this,
               SLOT(slotPickListDestroyed()));

    m_pickListLoader = NULL;

    disconnect (m_inVisibleWidget,
                SIGNAL(sigMouseClick()),
                this,
                SLOT(slotDeletePickList()));
    delete m_inVisibleWidget;
    forceActiveFocus();

    if(!m_loaderDeleteOnCancelClick)
    {
        m_selectedKey = m_newSelectedKey;
        m_currentValue = m_optionList.value(m_selectedKey);
        m_readOnlyElement->changeValue(m_optionList.value(m_selectedKey));
        emit sigValueChanged(m_selectedKey,
                             m_optionList.value(m_selectedKey),
                             m_indexInPage);
    }

    m_loaderDeleteOnCancelClick = false;
}

void PickList::slotDeletePickList()
{
    deletePicklistLoader ();
//    m_loaderDeleteOnCancelClick = true;
    forceActiveFocus();
}
