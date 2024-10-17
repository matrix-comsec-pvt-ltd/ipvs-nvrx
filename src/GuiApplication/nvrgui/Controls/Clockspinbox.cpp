#include "Clockspinbox.h"
#include <QPainter>
#include <QMouseEvent>

ClockSpinbox::ClockSpinbox(qint32 xParam,
                           qint32 yParam,
                           qint32 width,
                           qint32 height,
                           quint16 controlIndex,
                           CLK_SPIN_TYPE_e bgType,
                           QString labelStr,
                           qint8 maxElementWithoutScroll,
                           QWidget* parent,
                           QString suffixStr,
                           bool isBoxStartInCentre ,
                           quint16 leftMarginOfLabel,
                           BGTILE_TYPE_e bgTileType ,
                           bool isNavigationEnable,
                           quint8 cntValue,
                           bool minMaxReq,
                           bool isDropUpList, 
						   quint32 leftMarginfromCenter):
    BgTile(xParam,yParam,width,height,bgTileType,parent),
    NavigationControl(controlIndex, isNavigationEnable), currentImageType(MAX_IMAGE_TYPE),
    leftMargin(leftMarginOfLabel), m_leftMarginfromCenter(leftMarginfromCenter), countValue(cntValue),
    label(labelStr), suffix(suffixStr),minTotalSeconds(0), maxTotalSeconds(0)
{
    type = bgType;
    currClickedBox = MAX_TIME_BOX;
    isMinMaxReq = minMaxReq;
    isCentre = isBoxStartInCentre;
    m_isDropUpList = isDropUpList;
    m_maxElementWithoutScroll = maxElementWithoutScroll;

    labelText = NULL;
    suffixText = NULL;

    createDefaultComponents();

    if(type == CLK_SPINBOX_With_SEC)
    {
        assignValue("12", "05", "54");
    }
    else
    {
        assignValue("00", "00");
    }

    this->setEnabled(m_isEnabled);
    this->setMouseTracking(true);
    this->show();
}

