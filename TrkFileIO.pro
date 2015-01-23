TEMPLATE = lib
CONFIG += staticlib
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11
SOURCES += main.cpp \
    trkfileio.cpp

include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    trkfileio.h \
    defs.h

CONFIG(debug, debug|release){
    TARGET = TrkFileIOd
}
CONFIG(release, debug|release){
    TARGET = TrkFileIO
}

DESTDIR = $${OUT_PWD}
