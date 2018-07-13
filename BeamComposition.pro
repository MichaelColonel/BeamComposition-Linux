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

win32:INCLUDEPATH += C:\root\include

#INCLUDEPATH += /usr/local/GATE/include/root

#LIBS += -L/usr/local/GATE/lib/root -lCore -lCint -lRIO -lNet -lHist \
#        -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix \
#        -lPhysics -lMathCore -lThread -pthread -lm -ldl -rdynamic

# With C++11 support
greaterThan( QT_MAJOR_VERSION, 4) {
    QT += widgets
    CONFIG += c++11
} else {
    QMAKE_CXXFLAGS += -std=c++11
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
    opcuaclientdialog.cpp

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
    opcuaclientdialog.h

FORMS    += mainwindow.ui \
    rootcanvasdialog.ui \
    settingsdialog.ui \
    signalvaluedialog.ui \
    opcuaclientdialog.ui

unix:LIBS += -lftd2xx

RESOURCES += BeamComposition.qrc

TRANSLATIONS += BeamComposition_ru.ts

win32:LIBS += -L$$PWD/../FTDI_DriverNew/i386/ -lftd2xx
win32:INCLUDEPATH += $$PWD/../FTDI_DriverNew
