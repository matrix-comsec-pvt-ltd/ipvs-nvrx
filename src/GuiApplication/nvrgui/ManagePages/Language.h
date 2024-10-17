#ifndef LANGUAGE_H
#define LANGUAGE_H

#include "ManageMenuOptions.h"
#include "Controls/DropDown.h"

class Language : public ManageMenuOptions
{
    Q_OBJECT

private:
    DropDown*   m_languageDropDownBox;
    QString     selectedLangStr;

public:
    explicit Language(QString devName, QWidget *parent = 0);
    ~Language();
    void getLanguage(void);
    void getUserPreferredLanguage(void);
    void setUserPreferredLanguage(void);
    void handleInfoPageMessage(int index);
    void processDeviceResponse(DevCommParam *param, QString devName);

signals:
    void sigLanguageCfgChanged(QString str);

public slots:
    void slotDropDownBoxValueChanged(QString string, quint32);
};

#endif // LANGUAGE_H
