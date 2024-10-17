message("Configuring for BOARD_TYPE=$(BOARD_TYPE) OEM_TYPE=$(OEM_TYPE)")

# This variable contains the name of the template to use when generating the project
TEMPLATE = app

# Target file name
TARGET      = nvrTestApp
OBJECTS_DIR = obj
MOC_DIR     = moc
DESTDIR     = bin

# QT varaible control for add on feature in qt
QT += core gui network widgets printsupport

# Define Software Version & Communication Version
DEFINES += $(OEM_TYPE)
DEFINES += SOFTWARE_VERSION=$(SOFTWARE_VERSION)
DEFINES += SOFTWARE_REVISION=$(SOFTWARE_REVISION)
DEFINES += PRODUCT_SUB_REVISION=$(PRODUCT_SUB_REVISION)
DEFINES += COMMUNICATION_VERSION=$(COMMUNICATION_VERSION)
DEFINES += COMMUNICATION_REVISION=$(COMMUNICATION_REVISION)

INCLUDEPATH += ../../Application/DebugLog
INCLUDEPATH += ../../Application/Utils
INCLUDEPATH += $(GPIO_DRIVER_INCLUDE_PATH)
INCLUDEPATH += ../../../DecoderLib/include

contains(DEFINES, (HI3536_NVRL|HI3536_NVRH)) {

    INCLUDEPATH += $(HW_DECODER_INCLUDE_PATH)
}

# Header files for the project
HEADERS += \
    ../../*.h \
    ../../Controls/*.h \
    ../../Application/DebugLog/DebugLog.h \
    ../../Application/Utils/CommonApi.h

# Source files for the project
SOURCES += \
    ../../*.cpp \
    ../../Controls/*.cpp \
    ../../Application/DebugLog/DebugLog.c \
    ../../Application/Utils/CommonApi.c

QMAKE_CFLAGS += -DGUI_SYSTEST -Werror
QMAKE_CXXFLAGS += -DGUI_SYSTEST -Werror
!contains(DEFINES, (HI3536_NVRL|HI3536_NVRH)) {

    QMAKE_CXXFLAGS += -Wno-class-memaccess -Wno-format-truncation -Wno-deprecated-copy -Wno-deprecated-declarations
}

contains(DEFINES, (HI3536_NVRL|HI3536_NVRH)) {

    LIBS += -lEGL -lmali -lOpenCL -lGLESv2 -lGLESv1_CM
    LIBS += -L$(HW_DECODER_LIB_PATH) -lhdmi -live -lhive_common -lhive_EQ -lhive_GAIN -lhive_HDR -lhive_HPF -ldnvqe -lhive_AGC -lhive_MBC -lhive_RES -lhive_RNR -lmpi -lVoiceEngine -lupvqe
}

contains(DEFINES, (RK3568_NVRL|RK3588_NVRH)) {

    LIBS += -lrockit
}

LIBS += -L../../../DecoderLib/Build/$(APP_BUILD_TAG)/lib -lDecDisplay -ludev

RESOURCES += Resources/nvrTestApp.qrc
RESOURCES += Resources/Images/$$(BRAND_TAG)_Images/customBrandImages.qrc
