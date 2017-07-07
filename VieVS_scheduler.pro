#-------------------------------------------------
#
# Project created by QtCreator 2017-07-07T09:42:36
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++14

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VieVS_scheduler
TEMPLATE = app

INCLUDEPATH += /home/mschartn/boost_1_64_0
INCLUDEPATH += /home/mschartn/SOFA/c/src
LIBS += "-L/home/mschartn/boost_1_64_0/stage/lib/"
LIBS += /home/mschartn/SOFA/c/src/libsofa_c.a

SOURCES += main.cpp\
        mainwindow.cpp \
    VLBI_antenna.cpp \
    VLBI_baseline.cpp \
    VLBI_cableWrap.cpp \
    VLBI_equip.cpp \
    VLBI_flux.cpp \
    VLBI_initializer.cpp \
    VLBI_mask.cpp \
    VLBI_obs.cpp \
    VLBI_pointingVector.cpp \
    VLBI_position.cpp \
    VLBI_scan.cpp \
    VLBI_scheduler.cpp \
    VLBI_skyCoverage.cpp \
    VLBI_source.cpp \
    VLBI_station.cpp \
    VLBI_subcon.cpp

HEADERS  += mainwindow.h \
    VieVS_constants.h \
    VLBI_antenna.h \
    VLBI_baseline.h \
    VLBI_cableWrap.h \
    VLBI_equip.h \
    VLBI_flux.h \
    VLBI_initializer.h \
    VLBI_mask.h \
    VLBI_obs.h \
    VLBI_pointingVector.h \
    VLBI_position.h \
    VLBI_scan.h \
    VLBI_scheduler.h \
    VLBI_skyCoverage.h \
    VLBI_source.h \
    VLBI_station.h \
    VLBI_subcon.h

FORMS    += mainwindow.ui
