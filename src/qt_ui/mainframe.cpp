// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mainframe.h"

#include "bitcoingui.h"
#include "mainview.h"

#include <cstdio>
#include "util.h"
#include <QHBoxLayout>
#include <QLabel>

MainFrame::MainFrame(QString languageIn,BitcoinGUI *_gui,JsInterface *_js) :
    QFrame(_gui),
    jsInterface(_js),
    language(languageIn)
{
    // Leave HBox hook for adding a list view later
    QHBoxLayout *mainFrameLayout = new QHBoxLayout(this);
    setContentsMargins(0,0,0,0);
    widgetStack = new QStackedWidget(this);
    mainFrameLayout->setContentsMargins(0,0,0,0);
    mainFrameLayout->addWidget(widgetStack);

    //QLabel *noWallet = new QLabel(tr("No wallet has been loaded."));
    //noWallet->setAlignment(Qt::AlignCenter);
    //widgetStack->addWidget(noWallet);
    
}

MainFrame::~MainFrame()
{
}

//void MainFrame::setClientModel(ClientModel *clientModel)
//{
//    this->clientModel = clientModel;
//}

bool MainFrame::addWallet(const QString& name)//, WalletModel *walletModel)
{
    LogPrintf("mainframe addwallet \n");
    //if (!gui || !clientModel || !walletModel || mapMainViews.count(name) > 0)
    if (!gui)// ||mapMainViews.count(name) > 0)
        return false;
    LogPrintf("mainframe addwallet2 %i\n",gui);
    MainView *mainView = new MainView(language,this,jsInterface);
    LogPrintf("mainframe addwallet3 %i\n",gui);
    //mainView->setBitcoinGUI(gui);
    //mainView->setClientModel(clientModel);
    //mainView->setWalletModel(walletModel);
    //mainView->showOutOfSyncWarning(bOutOfSync);
 
     /* TODO we should goto the currently selected page once dynamically adding wallets is supported */
    mainView->gotoWebPage(1);
    LogPrintf("mainframe addwallet4 %i\n",gui);
    widgetStack->addWidget(mainView);
    LogPrintf("mainframe addwallet5 %i\n",gui);
    mapMainViews[name] = mainView;
    LogPrintf("mainframe addwallet6 %i\n",gui);

    // Ensure a mainView is able to show the main window
    connect(mainView, SIGNAL(showNormalIfMinimized()), gui, SLOT(showNormalIfMinimized()));
LogPrintf("mainframe addwallet7 %i\n",gui);
    return true;
}
MainView* MainFrame::getMainView(const QString& name){
    return mapMainViews[name];
}
//bool MainFrame::setCurrentWallet(const QString& name)
//{
//    if (mapMainViews.count(name) == 0)
//        return false;
//
//    MainView *mainView = mapMainViews.value(name);
//    widgetStack->setCurrentWidget(mainView);
//    mainView->updateEncryptionStatus();
//    return true;
//}
//
//bool MainFrame::removeWallet(const QString &name)
//{
//    if (mapMainViews.count(name) == 0)
//        return false;
//
//    MainView *mainView = mapMainViews.take(name);
//    widgetStack->removeWidget(mainView);
//    return true;
//}
//
//void MainFrame::removeAllWallets()
//{
//    QMap<QString, MainView*>::const_iterator i;
//    for (i = mapMainViews.constBegin(); i != mapMainViews.constEnd(); ++i)
//        widgetStack->removeWidget(i.value());
//    mapMainViews.clear();
//}
//
//bool MainFrame::handlePaymentRequest(const SendCoinsRecipient &recipient)
//{
//    MainView *mainView = currentMainView();
//    if (!mainView)
//        return false;
//
//    return mainView->handlePaymentRequest(recipient);
//}

void MainFrame::showOutOfSyncWarning(bool fShow)
{
//    bOutOfSync = fShow;
//    QMap<QString, MainView*>::const_iterator i;
//    for (i = mapMainViews.constBegin(); i != mapMainViews.constEnd(); ++i)
//        i.value()->showOutOfSyncWarning(fShow);
}
void MainFrame::gotoWebPage(int nPageID)
{
    QMap<QString, MainView*>::const_iterator i;
    for (i = mapMainViews.constBegin(); i != mapMainViews.constEnd(); ++i)
        i.value()->gotoWebPage(nPageID);
}
//void MainFrame::gotoOverviewPage()
//{
//    QMap<QString, MainView*>::const_iterator i;
//    for (i = mapMainViews.constBegin(); i != mapMainViews.constEnd(); ++i)
//        i.value()->gotoOverviewPage();
//}
//
//void MainFrame::gotoHistoryPage()
//{
//    QMap<QString, MainView*>::const_iterator i;
//    for (i = mapMainViews.constBegin(); i != mapMainViews.constEnd(); ++i)
//        i.value()->gotoHistoryPage();
//}
//
//void MainFrame::gotoReceiveCoinsPage()
//{
//    QMap<QString, MainView*>::const_iterator i;
//    for (i = mapMainViews.constBegin(); i != mapMainViews.constEnd(); ++i)
//        i.value()->gotoReceiveCoinsPage();
//}
//
//void MainFrame::gotoSendCoinsPage(QString addr)
//{
//    QMap<QString, MainView*>::const_iterator i;
//    for (i = mapMainViews.constBegin(); i != mapMainViews.constEnd(); ++i)
//        i.value()->gotoSendCoinsPage(addr);
//}

//void MainFrame::gotoSignMessageTab(QString addr)
//{
//    MainView *mainView = currentMainView();
//    if (mainView)
//        mainView->gotoSignMessageTab(addr);
//}
//
//void MainFrame::gotoVerifyMessageTab(QString addr)
//{
//    MainView *mainView = currentMainView();
//    if (mainView)
//        mainView->gotoVerifyMessageTab(addr);
//}
//
//void MainFrame::encryptWallet(bool status)
//{
//    MainView *mainView = currentMainView();
//    if (mainView)
//        mainView->encryptWallet(status);
//}
//
//void MainFrame::backupWallet()
//{
//    MainView *mainView = currentMainView();
//    if (mainView)
//        mainView->backupWallet();
//}
//
//void MainFrame::changePassphrase()
//{
//    MainView *mainView = currentMainView();
//    if (mainView)
//        mainView->changePassphrase();
//}
//
//void MainFrame::unlockWallet()
//{
//    MainView *mainView = currentMainView();
//    if (mainView)
//        mainView->unlockWallet();
//}
//
//void MainFrame::usedSendingAddresses()
//{
//    MainView *mainView = currentMainView();
//    if (mainView)
//        mainView->usedSendingAddresses();
//}
//
//void MainFrame::usedReceivingAddresses()
//{
//    MainView *mainView = currentMainView();
//    if (mainView)
//        mainView->usedReceivingAddresses();
//}

MainView *MainFrame::currentMainView()
{
    return qobject_cast<MainView*>(widgetStack->currentWidget());
}

