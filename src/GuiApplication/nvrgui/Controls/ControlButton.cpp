#include "ControlButton.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>

ControlButton::ControlButton(CONTROL_BUTTON_TYPE_e type,
                             int startX,
                             int startY,
                             int width,
                             int height,
                             QWidget *parent,
                             BGTILE_TYPE_e tileType,
                             int pixelAlign,
                             QString label,
                             bool isEnabled,
                             int indexInPage,
                             bool isLabel,
                             quint32 leftMarginFromCenter)
    : BgTile(startX, startY, width, height, tileType, parent),
      NavigationControl(indexInPage, isEnabled),
      m_label(label), m_leftMarginFromCenter(leftMarginFromCenter)
{
    m_isLabel = isLabel;
    m_controlButtonType = type;
    m_horizontalAlignment = 0;
    m_textLabel = NULL;
    m_pixelAlign = pixelAlign;

    changeImage((m_isEnabled ? IMAGE_TYPE_NORMAL : IMAGE_TYPE_DISABLE));
    setGeometryForElements();

    m_clickEffectTimer = new QTimer(this);
    connect(m_clickEffectTimer,
            SIGNAL(timeout()),
            this,
            SLOT(slotClickEffectTimerTimeout()));
    m_clickEffectTimer->setInterval(75);
    m_clickEffectTimer->setSingleShot(true);

    this->setEnabled(m_isEnabled);
    this->setMouseTracking(true);
}

ControlButton::~ControlButton()
{
    if(m_textLabel != NULL)
    {
        delete m_textLabel;
    }

    if(m_clickEffectTimer->isActive())
    {
        m_clickEffectTimer->stop();
    }
    disconnect(m_clickEffectTimer,
               SIGNAL(timeout()),
               this,
               SLOT(slotClickEffectTimerTimeout()));
    delete m_clickEffectTimer;
}

void ControlButton::setGeometryForElements()
{
    int textLabelWidth = 0;
    qint16 translatedlabelWidth = 0;
    int textLabelHeight = 0;
    int verticalOffSet = 0;

    if(m_label != "")
    {
        translatedlabelWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width(QApplication::translate(QT_TRANSLATE_STR, m_label.toUtf8().constData()));
        textLabelWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width(m_label);
        textLabelHeight = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).height();
        textLabelWidth += SCALE_WIDTH(10);
    }

    switch(m_bgTileType)
    {
    case NO_LAYER:
        m_width = m_iconImage.width() + textLabelWidth;
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

    if(m_pixelAlign == -1) // align image into middle of tile
    {
        m_imageRect.setRect((m_width / 2) - m_leftMarginFromCenter,
                            ((m_height - m_iconImage.height()) / 2) + verticalOffSet,
                            m_iconImage.width(),
                            m_iconImage.height());

        if(m_label != "")
        {
            if(m_isLabel)
            {
                textLabelWidth = (translatedlabelWidth > ((getWidth()/2) - SCALE_WIDTH(20)))? ((getWidth()/2) - SCALE_WIDTH(20)) : translatedlabelWidth;
                m_textLabel = new TextLabel((m_imageRect.x() - textLabelWidth - SCALE_WIDTH(10)),
                                            (((m_height - textLabelHeight) / 2) + verticalOffSet),
                                            NORMAL_FONT_SIZE,
                                            m_label,
                                            this,
                                            m_fontColor, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                            0, 0, textLabelWidth, 0, 0, Qt::AlignRight);
            }
            else
            {
                m_textLabel = new TextLabel((m_imageRect.x() + m_imageRect.width ()+ SCALE_WIDTH(10)),
                                            (((m_height - textLabelHeight) / 2) + verticalOffSet),
                                            NORMAL_FONT_SIZE,
                                            m_label,
                                            this,
                                            m_fontColor);
            }
        }
    }
    else
    {
        if(m_label != "")
        {
            if(m_isLabel)
            {
                m_imageRect.setRect((m_pixelAlign + textLabelWidth),
                                    (((m_height - m_iconImage.height()) / 2) + verticalOffSet),
                                    m_iconImage.width(),
                                    m_iconImage.height());
                translatedlabelWidth = (translatedlabelWidth > ((m_pixelAlign + textLabelWidth) - SCALE_WIDTH(8))) ? ((m_pixelAlign + textLabelWidth) - SCALE_WIDTH(17)) : (translatedlabelWidth);
                m_textLabel = new TextLabel(abs((abs(translatedlabelWidth - (m_pixelAlign + textLabelWidth))) - SCALE_WIDTH(5)),
                                            (((m_height - textLabelHeight) / 2) + verticalOffSet),
                                            NORMAL_FONT_SIZE,
                                            m_label,
                                            this,
                                            m_fontColor, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                            0, 0, translatedlabelWidth, 0, 0);
            }
            else
            {
                m_imageRect.setRect(m_pixelAlign,
                                    (((m_height - m_iconImage.height()) / 2) + verticalOffSet),
                                    m_iconImage.width(),
                                    m_iconImage.height());

                m_textLabel = new TextLabel(m_pixelAlign + m_imageRect.width () + SCALE_WIDTH(10),
                                            (((m_height - textLabelHeight) / 2) + verticalOffSet),
                                            NORMAL_FONT_SIZE,
                                            m_label,
                                            this,
                                            m_fontColor);
            }
        }
        else
        {
            m_imageRect.setRect(m_pixelAlign,
                                (((m_height - m_iconImage.height()) / 2) + verticalOffSet),
                                m_iconImage.width(),
                                m_iconImage.height());
        }
    }
}

