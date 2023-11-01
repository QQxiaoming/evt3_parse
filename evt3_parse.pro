!versionAtLeast(QT_VERSION, 6.2.0) {
    message("Cannot use Qt $$QT_VERSION")
    error("Use Qt 6.2.0 or newer")
}
QT += core gui network widgets
QT += core5compat

BUILD_VERSION=0.0.1
TARGET_ARCH=$${QT_ARCH}
CONFIG += c++11
DEFINES += QT_DEPRECATED_WARNINGS
QMAKE_CXXFLAGS += -Wno-deprecated-copy

SOURCES += \
    main.cpp

build_type =
CONFIG(debug, debug|release) {
    build_type = build_debug
} else {
    build_type = build_release
}

DESTDIR     = $$build_type/out
OBJECTS_DIR = $$build_type/obj
MOC_DIR     = $$build_type/moc
RCC_DIR     = $$build_type/rcc
UI_DIR      = $$build_type/ui

win32:{
    VERSION = $${BUILD_VERSION}.000
    RC_ICONS = "icons\ico.ico"
    QMAKE_TARGET_PRODUCT = "quardCRT"
    QMAKE_TARGET_DESCRIPTION = "quardCRT based on Qt $$[QT_VERSION]"
    QMAKE_TARGET_COPYRIGHT = "GNU General Public License v3.0"
}

unix:!macx:{
    QMAKE_RPATHDIR=$ORIGIN
    QMAKE_LFLAGS += -no-pie 
    LIBS += -lutil
}

macx:{
    QMAKE_RPATHDIR=$ORIGIN
    ICON = "icons\ico.icns"
    LIBS += -lutil
}

