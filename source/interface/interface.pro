QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    widget.cpp

HEADERS += \
    widget.h

FORMS += \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

isEmpty(TARGET_EXT) {
    win32 {
        TARGET_CUSTOM_EXT = .exe
    }
    macx {
        TARGET_CUSTOM_EXT = .app
    }
} else {
    TARGET_CUSTOM_EXT = $${TARGET_EXT}
}

win32 {
    DEPLOY_COMMAND = $$[QT_INSTALL_BINS]/windeployqt
    PLUGIN_INTERFACE_FINAL_DIR = $$shell_quote($$shell_path("C:/X-Plane 11/Resources/plugins/TeleportAircraft/win_x64/interface"))
}
macx {
    DEPLOY_COMMAND = $$[QT_INSTALL_BINS]/macdeployqt
}

CONFIG( debug, debug|release ) {
    # debug
    DEPLOY_FOLDER = $$shell_quote($$shell_path($${OUT_PWD}/debug))
    DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/debug/$${TARGET}$${TARGET_CUSTOM_EXT}))
} else {
    # release
    DEPLOY_FOLDER = $$shell_quote($$shell_path($${OUT_PWD}/release))
    DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/release/$${TARGET}$${TARGET_CUSTOM_EXT}))
}

QMAKE_POST_LINK = $${DEPLOY_COMMAND} $${DEPLOY_TARGET} --no-translations $$escape_expand(\n\t)
QMAKE_POST_LINK += if exist $${PLUGIN_INTERFACE_FINAL_DIR} del /S/Q $${PLUGIN_INTERFACE_FINAL_DIR}\*  $$escape_expand(\n\t)
QMAKE_POST_LINK += xcopy $${DEPLOY_FOLDER} $${PLUGIN_INTERFACE_FINAL_DIR}\ /E/C $$escape_expand(\n\t)
QMAKE_POST_LINK += del /S/Q $${PLUGIN_INTERFACE_FINAL_DIR}\*.exp  $$escape_expand(\n\t)
QMAKE_POST_LINK += del /S/Q $${PLUGIN_INTERFACE_FINAL_DIR}\*.ilk  $$escape_expand(\n\t)
QMAKE_POST_LINK += del /S/Q $${PLUGIN_INTERFACE_FINAL_DIR}\*.lib  $$escape_expand(\n\t)
QMAKE_POST_LINK += del /S/Q $${PLUGIN_INTERFACE_FINAL_DIR}\*.pdb  $$escape_expand(\n\t)
QMAKE_POST_LINK += del /S/Q $${PLUGIN_INTERFACE_FINAL_DIR}\*.obj  $$escape_expand(\n\t)
QMAKE_POST_LINK += del /S/Q $${PLUGIN_INTERFACE_FINAL_DIR}\*.h  $$escape_expand(\n\t)
QMAKE_POST_LINK += del /S/Q $${PLUGIN_INTERFACE_FINAL_DIR}\*.cpp  $$escape_expand(\n\t)
