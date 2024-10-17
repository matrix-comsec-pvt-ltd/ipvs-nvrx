#include "CameraList.h"
#include "Layout/Layout.h"
#include "Controls/MessageBanner.h"
#include <QKeyEvent>
#include <QWheelEvent>
#include <QtGui/qcolor.h>
#include "ValidationMessage.h"

#define MAX_ELEMENT_POSSIBLE_IN_CAMERALIST  23
#define MAX_ITEM_ON_PAGE        23

#define SLIDER_MARGIN           SCALE_WIDTH(5)
#define SLIDER_WIDTH            SCALE_WIDTH(30)

#define CAMERA_LIST_WIDTH       SCALE_WIDTH(350)
#define CAMERA_LIST_HEIGHT      SCALE_HEIGHT(760)

#define ASSIGN_ALL_STR      "Assign Unassigned Cam"
#define REMOVE_ALL_STR      "Clear All"

CameraList::CameraList(quint16 startx,
                       quint16 starty,
                       QWidget *parent,
                       int indexInPage, CAMERALIST_CALLED_BY_e parentClass, quint16 windowIndex) :
    KeyBoard(parent), NavigationControl(indexInPage, true), m_startx(startx), m_starty(starty),
    m_deviceCount(0), m_camAssignedCount(0), m_firstIndex(0), m_lastIndex(MAX_ITEM_ON_PAGE), m_windowIndex(windowIndex)
{
    INIT_OBJ(m_toolTip);
    INIT_OBJ(m_scrollbar);
    memset(m_deviceConnectionState, CONNECTED, sizeof(m_deviceConnectionState));
    //m_cameraIndex[MAX_DEVICES][MAX_CAMERAS] = {0};
    m_currentStyle = MAX_STYLE_TYPE;
    m_currentDisplayType = MAIN_DISPLAY;
    m_parentClass = parentClass;
    m_totalElement= 0;

    for(quint8 index = 0; index < MAX_DEVICES; index++)
    {
        INIT_OBJ(m_devices[index]);
        m_cameraCount[index]        = 0;
        m_deviceStartIndex[index]   = index;

        for(quint8 camindex = 0; camindex < MAX_CAMERA_TILE; camindex++)
        {
            m_cameras[index][camindex] = NULL;
        }
        for(quint8 camindex = 0; camindex < MAX_CAMERAS; camindex++)
        {
            m_cameraIndex[index][camindex] = 0;
        }
    }


    switch(m_parentClass)
    {
    case CALLED_BY_DISPLAY_SETTING:
    case CALLED_BY_WINDOWSEQ_SETTING:
        this->setGeometry(m_startx, m_starty,
                          (CAMERA_LIST_BUTTON_WIDTH + SLIDER_WIDTH),
                          (MAX_DEVICES * CAMERA_LIST_BUTTON_HEIGHT));
        break;

    case CALLED_BY_VIEWCAM_ADD_LIST:
        this->setGeometry(m_startx, m_starty, CAMERA_LIST_WIDTH, CAMERA_LIST_HEIGHT);
        break;

    default:
        break;
    }
    m_applController = ApplController::getInstance();
    getEnabledDevices();
    this->show();
}
CameraList::~CameraList()
{

    for(quint8 index = 0; index < m_deviceCount; index++)
    {

        disconnect(m_devices[index],
                   SIGNAL(sigDevStateChangeImgClick(int,DEVICE_STATE_TYPE_e)),
                   this,
                   SLOT(slotDevStateChangeBtnClick(int,DEVICE_STATE_TYPE_e)));
        disconnect(m_devices[index],
                   SIGNAL(sigButtonClicked(int)),
                   this,
                   SLOT(slotDeviceButtonCliked(int)));
        delete m_devices[index];
    }

    for(quint8 deviceIndex=0; deviceIndex < m_deviceCount; deviceIndex++)
    {
        for(quint8 index = 0; index < m_cameraCount[deviceIndex]; index++)
        {
            if(m_cameras[deviceIndex][index] != NULL)
            {

                disconnect(m_cameras[deviceIndex][index],
                           SIGNAL(sigButtonClicked(int,CAMERA_STATE_TYPE_e)),
                           this,
                           SLOT(slotCameraButtonClicked(int,CAMERA_STATE_TYPE_e)));
                switch(m_parentClass)
                {
                case CALLED_BY_DISPLAY_SETTING:
                case CALLED_BY_WINDOWSEQ_SETTING:
                    disconnect(m_cameras[deviceIndex][index],
                               SIGNAL(sigShowHideDeviceToolTip(quint16,quint16,int,int,bool)),
                               this,
                               SLOT(slotShowHideTooltip(quint16,quint16,int,int,bool)));
                    break;

                default:
                    break;

                }
                delete m_cameras[deviceIndex][index];
                m_cameras[deviceIndex][index] = NULL;
            }
        }
    }

    if(m_scrollbar != NULL)
    {
        disconnect(m_scrollbar,
                   SIGNAL(sigScroll(int)),
                   this,
                   SLOT(slotScroll(int)));
        delete m_scrollbar;
        m_scrollbar = NULL;
    }

    if(m_toolTip != NULL)
    {
        m_toolTip->deleteLater();
    }
}



void CameraList::getEnabledDevices()
{
    DEVICE_STATE_TYPE_e currState = DISCONNECTED;
    bool autoLoginstate = false;
    QMap<quint8, QString> deviceMapList;
    m_applController->GetDevNameDropdownMapList(deviceMapList);
    m_applController->GetEnableDevList(m_deviceCount, m_deviceNameList);
    m_dispDeviceName = deviceMapList.value(0);

    //create cameralistbutton only for enabled device
    for(int index = 0; index < m_deviceCount; index++)
    {
        currState = m_applController->GetDeviceConnectionState (m_deviceNameList.at(index));
        autoLoginstate = m_applController->GetDeviceAutoLoginstate (m_deviceNameList.at(index));

        switch(m_parentClass)
        {
        case CALLED_BY_DISPLAY_SETTING:
        case CALLED_BY_WINDOWSEQ_SETTING:
            m_devices[index] = new DeviceListButton(index,
                                                    deviceMapList.value(index),
                                                    currState,
                                                    this,
                                                    index,
                                                    true,
                                                    ((index == 0) ? false : (!autoLoginstate)));

            break;
        case CALLED_BY_VIEWCAM_ADD_LIST:
            m_devices[index] = new DeviceListButton(index,
                                                    deviceMapList.value(index),
                                                    currState,
                                                    this,
                                                    index,
                                                    true,
                                                    ((index == 0) ? false : (!autoLoginstate)),
                                                    SCALE_WIDTH(320));
            break;
        default:
            break;
        }
        connect(m_devices[index],
                SIGNAL(sigButtonClicked(int)),
                this,
                SLOT(slotDeviceButtonCliked(int)));
        connect(m_devices[index],
                SIGNAL(sigDevStateChangeImgClick(int,DEVICE_STATE_TYPE_e)),
                this,
                SLOT(slotDevStateChangeBtnClick(int,DEVICE_STATE_TYPE_e)));

    }

    setCameraListGeometry();
}


