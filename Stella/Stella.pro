# Add the Boost include path
INCLUDEPATH += F:\Boost\include\boost-1_65_1
# Add the Boost library path
LIBS += -LF:\Boost\lib
LIBS += -lboost_serialization-mgw112-mt-d-1_65_1 -lboost_system-mgw112-mt-d-1_65_1

# OpenCV configuration
INCLUDEPATH += D:/OpenCV/install/include
LIBS += -LD:\opencv\install\x64\mingw\bin -lopencv_world480
LIBS +=  -lws2_32

QT       += core gui
QT       += network
QT       += concurrent
#RC_ICONS = damotouicon.ico

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    StellaClient.cpp \
    StellaSessionManager.cpp \
    StellaSocketSession.cpp \
    main.cpp \
    receive.cpp \
    sender.cpp \
    widget.cpp

HEADERS += \
    StellaClient.h \
    StellaSerialization.h \
    StellaSessionManager.h \
    StellaSocketSession.h \
    StellaType.h \
    receive.h \
    sender.h \
    widget.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    qrc.qrc

DISTFILES += \
    damotouicon.ico
