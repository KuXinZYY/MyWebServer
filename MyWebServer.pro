QT += core
QT -= gui

CONFIG += c++11

TARGET = MyWebServer
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    thread.cpp \
    threadpool.cpp \
    countdownlatch.cpp \
    fileutil.cpp \
    logfile.cpp \
    asynlogging.cpp \
    logging.cpp

HEADERS += \
    thread.h \
    condition.h \
    mutexlock.h \
    threadpool.h \
    countdownlatch.h \
    currentthread.h \
    fileutil.h \
    logfile.h \
    logstream.h \
    asynlogging.h \
    logging.h

DISTFILES += \
    bugBuild.txt
