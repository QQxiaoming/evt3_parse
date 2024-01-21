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

include(./QGoodWindow/QGoodWindow/QGoodWindow.pri)
include(./QGoodWindow/QGoodCentralWidget/QGoodCentralWidget.pri)

SOURCES += \
    eventsensordatainput.cpp \
    eventsensorrender.cpp \
    eventsensorwidget.cpp \
    main.cpp \
    mainwindow.cpp

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
    QMAKE_TARGET_PRODUCT = "evt3_parse"
    QMAKE_TARGET_DESCRIPTION = "evt3_parse based on Qt $$[QT_VERSION]"
    QMAKE_TARGET_COPYRIGHT = "GNU General Public License v3.0"
}

unix:!macx:{
    QMAKE_RPATHDIR=$ORIGIN
    QMAKE_LFLAGS += -no-pie 
    LIBS += -lutil
}

macx:{
    QMAKE_RPATHDIR=$ORIGIN
    LIBS += -lutil
}

HEADERS += \
    eventsensordatainput.h \
    eventsensorrender.h \
    eventsensorwidget.h \
    config.h \
    mainwindow.h