void ClockSpinbox::createDefaultComponents()
{
    QString imagePath;
    qint8 verticalOffset = 0;
    quint16 width = 0;
    quint16 labelWidth = 0, translatedlabelWidth = 0;
    quint16 labelHeight = 0;
    quint16 suffixWidth = 0;
    quint16 strHeight = 0;
    m_dropDownList = NULL;
    m_dropDownInVisibleWidget = NULL;

    m_hourList.clear();
    for(quint8 index = 0; index <= MAX_HOUR; index++)
    {
        m_hourList.insert(index, QString("%1").arg(index, 2, 10, QLatin1Char('0')));
    }

    m_minuteList.clear();
    m_secondList.clear();
    for(quint8 index = 0; index < MAX_MIN_SEC; index += countValue)
    {
        m_minuteList.insert(index, QString("%1").arg(index, 2, 10, QLatin1Char('0')));
        m_secondList.insert(index, QString("%1").arg(index, 2, 10, QLatin1Char('0')));
    }

    if (m_isEnabled == true)
    {
        fontColor = NORMAL_FONT_COLOR;
        currentImageType = IMAGE_TYPE_NORMAL;
    }
    else
    {
        fontColor = DISABLE_FONT_COLOR;
        currentImageType = IMAGE_TYPE_DISABLE;
    }

    if (label != "")
    {
        QFont labelFont = TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
        translatedlabelWidth = QFontMetrics(labelFont).width(QApplication::translate(QT_TRANSLATE_STR, label.toUtf8().constData()));
        labelWidth = QFontMetrics(labelFont).width (label);
        labelHeight = QFontMetrics(labelFont).height ();
        width += SCALE_WIDTH(10);
    }

    if(suffix != "")
    {
        QFont suffixFont = TextLabel::getFont (NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE);
        suffixWidth = QFontMetrics(suffixFont).width (suffix);
        strHeight = QFontMetrics(suffixFont).height ();
        width += SCALE_WIDTH(10);
    }

    imagePath = CLK_SPIN_BG_IMAGE_PATH + clkSpinSubFldr[type] + imgTypePath[IMAGE_TYPE_NORMAL];
    bgImage = QPixmap (imagePath);
    SCALE_IMAGE(bgImage);

    imagePath = CLK_SPIN_CELL_IMAGE_PATH + imgTypePath[IMAGE_TYPE_NORMAL];
    hrsImage = QPixmap (imagePath);
    SCALE_IMAGE(hrsImage);

    minImage = QPixmap (imagePath);
    SCALE_IMAGE(minImage);

    secImage = QPixmap (imagePath);
    SCALE_IMAGE(secImage);

    hrsIndx = minIndx = secIndx = 0;
    width += bgImage.width () + labelWidth + suffixWidth + SCALE_WIDTH(30);
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

    if(isCentre == true)
    {
        if (label != "")
        {
            labelWidth = (translatedlabelWidth > ((getWidth()/2) - SCALE_WIDTH(20)))? ((getWidth()/2) - SCALE_WIDTH(20)) : translatedlabelWidth;
            labelText = new TextLabel(((this->width ()/2) - SCALE_WIDTH(10) - labelWidth) - m_leftMarginfromCenter,
                                      (this->height () - labelHeight)/2 + verticalOffset,
                                      NORMAL_FONT_SIZE, label,
                                      this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                      0, 0, labelWidth, 0, 0, Qt::AlignRight);
        }

        if (suffix != "")
        {
            suffixText = new TextLabel((this->width ()/2) + bgImage.width () + SCALE_WIDTH(10) - m_leftMarginfromCenter,
                                       (this->height () - strHeight)/2 + verticalOffset,
                                       NORMAL_FONT_SIZE, suffix,
                                       this,
                                       SUFFIX_FONT_COLOR);
        }
    }
    else
    {
        if (label != "")
        {
            translatedlabelWidth = (translatedlabelWidth > ((leftMargin + labelWidth) - SCALE_WIDTH(17))) ? ((leftMargin + labelWidth) - SCALE_WIDTH(17)) : (translatedlabelWidth);
            labelText = new TextLabel(abs((abs(translatedlabelWidth - (leftMargin + labelWidth))) - SCALE_WIDTH(5)),
                                      (this->height () - labelHeight)/2,
                                      NORMAL_FONT_SIZE, label,
                                      this,
                                      NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                      0, 0, translatedlabelWidth, 0, 0);
            labelWidth += SCALE_WIDTH(10);
        }

        if (suffix != "")
        {
            suffixText = new TextLabel(leftMargin + bgImage.width () + SCALE_WIDTH(20),
                                       (this->height () - strHeight)/2 + verticalOffset,
                                       NORMAL_FONT_SIZE, suffix,
                                       this,
                                       SUFFIX_FONT_COLOR);
        }
    }

    // BackGround for clockspinbox
    if (isCentre ==true)
    {
        bgRect.setRect (this->width ()/2 - m_leftMarginfromCenter,
                        (this->height () - bgImage.height ())/2 + verticalOffset,
                        bgImage.width (),
                        bgImage.height ());
    }
    else
    {
        bgRect.setRect (leftMargin + labelWidth  ,
                        (this->height () - bgImage.height ())/2 + verticalOffset,
                        bgImage.width (),
                        bgImage.height ());
    }

    // Hour
    hourRect.setRect ( bgRect.x () + SCALE_WIDTH(10),
                       bgRect.y () + SCALE_HEIGHT(5),
                       hrsImage.width (),
                       hrsImage.height ());

    hrsText = new TextLabel( hourRect.x () + SCALE_WIDTH(5),
                             hourRect.y (),
                             NORMAL_FONT_SIZE,
                             QString("%1").arg(hrsIndx),
                             this,fontColor);             //here SCALE_WIDTH(5) is to avoid trucated string

    // Sepreator1
    sepreator1 = new TextLabel( hourRect.x () + hourRect.width () + SCALE_WIDTH(3),
                                hourRect.y (),
                                SCALE_FONT(14), ":", this,fontColor);

    // Min
    minuteRect.setRect ( sepreator1->x () + sepreator1->width () + SCALE_WIDTH(3),
                         hourRect.y () ,
                         minImage.width (),
                         minImage.height ());

    minuteText = new TextLabel( minuteRect.x () + SCALE_WIDTH(5),
                                minuteRect.y ()  ,
                                NORMAL_FONT_SIZE,
                                QString("%1").arg(minIndx),
                                this,fontColor);              //here SCALE_WIDTH(5) is to avoid trucated string

    // Second
    if(type == CLK_SPINBOX_With_SEC)
    {
        sepreator2 = new TextLabel( minuteRect.x () + minuteRect.width () + SCALE_WIDTH(3),
                                    hourRect.y ()  ,
                                    SCALE_FONT(14),
                                    ":",
                                    this,fontColor);

        secRect.setRect ( sepreator2->x () + sepreator2->width () + SCALE_WIDTH(3),
                          hourRect.y () ,
                          secImage.width (),
                          secImage.height ());

        secText = new TextLabel( secRect.x () + SCALE_WIDTH(5),
                                 secRect.y () ,
                                 NORMAL_FONT_SIZE,
                                 QString("%1").arg(secIndx),
                                 this,fontColor);                 //here SCALE_WIDTH(5) is to avoid trucated string (Patch -_-)
    }

    enterMode = false;
}

