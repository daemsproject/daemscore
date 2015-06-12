// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "webpage.h"
#include "jsinterface.h"
#include "bitcoingui.h"
//#include "bitcoinunits.h"
//#include "guiconstants.h"
//#include "guiutil.h"
//#include "walletmodel.h"
#include <QtWebKitWidgets/QWebView>
#include <QAbstractItemDelegate>
#include <QPainter>
#include <QWebFrame>
#include <QWebElementCollection>
#include <QNetworkDiskCache>
#define DECORATION_SIZE 64
#define NUM_ITEMS 3



WebPage:: WebPage(QString languageIn,QWidget *parent,JsInterface *_js,QUrl urlIn,int nPageIDin): QWebView(parent),language(languageIn),jsInterface(_js),nPageID(nPageIDin)
 {
//    m_network = new QNetworkAccessManager(this);
//    m_cache = new QNetworkDiskCache(this);
//    m_cache->setCacheDirectory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/jsinterface");
//    m_cache->setMaximumCacheSize(1000000); //set the cache to 10megs
//    m_network->setCache(m_cache);
//    page()->setNetworkAccessManager(m_network);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    // Signal is emitted before frame loads any web content:
    QObject::connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
                     this, SLOT(addJSObject()));
    QObject::connect(jsInterface,  SIGNAL(feedback(QString,QString)),
                     this,  SIGNAL(feedback(QString,QString)));
    QObject::connect(jsInterface,  SIGNAL(notify(QString)),
                     this,  SIGNAL(notify(QString)));
    
    // Load web content now!
    setUrl(urlIn);
   
 }
WebPage::~WebPage()
{
   // delete ui;
}
void WebPage::addJSObject() {    
    //page()->mainFrame()->addToJavaScriptWindowObject(QString("jsinterface"), jsInterface);
    page()->mainFrame()->addToJavaScriptWindowObject(QString("jsinterface"), this);
}
QString WebPage::jscall(QString command,QString dataJson)
{
    return jsInterface->jscall(command,dataJson,nPageID);
}
QString WebPage::jscallasync(QString command,QString dataJson,QString successfunc,QString errorfunc)
{
    return jsInterface->jscallasync(command,dataJson,successfunc,errorfunc,nPageID);
}