void CameraList::updateDeviceList (QString devName,DEVICE_STATE_TYPE_e state)
{
    DEVICE_STATE_TYPE_e currState = DISCONNECTED;
    bool autoLoginstate = false;
    QStringList tempDevList;
    quint8 tempDevCount;

    m_applController->GetEnableDevList(tempDevCount, tempDevList);
    if ((tempDevList != m_deviceNameList) || (m_dispDeviceName != m_applController->GetDispDeviceName(tempDevList.at(0))))
    {
        for(quint8 index = 0; index < m_deviceCount; index++)
        {
            disconnect(m_devices[index],
                       SIGNAL(sigButtonClicked(int)),
                       this,
                       SLOT(slotDeviceButtonCliked(int)));
            disconnect(m_devices[index],
                       SIGNAL(sigDevStateChangeImgClick(int,DEVICE_STATE_TYPE_e)),
                       this,
                       SLOT(slotDevStateChangeBtnClick(int,DEVICE_STATE_TYPE_e)));

            delete m_devices[index];
        }

        for(quint8 deviceIndex =0; deviceIndex< m_deviceCount; deviceIndex++)
        {
            for(quint8 index = 0; index < m_cameraCount[deviceIndex]; index++)
            {
                if(m_cameras[deviceIndex][index] != NULL)
                {

                    disconnect(m_cameras[deviceIndex][index],
                               SIGNAL(sigButtonClicked(int,CAMERA_STATE_TYPE_e,int)),
                               this,
                               SLOT(slotCameraButtonClicked(int,CAMERA_STATE_TYPE_e,int)));
                    switch(m_parentClass)
                    {
                    case CALLED_BY_DISPLAY_SETTING:
                    case CALLED_BY_WINDOWSEQ_SETTING:
                        disconnect(m_cameras[deviceIndex][index],
                                   SIGNAL(sigShowHideDeviceToolTip(quint16,quint16,int,int,bool)),
                                   this,
                                   SLOT(slotShowHideTooltip(quint16,quint16,int,int,bool)));
                        break;

                    default:
                        break;

                    }
                    delete m_cameras[deviceIndex][index];
                    m_cameras[deviceIndex][index] = NULL;

                }
            }
            m_cameraCount[deviceIndex]=0;
        }

        if(m_scrollbar != NULL)
        {
            disconnect(m_scrollbar,
                       SIGNAL(sigScroll(int)),
                       this,
                       SLOT(slotScroll(int)));
            delete m_scrollbar;
            m_scrollbar = NULL;
        }

        if(m_toolTip != NULL)
        {
            m_toolTip->deleteLater();
            m_toolTip = NULL;
        }

        m_applController->GetEnableDevList(m_deviceCount, m_deviceNameList);
        m_dispDeviceName = m_applController->GetDispDeviceName(m_deviceNameList.at(0));
        if(state == DELETED)
        {
            m_deviceNameList.removeOne (devName);
            m_deviceCount = m_deviceNameList.length ();
        }

        //create cameralistbutton only for enabled device
        for(quint8 index = 0; index < m_deviceCount; index++)
        {
            currState = m_applController->GetDeviceConnectionState (m_deviceNameList.at(index));
            autoLoginstate = m_applController->GetDeviceAutoLoginstate (m_deviceNameList.at(index));

            switch(m_parentClass)
            {
            case CALLED_BY_DISPLAY_SETTING:
            case CALLED_BY_WINDOWSEQ_SETTING:
                m_devices[index] = new DeviceListButton(index,
                                                        index ? m_deviceNameList.at(index) : m_dispDeviceName,
                                                        currState,
                                                        this,
                                                        index,
                                                        true,
                                                        ((index == 0) ? false : (!autoLoginstate)));

                break;
            case CALLED_BY_VIEWCAM_ADD_LIST:
                m_devices[index] = new DeviceListButton(index,
                                                        index ? m_deviceNameList.at(index) : m_dispDeviceName,
                                                        currState,
                                                        this,
                                                        index,
                                                        true,
                                                        ((index == 0) ? false : (!autoLoginstate)),
                                                        SCALE_WIDTH(320));
                break;
            default:
                break;
            }
            connect(m_devices[index],
                    SIGNAL(sigButtonClicked(int)),
                    this,
                    SLOT(slotDeviceButtonCliked(int)));
            connect(m_devices[index],
                    SIGNAL(sigDevStateChangeImgClick(int,DEVICE_STATE_TYPE_e)),
                    this,
                    SLOT(slotDevStateChangeBtnClick(int,DEVICE_STATE_TYPE_e)));

        }

        setCameraListGeometry();
    }
    else
    {
        quint8 temIndex = m_deviceNameList.indexOf (devName);
        m_devices[temIndex]->updateConnectionState (state);
    }
}