ClockSpinbox::~ClockSpinbox()
{
    if(type == CLK_SPINBOX_With_SEC)
    {
        delete secText;
        delete sepreator2;
    }
    delete minuteText;
    delete sepreator1;
    delete hrsText;

    DELETE_OBJ(suffixText);
    DELETE_OBJ(labelText);

    unloadList();
    if (m_dropDownInVisibleWidget != NULL)
    {
        disconnect (m_dropDownInVisibleWidget,
                    SIGNAL(sigMouseClick()),
                    this,
                    SLOT(slotUnloadDropList()));
        DELETE_OBJ(m_dropDownInVisibleWidget);
    }
}

void ClockSpinbox::changeImage(CLK_SPIN_IMAGE_TYPE_e index,IMAGE_TYPE_e img_type)
{
    switch(index)
    {
        case CLK_SPIN_MAIN:
            bgImage = QPixmap(QString(CLK_SPIN_BG_IMAGE_PATH + clkSpinSubFldr[type] + imgTypePath[img_type]));
            SCALE_IMAGE(bgImage);
            break;

        case CLK_SPIN_HOUR:
            hrsImage = QPixmap(QString(CLK_SPIN_CELL_IMAGE_PATH + imgTypePath[img_type]));
            SCALE_IMAGE(hrsImage);
            break;

        case CLK_SPIN_MIN:
            minImage = QPixmap(QString(CLK_SPIN_CELL_IMAGE_PATH + imgTypePath[img_type]));
            SCALE_IMAGE(minImage);
            break;

        case CLK_SPIN_SEC:
            secImage = QPixmap(QString(CLK_SPIN_CELL_IMAGE_PATH + imgTypePath[img_type]));
            SCALE_IMAGE(secImage);
            break;
    }

    update ();
}

void ClockSpinbox::paintEvent (QPaintEvent *event)
{
    BgTile::paintEvent (event);
    QPainter painter(this);

    painter.drawPixmap (bgRect, bgImage);
    painter.drawPixmap (hourRect, hrsImage);
    painter.drawPixmap (minuteRect, minImage);
    if(type == CLK_SPINBOX_With_SEC)
    {
        painter.drawPixmap (secRect, secImage);
    }
}

void ClockSpinbox::hrsSelect()
{
    currClickedBox = HOUR_BOX;

    changeImage (CLK_SPIN_HOUR,IMAGE_TYPE_CLICKED);
    hrsText->changeColor ( HIGHLITED_FONT_COLOR );

    changeImage (CLK_SPIN_MIN, IMAGE_TYPE_NORMAL);
    minuteText->changeColor ( NORMAL_FONT_COLOR );

    if(type == CLK_SPINBOX_With_SEC)
    {
        changeImage (CLK_SPIN_SEC, IMAGE_TYPE_NORMAL);
        secText->changeColor ( NORMAL_FONT_COLOR );
    }
}

void ClockSpinbox::minSelect()
{
    currClickedBox = MINUTE_BOX;

    changeImage (CLK_SPIN_HOUR,IMAGE_TYPE_NORMAL);
    hrsText->changeColor ( NORMAL_FONT_COLOR );

    changeImage (CLK_SPIN_MIN, IMAGE_TYPE_CLICKED);
    minuteText->changeColor ( HIGHLITED_FONT_COLOR );

    if(type == CLK_SPINBOX_With_SEC)
    {
        changeImage (CLK_SPIN_SEC, IMAGE_TYPE_NORMAL);
        secText->changeColor ( NORMAL_FONT_COLOR );
    }
}

void ClockSpinbox::secSelect()
{
    currClickedBox = SEC_BOX;

    changeImage (CLK_SPIN_HOUR,IMAGE_TYPE_NORMAL);
    hrsText->changeColor ( NORMAL_FONT_COLOR );

    changeImage (CLK_SPIN_MIN, IMAGE_TYPE_NORMAL);
    minuteText->changeColor ( NORMAL_FONT_COLOR );

    if(type == CLK_SPINBOX_With_SEC)
    {
        changeImage (CLK_SPIN_SEC, IMAGE_TYPE_CLICKED);
        secText->changeColor ( HIGHLITED_FONT_COLOR );
    }
}

