    #include "ElementHeading.h"

ElementHeading::ElementHeading(qint32 startX,
								qint32 startY,
								qint32 width,
								qint32 height,
								QString headingText,
								BGTILE_TYPE_e bgtype,
								QWidget *parent,
								bool isCenter,
								qint32 leftMargin,
								quint32 fontSize, bool restrictWidth,
                                QString iTextColor,
                                bool isBold) :
    BgTile(startX,startY,width,height,bgtype,parent)
{
    quint16 labelHeight = 0, translatedlabelWidth = 0;
    quint16 labelWidth = 0;
    QFont labelFont;
    int verticalOffSet = 0;

    labelFont = TextLabel::getFont (NORMAL_FONT_FAMILY, fontSize,isBold);
    labelWidth = QFontMetrics(labelFont).width (headingText);
    translatedlabelWidth = QFontMetrics(labelFont).width(QApplication::translate(QT_TRANSLATE_STR, headingText.toUtf8().constData()));
    labelHeight = QFontMetrics(labelFont).height ();

    switch(m_bgTileType)
    {
    case TOP_TABLE_LAYER:
    case TOP_LAYER:
        verticalOffSet = (TOP_MARGIN / 2);
        break;

    case BOTTOM_TABLE_LAYER:
    case BOTTOM_LAYER:
        verticalOffSet = -(TOP_MARGIN / 2);
        break;

    default:
        break;
    }

    if(isCenter == true)
    {
        if(restrictWidth)
        {
            labelWidth = (translatedlabelWidth > m_width)? (m_width - SCALE_WIDTH(20)) : translatedlabelWidth;
        }
        eleheading = new TextLabel(((m_width - labelWidth)/2),
                                   (height - labelHeight)/2 + verticalOffSet,
                                   fontSize, headingText,
                                   this,iTextColor, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                   0, 0, labelWidth);
        eleheading->SetBold(isBold);
    }
    else
    {
        if(restrictWidth)
        {
            translatedlabelWidth = (translatedlabelWidth > (width - SCALE_WIDTH(5))) ? (width - SCALE_WIDTH(5)) : (translatedlabelWidth);
        }
        else
        {
            translatedlabelWidth = labelWidth;
        }
        eleheading = new TextLabel(leftMargin,
                                   (height - labelHeight)/2 + verticalOffSet,
                                   fontSize, headingText,
                                   this,iTextColor, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                   0, 0, translatedlabelWidth);
        eleheading->SetBold(isBold);
    }
}

ElementHeading:: ~ElementHeading()
{
    delete eleheading;
}
