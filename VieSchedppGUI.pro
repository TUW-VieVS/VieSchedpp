#-------------------------------------------------
#
# Project created by QtCreator 2017-09-29T11:36:13
#
#-------------------------------------------------
CONFIG += c++14

QMAKE_CXXFLAGS+= -fopenmp
LIBS += -fopenmp

QT       += core gui
QT       += charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VieSchedppGUI
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
LIBS += ../VieSchedpp/libsofa_c.a


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
        ../VieSchedpp/Antenna.cpp \
        ../VieSchedpp/Antenna_AzEl.cpp \
        ../VieSchedpp/Antenna_HaDc.cpp \
        ../VieSchedpp/Antenna_XYew.cpp \
        ../VieSchedpp/AstronomicalParameters.cpp \
        ../VieSchedpp/Baseline.cpp \
        ../VieSchedpp/CableWrap.cpp \
        ../VieSchedpp/CableWrap_AzEl.cpp \
        ../VieSchedpp/CableWrap_HaDc.cpp \
        ../VieSchedpp/CableWrap_XYew.cpp \
        ../VieSchedpp/CalibratorBlock.cpp \
        ../VieSchedpp/Equipment.cpp \
        ../VieSchedpp/Equipment_elDependent.cpp \
        ../VieSchedpp/Flux.cpp \
        ../VieSchedpp/Flux_B.cpp \
        ../VieSchedpp/Flux_M.cpp \
        ../VieSchedpp/HighImpactScanDescriptor.cpp \
        ../VieSchedpp/HorizonMask.cpp \
        ../VieSchedpp/HorizonMask_line.cpp \
        ../VieSchedpp/HorizonMask_step.cpp \
        ../VieSchedpp/Initializer.cpp \
        ../VieSchedpp/LogParser.cpp \
        ../VieSchedpp/LookupTable.cpp \
        ../VieSchedpp/MultiScheduling.cpp \
        ../VieSchedpp/Network.cpp \
        ../VieSchedpp/Observation.cpp \
        ../VieSchedpp/ObservationMode.cpp \
        ../VieSchedpp/Output.cpp \
        ../VieSchedpp/ParameterGroup.cpp \
        ../VieSchedpp/ParameterSettings.cpp \
        ../VieSchedpp/ParameterSetup.cpp \
        ../VieSchedpp/PointingVector.cpp \
        ../VieSchedpp/Position.cpp \
        ../VieSchedpp/Scan.cpp \
        ../VieSchedpp/ScanTimes.cpp \
        ../VieSchedpp/Scheduler.cpp \
        ../VieSchedpp/Skd.cpp \
        ../VieSchedpp/SkdCatalogReader.cpp \
        ../VieSchedpp/SkdParser.cpp \
        ../VieSchedpp/SkyCoverage.cpp \
        ../VieSchedpp/Source.cpp \
        ../VieSchedpp/Station.cpp \
        ../VieSchedpp/StationEndposition.cpp \
        ../VieSchedpp/Subcon.cpp \
        ../VieSchedpp/TimeSystem.cpp \
        ../VieSchedpp/Vex.cpp \
        ../VieSchedpp/VieSchedpp.cpp \
        ../VieSchedpp/VieVS_NamedObject.cpp \
        ../VieSchedpp/VieVS_Object.cpp \
        ../VieSchedpp/WeightFactors.cpp \
        ../VieSchedpp/util.cpp \
        textfileviewer.cpp \
        vieschedpp_analyser.cpp \
        qtutil.cpp

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
        ../VieSchedpp/Antenna.h \
        ../VieSchedpp/Antenna_AzEl.h \
        ../VieSchedpp/Antenna_HaDc.h \
        ../VieSchedpp/Antenna_XYew.h \
        ../VieSchedpp/AstronomicalParameters.h \
        ../VieSchedpp/Baseline.h \
        ../VieSchedpp/CableWrap.h \
        ../VieSchedpp/CableWrap_AzEl.h \
        ../VieSchedpp/CableWrap_HaDc.h \
        ../VieSchedpp/CableWrap_XYew.h \
        ../VieSchedpp/CalibratorBlock.h \
        ../VieSchedpp/Constants.h \
        ../VieSchedpp/Equipment.h \
        ../VieSchedpp/Equipment_elDependent.h \
        ../VieSchedpp/Flux.h \
        ../VieSchedpp/Flux_B.h \
        ../VieSchedpp/Flux_M.h \
        ../VieSchedpp/HighImpactScanDescriptor.h \
        ../VieSchedpp/HorizonMask.h \
        ../VieSchedpp/HorizonMask_line.h \
        ../VieSchedpp/HorizonMask_step.h \
        ../VieSchedpp/Initializer.h \
        ../VieSchedpp/LogParser.h \
        ../VieSchedpp/LookupTable.h \
        ../VieSchedpp/MultiScheduling.h \
        ../VieSchedpp/Network.h \
        ../VieSchedpp/Observation.h \
        ../VieSchedpp/ObservationMode.h \
        ../VieSchedpp/Output.h \
        ../VieSchedpp/ParameterGroup.h \
        ../VieSchedpp/ParameterSettings.h \
        ../VieSchedpp/ParameterSetup.h \
        ../VieSchedpp/PointingVector.h \
        ../VieSchedpp/Position.h \
        ../VieSchedpp/Scan.h \
        ../VieSchedpp/ScanTimes.h \
        ../VieSchedpp/Scheduler.h \
        ../VieSchedpp/Skd.h \
        ../VieSchedpp/SkdCatalogReader.h \
        ../VieSchedpp/SkdParser.h \
        ../VieSchedpp/SkyCoverage.h \
        ../VieSchedpp/Source.h \
        ../VieSchedpp/Station.h \
        ../VieSchedpp/StationEndposition.h \
        ../VieSchedpp/Subcon.h \
        ../VieSchedpp/Subnetting.h \
        ../VieSchedpp/TimeSystem.h \
        ../VieSchedpp/Vex.h \
        ../VieSchedpp/VieSchedpp.h \
        ../VieSchedpp/VieVS_NamedObject.h \
        ../VieSchedpp/VieVS_Object.h \
        ../VieSchedpp/WeightFactors.h \
        ../VieSchedpp/sofa.h \
        ../VieSchedpp/sofam.h \
        ../VieSchedpp/util.h \
        textfileviewer.h \
        vieschedpp_analyser.h \
        qtutil.h

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
        textfileviewer.ui \
        vieschedpp_analyser.ui

RESOURCES += \
        myresources.qrc

DISTFILES += \
        VLBI_Scheduler/libsofa_c.a \
        VLBI_Scheduler/CMakeLists.txt