void CameraList::updateCameraList(quint8 deviceIndex, bool isCreationNeeded)
{
    bool status = false;
    CAMERA_STATE_TYPE_e connectionState = CAM_STATE_NONE;
    quint16 configWindow = MAX_CHANNEL_FOR_SEQ;
    quint8 channelIndex = MAX_WINDOWS;
    quint16 windowIndex;
    QString connectStr = CONNECT_ALL_STR;

    m_camAssignedCount = 0;

    m_devices[deviceIndex]->setShowClickedImage(true);
    m_devices[deviceIndex]->changeDevSelectionstate(DEV_SELECTED);
    getAllCameraNamesOfDevice(deviceIndex);
    quint8 cameraIndexActual = 0;


    if(m_cameraCount[deviceIndex] > 1)
    {
        switch(m_parentClass)
        {
        case CALLED_BY_DISPLAY_SETTING:
        case CALLED_BY_WINDOWSEQ_SETTING:

            for(quint8 cameraIndex = 0; cameraIndex < m_cameraCount[deviceIndex]; cameraIndex++)
            {
                connectionState = CAM_STATE_NONE;
                if(cameraIndex != 0)
                {
                    cameraIndexActual = m_cameraIndex[deviceIndex][cameraIndex - 1];

                    status = findWindowIndexOfDisplayInfo(deviceIndex,
                                                          cameraIndexActual,
                                                          configWindow,
                                                          channelIndex);

                    if(status == true)
                    {
                        m_camAssignedCount++;
                        connectionState = CAM_STATE_ASSIGNED;
                        if(m_parentClass == CALLED_BY_DISPLAY_SETTING)
                        {
                            if((m_currentStyle == MAX_STYLE_TYPE)
                                    && (channelIndex == m_currentDisplayConfig.windowInfo[configWindow].currentChannel))
                            {
                                windowIndex = Layout::findWindowOfLiveStream(m_currentDisplayType,
                                                                             m_deviceNameList.at(deviceIndex),
                                                                             cameraIndexActual);

                                if(windowIndex != MAX_CHANNEL_FOR_SEQ)
                                {
                                    connectionState = Layout::changeVideoStatusToCameraStatus(Layout::streamInfoArray[m_currentDisplayType][windowIndex].m_videoStatus);
                                }
                            }
                        }
                    }
                }

                if(isCreationNeeded == false)
                {
                    updateCameraCurrentState(deviceIndex, cameraIndex, connectionState);
                }
                else if( m_cameras[deviceIndex][cameraIndex] == NULL)
                {
                    bool isVisible = true;
                    {
                        isVisible = (((cameraIndex + m_deviceStartIndex[deviceIndex]+1) > m_firstIndex)
                                     && (cameraIndex+m_deviceStartIndex[deviceIndex]+1 < m_lastIndex));
                    }

                    m_cameras[deviceIndex][cameraIndex] = new CameraListButton(cameraIndex,
                                                                               (cameraIndex == 0) ? m_cameraNameList.at(cameraIndex) :
                                                                                                    INT_TO_QSTRING(cameraIndexActual) + ":" + m_cameraNameList.at(cameraIndex),
                                                                               connectionState,
                                                                               this,
                                                                               (cameraIndex + 1),
                                                                               isVisible,deviceIndex,m_parentClass);
                    connect(m_cameras[deviceIndex][cameraIndex],
                            SIGNAL(sigButtonClicked(int,CAMERA_STATE_TYPE_e,int)),
                            this,
                            SLOT(slotCameraButtonClicked(int,CAMERA_STATE_TYPE_e,int)),
                            Qt::DirectConnection);

                    connect(m_cameras[deviceIndex][cameraIndex],
                            SIGNAL(sigShowHideDeviceToolTip(quint16,quint16,int,int,bool)),
                            this,
                            SLOT(slotShowHideTooltip(quint16,quint16,int,int,bool)),
                            Qt::DirectConnection);
                }
            }

            if(m_camAssignedCount == (m_cameraCount[deviceIndex] - 1))
            {
                switch(m_parentClass)
                {
                case CALLED_BY_DISPLAY_SETTING:
                    connectStr = DISCONNECT_ALL_STR;
                    break;
                case CALLED_BY_WINDOWSEQ_SETTING:
                    connectStr = REMOVE_ALL_STR;
                    break;
                default:
                    break;
                }
                m_cameras[deviceIndex][0]->updateConnectionState(CAM_STATE_ASSIGNED);
            }
            else
            {
                switch(m_parentClass)
                {
                case CALLED_BY_DISPLAY_SETTING:
                    connectStr = CONNECT_ALL_STR;
                    break;
                case CALLED_BY_WINDOWSEQ_SETTING:
                    connectStr = ASSIGN_ALL_STR;
                    break;
                default:
                    break;
                }
                m_cameras[deviceIndex][0]->updateConnectionState(CAM_STATE_NONE);
            }
            m_cameras[deviceIndex][0]->changeText(connectStr);

            break;

        case CALLED_BY_VIEWCAM_ADD_LIST:

            for(quint8 cameraIndex = 0; cameraIndex < m_cameraCount[deviceIndex]; cameraIndex++)
            {
                connectionState = CAM_STATE_NONE;
                cameraIndexActual = m_cameraIndex[deviceIndex][cameraIndex];
                windowIndex = Layout::findWindowIndexIfAssignOnCurrentPage(MAIN_DISPLAY,
                                                                           m_deviceNameList.at(deviceIndex),
                                                                           (cameraIndexActual));


                if(windowIndex != MAX_CHANNEL_FOR_SEQ)
                {
                    VIDEO_STATUS_TYPE_e videoStatus = Layout::streamInfoArray[MAIN_DISPLAY][windowIndex].m_videoStatus;
                    connectionState = Layout::changeVideoStatusToCameraStatus(videoStatus);
                }
                else
                {
                    status = Layout::findWindowIndexIfAssignOnAnyPage(MAIN_DISPLAY,
                                                                      m_deviceNameList.at(deviceIndex),
                                                                      cameraIndexActual,
                                                                      windowIndex,
                                                                      channelIndex);

                    if(status == true)
                    {
                        connectionState = CAM_STATE_ASSIGNED;
                    }


                }

                if( m_cameras[deviceIndex][cameraIndex] == NULL)
                {
                    bool isVisible = true;
                    {
                        isVisible = (((cameraIndex + m_deviceStartIndex[deviceIndex]+1) > m_firstIndex)
                                     && (cameraIndex+m_deviceStartIndex[deviceIndex]+1 < m_lastIndex));
                    }

                    m_cameras[deviceIndex][cameraIndex] = new CameraListButton(cameraIndex,
                                                                               INT_TO_QSTRING(cameraIndexActual) + ":" + m_cameraNameList.at(cameraIndex),
                                                                               connectionState,
                                                                               this,
                                                                               (cameraIndexActual),
                                                                               isVisible,deviceIndex,
                                                                               m_parentClass);


                    connect(m_cameras[deviceIndex][cameraIndex],
                            SIGNAL(sigButtonClicked(int,CAMERA_STATE_TYPE_e,int)),
                            this,
                            SLOT(slotCameraButtonClicked(int,CAMERA_STATE_TYPE_e,int)),
                            Qt::DirectConnection);
                }
            }
            break;
        default:
            break;
        }
    }
}

