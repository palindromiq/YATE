QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    analysisviewitem.cpp \
    analysisviewmodel.cpp \
    analysiswindow.cpp \
    huntinfo.cpp \
    livefeedbackoverlay.cpp \
    main.cpp \
    settingsdialog.cpp \
    yatewindow.cpp

HEADERS += \
    analysisviewitem.h \
    analysisviewmodel.h \
    analysiswindow.h \
    globals.h \
    huntinfo.h \
    livefeedbackoverlay.h \
    settingsdialog.h \
    yatewindow.h

FORMS += \
    analysiswindow.ui \
    livefeedbackoverlay.ui \
    settingsdialog.ui \
    yatewindow.ui

RESOURCES = breeze.qrc

RC_ICONS = yate.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
