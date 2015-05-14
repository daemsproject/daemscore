QT       += core gui
QT       += network
QT       += webkitwidgets concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = browserexe
TEMPLATE = app
DEFINES += ENABLE_WALLET=1

FORMS += \ 
    forms/helpmessagedialog.ui 

RESOURCES += \
    browser.qrc \
    browser_locale.qrc

include(../cccoin.pri)
include(qt.pri)
