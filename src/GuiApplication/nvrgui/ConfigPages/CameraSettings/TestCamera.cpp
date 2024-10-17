#include "TestCamera.h"
#include <QPainter>
#include <QKeyEvent>
#include "ValidationMessage.h"

#define TESTCAM_IMAGE_WIDTH        SCALE_WIDTH(500)
#define TESTCAM_IMAGE_HEIGHT       SCALE_HEIGHT(281)
#define TESTCAM_WIDTH              (TESTCAM_IMAGE_WIDTH + SCALE_WIDTH(40))
#define TESTCAM_HEIGHT             (TESTCAM_IMAGE_HEIGHT + SCALE_HEIGHT(240))
#define TESTCAM_DETAILS_WIDTH      (TESTCAM_IMAGE_WIDTH + SCALE_WIDTH(550))

#define EVENT_SIZE_WIDTH            SCALE_WIDTH(230)

#define TEST_IMAGE_SORCE           "/tmp/Tst_cam.jpeg"

typedef enum{

    OTHR_SUP_MOTION,
    OTHR_SUP_TEMPER,
    OTHR_SUP_PTZ,
    OTHR_SUP_AUDIO,
    OTHR_SUP_SENSOR,
    OTHR_SUP_ALARM,
    OTHR_SUP_MOTION_WINDOW,
    OTHR_SUP_PRIVACY_MASK,
    OTHR_SUP_LINE_DETECT,
    OTHR_SUP_INTRUCTION,
    OTHR_SUP_AUDIO_EXCEPTION,
    OTHR_SUP_MISSING_OBJECT,
    OTHR_SUP_SUSPIOUS_OBJECT,
    OTHR_SUP_LOITERING,
    OTHR_SUP_OBJECT_CNT,
    OTHR_SUP_NO_MOTION,

    MAX_CAM_EVENT_OTHER_SUP
}CAM_EVENT_OTHER_SUP_e;

static const QString tstCamLabel[] = { "Motion Detection",
                                       "View Tampering",
                                       "Object Intrusion",
                                       "Trip Wire",
                                       "Missing Object",
                                       "Suspicious Object",
                                       "Loitering",
                                       "Audio Exception",
                                       "PTZ",
                                       "Audio",
                                       "Sensor Input",
                                       "Alarm Output",
                                       "Object Counting",
                                       "No Motion Detection"};

TestCamera::TestCamera(quint8 cameraNum, QString cameraName,
                       PayloadLib *payloadlib, QWidget *parent,
                       quint8 idxInPage, CAMERA_TYPE_e camtype ) :
    KeyBoard(parent)
{
    this->setGeometry (0,0,parent->width (),parent->height ());

    m_indexInPage = idxInPage;
    m_cameraNumber = cameraNum;
    m_payloadLib = payloadlib;    
    m_camType = camtype;

    INIT_OBJ(m_alarmOutput);
    INIT_OBJ(m_alarmOutputLabel);
    INIT_OBJ(m_audioCheckBox);    
    INIT_OBJ(m_audioExceptionLabel);
    INIT_OBJ(m_audioSupportLabel);
    INIT_OBJ(m_eventSupportLabel);
    INIT_OBJ(m_featureSupportLabel);
    INIT_OBJ(m_hideButton);
    INIT_OBJ(m_loiteringLabel);
    INIT_OBJ(m_loitering_CheckBox);
    INIT_OBJ(m_missingLabel);
    INIT_OBJ(m_missing_CheckBox);
    INIT_OBJ(m_motionLabel);
    INIT_OBJ(m_motionCheckBox);
    INIT_OBJ(m_objectCntLabel);
    INIT_OBJ(m_objectIntrusionLabel);
    INIT_OBJ(m_objectIntrusionCheckBox);
    INIT_OBJ(m_tripWireCheckBox);
    INIT_OBJ(m_audioExceptionCheckBox);
    INIT_OBJ(m_object_cnt_CheckBox);
    INIT_OBJ(m_ptzCheckBox);
    INIT_OBJ(m_ptzLabel);
    INIT_OBJ(m_sensorInput);
    INIT_OBJ(m_sensorInputLabel);
    INIT_OBJ(m_suspiciousLabel);
    INIT_OBJ(m_suspicious_CheckBox);
    INIT_OBJ(m_temperCheckBox);    
    INIT_OBJ(m_tripWireLabel);
    INIT_OBJ(m_viewTemperLabel);
    INIT_OBJ(m_noMotionLabel);
    INIT_OBJ(m_noMotionCheckBox);

    createDefaultElements (cameraNum,cameraName);

    m_infoPage = new InfoPage (0,
                               0,
                               parent->width (),
                               parent->height (),
                               INFO_CONFIG_PAGE,
                               parentWidget ());
    connect (m_infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageBtnclick(int)));

    this->show ();
}