void CameraList::updateCameraListConnectionState()
{
    CAMERA_STATE_TYPE_e connectionState = CAM_STATE_NONE;
    bool status = false;
    quint16 configWindow = MAX_CHANNEL_FOR_SEQ;
    quint8 channelIndex = MAX_WINDOWS;
    quint16 windowIndex;
    QString connectStr = CONNECT_ALL_STR;
    quint8 cameraIndexActual=0;

    for(quint8 deviceIndex=0; deviceIndex< m_deviceCount; deviceIndex++)
    {
        if(IS_VALID_OBJ(m_cameras[deviceIndex][0]))
        {
            m_camAssignedCount = 0;
            for(quint8 cameraIndex = 0; cameraIndex < m_cameraCount[deviceIndex]; cameraIndex++)
            {
                connectionState = CAM_STATE_NONE;

                if(cameraIndex != 0)
                {
                    cameraIndexActual = m_cameraIndex[deviceIndex][cameraIndex - 1];
                    status = findWindowIndexOfDisplayInfo(deviceIndex,
                                                          cameraIndexActual,
                                                          configWindow,
                                                          channelIndex);
                    if(status == true)
                    {
                        m_camAssignedCount++;
                        connectionState = CAM_STATE_ASSIGNED;
                        if(m_parentClass == CALLED_BY_DISPLAY_SETTING)
                        {
                            if((m_currentStyle == MAX_STYLE_TYPE)
                                    && (channelIndex == m_currentDisplayConfig.windowInfo[configWindow].currentChannel))
                            {
                                windowIndex = Layout::findWindowOfLiveStream(m_currentDisplayType,
                                                                             m_deviceNameList.at(deviceIndex),
                                                                             cameraIndexActual);

                                if(windowIndex != MAX_CHANNEL_FOR_SEQ)
                                {
                                    connectionState = Layout::changeVideoStatusToCameraStatus(Layout::streamInfoArray[m_currentDisplayType][windowIndex].m_videoStatus);
                                }
                            }
                        }
                    }
                }

                m_cameras[deviceIndex][cameraIndex]->updateConnectionState(connectionState);
            }

            if(m_camAssignedCount == (m_cameraCount[deviceIndex] - 1))
            {
                switch(m_parentClass)
                {
                case CALLED_BY_DISPLAY_SETTING:
                    connectStr = DISCONNECT_ALL_STR;
                    break;
                case CALLED_BY_WINDOWSEQ_SETTING:
                    connectStr = REMOVE_ALL_STR;
                    break;
                default:
                    break;
                }
                m_cameras[deviceIndex][0]->updateConnectionState(CAM_STATE_ASSIGNED);
                m_cameras[deviceIndex][0]->changeText(connectStr);

            }
            else if(m_camAssignedCount == 0)
            {
                switch(m_parentClass)
                {
                case CALLED_BY_DISPLAY_SETTING:
                    connectStr = CONNECT_ALL_STR;
                    break;
                case CALLED_BY_WINDOWSEQ_SETTING:
                    connectStr = ASSIGN_ALL_STR;
                    break;
                default:
                    break;
                }
                m_cameras[deviceIndex][0]->updateConnectionState(CAM_STATE_NONE);
                m_cameras[deviceIndex][0]->changeText(connectStr);

            }
        }

    }
}

void CameraList::updateCurrDeviceCamStatus()
{
    for(quint8 deviceIndex=0; deviceIndex <m_deviceCount; deviceIndex++)
    {
        if(IS_VALID_OBJ(m_cameras[deviceIndex][0]))
        {
            updateCameraList(deviceIndex, false);
        }
    }
}

void CameraList::getAllCameraNamesOfDevice(int deviceIndex)
{
    //get cameraNameList of input device
    m_cameraNameList.clear ();

    bool status = m_applController->GetAllCameraNamesOfDevice(m_deviceNameList.at(deviceIndex),
                                                              m_cameraNameList,
                                                              LIVE_CAM_LIST_TYPE,
                                                              &m_cameraIndex[deviceIndex][0]);


    if(m_cameraNameList.length () > 0)
    {
        if(m_cameraNameList.contains (""))
        {
            m_cameraCount[deviceIndex] = 0;
            m_devices[deviceIndex]->setShowClickedImage(false);
            m_devices[deviceIndex]->changeDevSelectionstate (DEV_DESELECTED);
        }
        else
        {
            switch(m_parentClass)
            {
            case CALLED_BY_DISPLAY_SETTING:
                m_cameraNameList.insert(0, CONNECT_ALL_STR);
                break;

            case CALLED_BY_WINDOWSEQ_SETTING:
                m_cameraNameList.insert(0,ASSIGN_ALL_STR);
                break;

            default:
                break;
            }
            m_cameraCount[deviceIndex] = m_cameraNameList.length();
        }
    }
    else if(m_cameraNameList.isEmpty())
    {
        DEVICE_REPLY_TYPE_e response = (status) ? CMD_NO_PRIVILEGE : CMD_DEV_DISCONNECTED;
        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(response));
        m_devices[deviceIndex]->setShowClickedImage(false);
        m_devices[deviceIndex]->changeDevSelectionstate (DEV_DESELECTED);
    }
}

