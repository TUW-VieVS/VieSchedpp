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

INCLUDEPATH += /home/mschartn/boost_1_64_0/
LIBS += "-LC:/home/mschartn/boost_1_64_0//stage/lib/"

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    skdcatalogreader.cpp \
    chartview.cpp \
    callout.cpp \
    multischededitdialogint.cpp \
    multischededitdialogdouble.cpp \
    multischededitdialogdatetime.cpp \
    ParameterGroup.cpp \
    ParameterSettings.cpp \
    ParameterSetup.cpp \
    addgroupdialog.cpp \
    stationparametersdialog.cpp \
    sourceparametersdialog.cpp \
    baselineparametersdialog.cpp

HEADERS += \
        mainwindow.h \
    skdcatalogreader.h \
    chartview.h \
    callout.h \
    multischededitdialogint.h \
    multischededitdialogdouble.h \
    multischededitdialogdatetime.h \
    ParameterGroup.h \
    ParameterSettings.h \
    ParameterSetup.h \
    addgroupdialog.h \
    stationparametersdialog.h \
    sourceparametersdialog.h \
    baselineparametersdialog.h

FORMS += \
        mainwindow.ui \
    multischededitdialogint.ui \
    multischededitdialogdouble.ui \
    multischededitdialogdatetime.ui \
    addgroupdialog.ui \
    stationparametersdialog.ui \
    sourceparametersdialog.ui \
    baselineparametersdialog.ui

RESOURCES += \
    myresources.qrc