TestCamera::~TestCamera ()
{
    delete m_background;

    disconnect (m_closeButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (m_closeButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    delete m_closeButton;

    delete m_cameraIdLabel;

    delete m_cameraNameLabel;
    delete m_image;

    disconnect (m_detailsButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (m_detailsButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotDetailButtonClick(int)));
    delete m_detailsButton;

    deleteInformationElements ();

    disconnect (m_infoPage,
                SIGNAL(sigInfoPageCnfgBtnClick(int)),
                this,
                SLOT(slotInfoPageBtnclick(int)));
    delete m_infoPage;
}

void TestCamera::createDefaultElements(quint8 cameraNum, QString cameraName)
{
    m_applController = ApplController::getInstance ();

    m_background = new Rectangle (SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + SCALE_WIDTH(20) +
                                  (SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - TESTCAM_WIDTH)/2 ,
                                  (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)
                                  +(SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) - TESTCAM_HEIGHT)/2),
                                  TESTCAM_WIDTH,
                                  TESTCAM_HEIGHT,
                                  0,
                                  NORMAL_BKG_COLOR,
                                  NORMAL_BKG_COLOR,
                                  this);

    m_closeButton = new CloseButtton( m_background->x () + m_background->width () - SCALE_WIDTH(20),
                                      m_background->y () + SCALE_HEIGHT(20),
                                      this,
                                      CLOSE_BTN_TYPE_1,
                                      TEST_CAM_CLS_BTN);

    m_elementlist[TEST_CAM_CLS_BTN] = m_closeButton;

    connect (m_closeButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (m_closeButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    m_cameraIdLabel = new TextLabel(m_background->x () + SCALE_WIDTH(20),
                                    m_background->y () + SCALE_HEIGHT(32),
                                    SCALE_FONT(16),
                                    Multilang("Camera ID")+ QString(": ") + QString("%1").arg (cameraNum),
                                    this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                    0, 0, TESTCAM_IMAGE_WIDTH);

    m_cameraNameLabel = new TextLabel(m_cameraIdLabel->x (),
                                      m_cameraIdLabel->y () + m_cameraIdLabel->height () + SCALE_HEIGHT(10),
                                      SCALE_FONT(16),
                                      Multilang("Camera Name")+ QString(": ") + cameraName,
                                      this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                      0, 0, TESTCAM_IMAGE_WIDTH);

    m_image = new Image(m_cameraNameLabel->x (),
                        m_cameraNameLabel->y () + m_cameraNameLabel->height () + SCALE_HEIGHT(15),
                        TEST_IMAGE_SORCE,
                        this,
                        START_X_START_Y,
                        0,
                        false,
                        true,
                        true);

    m_image->scale (TESTCAM_IMAGE_WIDTH,TESTCAM_IMAGE_HEIGHT);

    m_detailsButton = new PageOpenButton((m_background->x () + m_background->width () - SCALE_WIDTH(150)),
                                         (m_background->y () + (TESTCAM_HEIGHT) - SCALE_HEIGHT(60)),
                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         TEST_CAM_DETAILS_BTN,
                                         PAGEOPENBUTTON_LARGE,
                                         "Show Details",
                                         this,
                                         "","",
                                         false,
                                         0,
                                         NO_LAYER);

    m_elementlist[TEST_CAM_DETAILS_BTN] = m_detailsButton;

    connect (m_detailsButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotDetailButtonClick(int)));
    connect (m_detailsButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    if(m_camType == ANALOG_CAMERA)
    {
        m_detailsButton->setIsEnabled (false);
        m_detailsButton->setVisible (false);
        m_currElement = TEST_CAM_CLS_BTN;
    }
    else
    {
        m_currElement = TEST_CAM_DETAILS_BTN;
    }
    m_elementlist[m_currElement]->forceActiveFocus();
}

void TestCamera::takeLeftKeyAction()
{
    do
    {
        m_currElement = (m_currElement - 1 + MAX_TEST_CAM_ELEMNTS) % MAX_TEST_CAM_ELEMNTS;
    }while(!m_elementlist[  m_currElement]->getIsEnabled());

    m_elementlist[m_currElement]->forceActiveFocus();
}

void TestCamera::takeRightKeyAction()
{
    do
    {
        m_currElement = (m_currElement + 1) % MAX_TEST_CAM_ELEMNTS;
    }while(!m_elementlist[m_currElement]->getIsEnabled());

    m_elementlist[m_currElement]->forceActiveFocus();
}

void TestCamera::deleteInformationElements()
{
    if(m_motionCheckBox != NULL)
    {
        delete m_eventSupportLabel;
        delete m_featureSupportLabel;

        delete m_motionLabel;
        delete m_viewTemperLabel;
        delete m_objectIntrusionLabel;
        delete m_tripWireLabel;
        delete m_audioExceptionLabel;
        DELETE_OBJ(m_missingLabel);
        DELETE_OBJ(m_suspiciousLabel);
        DELETE_OBJ(m_loiteringLabel);
        DELETE_OBJ(m_objectCntLabel);
        DELETE_OBJ(m_noMotionLabel);

        delete m_ptzLabel;
        delete m_audioSupportLabel;

        delete m_alarmOutputLabel;
        delete m_sensorInputLabel;

        delete m_motionCheckBox;
        delete m_temperCheckBox;
        delete m_objectIntrusionCheckBox;
        delete m_tripWireCheckBox;
        delete m_audioExceptionCheckBox;
        DELETE_OBJ(m_missing_CheckBox);
        DELETE_OBJ(m_suspicious_CheckBox);
        DELETE_OBJ(m_loitering_CheckBox);
        DELETE_OBJ(m_object_cnt_CheckBox);
        DELETE_OBJ(m_noMotionCheckBox);
        delete m_ptzCheckBox;
        delete m_audioCheckBox;
        delete m_sensorInput;
        delete m_alarmOutput;

        disconnect (m_hideButton,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        disconnect (m_hideButton,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotHideButtonClick(int)));
        DELETE_OBJ(m_hideButton);
        m_motionCheckBox = NULL;
    }
}

void TestCamera::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    if(m_elementlist[m_currElement] != NULL)
    {
        m_elementlist[m_currElement]->forceActiveFocus ();
    }
}

void TestCamera::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void TestCamera ::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currElement = TEST_CAM_CLS_BTN;
    m_elementlist[m_currElement]->forceActiveFocus ();
}

