unix:!macx {
MOC_DIR = moc

OBJECTS_DIR = obj

LIBS += \
    -L/usr/local/lib \
    -lqwt-qt5 \
    -liir \
    -lattyscomm \
    -lbluetooth

INCLUDEPATH += /usr/include/qwt

TMAKE_CXXFLAGS += -fno-exceptions

SOURCES = \
    vepplot.cpp \
    dataplot.cpp \
    mainwindow.cpp \
    attys-ep.cpp \
    stim.cpp \
    audiobeep.cpp

HEADERS = \
    attys-ep.h \
    vepplot.h \
    dataplot.h \
    stim.h \
    audiobeep.h \
    mainwindow.h

CONFIG		+= qt debug c++11

QT            	+= widgets
QT		+= multimedia

RESOURCES     = application.qrc

target.path     = /usr/local/bin
INSTALLS        += target

}








win32 {

MOC_DIR = moc

OBJECTS_DIR = obj

Debug:LIBS += \
    -L/qwt-6.1.3/lib \
    -lqwtd \
	-L/iir1/Debug \
    -liir_static \
    -lws2_32 \
    -L../attys-comm/Debug \
    -lattyscomm_static

Release:LIBS += \
    -L/qwt-6.1.3/lib \
    -lqwt \
	-L/iir1/Release \
    -liir_static \
    -lws2_32 \
    -L../attys-comm/Release \
    -lattyscomm_static

INCLUDEPATH += /iir1
INCLUDEPATH += ../attys-comm
INCLUDEPATH += /qwt-6.1.3/src

SOURCES = \
    vepplot.cpp \
    dataplot.cpp \
    mainwindow.cpp \
    attys-ep.cpp \
    stim.cpp \
    audiobeep.cpp

HEADERS = \
    attys-ep.h \
    vepplot.h \
    dataplot.h \
    stim.h \
    audiobeep.h \
    mainwindow.h

Debug:CONFIG		+= qt debug c++11
Release:CONFIG		+= qt release c++11

QT   	+= widgets
QT		+= multimedia

RESOURCES     = application.qrc

DEFINES += QWT_DLL

}
