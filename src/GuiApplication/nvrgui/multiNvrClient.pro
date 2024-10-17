message("Configuring for BOARD_TYPE=$(BOARD_TYPE) OEM_TYPE=$(OEM_TYPE)")

# This variable contains the name of the template to use when generating the project
TEMPLATE = app

# Target file name
TARGET      = multiNvrClient
OBJECTS_DIR = obj
MOC_DIR     = moc
DESTDIR     = bin

# QT variable control for add on feature in qt
QT += core gui network opengl widgets


# Define Software Version & Communication Version
DEFINES += $(OEM_TYPE)
DEFINES += SOFTWARE_VERSION=$(SOFTWARE_VERSION)
DEFINES += SOFTWARE_REVISION=$(SOFTWARE_REVISION)
DEFINES += PRODUCT_SUB_REVISION=$(PRODUCT_SUB_REVISION)
DEFINES += COMMUNICATION_VERSION=$(COMMUNICATION_VERSION)
DEFINES += COMMUNICATION_REVISION=$(COMMUNICATION_REVISION)

INCLUDEPATH += ../../Application/DebugLog
INCLUDEPATH += ../../Application/Utils
INCLUDEPATH += ../../Application/Include
INCLUDEPATH += $(QRENCODE_INCLUDE_PATH)

contains(DEFINES, (HI3536_NVRL|HI3536_NVRH)) {

	INCLUDEPATH += $(HW_DECODER_INCLUDE_PATH)
}

# Header files for the project
HEADERS += \
	../../*.h \
	../../ConfigPages/BasicSettings/*.h \
	../../ConfigPages/CameraSettings/*.h \
	../../ConfigPages/DeviceIOSettings/*.h \
	../../ConfigPages/Devices/*.h \
	../../ConfigPages/EventAndActionSettings/*.h \
	../../ConfigPages/Maintenance/*.h \
	../../ConfigPages/NetworkSettings/*.h \
	../../ConfigPages/StorageAndBackupSettings/*.h \
	../../ConfigPages/UserAccountManagmentSettings/*.h \
	../../Configuration/*.h \
	../../Controls/*.h \
	../../Controls/ImageControls/*.h \
	../../Controls/IntantPlaybackControl/*.h \
	../../Controls/LiveViewToolbar/*.h \
	../../Controls/PlaybackControl/*.h \
	../../Controls/PTZControl/*.h \
	../../Controls/SyncPlayback/*.h \
	../../Controls/ViewCamera/*.h \
	../../DebugLog/*.h \
	../../DeviceClient/*.h \
	../../DeviceClient/CommandRequest/*.h \
	../../DeviceClient/ConnectRequest/*.h \
	../../DeviceClient/GenericRequest/*.h \
	../../DeviceClient/StreamRequest/*.h \
	../../DeviceClient/StreamRequest/AnalogMedia/*.h \
	../../DeviceClient/StreamRequest/ClientMedia/*.h \
	../../DeviceClient/StreamRequest/InstantPlaybackMedia/*.h \
	../../DeviceClient/StreamRequest/LiveMedia/*.h \
	../../DeviceClient/StreamRequest/PlaybackMedia/*.h \
	../../DeviceClient/StreamRequest/SyncPbMedia/*.h \
	../../Layout/*.h \
	../../Login/*.h \
	../../ManagePages/*.h \
	../../Toolbar/*.h \
	../../Utils/*.h \
	../../Wizard/*.h \
	../../Application/DebugLog/DebugLog.h \
	../../Application/Utils/CommonApi.h \
	../../Application/Utils/VideoParser.h

# Source files for the project
SOURCES += \
	../../*.cpp \
	../../ConfigPages/BasicSettings/*.cpp \
	../../ConfigPages/CameraSettings/*.cpp \
	../../ConfigPages/DeviceIOSettings/*.cpp \
	../../ConfigPages/Devices/*.cpp \
	../../ConfigPages/EventAndActionSettings/*.cpp \
	../../ConfigPages/Maintenance/*.cpp \
	../../ConfigPages/NetworkSettings/*.cpp \
	../../ConfigPages/StorageAndBackupSettings/*.cpp \
	../../ConfigPages/UserAccountManagmentSettings/*.cpp \
	../../Configuration/*.cpp \
	../../Controls/*.cpp \
	../../Controls/ImageControls/*.cpp \
	../../Controls/IntantPlaybackControl/*.cpp \
	../../Controls/LiveViewToolbar/*.cpp \
	../../Controls/PlaybackControl/*.cpp \
	../../Controls/PTZControl/*.cpp \
	../../Controls/SyncPlayback/*.cpp \
	../../Controls/ViewCamera/*.cpp \
	../../DebugLog/*.c \
	../../DeviceClient/*.cpp \
	../../DeviceClient/CommandRequest/*.cpp \
	../../DeviceClient/ConnectRequest/*.cpp \
	../../DeviceClient/GenericRequest/*.cpp \
	../../DeviceClient/StreamRequest/*.cpp \
	../../DeviceClient/StreamRequest/AnalogMedia/*.cpp \
	../../DeviceClient/StreamRequest/ClientMedia/*.cpp \
	../../DeviceClient/StreamRequest/InstantPlaybackMedia/*.cpp \
	../../DeviceClient/StreamRequest/LiveMedia/*.cpp \
	../../DeviceClient/StreamRequest/PlaybackMedia/*.cpp \
	../../DeviceClient/StreamRequest/SyncPbMedia/*.cpp \
	../../Layout/*.cpp \
	../../Login/*.cpp \
	../../ManagePages/*.cpp \
	../../Toolbar/*.cpp \
	../../Utils/*.c \
	../../Wizard/*.cpp \
	../../Application/DebugLog/DebugLog.c \
	../../Application/Utils/CommonApi.c \
	../../Application/Utils/VideoParser.c

QMAKE_CFLAGS += -DGUI_SYSTEM -Werror
QMAKE_CXXFLAGS += -DGUI_SYSTEM -Werror

!contains(DEFINES, (HI3536_NVRL|HI3536_NVRH)) {

	QMAKE_CXXFLAGS += -Wno-class-memaccess -Wno-format-truncation -Wno-deprecated-copy -Wno-deprecated-declarations
}

contains(DEFINES, (HI3536_NVRL|HI3536_NVRH)) {

	LIBS += -lEGL -lmali -lOpenCL -lGLESv2 -lGLESv1_CM
    LIBS += -L$(HW_DECODER_LIB_PATH) -lhdmi -live -lhive_common -lhive_EQ -lhive_GAIN -lhive_HDR -lhive_HPF -ldnvqe -lhive_AGC -lhive_MBC -lhive_RES -lhive_RNR -lmpi -lVoiceEngine -lupvqe
}

contains(DEFINES, (RK3568_NVRL|RK3588_NVRH)) {

	LIBS += -L$(HW_DECODER_LIB_PATH) -lrockit
}

LIBS += -L../../../DecoderLib/Build/$(APP_BUILD_TAG)/lib -lDecDisplay -lpng16 -L$(QRENCODE_LIB_PATH) -lqrencode

RESOURCES += Resources/NVR_X_Resource.qrc
