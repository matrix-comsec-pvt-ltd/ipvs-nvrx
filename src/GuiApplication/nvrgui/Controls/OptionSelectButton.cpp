#include "OptionSelectButton.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QTranslator>

OptionSelectButton::OptionSelectButton(int startX,
                                       int startY,
                                       int width,
                                       int height,
                                       OPTION_SELECTION_BUTTON_TYPE_e buttonType,
                                       QWidget *parent,
                                       BGTILE_TYPE_e tileType,
                                       QString label,
                                       QString suffix,
                                       int pixelAlign,
                                       int indexInPage,
                                       bool isEnabled,
                                       int suffixPixelSize,
                                       QString suffixFontColor, bool multipleElemtInRow,
                                       quint32 leftMarginFromCenter)
    : BgTile(startX, startY, width, height, tileType, parent),
      NavigationControl(indexInPage, isEnabled), m_label(label), m_suffix(suffix),
      m_currentState(OFF_STATE), m_textLabelSize(NORMAL_FONT_SIZE),
      m_textSuffixSize(((suffixPixelSize == DEFAULT_PIXEL_SIZE) ? SCALE_FONT(SUFFIX_FONT_SIZE) : suffixPixelSize)),
      m_buttonType(buttonType),
      m_textLabel(NULL), m_textSuffix(NULL), m_textFontColor(NORMAL_FONT_COLOR),
      m_suffixFontColor(suffixFontColor), m_pixelAlign(pixelAlign), m_multiple(multipleElemtInRow), m_leftMarginfromCenter(leftMarginFromCenter)

{
    changeImage((m_isEnabled) ? IMAGE_TYPE_NORMAL : IMAGE_TYPE_DISABLE);
    setGeometryForElements();
    this->setEnabled(m_isEnabled);
    this->setMouseTracking(true);
    this->show();
}

OptionSelectButton::OptionSelectButton(int startX,
                                       int startY,
                                       int width,
                                       int height,
                                       OPTION_SELECTION_BUTTON_TYPE_e buttonType,
                                       QString text,
                                       QWidget *parent,
                                       BGTILE_TYPE_e tileType,
                                       int pixelAlign,
                                       MX_OPTION_TEXT_TYPE_e textType,
                                       int textPixelSize,
                                       int indexInPage,
                                       bool isEnabled,
                                       QString fontColor, bool multipleElemtInRow, 
									   quint32 leftMarginFromCenter)
    : BgTile(startX, startY, width, height, tileType, parent), NavigationControl(indexInPage, isEnabled)
{
    if(textType == MX_OPTION_TEXT_TYPE_LABEL)
    {
        m_label = text;
        m_textLabelSize = ((textPixelSize == DEFAULT_PIXEL_SIZE) ? NORMAL_FONT_SIZE : textPixelSize);
        m_suffix = "";
        m_textFontColor = fontColor;
    }
    else
    {
        m_suffix = text;
        m_textSuffixSize = ((textPixelSize == DEFAULT_PIXEL_SIZE) ? SCALE_FONT(SUFFIX_FONT_SIZE) : textPixelSize);
        m_label = "";
        m_suffixFontColor = fontColor;
    }

    m_buttonType = buttonType;
    m_currentState = OFF_STATE;
    m_pixelAlign = pixelAlign;
    m_textLabel = NULL;
    m_textSuffix = NULL;
    m_multiple = multipleElemtInRow;
    m_leftMarginfromCenter = leftMarginFromCenter;
    changeImage((m_isEnabled) ? IMAGE_TYPE_NORMAL : IMAGE_TYPE_DISABLE);
    setGeometryForElements();
    this->setEnabled(m_isEnabled);
    this->setMouseTracking(true);
    this->show();
}

OptionSelectButton::~OptionSelectButton()
{
    DELETE_OBJ(m_textLabel);
    DELETE_OBJ(m_textSuffix);
}

