#ifndef ELEMENTHEADING_H
#define ELEMENTHEADING_H

#include <QWidget>
#include "Controls/Bgtile.h"
#include "Controls/TextLabel.h"
#include "EnumFile.h"

class ElementHeading : public BgTile
{
	TextLabel*	eleheading;

public:
	ElementHeading(qint32 startX,
					qint32 startY,
					qint32 width,
					qint32 height,
					QString headingText,
					BGTILE_TYPE_e bgtype,
					QWidget *parent = 0,
					bool isCenter = true ,
					qint32 leftMargin = 0,
					quint32 fontSize = NORMAL_FONT_SIZE,
					bool restrictWidth = false,
                    QString iTextColor = HIGHLITED_FONT_COLOR,
                   bool isBold = false);
	~ElementHeading();
};

#endif // ELEMENTHEADING_H
