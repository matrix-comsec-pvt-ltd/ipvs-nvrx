#ifndef INFOPAGE_H
#define INFOPAGE_H

#include <QWidget>
#include "Controls/Rectangle.h"
#include "Controls/CnfgButton.h"
#include "Controls/TextLabel.h"
#include "Controls/BackGround.h"
#include "NavigationControl.h"
#include "Controls/ImageControls/Image.h"

#define CONFORMATION_MSG        "Do you want to save the changes made?"
#define CONFORMATION_BTN_YES    "Yes"
#define CONFORMATION_BTN_NO     "No"
#define CONFORMATION_BTN_OK     "OK"
#define CONFORMATION_BTN_CANCEL "Cancel"

typedef enum{

    INFO_ADVANCE_DETAILS,
    INFO_CONFIG_PAGE,
    INFO_EVENTLOG,
    INFO_LIVEVIEW,
    INFO_MANAGE,
    INFO_PLABACK_SEARCH,
    INFO_PRESET_TOUR_SCHEDULE,
    INFO_LOGIN,
    MAX_INFO_PAGE_TYPE
}INFO_PAGE_TYPE_e;

typedef enum
{
    INFO_OK_BTN,
    INFO_CANCEL_BTN,

    MAX_INFO_ELEMENTS
}INFO_MAX_ELEMENTS_e;

class InfoPage : public KeyBoard
{
    Q_OBJECT

private:
    quint16 m_startX, m_startY;
    quint16 m_height, m_width ;
    quint32 truncateWidth;

    bool m_isBackGroundVisible;
    QString label;
    BG_TYPE_e bgType;
    bool isCancelBtnIn;
    BackGround *bgRect;
    BackGround *m_backGround;
    TextLabel *labelText;
    TextLabel *warningText;
    CnfgButton *okButton;
    CnfgButton *cancelButton;

    QString imageSource;
    Image*  infoPageImage;
    Image*  warningImage;

    QRect mainRectHide;
    QRect complRectHide;

    int m_currElement;
    NavigationControl *m_elementList[MAX_INFO_ELEMENTS];
    QString m_firstBtnStr;
    QString m_secondBtnStr;

public:
    InfoPage(quint16 startX,
             quint16 startY,
             quint16 width,
             quint16 height,
             INFO_PAGE_TYPE_e infoPageType = MAX_INFO_PAGE_TYPE,
             QWidget *parent = 0,
             bool canceBtnNeed = false,
             bool isBackGroundVisible = true,
             quint32 msgWidthMax = SCALE_WIDTH(442));
    ~InfoPage();

    void createDefaultComponent();
    void destroyDefaultComponent();
    void changeText(QString str);
    QString getText(void);
    void showCancelBtn( bool flag);
    void loadInfoPage(QString infoMsg, bool isCancelButton = false,
                      bool isFocusToCancelButton = false,
                      QString warningMsg = "",
                      QString firstBtnStr = CONFORMATION_BTN_OK,
                      QString SecondBtnStr = CONFORMATION_BTN_CANCEL);
    void loadInfoPageNoButton(QString infoMsg, bool isOkButton = false, bool isCancelButton = false);
    void unloadInfoPage();
    void changeButtonsString(QString firstBtnStr, QString secondBtnStr);
    void resetButtonString();
    void takeLeftKeyAction();
    void takeRightKeyAction();
    void updateGeometry();
    void showEvent (QShowEvent *event);
    void forceActiveFocus();

    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);
    virtual void insertKeyPressed(QKeyEvent *event);
    void mousePressEvent (QMouseEvent *);
    void mouseMoveEvent (QMouseEvent *);

    QFont getFont(QString fontFamily, quint8 fontSize, bool isBold, bool isSetUnderLine);

    QString manipulateValidationMsg(QString str);

signals:
    void sigInfoPageCnfgBtnClick(int index);

public slots:
    void slotUpdateCurrentElement (int index);
    void slotCnfgBtnClicked (int index);

};

#endif // INFOPAGE_H
