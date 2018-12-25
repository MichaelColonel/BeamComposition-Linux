#-------------------------------------------------
#
# Project created by QtCreator 2015-12-10T11:21:43
#
#-------------------------------------------------

QT += core gui

CONFIG += qt warn_on thread

!exists ($(ROOTSYS)/include/rootcint.pri) {
    message (The Rootcint.pri was not found)
}
exists ($(ROOTSYS)/include/rootcint.pri) {
    include ($(ROOTSYS)/include/rootcint.pri)
#    CREATE_ROOT_DICT_FOR_CLASSES  = ${HEADERS} MyParticle.h MyDetector.h MyEvent.h ShowerMain.h
#    CREATE_ROOT_DICT_FOR_CLASSES *= ${HEADERS} RSLinkDef.h
}

#INCLUDEPATH += /usr/local/GATE/include/root

#LIBS += -L/usr/local/GATE/lib/root -lCore -lCint -lRIO -lNet -lHist \
#        -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix \
#        -lPhysics -lMathCore -lThread -pthread -lm -ldl -rdynamic

# With C++11 support
greaterThan( QT_MAJOR_VERSION, 4) {
    QT += widgets serialport
    CONFIG += c++11
} else {
    QMAKE_CXXFLAGS += -std=c++11
    CONFIG += serialport
}

TARGET = BeamComposition

TEMPLATE = app

SOURCES += main.cpp \
    canvas.cpp \
    rootcanvasdialog.cpp \
    acquisitionthread.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    writeprocess.cpp \
    diagramtreewidgetitem.cpp \
    diagramtreewidgetaction.cpp \
    ccmath_lib.c \
    channelscountsfit.cpp \
    signalvaluedelegate.cpp \
    backgroundvaluedelegate.cpp \
    signalvaluedialog.cpp \
    channelschargefit.cpp \
    commandthread.cpp \
    chargevaluedelegate.cpp \
    rundetailslistwidgetitem.cpp \
    opcuaclientdialog.cpp \
    opcuaclient.cpp \
    open62541.c \
    runevent.cpp

HEADERS  += mainwindow.h \
    canvas.h \
    rootcanvasdialog.h \
    acquisitionthread.h \
    typedefs.h \
    settingsdialog.h \
    diagramparameters.h \
    writeprocess.h \
    diagramtreewidgetitem.h \
    diagramtreewidgetaction.h \
    ccmath_lib.h \
    channelscountsfit.h \
    signalvaluedelegate.h \
    backgroundvaluedelegate.h \
    signalvaluedialog.h \
    runinfo.h \
    channelschargefit.h \
    commandthread.h \
    chargevaluedelegate.h \
    rundetailslistwidgetitem.h \
    opcuaclientdialog.h \
    opcuaclient.h \
    open62541.h \
    runevent.h

FORMS    += mainwindow.ui \
    rootcanvasdialog.ui \
    settingsdialog.ui \
    signalvaluedialog.ui \
    opcuaclientdialog.ui


unix {
#    CONFIG += link_pkgconfig
#    PKGCONFIG += open62541
    LIBS += -lftd2xx
}

win32 {
    LIBS += -L$$PWD/../FTDI_DriverNew/i386/ -lftd2xx
    INCLUDEPATH += C:\root\include
    INCLUDEPATH += $$PWD/../FTDI_DriverNew
}

RESOURCES += BeamComposition.qrc

TRANSLATIONS += BeamComposition_ru.ts

