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
    main.cpp \
    attys-ep.cpp \
    stim.cpp

HEADERS = \
    attys-ep.h \
    vepplot.h \
    dataplot.h \
    stim.h

CONFIG		+= qt release c++11

QT             	+= widgets

RESOURCES     = application.qrc

target.path     = /usr/local/bin
INSTALLS        += target

}



macx {
MOC_DIR = moc

OBJECTS_DIR = obj

LIBS += \
    -L/usr/local/lib \
    -liir \
    -lattyscomm

INCLUDEPATH += /usr/local/include

include ( /usr/local/Cellar/qwt/6.1.5/features/qwt.prf )
    
TMAKE_CXXFLAGS += -fno-exceptions

SOURCES = \
    vepplot.cpp \
    dataplot.cpp \
    main.cpp \
    attys-ep.cpp \
    stim.cpp

HEADERS = \
    attys-ep.h \
    vepplot.h \
    dataplot.h \
    stim.h

CONFIG		+= qt release c++11 qwt

QT             	+= widgets

RESOURCES     = application.qrc

target.path     = /usr/local/bin
INSTALLS        += target

}






win32 {

MOC_DIR = moc

OBJECTS_DIR = obj

Debug:LIBS += \
    -L/qwt/lib \
    -lqwtd \
	-L/iir1/Debug \
    -liir_static \
    -lws2_32 \
    -L../attys-comm/Debug \
    -lattyscomm_static

Release:LIBS += \
    -L/qwt/lib \
    -lqwt \
	-L/iir1/Release \
    -liir_static \
    -lws2_32 \
    -L../attys-comm/Release \
    -lattyscomm_static

INCLUDEPATH += /iir1
INCLUDEPATH += ../attys-comm
INCLUDEPATH += /qwt/src

SOURCES = \
    vepplot.cpp \
    dataplot.cpp \
    main.cpp \
    attys-ep.cpp \
    stim.cpp

HEADERS = \
    attys-ep.h \
    vepplot.h \
    dataplot.h \
    stim.h

Debug:CONFIG		+= qt debug c++11
Release:CONFIG		+= qt release c++11

QT   	+= widgets

RESOURCES     = application.qrc

}
