#include "KeypadButton.h"
#include "Enumfile.h"
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QPaintEvent>

#define unitBorderThickness  1
#define twiceBorderThickness  (2)
#define thriceBorderThickness  (3)

#define KEYPAD_IMG_PATH         IMAGE_PATH"/Keypad/"

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
    this->setMouseTracking(true);
    this->setAttribute(Qt::WA_Hover, true);
    this->installEventFilter(this);
    m_label = label;
    m_index = index;
    m_height = (30);
    m_startX = statrtX;
    m_startY = statrtY;
    keyType = key;
    m_fontSize = fontSize;

    m_textLabel= NULL;

    m_isMouseHover = false;
    isMousePress = false;
    m_isMousePressed = false;

//    QFont font = TextLabel::getFont (NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
//    int strHeight = QFontMetrics(font).height ();
//    int strWidth = 0;

    switch(keyType)
    {
    case KEY_ALPHANUM:
    case KEY_CLEAR:
    case KEY_DONE:
        m_width = (65);
//        strWidth = QFontMetrics(font).width (m_label);

        m_textLabel = new TextLabel(m_width/2,
                                    m_height/2,
                                    m_fontSize, m_label, this,
                                    NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY,
                                    ALIGN_CENTRE_X_CENTER_Y);
        break;

    case KEY_CAPS:
    case KEY_LEFT_ARROW:
    case KEY_RIGHT_ARROW:
        m_width = (65);      //65
        break;

    case KEY_BACKSPACE:
    case KEY_ENTER:
    case KEY_UP_ARROW:
    case KEY_DOWN_ARROW:
        m_width = (130);
        break;

    case KEY_SPACE:
        m_width = (585);     //579
        break;

    default:
        break;
    }

    QString imgPath = KEYPAD_IMG_PATH + normalImgName[keyType];
    m_image = QPixmap(imgPath);    
    //    m_horizontalOffset = horizontalOffset;
    //    m_textLabel = new TextLabel(50, 50 , 20, "ABCD", this);
    this->setGeometry(QRect(m_startX,
                            m_startY,
                            m_width,
                            m_height));
    setRectanglesGeometry();

    m_imgRect.setRect ((this->width () - m_image.width ())/2,
                            (this->height () - m_image.height ())/2,
                            m_image.width (),
                            m_image.height ());
    this->show ();
}


void KeypadButton::setRectanglesGeometry()
{
    m_mainRect = new QRect(0, 0,
                           m_width,
                           m_height);
    m_topRect = new QRect(0, 0,
                          m_width,
                          unitBorderThickness);
    m_bottomRect = new QRect(0,
                             m_height - unitBorderThickness,
                             m_width,
                             unitBorderThickness);
    m_leftRect = new QRect(0, 0,
                           unitBorderThickness,
                           m_height);
    m_rightRect = new QRect(m_width - unitBorderThickness,
                            0,
                            unitBorderThickness,
                            m_height);
    m_bottomRect_1 = new QRect(m_width - thriceBorderThickness,
                               m_height - thriceBorderThickness,
                               thriceBorderThickness,
                               unitBorderThickness);
    m_bottomRect_2 = new QRect(m_width - twiceBorderThickness,
                               m_height - twiceBorderThickness,
                               twiceBorderThickness,
                               unitBorderThickness);
    m_bottomRect_3 = new QRect(m_width - unitBorderThickness,
                               m_height - unitBorderThickness,
                               unitBorderThickness,
                               unitBorderThickness);
    m_leftRect_1 = new QRect(0, 0,
                             unitBorderThickness,
                             unitBorderThickness);
    m_leftRect_2 = new QRect(1, 0,
                             unitBorderThickness,
                             twiceBorderThickness);
    m_leftRect_3 = new QRect(2 , 0,
                             unitBorderThickness,
                             thriceBorderThickness);
}

KeypadButton::~KeypadButton()
{
    delete m_mainRect;
    delete m_topRect;
    delete m_rightRect;
    delete m_leftRect;
    delete m_bottomRect;
    delete m_leftRect_1;
    delete m_leftRect_2;
    delete m_leftRect_3;
    delete m_bottomRect_1;
    delete m_bottomRect_2;
    delete m_bottomRect_3;

    if(m_textLabel != NULL)
    {
        delete m_textLabel;
    }
}

bool KeypadButton::eventFilter(QObject * obj, QEvent * event)
{
    if(event->type() == QEvent::HoverEnter || event->type() == QEvent::Enter)
    {
        m_isMouseHover = true;
    }
    if(event->type() == QEvent::HoverLeave || event->type() == QEvent::Leave)
    {
        m_isMouseHover = false;
    }
    return QObject::eventFilter(obj,event);
}

void KeypadButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    drawRectangles(&painter);

    if((!(keyType == KEY_ALPHANUM) ||
        (keyType == KEY_CLEAR) ||
        (keyType == KEY_DONE) ||
        (keyType == KEY_SPACE)) )
    {
        painter.drawPixmap (m_imgRect, m_image);
    }
    //    if(m_isMouseHover == true)
    //    {
    //        m_isMouseHover = false;
    //    }
    //    if(m_isMousePressed == true)
    //    {
    //        m_isMousePressed = false;
    //    }
    QWidget::paintEvent(event);
}

void KeypadButton::mousePressEvent (QMouseEvent *event)
{
    QString imgPath;
    isMousePress = true;
    m_isMousePressed = true;
    mousePressPoint = event->pos ();

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
    update ();
    //    emit sigButtonClicked(m_index);
}


void KeypadButton::mouseReleaseEvent(QMouseEvent *)
{
    QString imgPath;
    m_isMousePressed = false;
    //    if((isMousePress == true) && (event->pos () == mousePressPoint))
    //    {
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
    update ();
    //    }
    isMousePress = false;
}


void KeypadButton::resetGeometry(int xOffset, int yOffset)
{
    m_startX = xOffset;
    m_startY = yOffset;
    this->setGeometry(QRect(xOffset,
                            yOffset,
                            m_width,
                            m_height));
}

void KeypadButton::drawRectangles(QPainter * painter, bool)
{
    if(m_isMousePressed == false)
    {
        if(m_isMouseHover == true)
        {
            m_bottomRect->setTop(m_height - thriceBorderThickness);
            m_rightRect->setLeft(m_width - thriceBorderThickness);
            m_rightRect->setWidth(thriceBorderThickness);
            m_topRect->setHeight(thriceBorderThickness);
            m_bottomRect->setHeight(thriceBorderThickness);
            m_leftRect->setWidth(thriceBorderThickness);
        }
        else
        {
            m_bottomRect->setTop(m_height - unitBorderThickness);
            m_rightRect->setLeft(m_width - unitBorderThickness);
            m_rightRect->setWidth(unitBorderThickness);
            m_topRect->setHeight(unitBorderThickness);
            m_bottomRect->setHeight(unitBorderThickness);
            m_leftRect->setWidth(unitBorderThickness);
        }
        //main rectangle
        painter->setBrush(QBrush(QColor(NORMAL_BKG_COLOR),Qt::SolidPattern));
        painter->drawRect(*m_mainRect);

        //right border
        painter->setBrush(QBrush(QColor(BORDER_1_COLOR),Qt::SolidPattern));
        painter->drawRect(*m_rightRect);

        //bottom border
        painter->setBrush(QBrush(QColor(BORDER_2_COLOR),Qt::SolidPattern));
        painter->drawRect(*m_bottomRect);

        if(m_isMouseHover == true )
        {
            painter->setBrush(QBrush(QColor(BORDER_1_COLOR), Qt::SolidPattern));
            painter->drawRect(*m_bottomRect_1);
            painter->drawRect(*m_bottomRect_2);
            painter->drawRect(*m_bottomRect_3);
        }

        //top border
        painter->setBrush(QBrush(QColor(BORDER_1_COLOR), Qt::SolidPattern));
        painter->drawRect(*m_topRect);

        //left border
        painter->setBrush(QBrush(QColor(BORDER_2_COLOR), Qt::SolidPattern));
        painter->drawRect(*m_leftRect);

        if(m_isMouseHover == true )
        {
            painter->setBrush(QBrush(QColor(BORDER_1_COLOR), Qt::SolidPattern));
            painter->drawRect(*m_leftRect_1);
            painter->drawRect(*m_leftRect_2);
            painter->drawRect(*m_leftRect_3);
        }
    }
    else
    {
        m_bottomRect->setTop(m_height - unitBorderThickness);
        m_rightRect->setLeft(m_width - unitBorderThickness);
        m_rightRect->setWidth(unitBorderThickness);
        m_topRect->setHeight(unitBorderThickness);
        m_bottomRect->setHeight(unitBorderThickness);
        m_leftRect->setWidth(unitBorderThickness);

        //main rectangle
        painter->setBrush(QBrush(QColor(CLICKED_BKG_COLOR),Qt::SolidPattern));
        painter->drawRect(*m_mainRect);

        //right border
        painter->setBrush(QBrush(QColor(BORDER_2_COLOR),Qt::SolidPattern));
        painter->drawRect(*m_rightRect);

        //bottom border
        painter->setBrush(QBrush(QColor(BORDER_1_COLOR),Qt::SolidPattern));
        painter->drawRect(*m_bottomRect);

        //top border
        painter->setBrush(QBrush(QColor(BORDER_2_COLOR), Qt::SolidPattern));
        painter->drawRect(*m_topRect);

        //left border
        painter->setBrush(QBrush(QColor(BORDER_1_COLOR), Qt::SolidPattern));
        painter->drawRect(*m_leftRect);
    }
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
