#ifndef UPLOADIMAGE_H
#define UPLOADIMAGE_H

#include <QObject>
#include "Controls/ConfigPageControl.h"
#include "Controls/SpinBox.h"
#include "Controls/TextBox.h"
#include "Controls/MessageBox.h"
#include "DataStructure.h"
#include "Controls/DropDown.h"

typedef enum {
    UPLD_CAMERA_NAME_SPINBOX_CTRL,
    UPLD_UPLOAD_SPINBOX_CTRL,
    UPLD_EMAIL_TEXTBOX,
    UPLD_SUBJECT_TEXTBOX,
    UPLD_MSGBOX,
    UPLD_IMAGE_RATE_SPINBOX,

    MAX_UPLOAD_IMAGE_CTRL
}UPLOAD_IMAGE_CTRL_e;


class UploadImage : public ConfigPageControl
{
    Q_OBJECT

    QString     resolution;
    quint8      currentCameraIndex;

    DropDown*      cameraNameDropDownBox;
    QMap<quint8, QString> cameraNameList;

    DropDown*      uploadListDropDownBox;

    TextboxParam*  emailTextBoxParam;
    TextBox*       emailTextBox;

    TextboxParam*  subjectTextBoxParam;
    TextBox*       subjectTextBox;

    MessageBox*    msgBox;
    TextLabel*     emailNoteLabel;

    DropDown*      imageRateDropDownBox;

public:
    explicit UploadImage(QString deviceName,QWidget *parent = 0,
                          DEV_TABLE_INFO_t *devTabInfo = NULL);
    ~UploadImage();

    void createDefaultComponent();
    void fillCameraList();
    void getConfig ();
    void defaultConfig ();
    void saveConfig ();
    bool saveConfigFeilds();
    void createPayload(REQ_MSG_ID_e msgType );
    
    void processDeviceResponse(DevCommParam *param, QString deviceName);
    bool isUserChangeConfig();
    void handleInfoPageMessage(int index);
signals:
    
public slots:
    void slotSpinboxValueChange(QString,quint32);
    void slotTextBoxInfoPage(int,INFO_MSG_TYPE_e);
    
};

#endif // UPLOADIMAGE_H
