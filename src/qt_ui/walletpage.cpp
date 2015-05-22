// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "walletpage.h"
#include "jsinterface.h"
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



WalletPage:: WalletPage(QString languageIn,QWidget *parent): QWebView(parent),language(languageIn)
 {
//    m_network = new QNetworkAccessManager(this);
//    m_cache = new QNetworkDiskCache(this);
//    m_cache->setCacheDirectory(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/jsinterface");
//    m_cache->setMaximumCacheSize(1000000); //set the cache to 10megs
//    m_network->setCache(m_cache);
//    page()->setNetworkAccessManager(m_network);
QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    jsinterface=new JsInterface(this);

    // Signal is emitted before frame loads any web content:
    QObject::connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
                     this, SLOT(addJSObject()));
    
    QUrl startURL;
    //if (language.indexOf("CN")>-1)        
//        startURL= QUrl("qrc:/wallet_cn.html");
    //else
        startURL= QUrl("file://" + QDir::currentPath().toUtf8() + "/res/html/browser_en.html");  
//        startURL= QUrl("file:///home/tong/projects/ccc1/src/qt_ui/res/html/browser_en.html");    
    
    //page()->mainFrame()->evaluateJavaScript("feedback()");
    // Load web content now!
    setUrl(startURL);
   
 }
WalletPage::~WalletPage()
{
   // delete ui;
}
void WalletPage::addJSObject() {    
    page()->mainFrame()->addToJavaScriptWindowObject(QString("jsinterface"), jsinterface);
}