void ClockSpinbox::loadList(QMap<quint8, QString>  maplist, qint8 maxElementOnList, qint32 listStartX)
{
    if(m_dropDownList == NULL)
    {
        if (m_dropDownInVisibleWidget == NULL)
        {
            m_dropDownInVisibleWidget = new InvisibleWidgetCntrl(parentWidget());
            m_dropDownInVisibleWidget->setGeometry(QRect(0,
                                                         0,
                                                         parentWidget()->width(),
                                                         parentWidget()->height()));
            connect (m_dropDownInVisibleWidget,
                     SIGNAL(sigMouseClick()),
                     this,
                     SLOT(slotUnloadDropList()));

            m_dropDownInVisibleWidget->show();
        }

        qint32 listStartY;
        if (m_isDropUpList)
        {
            /* Prepare list above the drop list button */
            listStartY = this->y() - (BUTTON_HEIGHT * ((maxElementOnList > m_maxElementWithoutScroll) ? m_maxElementWithoutScroll : maxElementOnList)) + SCALE_HEIGHT(1);
        }
        else
        {
            /* Prepare list below the drop list button */
            listStartY = this->y() + bgRect.y() + bgRect.height();
        }

        if (isMinMaxReq)
        {
            /* Prepare separate drpDwn list for HOUR/MIN/SEC only for values between Min & Max limit */
            QMap<quint8, QString> tMapList;
            tMapList.clear();

            if ( currClickedBox == HOUR_BOX )
            {
                for(quint8 index = 0; index <= MAX_HOUR; index++)
                {
                    if (((currTotalSeconds + ((index - hrsIndx)*3600)) >= minTotalSeconds) && ((currTotalSeconds + ((index - hrsIndx)*3600)) <= maxTotalSeconds))
                    {
                        tMapList.insert(index, QString("%1").arg(index, 2, 10, QLatin1Char('0')));
                    }
                }

            }
            else if( currClickedBox == MINUTE_BOX )
            {
                for(quint8 index = 0; index < MAX_MIN_SEC; index += countValue)
                {
                    if (((currTotalSeconds + ((index - minIndx)*60)) >= minTotalSeconds) && ((currTotalSeconds + ((index - minIndx)*60)) <= maxTotalSeconds))
                    {
                        tMapList.insert(index, QString("%1").arg(index, 2, 10, QLatin1Char('0')));
                    }
                }
            }
            else if( currClickedBox == SEC_BOX )
            {
                for(quint8 index = 0; index < MAX_MIN_SEC; index += countValue)
                {
                    if (((currTotalSeconds + (index - secIndx)) >= minTotalSeconds) && ((currTotalSeconds + (index - secIndx)) <= maxTotalSeconds))
                    {
                        tMapList.insert(index, QString("%1").arg(index, 2, 10, QLatin1Char('0')));
                    }
                }
            }

            maplist = tMapList;
        }

        quint8 currentKey;
        switch(currClickedBox)
        {
            case HOUR_BOX:
                currentKey = hrsIndx;
                break;

            case MINUTE_BOX:
                currentKey = minIndx;
                break;

            case SEC_BOX:
                currentKey = secIndx;
                break;

            default:
                currentKey = 0;
                break;
        }

        m_dropDownList = new DropDownList(this->x() + listStartX - SCALE_WIDTH(12),
                                          listStartY,
                                          (hourRect.width()+SCALE_WIDTH(15)),
                                          maplist,
                                          currentKey,
                                          parentWidget(),
                                          m_maxElementWithoutScroll);
        connect(m_dropDownList,
                SIGNAL(destroyed()),
                this,
                SLOT(slotDropListDestroyed()));
        connect(m_dropDownList,
                SIGNAL(sigValueChanged(quint8,QString)),
                this,
                SLOT(slotValueChanged(quint8,QString)));
    }
    else
    {
        unloadList();
        loadList(maplist, maxElementOnList, listStartX);
    }
}

void ClockSpinbox::mousePressEvent(QMouseEvent * event)
{
    if(hourRect.contains (event->pos ())  && (event->button() == m_leftMouseButton))
    {
        if( currClickedBox != HOUR_BOX )
        {
            hrsSelect();
            loadList(m_hourList, m_hourList.size(), hourRect.x());
        }
        else
        {
            hrsText->changeColor ( NORMAL_FONT_COLOR );
            changeImage (CLK_SPIN_HOUR,IMAGE_TYPE_NORMAL);
            currClickedBox = MAX_TIME_BOX;
        }
    }

    else if (minuteRect.contains (event->pos ())   && (event->button() == m_leftMouseButton))
    {
        if( currClickedBox != MINUTE_BOX )
        {
            minSelect();
            loadList(m_minuteList, m_minuteList.size(), minuteRect.x());
        }
        else
        {
            minuteText->changeColor ( NORMAL_FONT_COLOR );
            changeImage (CLK_SPIN_MIN,IMAGE_TYPE_NORMAL);
            currClickedBox = MAX_TIME_BOX;
        }
    }
    else if (secRect.contains (event->pos ()) && (type == CLK_SPINBOX_With_SEC)   && (event->button() == m_leftMouseButton))
    {
        if( currClickedBox != SEC_BOX )
        {
            secSelect();
            loadList(m_secondList, m_secondList.size(), secRect.x());
        }
        else
        {
            secText->changeColor ( NORMAL_FONT_COLOR );
            changeImage (CLK_SPIN_SEC,IMAGE_TYPE_NORMAL);
            currClickedBox = MAX_TIME_BOX;
        }
    }

    update ();
}

void ClockSpinbox::mouseMoveEvent(QMouseEvent * event)
{
    if(bgRect.contains (event->pos ()))
    {
        if (! (this->hasFocus ()))
        {
            forceActiveFocus ();
            emit sigUpdateCurrentElement(m_indexInPage);
        }
        else
        {
            selectControl ();
        }
    }

    update ();
}

