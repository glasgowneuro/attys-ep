MOC_DIR = moc

OBJECTS_DIR = obj

LIBS += \
    -L/usr/local/lib \
    -lqwt-qt5 \
    -liir \
    -lbluetooth

INCLUDEPATH += ../AttysComm/c/

TMAKE_CXXFLAGS += -fno-exceptions

SOURCES = \
    psthplot.cpp \
    dataplot.cpp \
    main.cpp \
    attys-ep.cpp \
    stim.cpp \
    ../AttysComm/c/AttysComm.cpp \
    ../AttysComm/c/AttysScan.cpp \
    ../AttysComm/c/base64.c

HEADERS = \
    attys-ep.h \
    psthplot.h \
    dataplot.h \
    stim.h \
    ../AttysComm/c/AttysComm.h \
    ../AttysComm/c/AttysScan.h \
    ../AttysComm/c/base64.h

CONFIG		+= qt release c++11

QT             	+= widgets
QT += printsupport
