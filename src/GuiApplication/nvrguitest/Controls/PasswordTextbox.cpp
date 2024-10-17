#include "PasswordTextbox.h"

PasswordTextbox::PasswordTextbox(quint32 startX,
                                 quint32 startY,
                                 quint32 width,
                                 quint32 height,
                                 quint16 controlIndex,
                                 TEXTBOX_SIZE_e size,
                                 QWidget *parent,
                                 TextboxParam *textBoxParam,
                                 BGTILE_TYPE_e bgType,
                                 bool isNavigationEnable)
    :TextBox(startX, startY, width, height,
             controlIndex, size, parent, textBoxParam, bgType,
             isNavigationEnable)
{

    quint8 length = param->textStr.length ();
    showPassword.clear ();
    showPassword.reserve (length);
    for(quint8 index = 0; index < length; index++)
    {
        showPassword.append ("*");
    }

    refreshTexboxTimer = new QTimer(this);
    connect (refreshTexboxTimer,
             SIGNAL(timeout()),
             this,
             SLOT(slotRefreshTimerTimeout()));
    refreshTexboxTimer->setInterval (500);
    refreshTexboxTimer->setSingleShot (true);

    lineEdit->setText (showPassword);
    this->show ();
}

PasswordTextbox::~PasswordTextbox()
{
    delete refreshTexboxTimer;
}

void PasswordTextbox::passwordDoneValidation()
{
    if(param->textStr.length () < param->minChar)
    {
        if(!param->isTotalBlankStrAllow)
        {
            param->textStr = oldTextValue;
            for(quint8 index = 0; index < param->textStr.length (); index++)
            {
                showPassword.insert (index,"*");
            }
        }
        // page info loader with start char error msg
        emit sigLoadInfopage(m_indexInPage, INFO_MSG_ERROR);
    }
    else
    {
        oldTextValue = param->textStr;
    }
}

void PasswordTextbox::slotTextBoxKeyPressed(KEY_TYPE_e keyType, QString str)
{
    QString newStr = param->textStr;
    bool isAlphaNumKeyAccept = false;

    if(refreshTexboxTimer->isActive ())
    {
        refreshTexboxTimer->stop ();
        slotRefreshTimerTimeout ();
    }

    switch(keyType)
    {
    case KEY_SPACE:
        str = " ";
        // fall through
    case KEY_ALPHANUM:
        // check for key to validator
        if((param->validation.isEmpty ()) ||
                (str.contains (param->validation)))
        {
            newStr = newStr.insert (currCurPos, str);
            if(newStr.length () <= param->maxChar)
            {
                isAlphaNumKeyAccept = true;
            }
        }

        if(isAlphaNumKeyAccept == true)
        {
            param->textStr = newStr;
            showPassword.insert (currCurPos, str);;
            currCurPos++;
            lineEdit->setText (showPassword);
            lineEdit->setCursorPosition  (currCurPos);
            refreshTexboxTimer->start ();
        }
        break;

    case KEY_ALPHANUM_SAME_INDEX:
        if(str.contains (param->validation))
        {
            newStr = newStr.remove (currCurPos - 1, 1);
            newStr = newStr.insert (currCurPos - 1, str);

            if(newStr.length () <= param->maxChar)
            {
                isAlphaNumKeyAccept = true;
            }
        }

        if(isAlphaNumKeyAccept == true)
        {
            param->textStr = newStr;
            showPassword.remove (currCurPos - 1, 1);
            showPassword.insert (currCurPos - 1, str);
            lineEdit->setText (showPassword);
            refreshTexboxTimer->start ();
        }
        break;

    case KEY_CLEAR:
        currCurPos = 0;
        param->textStr = "";
        showPassword = "";
        lineEdit->setText (showPassword);
        break;

    case KEY_BACKSPACE:
        if(currCurPos > 0)
        {
            currCurPos--;
            param->textStr = param->textStr.remove (currCurPos, 1);

            showPassword.remove (currCurPos, 1);
            lineEdit->setText (showPassword);
            lineEdit->setCursorPosition  (currCurPos);
        }
        else
        {
            currCurPos = 0;
        }
        break;

    case KEY_LEFT_ARROW:
        if(currCurPos > 0)
        {
            currCurPos--;
        }
        else
        {
            currCurPos = 0;
        }
        lineEdit->setCursorPosition  (currCurPos);
        break;

    case KEY_RIGHT_ARROW:
        currCurPos++;
        lineEdit->setCursorPosition  (currCurPos);
        break;

    case KEY_DONE:
        lineEdit->setText (showPassword);
        lineEdit->setCursorPosition  (currCurPos);
        editMode= false;

        if(entryByRemote == false)
        {
            unloadVirtualKeypad();
        }

        entryByRemote = false;
        forceActiveFocus ();
        passwordDoneValidation();

        lineEdit->setText (showPassword);
        lineEdit->setCursorPosition (currCurPos);
        break;

    case KEY_CLOSE:
        param->textStr = oldTextValue;
        showPassword = "";
        for(quint8 index = 0; index < param->textStr.length (); index++)
        {
            showPassword.append ("*");
        }
        lineEdit->setText (showPassword);
        lineEdit->setCursorPosition  (currCurPos);
        editMode = false;
        if(entryByRemote == false)
        {
            unloadVirtualKeypad ();
        }
        entryByRemote = false;
        forceActiveFocus ();
        break;

    default:
        break;
    }
    update ();
}