void ClockSpinbox::wheelEvent(QWheelEvent* event)
{
    if (currClickedBox >= MAX_TIME_BOX)
    {
        return;
    }

    if (currClickedBox == HOUR_BOX)
    {
        if (hourRect.contains(event->pos()))
        {
            if (event->delta() < 0)
            {
                incHrs();
            }
            else
            {
                decHrs();
            }
        }
    }
    else if (currClickedBox == MINUTE_BOX)
    {
        if (minuteRect.contains(event->pos()))
        {
            if (event->delta() < 0)
            {
                incMin();
            }
            else
            {
                decMin();
            }
        }
    }
    else if (currClickedBox == SEC_BOX)
    {
        if (secRect.contains(event->pos()))
        {
            if (event->delta() < 0)
            {
                incSec();
            }
            else
            {
                decSec();
            }
        }
    }
}

void ClockSpinbox::assignValue(QString hour, QString minute)
{
    hrsIndx = QString(hour).toInt ();
    hrsText->changeText (hour);

    minIndx = QString(minute).toInt ();
    minuteText->changeText (minute);

    if(isMinMaxReq)
    {
        currTotalSeconds = (hrsIndx*3600) + (minIndx*60) + secIndx;
    }

    update ();
}

void ClockSpinbox::assignValue(quint32 hour, quint32 minute)
{
    hrsIndx = hour;
    hrsText->changeText (QString("%1").arg(hrsIndx, 2, 10, QLatin1Char('0')));

    minIndx = minute;
    minuteText->changeText (QString("%1").arg(minIndx, 2, 10, QLatin1Char('0')));

    if(isMinMaxReq)
    {
        currTotalSeconds = (hrsIndx*3600) + (minIndx*60) + secIndx;
    }

    update ();
}

void ClockSpinbox::assignValue(quint32 hour, quint32 minute, quint32 sec)
{
    hrsIndx = hour;
    hrsText->changeText (QString("%1").arg(hrsIndx, 2, 10, QLatin1Char('0')));

    minIndx = minute;
    minuteText->changeText (QString("%1").arg(minIndx, 2, 10, QLatin1Char('0')));

    secIndx = sec;
    secText->changeText (QString("%1").arg(secIndx, 2, 10, QLatin1Char('0')));

    if(isMinMaxReq)
    {
        currTotalSeconds = (hrsIndx*3600) + (minIndx*60) + secIndx;
    }

    update ();
}

void ClockSpinbox::assignValue(QString hour, QString minute, QString sec)
{
    hrsIndx = QString(hour).toInt ();
    hrsText->changeText (hour);

    minIndx = QString(minute).toInt ();
    minuteText->changeText (minute);

    secIndx = QString(sec).toInt ();
    secText->changeText (sec);

    if(isMinMaxReq)
    {
        currTotalSeconds = (hrsIndx*3600) + (minIndx*60) + secIndx;
    }

    update ();
}

void ClockSpinbox:: currentValue(quint32 &currentHour, quint32 &currentMin, quint32 &currentSec)
{
    currentHour = hrsIndx;
    currentMin = minIndx;
    currentSec = secIndx;
}

void ClockSpinbox::currentValue(quint32 &currentHour,quint32 &currentMin)
{
    currentHour = hrsIndx;
    currentMin = minIndx;
}

void ClockSpinbox::incHrs()
{
    if(isMinMaxReq)
    {
        if(((currTotalSeconds + 3600) >= minTotalSeconds ) && ((currTotalSeconds + 3600) <= maxTotalSeconds ))
        {
            hrsIndx = (hrsIndx == MAX_HOUR) ? 0 : hrsIndx+1;
            currTotalSeconds = (hrsIndx*3600) + (minIndx*60) + secIndx;
            emit sigTotalCurrentSec(currTotalSeconds, m_indexInPage);
        }
    }
    else
    {
        hrsIndx = (hrsIndx == MAX_HOUR) ? 0 : hrsIndx+1;
    }

    updateText();
}

void ClockSpinbox::decHrs()
{
    if(isMinMaxReq)
    {
        if(((currTotalSeconds - 3600) >= minTotalSeconds ) && ((currTotalSeconds - 3600) <= maxTotalSeconds ))
        {
            if( hrsIndx == 0)
            {
                hrsIndx = MAX_HOUR ;
                currTotalSeconds = (hrsIndx*3600) + (minIndx*60) + secIndx;
            }
            else
            {
                hrsIndx--;
                currTotalSeconds = (hrsIndx*3600) + (minIndx*60) + secIndx;
            }
            emit sigTotalCurrentSec(currTotalSeconds, m_indexInPage);
        }
    }
    else
    {
        hrsIndx = (hrsIndx == 0) ? MAX_HOUR : hrsIndx-1;
    }

    updateText();
}