void TestCamera::processDeviceResponse (DevCommParam *param, QString)
{
    if(param->deviceStatus == CMD_SUCCESS)
    {
        m_payloadLib->parseDevCmdReply(true, param->payload);

        bool isMotionSupport = m_payloadLib->getCnfgArrayAtIndex (OTHR_SUP_MOTION).toBool ();
        bool isTemperSupport = m_payloadLib->getCnfgArrayAtIndex (OTHR_SUP_TEMPER).toBool ();
        bool isPTZSupport = m_payloadLib->getCnfgArrayAtIndex (OTHR_SUP_PTZ).toBool ();
        bool isAudioSupport = m_payloadLib->getCnfgArrayAtIndex (OTHR_SUP_AUDIO).toBool ();
        bool isTripWireSupport = m_payloadLib->getCnfgArrayAtIndex(OTHR_SUP_LINE_DETECT).toBool();
        bool isObjectIntrusionSupport = m_payloadLib->getCnfgArrayAtIndex(OTHR_SUP_INTRUCTION).toBool();
        bool isAudioExceptionSupport = m_payloadLib->getCnfgArrayAtIndex(OTHR_SUP_AUDIO_EXCEPTION).toBool();
        bool isMissingSupport = m_payloadLib->getCnfgArrayAtIndex(OTHR_SUP_MISSING_OBJECT).toBool();
        bool isSuspiciousSupport = m_payloadLib->getCnfgArrayAtIndex(OTHR_SUP_SUSPIOUS_OBJECT).toBool();
        bool isLoiteringSupport = m_payloadLib->getCnfgArrayAtIndex(OTHR_SUP_LOITERING).toBool();
        bool isobjectCntSupport = m_payloadLib->getCnfgArrayAtIndex(OTHR_SUP_OBJECT_CNT).toBool();
        bool isNoMotionSupport = m_payloadLib->getCnfgArrayAtIndex(OTHR_SUP_NO_MOTION).toBool();
        QString  maxSensor = m_payloadLib->getCnfgArrayAtIndex (OTHR_SUP_SENSOR).toString ();
        QString  maxAlarm =  m_payloadLib->getCnfgArrayAtIndex (OTHR_SUP_ALARM).toString ();

        m_background->resetGeometry (SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) +
                                     ( SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - TESTCAM_DETAILS_WIDTH)/2 ,
                                     (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)
                                     + (SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)- TESTCAM_HEIGHT)/2),
                                     TESTCAM_DETAILS_WIDTH,
                                     TESTCAM_HEIGHT);

        m_closeButton->resetGeometry (m_background->x () + m_background->width () - SCALE_WIDTH(20),
                                      m_background->y () + SCALE_HEIGHT(20));

        m_cameraIdLabel->setOffset (m_background->x () + SCALE_WIDTH(20),
                                    m_background->y () + SCALE_HEIGHT(15));

        m_cameraNameLabel->setOffset (m_cameraIdLabel->x (),
                                      m_cameraIdLabel->y () + m_cameraIdLabel->height () + SCALE_HEIGHT(8));

        m_image->resetGeometry (m_cameraNameLabel->x (),
                                m_cameraNameLabel->y () + m_cameraNameLabel->height () + SCALE_HEIGHT(8));
        m_image->scale (TESTCAM_IMAGE_WIDTH,TESTCAM_IMAGE_HEIGHT);

        m_detailsButton->setVisible (false);

        if(m_motionCheckBox == NULL)
        {
            m_eventSupportLabel = new ElementHeading(m_cameraIdLabel->x () + TESTCAM_IMAGE_WIDTH + SCALE_WIDTH(20),
                                                     m_cameraNameLabel->y () + m_cameraNameLabel->height () + SCALE_HEIGHT(8),
                                                     EVENT_SIZE_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     "Events Supported",
                                                     COMMON_LAYER,
                                                     this,
                                                     false,SCALE_WIDTH(10),SCALE_FONT(18), true);

            m_motionCheckBox = new OptionSelectButton(m_eventSupportLabel->x (),
                                                      m_eventSupportLabel->y () + m_eventSupportLabel->height (),
                                                      EVENT_SIZE_WIDTH,
                                                      BGTILE_HEIGHT,
                                                      CHECK_BUTTON_INDEX,
                                                      "",
                                                      //                                                    tstCamLabel[0],
                                                      this,
                                                      COMMON_LAYER,
                                                      (EVENT_SIZE_WIDTH - SCALE_WIDTH(40)),
                                                      MX_OPTION_TEXT_TYPE_LABEL,
                                                      NORMAL_FONT_SIZE,
                                                      MAX_TEST_CAM_ELEMNTS,
                                                      false);

            m_motionCheckBox->changeState((isMotionSupport) ? ON_STATE : OFF_STATE);

            m_motionLabel = new TextLabel((m_motionCheckBox->x () + SCALE_WIDTH(10)),
                                          (m_motionCheckBox->y () + SCALE_HEIGHT(20)),
                                          NORMAL_FONT_SIZE,
                                          tstCamLabel[0],
                                          this,
                                          NORMAL_FONT_COLOR,
                                          NORMAL_FONT_FAMILY,
                                          ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(200));


            m_temperCheckBox = new OptionSelectButton(m_motionCheckBox->x (),
                                                      m_motionCheckBox->y () + m_motionCheckBox->height (),
                                                      EVENT_SIZE_WIDTH,
                                                      BGTILE_HEIGHT,
                                                      CHECK_BUTTON_INDEX,
                                                      //                                                    tstCamLabel[1],
                                                      "",
                                                      this,
                                                      COMMON_LAYER,
                                                      (EVENT_SIZE_WIDTH - SCALE_WIDTH(40)),
                                                      MX_OPTION_TEXT_TYPE_LABEL,
                                                      NORMAL_FONT_SIZE,
                                                      MAX_TEST_CAM_ELEMNTS,
                                                      false);

            m_temperCheckBox->changeState ((isTemperSupport) ? ON_STATE : OFF_STATE);

            m_viewTemperLabel = new TextLabel((m_temperCheckBox->x () + SCALE_WIDTH(10)),
                                              (m_temperCheckBox->y () + SCALE_HEIGHT(20)),
                                              NORMAL_FONT_SIZE,
                                              tstCamLabel[1],
                                              this,
                                              NORMAL_FONT_COLOR,
                                              NORMAL_FONT_FAMILY,
                                              ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(200));


            m_objectIntrusionCheckBox = new OptionSelectButton(m_temperCheckBox->x (),
                                                               (m_temperCheckBox->y () +
                                                                m_temperCheckBox->height ()),
                                                               EVENT_SIZE_WIDTH,
                                                               BGTILE_HEIGHT,
                                                               CHECK_BUTTON_INDEX,
                                                               //                                                    tstCamLabel[1],
                                                               "",
                                                               this,
                                                               COMMON_LAYER,
                                                               (EVENT_SIZE_WIDTH - SCALE_WIDTH(40)),
                                                               MX_OPTION_TEXT_TYPE_LABEL,
                                                               NORMAL_FONT_SIZE,
                                                               MAX_TEST_CAM_ELEMNTS,
                                                               false);

            m_objectIntrusionCheckBox->changeState ((isObjectIntrusionSupport) ? ON_STATE : OFF_STATE);

            m_objectIntrusionLabel = new TextLabel((m_objectIntrusionCheckBox->x () + SCALE_WIDTH(10)),
                                                   (m_objectIntrusionCheckBox->y () + SCALE_HEIGHT(20)),
                                                   NORMAL_FONT_SIZE,
                                                   tstCamLabel[2],
                                                   this,
                                                   NORMAL_FONT_COLOR,
                                                   NORMAL_FONT_FAMILY,
                                                   ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(200));

            m_tripWireCheckBox = new OptionSelectButton(m_objectIntrusionCheckBox->x (),
                                                        (m_objectIntrusionCheckBox->y () +
                                                         m_objectIntrusionCheckBox->height ()),
                                                        EVENT_SIZE_WIDTH,
                                                        BGTILE_HEIGHT,
                                                        CHECK_BUTTON_INDEX,
                                                        "",
                                                        this,
                                                        COMMON_LAYER,
                                                        (EVENT_SIZE_WIDTH - SCALE_WIDTH(40)),
                                                        MX_OPTION_TEXT_TYPE_LABEL,
                                                        NORMAL_FONT_SIZE,
                                                        MAX_TEST_CAM_ELEMNTS,
                                                        false);

            m_tripWireCheckBox->changeState ((isTripWireSupport) ? ON_STATE : OFF_STATE);

            m_tripWireLabel = new TextLabel((m_tripWireCheckBox->x () + SCALE_WIDTH(10)),
                                            (m_tripWireCheckBox->y () +  SCALE_HEIGHT(20)),
                                            NORMAL_FONT_SIZE,
                                            tstCamLabel[3],
                                            this,
                                            NORMAL_FONT_COLOR,
                                            NORMAL_FONT_FAMILY,
                                            ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(200));

            m_missing_CheckBox = new OptionSelectButton(m_tripWireCheckBox->x (),
                                                        (m_tripWireCheckBox->y () +
                                                         m_tripWireCheckBox->height ()),
                                                        EVENT_SIZE_WIDTH,
                                                        BGTILE_HEIGHT,
                                                        CHECK_BUTTON_INDEX,
                                                        "",
                                                        this,
                                                        COMMON_LAYER,
                                                        (EVENT_SIZE_WIDTH - SCALE_WIDTH(40)),
                                                        MX_OPTION_TEXT_TYPE_LABEL,
                                                        NORMAL_FONT_SIZE,
                                                        MAX_TEST_CAM_ELEMNTS,
                                                        false);

            m_missing_CheckBox->changeState ((isMissingSupport) ? ON_STATE : OFF_STATE);

            m_missingLabel = new TextLabel((m_missing_CheckBox->x () + SCALE_WIDTH(10)),
                                            (m_missing_CheckBox->y () +  SCALE_HEIGHT(20)),
                                            NORMAL_FONT_SIZE,
                                            tstCamLabel[4],
                                            this,
                                            NORMAL_FONT_COLOR,
                                            NORMAL_FONT_FAMILY,
                                            ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(200));


            m_suspicious_CheckBox = new OptionSelectButton(m_missing_CheckBox->x (),
                                                        (m_missing_CheckBox->y () +
                                                         m_missing_CheckBox->height ()),
                                                        EVENT_SIZE_WIDTH,
                                                        BGTILE_HEIGHT,
                                                        CHECK_BUTTON_INDEX,
                                                        "",
                                                        this,
                                                        COMMON_LAYER,
                                                        (EVENT_SIZE_WIDTH - SCALE_WIDTH(40)),
                                                        MX_OPTION_TEXT_TYPE_LABEL,
                                                        NORMAL_FONT_SIZE,
                                                        MAX_TEST_CAM_ELEMNTS,
                                                        false);

            m_suspicious_CheckBox->changeState ((isSuspiciousSupport) ? ON_STATE : OFF_STATE);

            m_suspiciousLabel = new TextLabel((m_suspicious_CheckBox->x () + SCALE_WIDTH(10)),
                                            (m_suspicious_CheckBox->y () +  SCALE_HEIGHT(20)),
                                            NORMAL_FONT_SIZE,
                                            tstCamLabel[5],
                                            this,
                                            NORMAL_FONT_COLOR,
                                            NORMAL_FONT_FAMILY,
                                            ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(200));

            m_loitering_CheckBox = new OptionSelectButton(m_suspicious_CheckBox->x (),
                                                        (m_suspicious_CheckBox->y () +
                                                         m_suspicious_CheckBox->height ()),
                                                        EVENT_SIZE_WIDTH,
                                                        BGTILE_HEIGHT,
                                                        CHECK_BUTTON_INDEX,
                                                        "",
                                                        this,
                                                        COMMON_LAYER,
                                                        (EVENT_SIZE_WIDTH - SCALE_WIDTH(40)),
                                                        MX_OPTION_TEXT_TYPE_LABEL,
                                                        NORMAL_FONT_SIZE,
                                                        MAX_TEST_CAM_ELEMNTS,
                                                        false);

            m_loitering_CheckBox->changeState ((isLoiteringSupport) ? ON_STATE : OFF_STATE);

            m_loiteringLabel = new TextLabel((m_loitering_CheckBox->x () + SCALE_WIDTH(10)),
                                            (m_loitering_CheckBox->y () +  SCALE_HEIGHT(20)),
                                            NORMAL_FONT_SIZE,
                                            tstCamLabel[6],
                                            this,
                                            NORMAL_FONT_COLOR,
                                            NORMAL_FONT_FAMILY,
                                            ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(200));


            m_audioExceptionCheckBox = new OptionSelectButton(m_loitering_CheckBox->x (),
                                                              (m_loitering_CheckBox->y () +
                                                               m_loitering_CheckBox->height ()),
                                                              EVENT_SIZE_WIDTH,
                                                              BGTILE_HEIGHT,
                                                              CHECK_BUTTON_INDEX,
                                                              "",
                                                              this,
                                                              COMMON_LAYER,
                                                              (EVENT_SIZE_WIDTH - SCALE_WIDTH(40)),
                                                              MX_OPTION_TEXT_TYPE_LABEL,
                                                              NORMAL_FONT_SIZE,
                                                              MAX_TEST_CAM_ELEMNTS,
                                                              false);

            m_audioExceptionCheckBox->changeState ((isAudioExceptionSupport) ? ON_STATE : OFF_STATE);

            m_audioExceptionLabel = new TextLabel((m_audioExceptionCheckBox->x () + SCALE_WIDTH(10)),
                                                  (m_audioExceptionCheckBox->y () +  SCALE_HEIGHT(20)),
                                                  NORMAL_FONT_SIZE,
                                                  tstCamLabel[7],
                                                  this,
                                                  NORMAL_FONT_COLOR,
                                                  NORMAL_FONT_FAMILY,
                                                  ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(200));
            m_object_cnt_CheckBox = new OptionSelectButton(m_audioExceptionCheckBox->x (),
                                                        (m_audioExceptionCheckBox->y () +
                                                         m_audioExceptionCheckBox->height ()),
                                                        EVENT_SIZE_WIDTH,
                                                        BGTILE_HEIGHT,
                                                        CHECK_BUTTON_INDEX,
                                                        "",
                                                        this,
                                                        COMMON_LAYER,
                                                        (EVENT_SIZE_WIDTH - SCALE_WIDTH(40)),
                                                        MX_OPTION_TEXT_TYPE_LABEL,
                                                        NORMAL_FONT_SIZE,
                                                        MAX_TEST_CAM_ELEMNTS,
                                                        false);

            m_object_cnt_CheckBox->changeState ((isobjectCntSupport) ? ON_STATE : OFF_STATE);

            m_objectCntLabel = new TextLabel((m_object_cnt_CheckBox->x () + SCALE_WIDTH(10)),
                                            (m_object_cnt_CheckBox->y () +  SCALE_HEIGHT(20)),
                                            NORMAL_FONT_SIZE,
                                            tstCamLabel[12],
                                            this,
                                            NORMAL_FONT_COLOR,
                                            NORMAL_FONT_FAMILY,
                                            ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(200));

            m_noMotionCheckBox = new OptionSelectButton(m_object_cnt_CheckBox->x (),
                                                        (m_object_cnt_CheckBox->y () +
                                                        m_object_cnt_CheckBox->height ()),
                                                        EVENT_SIZE_WIDTH,
                                                        BGTILE_HEIGHT,
                                                        CHECK_BUTTON_INDEX,
                                                        "",
                                                        this,
                                                        COMMON_LAYER,
                                                        (EVENT_SIZE_WIDTH - SCALE_WIDTH(40)),
                                                        MX_OPTION_TEXT_TYPE_LABEL,
                                                        NORMAL_FONT_SIZE,
                                                        MAX_TEST_CAM_ELEMNTS,
                                                        false);

            m_noMotionCheckBox->changeState ((isNoMotionSupport) ? ON_STATE : OFF_STATE);

            m_noMotionLabel = new TextLabel((m_noMotionCheckBox->x () + SCALE_WIDTH(10)),
                                            (m_noMotionCheckBox->y () +  SCALE_HEIGHT(20)),
                                            NORMAL_FONT_SIZE,
                                            tstCamLabel[13],
                                            this,
                                            NORMAL_FONT_COLOR,
                                            NORMAL_FONT_FAMILY,
                                            ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(200));



            m_featureSupportLabel = new ElementHeading(m_eventSupportLabel->x () + m_eventSupportLabel->width () + SCALE_WIDTH(10),
                                                     m_eventSupportLabel->y (),
                                                     EVENT_SIZE_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     "Features Supported",
                                                     COMMON_LAYER,
                                                     this,
                                                     false,SCALE_WIDTH(10),SCALE_FONT(18), true);

            m_ptzCheckBox = new OptionSelectButton(m_motionCheckBox->x () + m_motionCheckBox->width () + SCALE_WIDTH(10),
                                                   m_motionCheckBox->y (),
                                                   EVENT_SIZE_WIDTH,
                                                   BGTILE_HEIGHT,
                                                   CHECK_BUTTON_INDEX,
                                                   "",
                                                   this,
                                                   COMMON_LAYER,
                                                   (EVENT_SIZE_WIDTH - SCALE_WIDTH(40)),
                                                   MX_OPTION_TEXT_TYPE_LABEL,
                                                   NORMAL_FONT_SIZE,
                                                   MAX_TEST_CAM_ELEMNTS,
                                                   false);

            m_ptzCheckBox->changeState ((isPTZSupport) ? ON_STATE : OFF_STATE);

            m_ptzLabel = new TextLabel((m_ptzCheckBox->x () + SCALE_WIDTH(10)),
                                       (m_ptzCheckBox->y () + SCALE_HEIGHT(20)),
                                       NORMAL_FONT_SIZE,
                                       tstCamLabel[8],
                                       this,
                                       NORMAL_FONT_COLOR,
                                       NORMAL_FONT_FAMILY,
                                       ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(200));


            m_audioCheckBox = new OptionSelectButton(m_ptzCheckBox->x (),
                                                     m_ptzCheckBox->y () + m_ptzCheckBox->height (),
                                                     EVENT_SIZE_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     CHECK_BUTTON_INDEX,
                                                     "",
                                                     this,
                                                     COMMON_LAYER,
                                                     (EVENT_SIZE_WIDTH - SCALE_WIDTH(40)),
                                                     MX_OPTION_TEXT_TYPE_LABEL,
                                                     NORMAL_FONT_SIZE,
                                                     MAX_TEST_CAM_ELEMNTS,
                                                     false);

            m_audioCheckBox->changeState ((isAudioSupport) ? ON_STATE : OFF_STATE);

            m_audioSupportLabel = new TextLabel((m_audioCheckBox->x () + SCALE_WIDTH(10)),
                                                (m_audioCheckBox->y () + SCALE_HEIGHT(20)),
                                                NORMAL_FONT_SIZE,
                                                tstCamLabel[9],
                                                this,
                                                NORMAL_FONT_COLOR,
                                                NORMAL_FONT_FAMILY,
                                                ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(200));

            m_sensorInput = new ReadOnlyElement(m_audioCheckBox->x (),
                                                m_audioCheckBox->y () + m_audioCheckBox->height (),
                                                EVENT_SIZE_WIDTH,
                                                BGTILE_HEIGHT,
                                                SCALE_WIDTH(25),
                                                SCALE_HEIGHT(30),
                                                maxSensor,
                                                this,
                                                COMMON_LAYER,
                                                (EVENT_SIZE_WIDTH - SCALE_WIDTH(40)),
                                                SCALE_WIDTH(10),
                                                "");

            m_sensorInputLabel = new TextLabel((m_sensorInput->x () + SCALE_WIDTH(10)),
                                               (m_sensorInput->y () + SCALE_HEIGHT(20)),
                                               NORMAL_FONT_SIZE,
                                               tstCamLabel[10],
                                               this,
                                               NORMAL_FONT_COLOR,
                                               NORMAL_FONT_FAMILY,
                                               ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(200));

            m_alarmOutput = new ReadOnlyElement(m_sensorInput->x (),
                                                m_sensorInput->y () + m_sensorInput->height (),
                                                EVENT_SIZE_WIDTH,
                                                BGTILE_HEIGHT,
                                                SCALE_WIDTH(25),
                                                SCALE_HEIGHT(30),
                                                maxAlarm,
                                                this,
                                                COMMON_LAYER,
                                                (EVENT_SIZE_WIDTH - SCALE_WIDTH(40)),
                                                SCALE_WIDTH(10),
                                                "");

            m_alarmOutputLabel = new TextLabel((m_alarmOutput->x () + SCALE_WIDTH(10)),
                                              (m_alarmOutput->y () + SCALE_HEIGHT(20)),
                                              NORMAL_FONT_SIZE,
                                              tstCamLabel[11],
                                              this,
                                              NORMAL_FONT_COLOR,
                                              NORMAL_FONT_FAMILY,
                                              ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(200));

            m_hideButton = new PageOpenButton(m_background->x () + m_background->width () - SCALE_WIDTH(150),
                                              m_detailsButton->y (),
                                              BGTILE_MEDIUM_SIZE_WIDTH,
                                              BGTILE_HEIGHT,
                                              TEST_CAM_DETAILS_BTN,
                                              PAGEOPENBUTTON_LARGE_BACK,
                                              "Hide Details",
                                              this,
                                              "","",
                                              false,
                                              0,
                                              NO_LAYER);

            m_elementlist[TEST_CAM_DETAILS_BTN] = m_hideButton;

            connect (m_hideButton,
                     SIGNAL(sigButtonClick(int)),
                     this,
                     SLOT(slotHideButtonClick(int)));
            connect (m_hideButton,
                     SIGNAL(sigUpdateCurrentElement(int)),
                     this,
                     SLOT(slotUpdateCurrentElement(int)));

            m_currElement = TEST_CAM_DETAILS_BTN;
            m_elementlist[m_currElement]->forceActiveFocus ();
        }
    }
    else
    {
        m_infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
    }
}

