QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 DISCORD_ENABLED

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
VERSION = 1.0.4
SOURCES += \
    analysisviewitem.cpp \
    analysisviewmodel.cpp \
    analysiswindow.cpp \
    discordmanager.cpp \
    downloader.cpp \
    eeparser.cpp \
    filewatcher.cpp \
    huntimagegenerator.cpp \
    huntinfo.cpp \
    huntinfogenerator.cpp \
    livefeedbackoverlay.cpp \
    logevent.cpp \
    main.cpp \
    miniz.c \
    settingsdialog.cpp \
    updater.cpp \
    yatewindow.cpp \
    zipmanager.cpp

SOURCES += \
    discord_game_sdk/achievement_manager.cpp \
    discord_game_sdk/activity_manager.cpp \
    discord_game_sdk/application_manager.cpp \
    discord_game_sdk/core.cpp \
    discord_game_sdk/image_manager.cpp \
    discord_game_sdk/lobby_manager.cpp \
    discord_game_sdk/network_manager.cpp \
    discord_game_sdk/overlay_manager.cpp \
    discord_game_sdk/relationship_manager.cpp \
    discord_game_sdk/storage_manager.cpp \
    discord_game_sdk/store_manager.cpp \
    discord_game_sdk/types.cpp \
    discord_game_sdk/user_manager.cpp \
    discord_game_sdk/voice_manager.cpp

HEADERS += \
    analysisviewitem.h \
    analysisviewmodel.h \
    analysiswindow.h \
    discordmanager.h \
    downloader.h \
    eeparser.h \
    filewatcher.h \
    globals.h \
    huntimagegenerator.h \
    huntinfo.h \
    huntinfogenerator.h \
    livefeedbackoverlay.h \
    logevent.h \
    miniz.h \
    settingsdialog.h \
    updater.h \
    yatewindow.h \
    zipmanager.h

HEADERS += \
    discord_game_sdk/achievement_manager.h \
    discord_game_sdk/activity_manager.h \
    discord_game_sdk/application_manager.h \
    discord_game_sdk/core.h \
    discord_game_sdk/discord.h \
    discord_game_sdk/event.h \
    discord_game_sdk/ffi.h \
    discord_game_sdk/image_manager.h \
    discord_game_sdk/lobby_manager.h \
    discord_game_sdk/network_manager.h \
    discord_game_sdk/overlay_manager.h \
    discord_game_sdk/relationship_manager.h \
    discord_game_sdk/storage_manager.h \
    discord_game_sdk/store_manager.h \
    discord_game_sdk/types.h \
    discord_game_sdk/user_manager.h \
    discord_game_sdk/voice_manager.h

FORMS += \
    analysiswindow.ui \
    livefeedbackoverlay.ui \
    settingsdialog.ui \
    yatewindow.ui

RESOURCES = assets/breeze.qrc

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT
CONFIG(debug, debug|release) {
    message("Debug Mode")
    CONFIG += console
}

RC_ICONS = yate.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/discord_game_sdk/lib/x86_64/ -ldiscord_game_sdk.dll
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/discord_game_sdk/lib/x86_64/ -ldiscord_game_sdk.dll

INCLUDEPATH += $$PWD/discord_game_sdk/lib/x86_64
DEPENDPATH += $$PWD/discord_game_sdk/lib/x86_64
