// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_MainFrame_H
#define BITCOIN_QT_MainFrame_H

#include <QFrame>
#include <QMap>

class BitcoinGUI;
//class ClientModel;
//class SendCoinsRecipient;
//class WalletModel;
class MainView;

QT_BEGIN_NAMESPACE
class QStackedWidget;
QT_END_NAMESPACE

class MainFrame : public QFrame
{
    Q_OBJECT

public:
    explicit MainFrame(QString languageIn,BitcoinGUI *_gui = 0,JsInterface *_js=0);
    ~MainFrame();
    QString language;
    //void setClientModel(ClientModel *clientModel);
//
    bool addWallet(const QString& name);//, WalletModel *walletModel);
    MainView* getMainView(const QString& name);
    JsInterface jsInterface;
//    bool setCurrentWallet(const QString& name);
//    bool removeWallet(const QString &name);
//    void removeAllWallets();
//
//    bool handlePaymentRequest(const SendCoinsRecipient& recipient);
//
    void showOutOfSyncWarning(bool fShow);
    
private:
    QStackedWidget *widgetStack;
    BitcoinGUI *gui;
    //ClientModel *clientModel;
    QMap<QString, MainView*> mapMainViews;

    bool bOutOfSync;

    MainView *currentMainView();

public slots:
        void gotoWebPage(int nPageID);
    /** Switch to overview (home) page */
//    void gotoOverviewPage();
//    /** Switch to history (transactions) page */
//    void gotoHistoryPage();
//    /** Switch to receive coins page */
//    void gotoReceiveCoinsPage();
//    /** Switch to send coins page */
//    void gotoSendCoinsPage(QString addr = "");
//
//    /** Show Sign/Verify Message dialog and switch to sign message tab */
//    void gotoSignMessageTab(QString addr = "");
//    /** Show Sign/Verify Message dialog and switch to verify message tab */
//    void gotoVerifyMessageTab(QString addr = "");
//
//    /** Encrypt the wallet */
//    void encryptWallet(bool status);
//    /** Backup the wallet */
//    void backupWallet();
//    /** Change encrypted wallet passphrase */
//    void changePassphrase();
//    /** Ask for passphrase to unlock wallet temporarily */
//    void unlockWallet();
//
//    /** Show used sending addresses */
//    void usedSendingAddresses();
//    /** Show used receiving addresses */
//    void usedReceivingAddresses();
};

#endif // BITCOIN_QT_MainFrame_H
