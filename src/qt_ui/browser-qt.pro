QT       += core gui
QT       += network
QT       += webkitwidgets concurrent widgets printsupport

qtHaveModule(uitools):!embedded: QT += uitools
else: DEFINES += QT_NO_UITOOLS

TARGET = browserexe
TEMPLATE = app
DEFINES += ENABLE_WALLET=1

FORMS += \ 
    forms/helpmessagedialog.ui \
    forms/bookmarks.ui \
    forms/downloaditem.ui \
    forms/downloads.ui \
    forms/userconfirmdialog.ui \
    forms/intro.ui \
    forms/passworddialog.ui \
    forms/proxy.ui \
    forms/accountdialog.ui \
    forms/addbookmarkdialog.ui \
    forms/bookmarks.ui \
    forms/cookies.ui \
    forms/cookiesexceptions.ui \
    forms/history.ui \
    forms/settings.ui

RESOURCES += \
    browser.qrc \
    browser_locale.qrc

CODECFORTR = UTF-8

TRANSLATIONS += \
   locale/bitcoin_en.ts \
    locale/bitcoin_zh_CN.ts

include(../faicoin.pri)
include(qt.pri)
