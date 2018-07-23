QT += core
QT -= gui

CONFIG += c++11

TARGET = MyWebServer
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    base/AsynLogging.cpp \
    base/CountDownLatch.cpp \
    base/FileUtil.cpp \
    base/LogFile.cpp \
    base/Logging.cpp \
    base/Thread.cpp \
    base/ThreadPool.cpp \
    Epoll.cpp \
    FileEvent.cpp \
    EventLoop.cpp \
    Main.cpp \
    TimerEvent.cpp \
    EventLoopThread.cpp \
    EventLoopThreadPool.cpp \
    Server.cpp \
    Util.cpp \
    ServerConnection.cpp

HEADERS += \
    FileEvent.h \
    base/AsynLogging.h \
    base/Condition.h \
    base/CountDownLatch.h \
    base/CurrentThread.h \
    base/FileUtil.h \
    base/LogFile.h \
    base/Logging.h \
    base/LogStream.h \
    base/MutexLock.h \
    base/Thread.h \
    base/ThreadPool.h \
    Epoll.h \
    EventLoop.h \
    TimeEvent.h \
    EventLoopThread.h \
    EventLoopThreadPool.h \
    Server.h \
    Util.h \
    ServerConnection.h

DISTFILES += \
    bugBuild.txt \
    Debug/MyWebServer \
    Debug/valgrind.sh \
    Debug/Test.log
