MOC_DIR = moc

OBJECTS_DIR = obj

LIBS += \
    -L/usr/local/lib \
    -lqwt-qt5 \
    -liir \
    -lbluetooth

INCLUDEPATH += ../attys_scope/

TMAKE_CXXFLAGS += -fno-exceptions

SOURCES = \
    psthplot.cpp \
    dataplot.cpp \
    main.cpp \
    attys-vep.cpp \
    stim.cpp \
    ../attys_scope/AttysComm.cpp \
    ../attys_scope/base64.c

HEADERS = \
    attys-vep.h \
    psthplot.h \
    dataplot.h \
    stim.h \
    ../attys_scope/AttysComm.h \
    ../attys_scope/base64.h

CONFIG		+= qt release c++11

QT             	+= widgets
QT += printsupport