void CameraList::setCameraListGeometry()
{
    quint16 total=0;
    quint8 visibleIdx=0;

    for(quint8 deviceIndex=0; deviceIndex<m_deviceCount; deviceIndex++)
    {
        if((total >= m_firstIndex) && (total <= m_lastIndex))
        {
            if(!m_devices[deviceIndex]->isEnabled())
            {
                m_devices[deviceIndex]->setIsEnabled(true);
            }

            if(!m_devices[deviceIndex]->isVisible())
            {
                m_devices[deviceIndex]->setVisible(true);
            }

            m_devices[deviceIndex]->resetGeometryCustIndex(0, visibleIdx);
            ++visibleIdx;
            total++;
        }
        else
        {
            //hide index which does not fall between start and end Idx
            if(m_devices[deviceIndex]->isEnabled())
            {
                m_devices[deviceIndex]->setIsEnabled(false);
            }

            if(m_devices[deviceIndex]->isVisible())
            {
                m_devices[deviceIndex]->setVisible(false);
            }

            total++;
        }

        if(IS_VALID_OBJ(m_cameras[deviceIndex][0]))
        {
            for(quint8 index=0 ; index < m_cameraCount[deviceIndex]; index++)
            {
                if((total >= m_firstIndex) && (total < m_lastIndex))
                {
                    if(!m_cameras[deviceIndex][index]->isEnabled())
                    {
                        m_cameras[deviceIndex][index]->setIsEnabled(true);
                    }

                    if(!m_cameras[deviceIndex][index]->isVisible())
                    {
                        m_cameras[deviceIndex][index]->setVisible(true);
                    }

                    m_cameras[deviceIndex][index]->resetGeometryCustIndex(0, visibleIdx);
                    ++visibleIdx;
                    total++;

                }
                else
                {
                    //hide index which does not fall between start and end Idx
                    if(m_cameras[deviceIndex][index]->isEnabled())
                    {
                        m_cameras[deviceIndex][index]->setIsEnabled(false);
                    }

                    if(m_cameras[deviceIndex][index]->isVisible())
                    {
                        m_cameras[deviceIndex][index]->setVisible(false);
                    }

                    total++;
                }
            }
        }
    }

    if(IS_VALID_OBJ(m_scrollbar))
    {
        disconnect(m_scrollbar,
                   SIGNAL(sigScroll(int)),
                   this,
                   SLOT(slotScroll(int)));
        DELETE_OBJ(m_scrollbar);
    }

    if(total <= MAX_ITEM_ON_PAGE)
    {
        m_firstIndex = 0;
        m_lastIndex  = MAX_ELEMENT_POSSIBLE_IN_CAMERALIST;
    }
    else
    {
        if(!IS_VALID_OBJ(m_scrollbar))
        {
            m_scrollbar = new ScrollBar((CAMERA_LIST_BUTTON_WIDTH + SLIDER_MARGIN + ((m_parentClass == CALLED_BY_VIEWCAM_ADD_LIST)? SCALE_WIDTH(60):SCALE_WIDTH(0))),
                                        (0),
                                        SCALE_WIDTH(13),
                                        MAX_ITEM_ON_PAGE,
                                        CAMERA_LIST_BUTTON_HEIGHT,
                                        (total),
                                        m_firstIndex,
                                        this,
                                        VERTICAL_SCROLLBAR,
                                        0,
                                        true, false,true);

            connect(m_scrollbar,
                    SIGNAL(sigScroll(int)),
                    this,
                    SLOT(slotScroll(int)));

        }

        if(m_lastIndex > total)
        {
            m_lastIndex = total;
            m_firstIndex = m_lastIndex -MAX_ELEMENT_POSSIBLE_IN_CAMERALIST;
            if(m_firstIndex < 0)
            {
                m_firstIndex = 0;
            }
        }

    }
    m_totalElement = total;
    resetGeometryForScroll();

    if((m_parentClass == CALLED_BY_DISPLAY_SETTING) || (m_parentClass == CALLED_BY_WINDOWSEQ_SETTING))
    {
        this->setGeometry(m_startx,
                          m_starty,
                          (CAMERA_LIST_BUTTON_WIDTH + SLIDER_WIDTH),
                          (MAX_ELEMENT_POSSIBLE_IN_CAMERALIST * CAMERA_LIST_BUTTON_HEIGHT));

    }
    else if (m_parentClass == CALLED_BY_VIEWCAM_ADD_LIST)
    {
        this->setGeometry(m_startx,
                          m_starty,
                          CAMERA_LIST_WIDTH,
                          MAX_ELEMENT_POSSIBLE_IN_CAMERALIST*CAMERA_LIST_BUTTON_HEIGHT);
    }
}


void CameraList::updateCameraCurrentState(quint8 deviceIndex, quint8 cameraId,
                                          CAMERA_STATE_TYPE_e currentState)
{
    quint8 camIndex;
    for(camIndex=1; camIndex < m_cameraCount[deviceIndex]; camIndex++)
    {
        if(m_cameraIndex[deviceIndex][camIndex - 1] == cameraId)
            break;
    }

    if((m_cameraCount[deviceIndex] != 0) && (camIndex < m_cameraCount[deviceIndex]))
    {
        if(IS_VALID_OBJ(m_cameras[deviceIndex][camIndex]))
        {

            m_cameras[deviceIndex][camIndex]->updateConnectionState(currentState);
        }
    }
}
quint8 CameraList::getIndexofdeivce(QString deviceName)
{
    qint8 retVal = 0;
    if(m_deviceNameList.contains(deviceName) == true)
    {
        retVal =  m_deviceNameList.indexOf(deviceName);
    }
    return retVal;
}

//gets device list state
void CameraList::updateDeviceCurrentState(QString devName,
                                          DEVICE_STATE_TYPE_e currentState)
{
    if(m_deviceNameList.contains(devName))
    {
        qint8 index = MAX_DEVICES;
        index = m_deviceNameList.indexOf (devName);
        if(index >= 0 )
        {
            if(currentState == DELETED)
            {
                if(IS_VALID_OBJ(m_cameras[index][0]))
                {
                    slotDeviceButtonCliked(index);
                }
                m_deviceNameList.removeOne (devName);
            }
            else
            {
                if(currentState != CONNECTED)
                {
                    m_devices[index]->updateConnectionState(currentState);
                    if(IS_VALID_OBJ(m_cameras[index][0]))
                    {
                        slotDeviceButtonCliked(index);
                    }
                }
            }
        }
    }
    updateDeviceList (devName,currentState);
}

bool CameraList::findWindowIndexOfDisplayInfo(quint8 deviceindex, quint8 cameraIndex,
                                              quint16 &windowIndex,
                                              quint8 &channelIndex)
{
    bool status = false;

    for(windowIndex = 0; windowIndex < MAX_CHANNEL_FOR_SEQ; windowIndex++)
    {
        for(channelIndex = 0; channelIndex < MAX_WIN_SEQ_CAM; channelIndex++)
        {
            if((strcmp(QString(m_deviceNameList.at(deviceindex)).toUtf8().constData(),
                       m_currentDisplayConfig.windowInfo[windowIndex].camInfo[channelIndex].deviceName) == 0)
                    && (m_currentDisplayConfig.windowInfo[windowIndex].camInfo[channelIndex].defChannel == cameraIndex))
            {
                return true;
            }
        }
    }

    return status;
}

bool CameraList::getCurrSelectedDeviceName(QString deviceName )
{
    bool available =false;
    for(quint8 deviceIndex=0; deviceIndex<m_deviceCount; deviceIndex++)
    {
        if(IS_VALID_OBJ(m_cameras[deviceIndex][0]) && m_deviceNameList.contains(deviceName))
            available =true;
    }
    return available;
}