void ClockSpinbox::incMin()
{
    if(isMinMaxReq)
    {
        if(((currTotalSeconds  + 60 )>= minTotalSeconds ) && ((currTotalSeconds + 60) <= maxTotalSeconds ))
        {
            if( minIndx == ( MAX_MIN_SEC-countValue ))
            {
                minIndx = 0;
                currTotalSeconds = (hrsIndx*3600) + (minIndx*60) + secIndx;
                incHrs ();
            }
            else
            {
                if(countValue > 1 )
                {
                    countValue = (minIndx % countValue) == 0 ? countValue - (minIndx % countValue) : countValue;
                }
                minIndx += countValue;
                currTotalSeconds = (hrsIndx*3600) + (minIndx*60) + secIndx;
            }

            emit sigTotalCurrentSec(currTotalSeconds, m_indexInPage);
        }
    }
    else
    {
        if( minIndx == (MAX_MIN_SEC-countValue ))
        {
            minIndx = 0;
            incHrs ();
        }
        else
        {
            if(countValue > 1 )
            {
                countValue = (minIndx % countValue)  == 0 ? countValue - (minIndx % countValue) : countValue;
            }
            minIndx += countValue;
        }
    }

    updateText();
}

void ClockSpinbox::decMin()
{
    if(isMinMaxReq)
    {
        if(((currTotalSeconds - 60) >= minTotalSeconds ) && ((currTotalSeconds - 60) <= maxTotalSeconds ))
        {
            if( minIndx == 0)
            {
                minIndx = (MAX_MIN_SEC - countValue) ;
                currTotalSeconds = (hrsIndx*3600) + (minIndx*60) + secIndx;
                decHrs ();
            }
            else
            {
                if(countValue > 1 )
                {
                    countValue = (minIndx % countValue)  == 0 ? countValue - (minIndx % countValue) : countValue;
                }
                minIndx -= countValue;
                currTotalSeconds = (hrsIndx*3600) + (minIndx*60) + secIndx;
            }

            emit sigTotalCurrentSec(currTotalSeconds, m_indexInPage);
        }
    }
    else
    {
        if(minIndx == 0)
        {
            minIndx = (MAX_MIN_SEC - countValue);
            decHrs ();
        }
        else
        {
            if(countValue > 1 )
            {
                countValue = (minIndx % countValue)  == 0 ? countValue - (minIndx % countValue) : countValue;
            }
            minIndx -= countValue;
        }
    }

    updateText();
}

void ClockSpinbox::incSec()
{
    if(isMinMaxReq)
    {
        if(((currTotalSeconds + 1) >= minTotalSeconds ) && ((currTotalSeconds + 1) <= maxTotalSeconds ))
        {
            if( secIndx == (MAX_MIN_SEC - countValue))
            {
                secIndx = 0;
                currTotalSeconds = (hrsIndx*3600) + (minIndx*60) + secIndx;
                incMin ();
            }
            else
            {
                secIndx++;
                currTotalSeconds = (hrsIndx*3600) + (minIndx*60) + secIndx;
            }

            emit sigTotalCurrentSec(currTotalSeconds, m_indexInPage);
        }
    }
    else
    {
        if( secIndx == (MAX_MIN_SEC - countValue))
        {
            secIndx = 0;
            incMin ();
        }
        else
        {
            secIndx++;
        }
    }

    updateText();
}

void ClockSpinbox::decSec()
{
    if(isMinMaxReq)
    {
        if(((currTotalSeconds - 1) >= minTotalSeconds ) && ((currTotalSeconds - 1) <= maxTotalSeconds ))
        {
            if( secIndx == 0)
            {
                secIndx = MAX_MIN_SEC - countValue;
                currTotalSeconds = (hrsIndx*3600) + (minIndx*60) + secIndx;
                decMin ();
            }
            else
            {
                secIndx--;
                currTotalSeconds = (hrsIndx*3600) + (minIndx*60) + secIndx;
            }

            emit sigTotalCurrentSec(currTotalSeconds, m_indexInPage);
        }
    }
    else
    {
        if( secIndx == 0)
        {
            secIndx = MAX_MIN_SEC - countValue;
            decMin ();
        }
        else
        {
            secIndx--;
        }
    }

    updateText();
}

void ClockSpinbox::updateText()
{
    hrsText->changeText (QString("%1").arg(hrsIndx, 2, 10, QLatin1Char('0')));
    minuteText->changeText (QString("%1").arg(minIndx, 2, 10, QLatin1Char('0')));

    if(type == CLK_SPINBOX_With_SEC)
    {
        secText->changeText (QString("%1").arg(secIndx, 2, 10, QLatin1Char('0')));
    }

    update ();
}

void ClockSpinbox::forceActiveFocus()
{
    this->setFocus();
}

