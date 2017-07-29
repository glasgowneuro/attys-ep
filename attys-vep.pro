MOC_DIR = moc

OBJECTS_DIR = obj

LIBS += \
    -L/usr/local/lib \
    -lqwt-qt5 \
    -liir \
    -lbluetooth

INCLUDEPATH += ../AttysComm/

TMAKE_CXXFLAGS += -fno-exceptions

SOURCES = \
    psthplot.cpp \
    dataplot.cpp \
    main.cpp \
    attys-vep.cpp \
    stim.cpp \
    ../AttysComm/AttysComm.cpp \
    ../AttysComm/AttysScan.cpp \
    ../AttysComm/base64.c

HEADERS = \
    attys-vep.h \
    psthplot.h \
    dataplot.h \
    stim.h \
    ../AttysComm/AttysComm.h \
    ../AttysComm/AttysScan.h \
    ../AttysComm/base64.h

CONFIG		+= qt release c++11

QT             	+= widgets
QT += printsupport
