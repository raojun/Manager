#-------------------------------------------------
#
# Project created by QtCreator 2014-04-17T21:06:29
#
#-------------------------------------------------

QT       += core gui

QT       +=sql xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Manager
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    pieview.cpp \
    logindialog.cpp

HEADERS  += widget.h \
    connection.h \
    pieview.h \
    logindialog.h

FORMS    += widget.ui \
    logindialog.ui
