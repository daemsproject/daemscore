// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_WalletPage_H
#define BITCOIN_QT_WalletPage_H

//#include "amount.h"
#include <QObject>
#include <QWidget>
#include <QWebView>

QT_BEGIN_NAMESPACE
class QModelIndex;
class JsInterface;
class QNetworkDiskCache;
class QNetworkAccessManager;
QT_END_NAMESPACE


class WalletPage:public QWebView
{
        Q_OBJECT

public:
    explicit WalletPage(QString languageIn,QWidget *parent = 0);
    ~WalletPage();
    QString language;
private slots:    
    void addJSObject();
private:    
    JsInterface *jsinterface;
    //WalletModel *walletModel;
    //QNetworkAccessManager * m_network;
    //QNetworkDiskCache * m_cache;
};
#endif // BITCOIN_QT_WalletPage_H
