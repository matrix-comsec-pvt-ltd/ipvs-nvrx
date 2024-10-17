#include "KeypadButton.h"
#include "EnumFile.h"
#include "VirtualKeypad.h"
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QPaintEvent>

#define KEYPAD_IMG_PATH         IMAGE_PATH"Keypad/"

const QString normalImgName[KEY_MAX_TYPE]=
{
    "",                     //KEY_ALPHANUM,
    "",                     //KEY_CLEAR,
    "",                     //KEY_DONE,
    "",                     //KEY_SPACE,
    "Backspace_1.png",      //KEY_BACKSPACE,
    "Enter_1.png",          //KEY_ENTER,
    "CapsLock_1.png",           //KEY_CAPS,
    "LeftArrow_1.png",      //KEY_LEFT_ARROW,
    "RightArrow_1.png",     //KEY_RIGHT_ARROW,
    "UpArrow_1.png",        //KEY_UP_ARROW,
    "DownArrow_1.png"       //KEY_DOWN_ARROW,
};

const QString clickedImgName[KEY_MAX_TYPE]=
{
    "",                     //KEY_ALPHANUM,
    "",                     //KEY_CLEAR,
    "",                     //KEY_DONE,
    "",                     //KEY_SPACE,
    "Backspace_3.png",      //KEY_BACKSPACE,
    "Enter_3.png",          //KEY_ENTER,
    "CapsLock_3.png",           //KEY_CAPS,
    "LeftArrow_3.png",      //KEY_LEFT_ARROW,
    "RightArrow_3.png",     //KEY_RIGHT_ARROW,
    "UpArrow_3.png",        //KEY_UP_ARROW,
    "DownArrow_3.png"       //KEY_DOWN_ARROW,
};

KeypadButton::KeypadButton(int statrtX,
                           int statrtY,
                           int index,
                           KEY_TYPE_e key,
                           QString label,
                           QWidget *parent, quint8 fontSize)
    :QWidget(parent)
{
    this->setEnabled(true);

    m_label = label;
    m_index = index;
    m_height = SCALE_HEIGHT(KEYPAD_KEY_SIZE);
    m_startX = statrtX;
    m_startY = statrtY;
    keyType = key;
    m_fontSize = fontSize;

    m_textLabel= NULL;

    m_isMousePressed = false;

    switch(keyType)
    {
    case KEY_ALPHANUM:
        m_width = SCALE_WIDTH(KEYPAD_KEY_SIZE);
        m_textLabel = new TextLabel(m_width/2,
                                    m_height/2,
                                    m_fontSize, m_label, this,
                                    NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY,
                                    ALIGN_CENTRE_X_CENTER_Y);
        break;

    case KEY_DONE:
    case KEY_CLEAR:
    case KEY_CAPS:
        m_width = SCALE_WIDTH(KEYPAD_SPECIAL_KEY_1_SIZE);
        m_textLabel = new TextLabel(m_width/2,
                                    m_height/2,
                                    m_fontSize, m_label, this,
                                    NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY,
                                    ALIGN_CENTRE_X_CENTER_Y);
        break;

    case KEY_LEFT_ARROW:
    case KEY_RIGHT_ARROW:
        m_width = SCALE_WIDTH(KEYPAD_KEY_SIZE);
        break;

    case KEY_BACKSPACE:
    case KEY_ENTER:
    case KEY_UP_ARROW:
    case KEY_DOWN_ARROW:
        m_width = SCALE_WIDTH(KEYPAD_SPECIAL_KEY_2_SIZE);
        break;

    case KEY_SPACE:
        m_width = ((SCALE_WIDTH(KEYPAD_KEY_SIZE)*12) + (SCALE_WIDTH(KEYPAD_KEY_MARGIN)*12))
                - ((SCALE_WIDTH(KEYPAD_SPECIAL_KEY_1_SIZE)*3) + (SCALE_WIDTH(KEYPAD_KEY_MARGIN)*4));
        break;

    default:
        break;
    }

    QString imgPath = KEYPAD_IMG_PATH + normalImgName[keyType];
    m_image = QPixmap(imgPath);
    SCALE_IMAGE(m_image);
    this->setGeometry(QRect(m_startX, m_startY, m_width, m_height));

    m_mainRect = new QRect(0, 0, m_width, m_height);
    m_imgRect.setRect ((this->width () - m_image.width ())/2,
                            (this->height () - m_image.height ())/2,
                            m_image.width (),
                            m_image.height ());
    this->show ();
}

KeypadButton::~KeypadButton()
{
    delete m_mainRect;

    if(m_textLabel != NULL)
    {
        delete m_textLabel;
    }
}

void KeypadButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.setPen(Qt::NoPen);

    if (m_isMousePressed == false)
    {
        painter.setBrush(QBrush(QColor(NORMAL_BKG_COLOR), Qt::SolidPattern));
    }
    else
    {
        painter.setBrush(QBrush(QColor(BORDER_2_COLOR), Qt::SolidPattern));
    }

    painter.drawRoundedRect(*m_mainRect, KEYPAD_KEY_BORDER_RADIUS, KEYPAD_KEY_BORDER_RADIUS);

    if((!(keyType == KEY_ALPHANUM) ||
        (keyType == KEY_CLEAR) ||
        (keyType == KEY_DONE) ||
        (keyType == KEY_SPACE)) )
    {
        painter.drawPixmap (m_imgRect, m_image);
    }
    QWidget::paintEvent(event);
}

void KeypadButton::mousePressEvent (QMouseEvent *event)
{
    QString imgPath;

    m_isMousePressed = true;

    switch(keyType)
    {
    case KEY_ALPHANUM:
    case KEY_CLEAR:
    case KEY_DONE:
        m_textLabel->changeColor (HIGHLITED_FONT_COLOR);
        break;

    case KEY_SPACE:
        break;

    case KEY_CAPS:
    case KEY_LEFT_ARROW:
    case KEY_RIGHT_ARROW:
    case KEY_BACKSPACE:
    case KEY_ENTER:
    case KEY_UP_ARROW:
    case KEY_DOWN_ARROW:
        imgPath = KEYPAD_IMG_PATH + clickedImgName[keyType];
        break;

    default:
        break;
    }
    m_image = QPixmap(imgPath);
    SCALE_IMAGE(m_image);
    update ();
    Q_UNUSED(event);
}


void KeypadButton::mouseReleaseEvent(QMouseEvent *)
{
    QString imgPath;
    m_isMousePressed = false;
    // change text color
    // cnhange image if image is there
    switch(keyType)
    {
    case KEY_ALPHANUM:
    case KEY_CLEAR:
    case KEY_DONE:
        m_textLabel->changeColor (NORMAL_FONT_COLOR);
        break;

    case KEY_SPACE:
        break;

    case KEY_CAPS:
    case KEY_LEFT_ARROW:
    case KEY_RIGHT_ARROW:
    case KEY_BACKSPACE:
    case KEY_ENTER:
    case KEY_UP_ARROW:
    case KEY_DOWN_ARROW:
        imgPath = KEYPAD_IMG_PATH + normalImgName[keyType];
        break;

    default:
        break;
    }

    emit sigKeyPressed (keyType, m_index);
    m_image = QPixmap(imgPath);
    SCALE_IMAGE(m_image);
    update ();
}

void KeypadButton::changeButtonText(QString str)
{
    m_textLabel->changeText (str);
}

void KeypadButton::changeFontSize (quint8 fontSize)
{
    if(m_textLabel != NULL)
    {
        m_textLabel->setFontSize (fontSize);
    }
}