void CameraList::resetGeometryForScroll()
{
    quint8 customIndex=0;
    for(quint8 deviceIndex=0; deviceIndex<m_deviceCount; deviceIndex++)
    {
        m_devices[deviceIndex]->setIsEnabled(true);
        m_devices[deviceIndex]->resetGeometryCustIndex(0, m_deviceStartIndex[deviceIndex]);
        if(((m_deviceStartIndex[deviceIndex]) < m_firstIndex) || (m_deviceStartIndex[deviceIndex] > m_lastIndex))
        {
            m_devices[deviceIndex]->setVisible(false);
            m_devices[deviceIndex]->setIsEnabled(false);
            m_devices[deviceIndex]->setCatchKey(false);
        }
        else
        {
            m_devices[deviceIndex]->setVisible(true);
            m_devices[deviceIndex]->setIsEnabled(true);
            m_devices[deviceIndex]->setCatchKey(true);
            m_devices[deviceIndex]->resetGeometryCustIndex(0, customIndex++);
        }
        if(IS_VALID_OBJ(m_cameras[deviceIndex][0]))
        {
            for(quint8 index=0 ; index < m_cameraCount[deviceIndex]; index++)
            {
                m_cameras[deviceIndex][index]->resetGeometryCustIndex(0,m_deviceStartIndex[deviceIndex]+index+1);
                if(((m_deviceStartIndex[deviceIndex]+index+1) < m_firstIndex) || ((m_deviceStartIndex[deviceIndex]+index+1) > m_lastIndex))
                {
                    m_cameras[deviceIndex][index]->setVisible(false);
                    m_cameras[deviceIndex][index]->setIsEnabled(false);
                    m_cameras[deviceIndex][index]->setCatchKey(false);
                }
                else
                {
                    m_cameras[deviceIndex][index]->setVisible(true);

                    m_cameras[deviceIndex][index]->setIsEnabled(true);
                    m_cameras[deviceIndex][index]->setCatchKey(true);
                    m_cameras[deviceIndex][index]->resetGeometryCustIndex(0, customIndex++);
                }
            }

        }

    }
}

void CameraList::setCurrentStyle(STYLE_TYPE_e styleType)
{
    m_currentStyle = styleType;
}

void CameraList::setCurrentDisplayType(DISPLAY_TYPE_e displayType)
{
    m_currentDisplayType = displayType;
}

void CameraList::setCurrentDisplayConfig(DISPLAY_CONFIG_t displayConfig)
{
    memcpy((DISPLAY_CONFIG_t*)&m_currentDisplayConfig,
           (DISPLAY_CONFIG_t*)&displayConfig,
           sizeof(DISPLAY_CONFIG_t));
    updateCameraListConnectionState();
}

void CameraList::takeUpKeyAction()
{

    int temIndex=0;

    for(qint8 deviceIndex = 0; deviceIndex < m_deviceCount; deviceIndex++)
    {
        temIndex++;

        if(m_devices[deviceIndex]->hasFocus())
        {
            deviceIndex = (deviceIndex - 1);

            if ((deviceIndex >= 0) && (deviceIndex < MAX_DEVICES))
            {
                if(IS_VALID_OBJ(m_cameras[deviceIndex][0]))
                {
                    if(temIndex -1 <= m_firstIndex && (IS_VALID_OBJ(m_scrollbar)))
                    {
                        m_scrollbar->updateBarGeometry(-1);
                    }
                    if(((m_cameraCount[deviceIndex] -1 ) < MAX_CAMERA_TILE) && (m_cameraCount[deviceIndex] > 0))
                    {
                        m_cameras[deviceIndex][m_cameraCount[deviceIndex]-1]->forceActiveFocus();
                    }
                    return;
                }
            }

            if(deviceIndex < 0)
                deviceIndex = 0;


            if(temIndex-1 <= m_firstIndex && (IS_VALID_OBJ(m_scrollbar)))
            {
                m_scrollbar->updateBarGeometry(-1);
            }
            m_devices[deviceIndex]->forceActiveFocus();
            return;

        }


        if(IS_VALID_OBJ(m_cameras[deviceIndex][0]))
        {
            for(int camIndex=0; camIndex < m_cameraCount[deviceIndex]; camIndex++)
            {
                temIndex++;

                if(m_cameras[deviceIndex][camIndex]->hasFocus())
                {
                    if(camIndex - 1 >= 0 )
                    {

                        if(temIndex -1 <= m_firstIndex && (IS_VALID_OBJ(m_scrollbar)))
                        {
                            m_scrollbar->updateBarGeometry(-1);
                        }
                        m_cameras[deviceIndex][camIndex -1]->forceActiveFocus();


                        return;

                    }
                    else
                    {


                        if(temIndex -1 <= m_firstIndex  && (IS_VALID_OBJ(m_scrollbar)))
                        {
                            m_scrollbar->updateBarGeometry(-1);
                        }
                        m_devices[deviceIndex]->forceActiveFocus();


                        return;

                    }
                }
            }
        }
    }
}
void CameraList::takeDownKeyAction()
{
    int temIndex=0;
    for(quint8 deviceIndex =0; deviceIndex<m_deviceCount; deviceIndex++)
    {
        temIndex++;
        if(m_devices[deviceIndex]->hasFocus())
        {

            if(IS_VALID_OBJ(m_cameras[deviceIndex][0]))
            {

                if((temIndex  >= m_lastIndex) && (IS_VALID_OBJ(m_scrollbar)))
                {
                    m_scrollbar->updateBarGeometry(1);
                }
                m_cameras[deviceIndex][0]->forceActiveFocus();
                return;
            }

            else
            {
                deviceIndex = (deviceIndex + 1);
                if(deviceIndex > (m_deviceCount - 1))
                    deviceIndex = m_deviceCount-1;

                if((temIndex  >= m_lastIndex) && (IS_VALID_OBJ(m_scrollbar)))
                {

                    m_scrollbar->updateBarGeometry(1);
                }

                m_devices[deviceIndex]->forceActiveFocus();
                return;

            }
        }

        if(IS_VALID_OBJ(m_cameras[deviceIndex][0]))
        {
            for(quint8 camIndex=0; camIndex < m_cameraCount[deviceIndex]; camIndex++)
            {
                temIndex++;
                if(m_cameras[deviceIndex][camIndex]->hasFocus())
                {
                    if((camIndex +1) < m_cameraCount[deviceIndex] )
                    {

                        if((temIndex >= m_lastIndex) && (IS_VALID_OBJ(m_scrollbar)))
                        {
                            m_scrollbar->updateBarGeometry(1);
                        }

                        m_cameras[deviceIndex][camIndex+1]->forceActiveFocus();

                        return;

                    }
                    else
                    {
                        deviceIndex = (deviceIndex + 1);
                        if(deviceIndex > (m_deviceCount - 1))
                            deviceIndex = m_deviceCount-1;

                        if((temIndex >= m_lastIndex) && (IS_VALID_OBJ(m_scrollbar)))
                        {
                            m_scrollbar->updateBarGeometry(1);
                        }

                        m_devices[deviceIndex]->forceActiveFocus();

                        return;

                    }
                }


            }
        }
    }
}


void CameraList::forceFocusToPage(bool isFirstElement)
{
    quint8 total=0;
    for(quint8 deviceIndex = 0; deviceIndex <m_deviceCount; deviceIndex++)
    {
        if(total == m_firstIndex)
        {
            m_devices[deviceIndex]->forceActiveFocus();
            return;
        }
        total++;
        if(IS_VALID_OBJ(m_cameras[deviceIndex][0]))
        {
            for(quint8 camIndex=0; camIndex < m_cameraCount[deviceIndex]; camIndex++)
            {
                if(total == m_firstIndex)
                {
                    m_cameras[deviceIndex][camIndex]->forceActiveFocus();

                    return;

                }
                total++;
            }
        }
    }
    Q_UNUSED(isFirstElement);
}

