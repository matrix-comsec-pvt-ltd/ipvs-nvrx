#ifndef PICKLIST_H
#define PICKLIST_H

#include "Controls/PickListLoader.h"
#include "NavigationControl.h"
#include "Bgtile.h"
#include "TextLabel.h"
#include "ReadOnlyElement.h"
#include "Controls/InvisibleWidgetCntrl.h"

#define PICKLIST_IMAGE_PATH         ":/Images_Nvrx/ControlButtons/PicklistButton/"

class PickList : public BgTile, public NavigationControl
{
    Q_OBJECT
private:
    PickListLoader* m_pickListLoader;

    QString m_label, m_imageSource;
    QRect m_imageRect;
    IMAGE_TYPE_e m_imageType;
    QPixmap m_picklistIcon;
    QMap<quint8, QString> m_optionList;
    int m_pixelAlign;
    int m_rectWidth, m_rectHeight;
    QString m_pickListHeading;
    InvisibleWidgetCntrl* m_inVisibleWidget;
    TextLabel* m_textLabel;
    ReadOnlyElement* m_readOnlyElement;
    bool m_isOuterBorderNeeded;
    quint8 m_newSelectedKey, m_selectedKey;
    bool m_isPickListLoadOnResponse;
    bool m_loaderDeleteOnCancelClick;
    QString m_currentValue;
    quint32 m_leftMarginFromCenter;

public:
    PickList(int startX,
             int startY,
             int tileWidth,
             int tileHeight,
             int width,
             int height,
             QString label,
             QMap<quint8, QString> optionList,
             quint8 selectedKey,
             QString heading,
             QWidget *parent = 0,
             BGTILE_TYPE_e tileType = COMMON_LAYER,
             int pixelAlign = -1,
             int indexInPage = 0,
             bool isEnabled = true,
             bool isOuterBorderNeeded = true,
             bool isPickListLoadOnResponse = false,
             quint32 leftMarginFromCenter = 0);
    ~PickList();

    void setGeometryForElements();
    void changeImage(IMAGE_TYPE_e imageType);
    void changeValue(int selectedIndex);
    quint8 getCurrentValue () const;
    void selectControl();
    void deSelectControl();

    void changeOptionList(QMap<quint8, QString> optionList, quint32 selectedKey = 0, bool updateCurrentValueFlag = true);
    void loadPickList();
    void loadPickListOnResponse(QMap<quint8, QString> optionList);
    void loadPickListOnResponse(QMap<quint8, QString> optionList, qint32 selectKey);

    QString getCurrentPickStr() const;
    void setValue(QString str);
    void deletePicklistLoader();

    void forceActiveFocus();
    void setIsEnabled(bool isEnable);

    void takeEnterKeyAction();

    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    virtual void enterKeyPressed(QKeyEvent *event);
    void hideEvent (QHideEvent *event);
    void showEvent (QShowEvent *event);

signals:
    void sigUpdateCurrentElement(int index);
    void sigValueChanged(quint8 key, QString value, int indexInPage);
    void sigButtonClick(int);
    void sigPicklistLoad(quint8 index);
    void sigShowHideToolTip(int index, bool toShowTooltip);


public slots:
    void slotValueChanged(quint8 key, QString value, bool isCancleClick);
    void slotPickListDestroyed();
    void slotDeletePickList();
};

#endif // PICKLIST_H
