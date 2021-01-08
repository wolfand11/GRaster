#-------------------------------------------------
#
# Project created by QtCreator 2021-01-08T13:39:43
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GRaster
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        gbuffer.cpp \
        ggraphiclibapi.cpp \
        gmathutils.cpp \
        grasterapi.cpp \
        main.cpp \
        graster.cpp

HEADERS += \
        gbuffer.h \
        ggraphiclibapi.h \
        glog.h \
        gmathutils.h \
        graster.h \
        grasterapi.h

FORMS += \
        graster.ui