void OptionSelectButton::setGeometryForElements()
{
    int width = 0;
    int textLabelWidth = 0, textLabelHeight = 0;
    int suffixWidth = 0, suffixHeight = 0;
    int verticalOffSet = 0, translatedlabelWidth = 0, translatedsuffixWidth = 0;

    if(m_label != "")
    {
        translatedlabelWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, m_textLabelSize)).width(QApplication::translate(QT_TRANSLATE_STR, m_label.toUtf8().constData()));
        textLabelWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, m_textLabelSize)).width(m_label);
        textLabelHeight = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, m_textLabelSize)).height();
        width = SCALE_WIDTH(10);
    }

    if(m_suffix != "")
    {
        suffixWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, m_textSuffixSize)).width(m_suffix);
        translatedsuffixWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, m_textSuffixSize)).width(QApplication::translate(QT_TRANSLATE_STR, m_suffix.toUtf8().constData()));
        suffixHeight = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, m_textSuffixSize)).height();
        width = SCALE_WIDTH(10);
    }

    width += (m_iconImage.width() + textLabelWidth + suffixWidth);

    switch(m_bgTileType)
    {
        case NO_LAYER:
            m_width = width;
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
        m_imageRect.setRect((m_width / 2) - m_leftMarginfromCenter,
                            ((m_height - m_iconImage.height()) / 2) + verticalOffSet,
                            m_iconImage.width(),
                            m_iconImage.height());
        if(m_label != "")
        {
            textLabelWidth = (translatedlabelWidth > ((getWidth()/2) - SCALE_WIDTH(20))) ? ((getWidth()/2) - SCALE_WIDTH(20)) : translatedlabelWidth;
            m_textLabel = new TextLabel((m_imageRect.x() - SCALE_WIDTH(10) - textLabelWidth),
                                        (((m_height - textLabelHeight) / 2) + verticalOffSet),
                                        m_textLabelSize,
                                        m_label,
                                        this,
                                        m_textFontColor, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                        0, 0, textLabelWidth, 0, 0, Qt::AlignRight);
        }

        if(m_suffix != "")
        {
            if(m_multiple)
            {
                maxPossibleWidth = this->width() - ((m_imageRect.x() + m_imageRect.width() + SCALE_WIDTH(10)));
                suffixWidth = (translatedsuffixWidth > maxPossibleWidth) ? maxPossibleWidth : translatedsuffixWidth;
            }

            m_textSuffix = new TextLabel((m_imageRect.x() + m_imageRect.width() + SCALE_WIDTH(10)),
                                         (((m_height - suffixHeight) / 2) + verticalOffSet),
                                         m_textSuffixSize,
                                         m_suffix,
                                         this,
                                         m_suffixFontColor, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                         0, 0, suffixWidth);
        }
    }
    else
    {
        if(m_label != "")
        {
            translatedlabelWidth = (translatedlabelWidth > (m_pixelAlign + textLabelWidth)) ? ((m_pixelAlign + textLabelWidth) - SCALE_WIDTH(17)) : (translatedlabelWidth);
            m_textLabel = new TextLabel(abs((abs(translatedlabelWidth - (m_pixelAlign + textLabelWidth))) - SCALE_WIDTH(5)),
                                        (((m_height -textLabelHeight) / 2) + verticalOffSet),
                                        m_textLabelSize,
                                        m_label,
                                        this,
                                        m_textFontColor, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                        0, 0, translatedlabelWidth, 0, 0, Qt::AlignRight);
            textLabelWidth += SCALE_WIDTH(10);
        }

        m_imageRect.setRect((m_pixelAlign + textLabelWidth),
                            (((m_height - m_iconImage.height()) / 2) + verticalOffSet),
                            m_iconImage.width(),
                            m_iconImage.height());

        if(m_suffix != "")
        {
            if(m_multiple)
            {
                maxPossibleWidth = this->width() - ((m_imageRect.x() + m_imageRect.width() + SCALE_WIDTH(10)));
                suffixWidth = (translatedsuffixWidth > maxPossibleWidth) ? maxPossibleWidth : translatedsuffixWidth;
            }

            m_textSuffix = new TextLabel((m_pixelAlign + textLabelWidth + m_imageRect.width() + SCALE_WIDTH(10)),
                                         (((m_height - suffixHeight) / 2) + verticalOffSet),
                                         m_textSuffixSize,
                                         m_suffix,
                                         this,
                                         m_suffixFontColor, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                         0, 0, suffixWidth, 0, 0);
        }
    }
}

void OptionSelectButton::changeImage(IMAGE_TYPE_e imageType)
{
    m_currentImageType = imageType;
    m_imageSource = QString(OPTION_SELECTION_BOTTON_IMG_PATH) + imageFolderPath[m_buttonType] + imageStatePath[m_currentState] + imgTypePath[m_currentImageType];
    m_iconImage = QPixmap(m_imageSource);
    SCALE_IMAGE(m_iconImage);
}