void ControlButton::changeImage(IMAGE_TYPE_e type)
{
    m_currentImageType = type;
    m_imageSource = QString(CONTROL_BUTTON_IMG_PATH) + controlImageFolderPath[m_controlButtonType] + imgTypePath[m_currentImageType];
    m_iconImage = QPixmap(m_imageSource);
    SCALE_IMAGE(m_iconImage);

    m_fontColor = (m_isEnabled ? NORMAL_FONT_COLOR : DISABLE_FONT_COLOR);
    if((m_label != "") && (m_textLabel != NULL))
    {
        m_textLabel->changeColor(m_fontColor);
    }
}

void ControlButton::changeImageType (CONTROL_BUTTON_TYPE_e type)
{
    if(type != m_controlButtonType )
    {
        m_controlButtonType = type;
        changeImage(IMAGE_TYPE_NORMAL);
    }
}

void ControlButton::selectControl()
{
    if(m_currentImageType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
        update();
    }
}

void ControlButton::deSelectControl()
{
    if(m_isEnabled)
    {
        changeImage(IMAGE_TYPE_NORMAL);
    }
    else
    {
        changeImage(IMAGE_TYPE_DISABLE);
    }
    update();
}

void ControlButton::forceActiveFocus()
{
    this->setFocus();
}

void ControlButton::takeEnterKeyAction()
{
    if(!m_clickEffectTimer->isActive())
    {
        changeImage(IMAGE_TYPE_CLICKED);
        update();
        m_clickEffectTimer->start();
    }
}

void ControlButton::setIsEnabled(bool isEnable)
{
    if(m_isEnabled != isEnable)
    {
        NavigationControl::setIsEnabled(isEnable);
        m_isEnabled = isEnable;
        this->setEnabled(m_isEnabled);
        if(isEnable == true)
        {
            setVisible(true);
            changeImage(IMAGE_TYPE_NORMAL);
        }
        else
        {
            changeImage(IMAGE_TYPE_DISABLE);
        }
        update();
    }
}

void ControlButton::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    BgTile::paintEvent (event);
    QPainter painter(this);
    painter.drawPixmap (m_imageRect, m_iconImage);
}

void ControlButton::mousePressEvent(QMouseEvent * event)
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

void ControlButton::mouseReleaseEvent(QMouseEvent * event)
{
    if((m_imageRect.contains(event->pos()))
            && (m_mouseClicked)
            && (event->button() == m_leftMouseButton))
    {
        takeEnterKeyAction();
    }
    m_mouseClicked = false;
}

void ControlButton::mouseMoveEvent(QMouseEvent *event)
{
    if((m_imageRect.contains(event->pos()))
            && m_isControlActivated)
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

void ControlButton::focusInEvent(QFocusEvent *)
{
    selectControl();
}

void ControlButton::focusOutEvent(QFocusEvent *)
{
    deSelectControl();
}

void ControlButton::enterKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        event->accept();
        takeEnterKeyAction();
    }
}

void ControlButton::slotClickEffectTimerTimeout()
{
    if(this->hasFocus())
    {
        selectControl();
    }
    emit sigButtonClick(m_indexInPage);
}
