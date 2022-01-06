QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
VERSION = 1.0.0
SOURCES += \
    analysisviewitem.cpp \
    analysisviewmodel.cpp \
    analysiswindow.cpp \
    downloader.cpp \
    eeparser.cpp \
    filewatcher.cpp \
    huntimagegenerator.cpp \
    huntinfo.cpp \
    huntinfogenerator.cpp \
    livefeedbackoverlay.cpp \
    logevent.cpp \
    main.cpp \
    settingsdialog.cpp \
    updater.cpp \
    yatewindow.cpp

HEADERS += \
    analysisviewitem.h \
    analysisviewmodel.h \
    analysiswindow.h \
    downloader.h \
    eeparser.h \
    filewatcher.h \
    globals.h \
    huntimagegenerator.h \
    huntinfo.h \
    huntinfogenerator.h \
    livefeedbackoverlay.h \
    logevent.h \
    settingsdialog.h \
    updater.h \
    yatewindow.h

FORMS += \
    analysiswindow.ui \
    livefeedbackoverlay.ui \
    settingsdialog.ui \
    yatewindow.ui

RESOURCES = assets/breeze.qrc

RC_ICONS = yate.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