void PasswordTextbox::slotRefreshTimerTimeout()
{
    showPassword.replace ((currCurPos - 1),1, "*");
    lineEdit->setText (showPassword);
    lineEdit->setCursorPosition  (currCurPos);
    update ();
}

void PasswordTextbox::setInputText (QString str)
{
    param->textStr = str;

    quint8 length = param->textStr.length ();
    showPassword.clear ();
    showPassword.reserve (length);
    for(quint8 index = 0; index < length; index++)
    {
        showPassword.append ("*");
    }
    currCurPos = 0;
    lineEdit->setText (showPassword);
    lineEdit->setCursorPosition  (currCurPos);

    update ();
}

void PasswordTextbox::changeLabel(QString str)
{
    quint16 labelWidth = 0,suffixWidth = 0, labelHeight = 0;
    quint16 width = 0;
    qint8 verticalOffset = 0;
    QFont labelFont = TextLabel::getFont (NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);

    if(str != "")
    {
        param->labelStr = str;
        labelFont = TextLabel::getFont (NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
        labelWidth = QFontMetrics(labelFont).width (param->labelStr);
        labelHeight = QFontMetrics(labelFont).height ();
        width += (10);
    }

    width += image.width () + labelWidth + suffixWidth;

    switch(m_bgTileType)
    {
    case NO_LAYER:
        m_width = width;
        this->setGeometry(m_startX, m_startY, m_width, m_height);
        param->isCentre = false;
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

    if(param->isCentre == true)
    {
        if(param->labelStr != "")
        {
            labelText->setOffset(((this->width ()/2) - (10) - labelWidth),
                                 (this->height () - labelHeight)/2 + verticalOffset);
            labelText->changeText(param->labelStr);
        }

        imgRect.setRect (this->width ()/2,
                         ((this->height () - image.height ())/2) + verticalOffset,
                         image.width (),
                         image.height ());
    }
    else
    {
        if(param->labelStr != "")
        {
            labelText->setOffset(param->leftMargin,
                                 (this->height () - labelHeight)/2 + verticalOffset);
            labelText->changeText(param->labelStr);
            labelWidth += (10);
        }

        imgRect.setRect (param->leftMargin+labelWidth,
                         (this->height () - image.height ())/2 + verticalOffset,
                         image.width (),
                         image.height ());

    }

    lineEdit->setFrame (false);

    if(m_isEnabled)
    {
        palette->setColor(QPalette::Text, QColor(NORMAL_FONT_COLOR).lighter (200));

    }
    else
    {
        palette->setColor(QPalette::Text, QColor(QString(DISABLE_FONT_COLOR)));
    }

    palette->setColor(QPalette::Base, QColor(Qt::transparent));
    lineEdit->setPalette(*palette);
    lineEdit->setFont (labelFont);

    setInputText(param->textStr);
    update();
}
