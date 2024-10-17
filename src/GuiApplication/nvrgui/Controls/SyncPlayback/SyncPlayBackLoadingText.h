#ifndef SYNCPLAYBACKLOADINGTEXT_H
#define SYNCPLAYBACKLOADINGTEXT_H

#include <QWidget>
#include "Controls/TextLabel.h"
#include "Controls/Rectangle.h"

class SyncPlayBackLoadingText : public QWidget
{
private:
    TextLabel* m_loadingText;
    Rectangle* m_textBackgroungRectangle;
    Rectangle* m_rightBackground;
    Rectangle* m_bottomBackground;

    quint16 m_loadingTextAreaWidth, m_loadingTextAreaHeight;
public:
    SyncPlayBackLoadingText(quint16 loadingTextAreaWidth,
                            quint16 loadingTextAreaHeight,
                            QWidget* parent = 0);
    ~SyncPlayBackLoadingText();
};

#endif // SYNCPLAYBACKLOADINGTEXT_H
