unix {
MOC_DIR = moc

OBJECTS_DIR = obj

LIBS += \
    -L/usr/local/lib \
    -lqwt-qt5 \
    -liir \
    -lattyscomm \
    -lbluetooth

INCLUDEPATH += ../AttysComm/c/
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

}






win32 {

MOC_DIR = moc

OBJECTS_DIR = obj

LIBS += \
    -L/qwt/lib \
    -lqwtd \
	-L/iir1/Debug \
    -liir_static \
    -lws2_32 \
    -L ../AttysComm/cpp \
    -lattyscomm_static

INCLUDEPATH += /iir1
INCLUDEPATH += ../AttysComm/cpp
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

CONFIG		+= qt debug c++11

QT   	+= widgets

RESOURCES     = application.qrc

}
