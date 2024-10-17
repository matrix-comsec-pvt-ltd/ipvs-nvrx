#ifndef OPTIONSELECTBUTTON_H
#define OPTIONSELECTBUTTON_H

#include "TextLabel.h"
#include "NavigationControl.h"
#include "Bgtile.h"

#define OPTION_SELECTION_BOTTON_IMG_PATH    ":/Images_Nvrx/OptionSelectionButton/"

#define DEFAULT_PIXEL_SIZE                  -1

const QString imageFolderPath[] = {"CheckBox/",
                                   "RadioButton/"
                                  };

const QString imageStatePath[] = {"OffState/",
                                  "OnState/"
                                 };

typedef enum
{
    CHECK_BUTTON_INDEX = 0,
    RADIO_BUTTON_INDEX,
    MAX_OTHER_BUTTON
}OPTION_SELECTION_BUTTON_TYPE_e;

typedef enum
{
    OFF_STATE = 0,
    ON_STATE,
    MAX_STATE
}OPTION_STATE_TYPE_e;

typedef enum
{
    MX_OPTION_TEXT_TYPE_LABEL = 0,
    MX_OPTION_TEXT_TYPE_SUFFIX,
    MAX_MX_OPTION_TEXT_TYPE

}MX_OPTION_TEXT_TYPE_e;


class OptionSelectButton : public BgTile, public NavigationControl
{
    Q_OBJECT
private :
    QString m_label, m_suffix, m_imageSource;
    OPTION_STATE_TYPE_e m_currentState;
    int m_textLabelSize, m_textSuffixSize;
    OPTION_SELECTION_BUTTON_TYPE_e m_buttonType;
    TextLabel* m_textLabel;
    TextLabel* m_textSuffix;
    QRect m_imageRect;
    QPixmap m_iconImage;
    IMAGE_TYPE_e m_currentImageType;
    QString m_textFontColor, m_suffixFontColor;
    int m_pixelAlign;
    int maxPossibleWidth;
    bool m_multiple;
    quint32 m_leftMarginfromCenter = 0;


public:
    OptionSelectButton(int startX,
                       int startY,              //PAINT LEFT TO RIGHT
                       int width,
                       int height,
                       OPTION_SELECTION_BUTTON_TYPE_e buttonType,
                       QWidget *parent = 0,
                       BGTILE_TYPE_e tileType = COMMON_LAYER,
                       QString label = "",
                       QString suffix = "",
                       int pixelAlign = -1,
                       int indexInPage = 0,
                       bool isEnabled = true,
                       int suffixPixelSize = -1,
                       QString suffixFontColor = SUFFIX_FONT_COLOR,
                       bool multipleElemtInRow = false,  //multipleElemtInRow = false : Whole maxWidth is utilized (default)
                       quint32 leftMarginfromCenter = 0); 

    OptionSelectButton(int startX,
                       int startY,
                       int width,                       //PAINT RIGHT TO LEFT
                       int height,
                       OPTION_SELECTION_BUTTON_TYPE_e buttonType,
                       QString text,
                       QWidget *parent = 0,
                       BGTILE_TYPE_e tileType = COMMON_LAYER,
                       int pixelAlign = -1,
                       MX_OPTION_TEXT_TYPE_e textType = MX_OPTION_TEXT_TYPE_LABEL,
                       int textPixelSize = DEFAULT_PIXEL_SIZE,
                       int indexInPage = 0,
                       bool isEnabled = true,
                       QString fontColor = NORMAL_FONT_COLOR,
                       bool multipleElemtInRow = false,
                       quint32 leftMarginFromCenter = 0);

    ~OptionSelectButton();

    void setGeometryForElements();
    void changeImage(IMAGE_TYPE_e imageType);
    void changeState(OPTION_STATE_TYPE_e state);
    void changeFontColor(MX_OPTION_TEXT_TYPE_e textType, QString fontColor);
    void changeLabel(MX_OPTION_TEXT_TYPE_e textType, QString string);
    OPTION_STATE_TYPE_e getCurrentState();
    void selectControl();
    void deSelectControl();
	quint8 getButtonIndex(void);

    void forceActiveFocus();
    void setIsEnabled(bool isEnable);

    void takeEnterKeyAction();

    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    virtual void enterKeyPressed(QKeyEvent *event);

signals:
    void sigButtonClicked(OPTION_STATE_TYPE_e currentState, int indexInPage);
    void sigUpdateCurrentElement(int index);
};

#endif // OPTIONSELECTBUTTON_H