void CameraList::navigationKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Up:
        event->accept();
        takeUpKeyAction();
        break;

    case Qt::Key_Down:
        event->accept();
        takeDownKeyAction();
        break;

    default:
        event->accept();
        break;
    }
}

void CameraList::wheelEvent(QWheelEvent *event)
{
    if((m_scrollbar != NULL))
    {
        if((event->y () >= m_scrollbar->y ()) &&
                event->y () <= (m_scrollbar->y () + m_scrollbar->height ()))
        {
            m_scrollbar->wheelEvent(event);
        }
    }
}

void CameraList::slotCameraButtonClicked(int index,
                                         CAMERA_STATE_TYPE_e connectionState ,int deviceIndex)
{
    bool isChangeSelection = false;
    bool pageSwitchFlag = (connectionState == CAM_STATE_NONE);
    quint16 windowIndex = MAX_CHANNEL_FOR_SEQ;
    quint8 channelIndex = MAX_WINDOWS;
    bool status = true;

    switch(m_parentClass)
    {
    case CALLED_BY_DISPLAY_SETTING:

        if(index == 0)
        {
            quint8 cameraId[MAX_CAMERAS];
            quint8 lastCameraIndex = 0;

            for(quint8 cameraIndex = 1; cameraIndex < m_cameraCount[deviceIndex]; cameraIndex++)
            {
                if((connectionState == CAM_STATE_ASSIGNED)
                        || ((connectionState == CAM_STATE_NONE)
                            && (m_cameras[deviceIndex][cameraIndex]->getConnectionState() == CAM_STATE_NONE)))
                {
                    cameraId[lastCameraIndex++] = cameraIndex;
                }
            }

            for(quint8 cameraIndex = 0; cameraIndex < lastCameraIndex; cameraIndex++)
            {
                if((connectionState == CAM_STATE_NONE)
                        && (cameraIndex < (lastCameraIndex - 1)))
                {
                    pageSwitchFlag = false;
                }
                else if(connectionState == CAM_STATE_NONE)
                {
                    pageSwitchFlag = true;
                }
                else
                {
                    pageSwitchFlag = false;
                }

                cameraIndex == (lastCameraIndex - 1) ? isChangeSelection = true : isChangeSelection = false;
                emit sigCameraButtonClicked(m_cameraIndex[deviceIndex][cameraId[cameraIndex] - 1],
                        m_deviceNameList.at(deviceIndex),
                        connectionState,
                        pageSwitchFlag,
                        isChangeSelection);
            }

            emit sigCameraConfigListUpdate();
        }
        else
        {
            isChangeSelection = true;
            emit sigCameraButtonClicked(m_cameraIndex[deviceIndex][index - 1],
                    m_deviceNameList.at(deviceIndex),
                    connectionState,
                    pageSwitchFlag,
                    isChangeSelection);

            emit sigCameraConfigListUpdate();
        }
        break;
    case CALLED_BY_WINDOWSEQ_SETTING:
        if(index == 0)
        {
            quint8 cameraId[MAX_CAMERAS];
            quint8 lastCameraIndex = 0;
            CAMERA_STATE_TYPE_e tempConnectionState = connectionState;

            for(quint8 cameraIndex = 1; cameraIndex < m_cameraCount[deviceIndex]; cameraIndex++)
            {
                if((connectionState == CAM_STATE_ASSIGNED)
                        || ((connectionState == CAM_STATE_NONE)
                            && (m_cameras[deviceIndex][cameraIndex]->getConnectionState() == CAM_STATE_NONE)))
                {
                    cameraId[lastCameraIndex++] = cameraIndex;
                }
            }

            for(quint8 cameraIndex = 0; cameraIndex < lastCameraIndex; cameraIndex++)
            {
                emit sigCameraButtonClickedWinSeq(m_cameraIndex[deviceIndex][cameraId[cameraIndex] - 1],
                        m_deviceNameList.at(deviceIndex),
                        connectionState,
                        tempConnectionState);
            }
        }
        else
        {
            emit sigCameraButtonClickedWinSeq(m_cameraIndex[deviceIndex][index - 1],
                    m_deviceNameList.at(deviceIndex),
                    connectionState);
        }
        m_cameras[deviceIndex][index]->forceActiveFocus();
        break;
    case CALLED_BY_VIEWCAM_ADD_LIST:

        index = m_cameraIndex[deviceIndex][index];

        switch(connectionState)
        {
        case CAM_STATE_NONE:
        case CAM_STATE_ASSIGNED:
            if(connectionState == CAM_STATE_NONE)
            {
                channelIndex = Layout::currentDisplayConfig[MAIN_DISPLAY].windowInfo[m_windowIndex].currentChannel;
                if(channelIndex < MAX_WIN_SEQ_CAM){
                    Layout::currentDisplayConfig[MAIN_DISPLAY].windowInfo[m_windowIndex].camInfo[channelIndex].defChannel = (index);
                    snprintf(Layout::currentDisplayConfig[MAIN_DISPLAY].windowInfo[m_windowIndex].camInfo[channelIndex].deviceName,MAX_DEVICE_NAME_SIZE,"%s",
                           QString(m_deviceNameList.at(deviceIndex)).toUtf8().constData());
                    windowIndex = m_windowIndex;
                }
            }
            else
            {
                status = Layout::findWindowIndexIfAssignOnAnyPage(MAIN_DISPLAY,
                                                                  m_deviceNameList.at(deviceIndex),
                                                                  (index),
                                                                  windowIndex,
                                                                  channelIndex);
                if(status == true)
                {
                    if(channelIndex < MAX_WIN_SEQ_CAM) {
                        Layout::currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[channelIndex].defChannel = INVALID_CAMERA_INDEX;
                        snprintf(Layout::currentDisplayConfig[MAIN_DISPLAY].windowInfo[windowIndex].camInfo[channelIndex].deviceName,MAX_DEVICE_NAME_SIZE,"%s", "");
                    }

                    channelIndex = Layout::currentDisplayConfig[MAIN_DISPLAY].windowInfo[m_windowIndex].currentChannel;
                    if(channelIndex < MAX_WIN_SEQ_CAM) {
                        Layout::currentDisplayConfig[MAIN_DISPLAY].windowInfo[m_windowIndex].camInfo[channelIndex].defChannel = (index);
                        snprintf(Layout::currentDisplayConfig[MAIN_DISPLAY].windowInfo[m_windowIndex].camInfo[channelIndex].deviceName,MAX_DEVICE_NAME_SIZE,"%s",
                               QString(m_deviceNameList.at(deviceIndex)).toUtf8().constData());
                    }
                }
            }

            if(status == true)
            {
                emit sigStartStreamInWindow(MAIN_DISPLAY,
                                            m_deviceNameList.at(deviceIndex),
                                            (index),
                                            m_windowIndex);
            }
            break;

        case CAM_STATE_CONNECTING:
        case CAM_STATE_LIVE_STREAM:
        case CAM_STATE_RETRY:
            windowIndex = Layout::findWindowIndexIfAssignOnCurrentPage(MAIN_DISPLAY,
                                                                       m_deviceNameList.at(deviceIndex),
                                                                       (index));
            if(windowIndex != MAX_CHANNEL_FOR_SEQ)
            {
                emit sigSwapWindows(windowIndex, m_windowIndex);
            }
            break;

        default:
            break;
        }

        emit sigClosePage(MAX_TOOLBAR_BUTTON);


        break;
    default:
        break;
    }
}