void ClockSpinbox::selectControl()
{
    if(m_isEnabled == true)
    {
        changeImage(CLK_SPIN_MAIN,IMAGE_TYPE_MOUSE_HOVER);
    }
}

void ClockSpinbox::deSelectControl()
{
    if(m_isEnabled == true)
    {
        switch(currClickedBox)
        {
            case HOUR_BOX:
                changeImage (CLK_SPIN_HOUR,IMAGE_TYPE_NORMAL);
                hrsText->changeColor ( NORMAL_FONT_COLOR );
                break;

            case MINUTE_BOX:
                changeImage (CLK_SPIN_MIN, IMAGE_TYPE_NORMAL);
                minuteText->changeColor ( NORMAL_FONT_COLOR );
                break;

            case SEC_BOX:
                changeImage (CLK_SPIN_SEC, IMAGE_TYPE_NORMAL);
                secText->changeColor ( NORMAL_FONT_COLOR );
                break;

            default:
                break;
        }
        currClickedBox = MAX_TIME_BOX;
        changeImage(CLK_SPIN_MAIN,IMAGE_TYPE_NORMAL);
    }
}

void ClockSpinbox::focusInEvent(QFocusEvent *)
{
    selectControl();
}

void ClockSpinbox::focusOutEvent(QFocusEvent *)
{
    deSelectControl();
}

void ClockSpinbox::takeRightKeyAction()
{
    if(type == CLK_SPINBOX_With_SEC)
    {
        switch(currClickedBox)
        {
            case HOUR_BOX:
                currClickedBox = MINUTE_BOX;
                minSelect ();
                break;

            case MINUTE_BOX:
                currClickedBox = SEC_BOX;
                secSelect ();
                break;

            case SEC_BOX:
                currClickedBox = HOUR_BOX;
                hrsSelect ();
                break;

            default:
                currClickedBox = HOUR_BOX;
                hrsSelect ();
        }
    }
    else
    {
        switch(currClickedBox)
        {
            case HOUR_BOX:
                currClickedBox = MINUTE_BOX;
                minSelect ();
                break;

            case MINUTE_BOX:
                currClickedBox = HOUR_BOX;
                hrsSelect ();
                break;

            default:
                currClickedBox = HOUR_BOX;
                hrsSelect ();
        }
    }
    update ();
}

void ClockSpinbox:: takeLeftKeyAction()
{
    if(type == CLK_SPINBOX_With_SEC)
    {
        switch(currClickedBox)
        {
            case HOUR_BOX:
                currClickedBox = SEC_BOX;
                secSelect ();
                break;

            case MINUTE_BOX:
                currClickedBox = HOUR_BOX;
                hrsSelect ();
                break;

            case SEC_BOX:
                currClickedBox = MINUTE_BOX;
                minSelect ();
                break;

            default:
                currClickedBox = HOUR_BOX;
                hrsSelect ();
        }
    }
    else
    {
        switch(currClickedBox)
        {
            case HOUR_BOX:
                currClickedBox = MINUTE_BOX;
                minSelect ();
                break;

            case MINUTE_BOX:
                currClickedBox = HOUR_BOX;
                hrsSelect ();
                break;

            default:
                currClickedBox = HOUR_BOX;
                hrsSelect ();
        }
    }
    update ();
}

void ClockSpinbox::takeDownKeyAction ()
{
    switch(currClickedBox)
    {
        case HOUR_BOX:
            incHrs ();
            break;

        case MINUTE_BOX:
            incMin ();
            break;

        case SEC_BOX:
            incSec ();
            break;

        default:
            break;
    }
}


void ClockSpinbox::takeUpKeyAction ()
{
    switch(currClickedBox)
    {
        case HOUR_BOX:
            decHrs ();
            break;

        case MINUTE_BOX:
            decMin ();
            break;

        case SEC_BOX:
            decSec ();
            break;

        default:
            break;
    }
}

void ClockSpinbox:: takeEnterKeyAction()
{
    forceActiveFocus ();

    if(currClickedBox == MAX_TIME_BOX)
    {
        currClickedBox = HOUR_BOX;
        changeImage (CLK_SPIN_MAIN,IMAGE_TYPE_MOUSE_HOVER);
        hrsSelect ();
        enterMode = true;
    }
    else
    {
        switch(currClickedBox)
        {
            case HOUR_BOX:
                changeImage (CLK_SPIN_HOUR,IMAGE_TYPE_NORMAL);
                hrsText->changeColor ( NORMAL_FONT_COLOR );
                break;

            case MINUTE_BOX:
                changeImage (CLK_SPIN_MIN, IMAGE_TYPE_NORMAL);
                minuteText->changeColor ( NORMAL_FONT_COLOR );
                break;

            case SEC_BOX:
                changeImage (CLK_SPIN_SEC, IMAGE_TYPE_NORMAL);
                secText->changeColor ( NORMAL_FONT_COLOR );
                break;

            default:
                hrsSelect();
                break;
        }

        currClickedBox = MAX_TIME_BOX;
        changeImage (CLK_SPIN_MAIN,IMAGE_TYPE_NORMAL);
        enterMode = false;
    }
    update();
}

