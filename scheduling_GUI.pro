#-------------------------------------------------
#
# Project created by QtCreator 2017-09-29T11:36:13
#
#-------------------------------------------------

QT       += core gui
QT       += charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = scheduling_GUI
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#INCLUDEPATH += /home/mschartn/boost_1_64_0/
#LIBS += "-LC:/home/mschartn/boost_1_64_0//stage/lib/"
INCLUDEPATH += C:/boost/
LIBS += "-LC:/boost/stage/lib/" \
         /home/mschartn/programming/scheduling_GUI/VLBI_Scheduler/libsofa_c.a


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    chartview.cpp \
    callout.cpp \
    multischededitdialogint.cpp \
    multischededitdialogdouble.cpp \
    multischededitdialogdatetime.cpp \
    addgroupdialog.cpp \
    stationparametersdialog.cpp \
    sourceparametersdialog.cpp \
    baselineparametersdialog.cpp \
    settingsloadwindow.cpp \
    addbanddialog.cpp \
    savetosettingsdialog.cpp \
    mytextbrowser.cpp \
    VLBI_Scheduler/Antenna.cpp \
    VLBI_Scheduler/Baseline.cpp \
    VLBI_Scheduler/CableWrap.cpp \
    VLBI_Scheduler/CalibratorBlock.cpp \
    VLBI_Scheduler/Earth.cpp \
    VLBI_Scheduler/Equipment.cpp \
    VLBI_Scheduler/FillinmodeEndposition.cpp \
    VLBI_Scheduler/Flux.cpp \
    VLBI_Scheduler/HorizonMask.cpp \
    VLBI_Scheduler/Initializer.cpp \
    VLBI_Scheduler/LookupTable.cpp \
    VLBI_Scheduler/MultiScheduling.cpp \
    VLBI_Scheduler/Nutation.cpp \
    VLBI_Scheduler/ObservationMode.cpp \
    VLBI_Scheduler/Output.cpp \
    VLBI_Scheduler/ParameterGroup.cpp \
    VLBI_Scheduler/ParameterSettings.cpp \
    VLBI_Scheduler/ParameterSetup.cpp \
    VLBI_Scheduler/PointingVector.cpp \
    VLBI_Scheduler/Position.cpp \
    VLBI_Scheduler/Scan.cpp \
    VLBI_Scheduler/ScanTimes.cpp \
    VLBI_Scheduler/Scheduler.cpp \
    VLBI_Scheduler/Skd.cpp \
    VLBI_Scheduler/SkdCatalogReader.cpp \
    VLBI_Scheduler/SkyCoverage.cpp \
    VLBI_Scheduler/Source.cpp \
    VLBI_Scheduler/Station.cpp \
    VLBI_Scheduler/Subcon.cpp \
    VLBI_Scheduler/TimeSystem.cpp \
    VLBI_Scheduler/Vex.cpp \
    VLBI_Scheduler/WeightFactors.cpp \
    VLBI_Scheduler/VieVS_Scheduler.cpp \
    VLBI_Scheduler/VieVS_Object.cpp \
    textfileviewer.cpp

HEADERS += \
        mainwindow.h \
    chartview.h \
    callout.h \
    multischededitdialogint.h \
    multischededitdialogdouble.h \
    multischededitdialogdatetime.h \
    addgroupdialog.h \
    stationparametersdialog.h \
    sourceparametersdialog.h \
    baselineparametersdialog.h \
    settingsloadwindow.h \
    addbanddialog.h \
    savetosettingsdialog.h \
    mytextbrowser.h \
    VLBI_Scheduler/Antenna.h \
    VLBI_Scheduler/Baseline.h \
    VLBI_Scheduler/CableWrap.h \
    VLBI_Scheduler/CalibratorBlock.h \
    VLBI_Scheduler/Constants.h \
    VLBI_Scheduler/Earth.h \
    VLBI_Scheduler/Equipment.h \
    VLBI_Scheduler/FillinmodeEndposition.h \
    VLBI_Scheduler/Flux.h \
    VLBI_Scheduler/HorizonMask.h \
    VLBI_Scheduler/Initializer.h \
    VLBI_Scheduler/LookupTable.h \
    VLBI_Scheduler/MultiScheduling.h \
    VLBI_Scheduler/Nutation.h \
    VLBI_Scheduler/ObservationMode.h \
    VLBI_Scheduler/Output.h \
    VLBI_Scheduler/ParameterGroup.h \
    VLBI_Scheduler/ParameterSettings.h \
    VLBI_Scheduler/ParameterSetup.h \
    VLBI_Scheduler/PointingVector.h \
    VLBI_Scheduler/Position.h \
    VLBI_Scheduler/Scan.h \
    VLBI_Scheduler/ScanTimes.h \
    VLBI_Scheduler/Scheduler.h \
    VLBI_Scheduler/Skd.h \
    VLBI_Scheduler/SkdCatalogReader.h \
    VLBI_Scheduler/SkyCoverage.h \
    VLBI_Scheduler/sofa.h \
    VLBI_Scheduler/sofam.h \
    VLBI_Scheduler/Source.h \
    VLBI_Scheduler/Station.h \
    VLBI_Scheduler/Subcon.h \
    VLBI_Scheduler/TimeSystem.h \
    VLBI_Scheduler/Vex.h \
    VLBI_Scheduler/WeightFactors.h \
    VLBI_Scheduler/VieVS_Scheduler.h \
    VLBI_Scheduler/VieVS_Object.h \
    textfileviewer.h

FORMS += \
        mainwindow.ui \
    multischededitdialogint.ui \
    multischededitdialogdouble.ui \
    multischededitdialogdatetime.ui \
    addgroupdialog.ui \
    stationparametersdialog.ui \
    sourceparametersdialog.ui \
    baselineparametersdialog.ui \
    settingsloadwindow.ui \
    addbanddialog.ui \
    savetosettingsdialog.ui \
    textfileviewer.ui

RESOURCES += \
    myresources.qrc

DISTFILES += \
    VLBI_Scheduler/libsofa_c.a \
    VLBI_Scheduler/CMakeLists.txt
