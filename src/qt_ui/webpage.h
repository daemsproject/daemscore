// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BROWSER_QT_WEBPAGE_H
#define BROWSER_QT_WEBPAGE_H

//#include "amount.h"
#include <QObject>
#include <QWidget>
#include <QWebView>

//QT_BEGIN_NAMESPACE
//class QModelIndex;
class JsInterface;
//class QNetworkDiskCache;
//class QNetworkAccessManager;
//QT_END_NAMESPACE
//class BitcoinGUI;

class WebPage:public QWebView
{
        Q_OBJECT

public:
    explicit WebPage(QString languageIn,QWidget *parent = 0,JsInterface *_js=0,QUrl urlIn=QUrl(""),int nPageIDin=0);
    ~WebPage();
    QString language;
    int nPageID;    
private slots:    
    void addJSObject();
private:    
    JsInterface* jsInterface;    
    //WalletModel *walletModel;
    //QNetworkAccessManager * m_network;
    //QNetworkDiskCache * m_cache;
public slots:    
    Q_INVOKABLE    
    QString jscall(QString command,QString dataJson);
    QString jscallasync(QString command,QString dataJson,QString successfunc,QString errorfunc);
signals:
    //Q_SIGNAL
    void feedback(QString str,QString func);    
    void notify(QString result);
    
};
#endif // BROWSER_QT_WEBPAGE_H