void TestCamera::paintEvent (QPaintEvent *)
{
    QPainter painter(this);
    QColor color;

    color.setAlpha (0);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);

    painter.drawRoundedRect (QRect(0,
                                   0,
                                   SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT)),
                             SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));

    color.setAlpha (150);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);

    painter.drawRoundedRect (QRect(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                                  SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) -SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT),
                                   SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)),
                             SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));
}

void TestCamera::loadInfoPage (QString str)
{
    m_infoPage->loadInfoPage (str);
}

void TestCamera::slotButtonClick (int)
{
    QList<QVariant> paramList;
    quint8 temp;

    m_applController->processActivity(TST_CAM_IMG_DELETE,&temp);
    paramList.clear ();
    emit sigDeleteObject(m_indexInPage);
}

void TestCamera::slotDetailButtonClick (int)
{
    m_payloadLib->setCnfgArrayAtIndex (0,   m_cameraNumber);
    emit sigCreateCMDRequest(OTHR_SUP, 1);
}

void TestCamera::slotHideButtonClick (int)
{
    m_background->resetGeometry ((SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) +
                                 ( SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - TESTCAM_WIDTH)/2) ,
                                 (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)
                                 + (SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)- TESTCAM_HEIGHT)/2),
                                 TESTCAM_WIDTH,
                                 TESTCAM_HEIGHT);

    m_closeButton->resetGeometry (m_background->x () + m_background->width () - SCALE_WIDTH(20),
                                  m_background->y () + SCALE_HEIGHT(20));

    m_cameraIdLabel->setOffset (m_background->x () + SCALE_WIDTH(20),
                                m_background->y () + SCALE_HEIGHT(32));

    m_cameraNameLabel->setOffset (m_cameraIdLabel->x (),
                                  m_cameraIdLabel->y () + m_cameraIdLabel->height () + SCALE_HEIGHT(10));

    m_image->resetGeometry (m_cameraNameLabel->x (),
                            m_cameraNameLabel->y () + m_cameraNameLabel->height () + SCALE_HEIGHT(15));
    m_image->scale (TESTCAM_IMAGE_WIDTH,TESTCAM_IMAGE_HEIGHT);

    m_detailsButton->resetGeometry ((m_background->x () + m_background->width () - SCALE_WIDTH(150)),
                                    (m_background->y () + TESTCAM_HEIGHT - SCALE_HEIGHT(60)));
    m_detailsButton->setVisible (true);

    deleteInformationElements ();

    m_elementlist[TEST_CAM_DETAILS_BTN] = m_detailsButton;
    m_currElement = TEST_CAM_DETAILS_BTN;
    m_elementlist[m_currElement]->forceActiveFocus ();
}

void TestCamera::slotUpdateCurrentElement (int indexInPage)
{
    m_currElement = indexInPage;
}

void TestCamera::slotInfoPageBtnclick (int)
{
    m_infoPage->unloadInfoPage ();
    m_elementlist[m_currElement]->forceActiveFocus ();
}

void TestCamera::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void TestCamera::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void TestCamera::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}