void CameraList::slotDeviceButtonCliked(int index)
{

    if(IS_VALID_OBJ(m_cameras[index][0])) //same device clicked
    {
        m_devices[index]->setShowClickedImage(false);
        m_devices[index]->changeDevSelectionstate(DEV_DESELECTED);
        deleteObject(index);
        updateIndex();
        setCameraListGeometry();

        if(IS_VALID_OBJ(m_toolTip))
        {
            m_toolTip->deleteLater();
            m_toolTip = NULL;
        }
    }
    else
    {
        if((m_devices[index]->getConnectionstate () == CONNECTED)
                || (m_devices[index]->getConnectionstate () == CONFLICT))
        {

            updateCameraList(index);
            updateIndex();
            if(!m_cameraNameList.isEmpty())
                setCameraListGeometry();

        }
        else
        {

            m_devices[index]->setShowClickedImage(false);
            m_devices[index]->changeDevSelectionstate (DEV_DESELECTED);
            deleteObject(index);
            updateIndex();
            setCameraListGeometry();

            m_devices[index]->setShowClickedImage(false);
            m_devices[index]->changeDevSelectionstate (DEV_DESELECTED);
        }
    }
}



void CameraList::slotDevStateChangeBtnClick(int index,
                                            DEVICE_STATE_TYPE_e connectionState)
{

    switch(connectionState)
    {
    case CONNECTED:
        m_applController->processActivity(m_deviceNameList.at (index),
                                          DISCONNECT_DEVICE, NULL);
        break;

    case DISCONNECTED:
    case LOGGED_OUT:
        m_applController->processActivity(m_deviceNameList.at (index),
                                          CONNECT_DEVICE, NULL);
        break;

    default:
        break;
    }
}

void CameraList::slotScroll(int numberOfSteps)
{
    if(m_toolTip != NULL)
    {
        m_toolTip->deleteLater();
        m_toolTip = NULL;
    }

    if(((m_firstIndex+ numberOfSteps) >= 0) && ((m_lastIndex + numberOfSteps) <= m_totalElement))
    {
        m_firstIndex = m_firstIndex + numberOfSteps;
        m_lastIndex = m_lastIndex + numberOfSteps;
        resetGeometryForScroll();
    }
    else
    {
        int tempVal = numberOfSteps > 0 ? -1 : 1;
        while( (numberOfSteps + tempVal) != 0)
        {
            numberOfSteps = numberOfSteps + tempVal;
            if(((m_firstIndex+ numberOfSteps) >= 0) && ((m_lastIndex + numberOfSteps) <= m_totalElement))
            {
                m_firstIndex = m_firstIndex + numberOfSteps;
                m_lastIndex = m_lastIndex + numberOfSteps;
                resetGeometryForScroll();
                break;
            }
        }
    }
}

//This function shows the tool tip when mouse hover on the list
void CameraList::slotShowHideTooltip(quint16 startX,
                                     quint16 startY,
                                     int deviceIndex,
                                     int index,
                                     bool toShowTooltip)
{
    if(index > 0)
    {
        index = m_cameraIndex[deviceIndex][index - 1];
        if(toShowTooltip == true)
        {
            quint16 windowIndex;
            quint8 channelIndex;
            if(findWindowIndexOfDisplayInfo(deviceIndex,index, windowIndex, channelIndex))
            {
                if(!IS_VALID_OBJ(m_toolTip))
                {
                    /* PARASOFT: Memory Deallocated when slot called for Hide Tooltip (toShowTooltip = fasle) */
                    m_toolTip = new ToolTip(startX,
                                            startY+CAMERA_LIST_BUTTON_HEIGHT,
                                            QString("window") + QString(" %1").arg(windowIndex + 1),
                                            this,
                                            END_X_START_Y);
                    m_toolTip->show();
                }
            }
        }
        else
        {
            if(IS_VALID_OBJ(m_toolTip))
            {
                m_toolTip->deleteLater();
                m_toolTip = NULL;
            }
        }
    }
}

void CameraList::deleteObject(int deviceIndex)
{
    for(quint8 index=0; index <m_cameraCount[deviceIndex]; index++)
    {
        if(IS_VALID_OBJ(m_cameras[deviceIndex][index]))
        {
            disconnect(m_cameras[deviceIndex][index],
                       SIGNAL(sigButtonClicked(int,CAMERA_STATE_TYPE_e,int)),
                       this,
                       SLOT(slotCameraButtonClicked(int,CAMERA_STATE_TYPE_e,int)));

            disconnect(m_cameras[deviceIndex][index],
                       SIGNAL(sigShowHideDeviceToolTip(quint16,quint16,int,int,bool)),
                       this,
                       SLOT(slotShowHideTooltip(quint16,quint16,int,int,bool)));

            delete m_cameras[deviceIndex][index];
            m_cameras[deviceIndex][index]= NULL;

        }
    }
    m_cameraCount[deviceIndex]=0;
}


void CameraList::updateIndex()
{
    m_deviceStartIndex[0]=0;
    if(!IS_VALID_OBJ(m_cameras[0][0]))
        m_cameraCount[0]=0;

    for(quint8 deviceIndex=1; deviceIndex <m_deviceCount; deviceIndex++)
    {
        m_deviceStartIndex[deviceIndex]= m_deviceStartIndex[deviceIndex-1] + m_cameraCount[deviceIndex-1] + 1;
        if(!IS_VALID_OBJ(m_cameras[deviceIndex][0]))
            m_cameraCount[deviceIndex]=0;
    }
}
