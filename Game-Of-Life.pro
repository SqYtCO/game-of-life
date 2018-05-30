# © Copyright (c) 2018 SqYtCO

#-------------------------------------------------
#
# Project created by QtCreator 2018-05-02T16:25:49
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = game-of-life
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += -std=c++17
LIBS += -lstdc++fs

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    configuration.cpp \
    cellsystem.cpp \
    core.cpp \
    graphiccore.cpp \
    graphicconfiguration.cpp \
    openglwidget.cpp \
    preferenceswidget.cpp \
    toolwidget.cpp \
    helpwidget.cpp \
    startupdialog.cpp

HEADERS += \
    mainwindow.h \
    configuration.h \
    cellsystem.h \
    core.h \
    basesystem.h \
    graphiccore.h \
    graphicconfiguration.h \
    openglwidget.h \
    preferenceswidget.h \
    toolwidget.h \
    helpwidget.h \
    startupdialog.h

RESOURCES += \
    resources.qrc

TRANSLATIONS += \
    gol_de.ts

DISTFILES += \
    README.md