void ClockSpinbox::navigationKeyPressed(QKeyEvent *event)
{
    if(false == m_catchKey)
    {
        return;
    }

    if(false == enterMode)
    {
        QWidget::keyPressEvent(event);
        return;
    }

    switch(event->key())
    {
        case Qt::Key_Left:
            event->accept();
            takeLeftKeyAction();
            break;

        case Qt::Key_Right:
            event->accept();
            takeRightKeyAction();
            break;

        case Qt::Key_Up:
            event->accept();
            takeUpKeyAction();
            break;

        case Qt::Key_Down:
            event->accept();
            takeDownKeyAction();
            break;

        default:
            QWidget::keyPressEvent(event);
            break;
    }
}

void ClockSpinbox::enterKeyPressed(QKeyEvent *event)
{
    if(m_catchKey)
    {
        event->accept();
        takeEnterKeyAction();
    }
}

void ClockSpinbox::setIsEnabled(bool isEnable)
{
    if(m_isEnabled == isEnable)
    {
        return;
    }

    m_isEnabled = isEnable;
    this->setEnabled (m_isEnabled);

    if(isEnable == true)
    {
        changeImage(CLK_SPIN_MAIN, IMAGE_TYPE_NORMAL);
        hrsText->changeColor (NORMAL_FONT_COLOR);
        minuteText->changeColor (NORMAL_FONT_COLOR);
        sepreator1->changeColor (NORMAL_FONT_COLOR);
        if(type == CLK_SPINBOX_With_SEC)
        {
            secText->changeColor (NORMAL_FONT_COLOR);
            sepreator2->changeColor (NORMAL_FONT_COLOR);
        }
    }
    else
    {
        changeImage(CLK_SPIN_MAIN, IMAGE_TYPE_DISABLE);
        hrsText->changeColor (DISABLE_FONT_COLOR);
        minuteText->changeColor (DISABLE_FONT_COLOR);
        sepreator1->changeColor (DISABLE_FONT_COLOR);
        if(type == CLK_SPINBOX_With_SEC)
        {
            secText->changeColor (DISABLE_FONT_COLOR);
            sepreator2->changeColor (DISABLE_FONT_COLOR);
        }
    }
    update();
}

void ClockSpinbox::setMinTotalSeconds (quint16 minSec)
{
    minTotalSeconds = minSec;
}

void ClockSpinbox::setMaxTotalSeconds (quint16 maxSec)
{
    maxTotalSeconds = maxSec;
}

void ClockSpinbox::unloadList()
{
    if(m_dropDownList != NULL)
    {
        disconnect(m_dropDownList,
                   SIGNAL(destroyed()),
                   this,
                   SLOT(slotDropListDestroyed()));
        disconnect(m_dropDownList,
                   SIGNAL(sigValueChanged(quint8,QString)),
                   this,
                   SLOT(slotValueChanged(quint8,QString)));
        DELETE_OBJ(m_dropDownList);
    }
}

void ClockSpinbox::slotUnloadDropList()
{
    unloadList();

    disconnect (m_dropDownInVisibleWidget,
                SIGNAL(sigMouseClick()),
                this,
                SLOT(slotUnloadDropList()));
    DELETE_OBJ(m_dropDownInVisibleWidget);

    forceActiveFocus();
}

void ClockSpinbox::slotDropListDestroyed()
{
    disconnect(m_dropDownList,
               SIGNAL(destroyed()),
               this,
               SLOT(slotDropListDestroyed()));
    m_dropDownList = NULL;

    disconnect (m_dropDownInVisibleWidget,
                SIGNAL(sigMouseClick()),
                this,
                SLOT(slotUnloadDropList()));
    DELETE_OBJ(m_dropDownInVisibleWidget);

    forceActiveFocus();
}

void ClockSpinbox::slotValueChanged(quint8 key, QString)
{
    disconnect(m_dropDownList,
               SIGNAL(sigValueChanged(quint8,QString)),
               this,
               SLOT(slotValueChanged(quint8,QString)));

    if (currClickedBox == HOUR_BOX)
    {
        hrsIndx = key;
    }
    else if (currClickedBox == MINUTE_BOX)
    {
        minIndx = key;
    }
    else if (currClickedBox == SEC_BOX)
    {
        secIndx = key;
    }

    if (isMinMaxReq)
    {
        currTotalSeconds = (hrsIndx*3600) + (minIndx*60) + secIndx;
        emit sigTotalCurrentSec(currTotalSeconds, m_indexInPage);
    }

    updateText();
}