void OptionSelectButton::changeState(OPTION_STATE_TYPE_e state)
{
    if ((state >= MAX_STATE) || (state == m_currentState))
    {
        return;
    }

    m_currentState = state;
    changeImage(m_currentImageType);
    update();
}

void OptionSelectButton::changeFontColor(MX_OPTION_TEXT_TYPE_e textType, QString fontColor)
{
    switch(textType)
    {
        case MX_OPTION_TEXT_TYPE_LABEL:
            m_textFontColor = fontColor;
            m_textLabel->changeColor(fontColor);
            break;

        case MX_OPTION_TEXT_TYPE_SUFFIX:
            m_suffixFontColor = fontColor;
            m_textSuffix->changeColor(fontColor);
            break;

        default:
            break;
    }
    update ();
}

void OptionSelectButton::changeLabel(MX_OPTION_TEXT_TYPE_e textType, QString string)
{
    switch(textType)
    {
        case MX_OPTION_TEXT_TYPE_LABEL:
            if(IS_VALID_OBJ(m_textLabel))
            {
                m_label = string;
                m_textLabel->changeText(string, 0, true);
            }
            break;

        case MX_OPTION_TEXT_TYPE_SUFFIX:
            if(IS_VALID_OBJ(m_textSuffix))
            {
                m_suffix = string;
                m_textSuffix->changeText(string, 0, true);
            }
            break;

        default:
            break;
    }
    update ();
}

OPTION_STATE_TYPE_e OptionSelectButton::getCurrentState()
{
    return m_currentState;
}

quint8 OptionSelectButton::getButtonIndex(void)
{
	return (m_indexInPage);
}

void OptionSelectButton::selectControl()
{
    if(m_currentImageType != IMAGE_TYPE_MOUSE_HOVER)
    {
        changeImage(IMAGE_TYPE_MOUSE_HOVER);
        update();
    }
}

void OptionSelectButton::deSelectControl()
{
    changeImage((m_isEnabled) ? IMAGE_TYPE_NORMAL : IMAGE_TYPE_DISABLE);
    update();
}

void OptionSelectButton::forceActiveFocus()
{
    this->setFocus();
}

void OptionSelectButton::setIsEnabled(bool isEnable)
{
    if(m_isEnabled == isEnable)
    {
        return;
    }

    m_isEnabled = isEnable;
    this->setEnabled(m_isEnabled);
    changeImage((m_isEnabled) ? IMAGE_TYPE_NORMAL : IMAGE_TYPE_DISABLE);
    update();
}

void OptionSelectButton::takeEnterKeyAction()
{
    if((m_buttonType != RADIO_BUTTON_INDEX) || (m_currentState != ON_STATE))
    {
        changeState((OPTION_STATE_TYPE_e)((m_currentState + 1) % MAX_STATE));
        emit sigButtonClicked(m_currentState, m_indexInPage);
    }
}

void OptionSelectButton::paintEvent(QPaintEvent * event)
{
    QWidget::paintEvent(event);
    BgTile::paintEvent(event);
    QPainter painter(this);
    painter.drawPixmap(m_imageRect, m_iconImage);
}

void OptionSelectButton::mouseMoveEvent(QMouseEvent * event)
{
    if((m_imageRect.contains(event->pos())) && (m_isControlActivated))
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

void OptionSelectButton::mouseReleaseEvent(QMouseEvent * event)
{
    if((m_imageRect.contains(event->pos())) && (m_mouseClicked) && (event->button() == m_leftMouseButton))
    {
        takeEnterKeyAction();
    }
    m_mouseClicked = false;
}

void OptionSelectButton::mousePressEvent(QMouseEvent * event)
{
    if((m_imageRect.contains(event->pos())) && (event->button() == m_leftMouseButton))
    {
        m_mouseClicked = true;
        if(!this->hasFocus())
        {
            forceActiveFocus();
            emit sigUpdateCurrentElement(m_indexInPage);
        }
    }
}

void OptionSelectButton::focusInEvent(QFocusEvent *)
{
    selectControl();
}

void OptionSelectButton::focusOutEvent(QFocusEvent *)
{
    deSelectControl();
}

void OptionSelectButton::enterKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        event->accept();
        takeEnterKeyAction();
    }
}
