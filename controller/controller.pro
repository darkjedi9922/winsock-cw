#-------------------------------------------------
#
# Project created by QtCreator 2019-11-05T17:25:46
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = controller
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11
LIBS += -lws2_32
INCLUDEPATH += ../common

SOURCES += \
        main.cpp \
    ../common/Logger.cpp \
    ../common/WinSock.cpp \
    ControllerWindow.cpp \
    ../common/ClientSocket.cpp \
    ../common/SocketEventManager.cpp \
    Controller.cpp

HEADERS += \
    ../common/Logger.h \
    ../common/WinSock.h \
    ControllerWindow.h \
    ../common/ClientSocket.h \
    ../common/SocketEventManager.h \
    ../common/definitions.h \
    Controller.h

FORMS += \
    ControllerWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
