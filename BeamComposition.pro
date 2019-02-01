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
}

# With C++11 support
# Add QSerialPort to write and read data from FT2232H chip
# via ftdi_sio kernel module as serial port
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
    chargevaluedelegate.cpp \
    rundetailslistwidgetitem.cpp \
    opcuaclientdialog.cpp \
    opcuaclient.cpp \
    open62541.c \
    runevent.cpp \
    rundetailsdialog.cpp \
    port.c \
    serialdevice.cpp

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
    chargevaluedelegate.h \
    rundetailslistwidgetitem.h \
    opcuaclientdialog.h \
    opcuaclient.h \
    open62541.h \
    runevent.h \
    rundetailsdialog.h \
    port.h \
    serialdevice.h

FORMS    += mainwindow.ui \
    rootcanvasdialog.ui \
    settingsdialog.ui \
    signalvaluedialog.ui \
    opcuaclientdialog.ui \
    rundetailsdialog.ui

win32 {
    LIBS += -L$$PWD/../FTDI_DriverNew/i386/ -lftd2xx
    INCLUDEPATH += C:/root/include
    INCLUDEPATH += $$PWD/../FTDI_DriverNew
}

RESOURCES += BeamComposition.qrc

TRANSLATIONS += BeamComposition_ru.ts
