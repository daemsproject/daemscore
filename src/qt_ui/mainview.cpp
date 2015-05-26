// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mainview.h"

//#include "addressbookpage.h"
//#include "askpassphrasedialog.h"
#include "bitcoingui.h"
//#include "clientmodel.h"
#include "guiutil.h"
//#include "optionsmodel.h"
#include "webpage.h"
//#include "overviewpage.h"
///#include "receivecoinsdialog.h"
//#include "sendcoinsdialog.h"
//#include "signverifymessagedialog.h"
//#include "transactiontablemodel.h"
//#include "transactionview.h"
#include "walletmodel.h"
#include "jsinterface.h"
#include "ui_interface.h"
#include "util.h"
#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QProgressDialog>
#include <QPushButton>
#include <QVBoxLayout>

MainView::MainView(QString languageIn,BitcoinGUI *parent,JsInterface *_js):
    QStackedWidget(parent),
        language(languageIn),
        jsInterface(_js)
    //clientModel(0)
    //walletModel(0)
{
    // Create tabs    
    //QUrl walletUrl= QUrl("file:///home/alan/projects/ccc/src/qt_ui/res/html/wallet_en.html"); 
    //QUrl walletUrl= QUrl("file://"+QDir::currentPath().toUtf8() + "/res/html/wallet_en.html"); 
    QUrl browserUrl= QUrl("file://"+QDir::currentPath().toUtf8() + "/res/html/browser_en.html"); 
    //LogPrintf(QDir::currentPath().toUtf8() + "/res/html/wallet_en.html");
    
    //vWebPages.push_back(new WebPage(language,parent,jsInterface,walletUrl,1));    
    vWebPages.push_back(new WebPage(language,this,jsInterface,browserUrl,2));  
    
    //overviewPage = new OverviewPage();

    //transactionsPage = new QWidget(this);
    QVBoxLayout *vbox = new QVBoxLayout();
    QHBoxLayout *hbox_buttons = new QHBoxLayout();
    //transactionView = new TransactionView(this);
    //vbox->addWidget(transactionView);
    QPushButton *exportButton = new QPushButton(tr("&Export"), this);
    exportButton->setToolTip(tr("Export the data in the current tab to a file"));
#ifndef Q_OS_MAC // Icons on push buttons are very uncommon on Mac
    exportButton->setIcon(QIcon(":/icons/export"));
#endif
    hbox_buttons->addStretch();
    hbox_buttons->addWidget(exportButton);
    vbox->addLayout(hbox_buttons);
    //transactionsPage->setLayout(vbox);

    //receiveCoinsPage = new ReceiveCoinsDialog();
    //sendCoinsPage = new SendCoinsDialog();
     for(unsigned int i=0;i<vWebPages.size();i++)        
            addWidget(vWebPages[i]);
    if (vWebPages.size()>0)
        setCurrentWidget(*vWebPages.begin());
//    for (std::vector<WebPage*>::iterator it=vWebPages.begin();it!=vWebPages.end();it++)
//        addWidget(*it);
    //addWidget(overviewPage);
    //addWidget(transactionsPage);
    //addWidget(receiveCoinsPage);
    //addWidget(sendCoinsPage);

    // Clicking on a transaction on the overview pre-selects the transaction on the transaction history page
    //connect(overviewPage, SIGNAL(transactionClicked(QModelIndex)), transactionView, SLOT(focusTransaction(QModelIndex)));

    // Double-clicking on a transaction on the transaction history page shows details
    //connect(transactionView, SIGNAL(doubleClicked(QModelIndex)), transactionView, SLOT(showDetails()));

    // Clicking on "Export" allows to export the transaction list
    //connect(exportButton, SIGNAL(clicked()), transactionView, SLOT(exportClicked()));

    // Pass through messages from sendCoinsPage
    //connect(sendCoinsPage, SIGNAL(message(QString,QString,unsigned int)), this, SIGNAL(message(QString,QString,unsigned int)));
    // Pass through messages from transactionView
    //connect(transactionView, SIGNAL(message(QString,QString,unsigned int)), this, SIGNAL(message(QString,QString,unsigned int)));
    //if (gui)
    //{
        // Clicking on a transaction on the overview page simply sends you to transaction history page
        //connect(overviewPage, SIGNAL(transactionClicked(QModelIndex)), gui, SLOT(gotoHistoryPage()));

        // Receive and report messages
       // connect(this, SIGNAL(message(QString,QString,unsigned int)), gui, SLOT(message(QString,QString,unsigned int)));

        // Pass through encryption status changed signals
        //connect(this, SIGNAL(encryptionStatusChanged(int)), gui, SLOT(setEncryptionStatus(int)));

        // Pass through transaction notifications
        //connect(this, SIGNAL(incomingTransaction(QString,int,CAmount,QString,QString)), gui, SLOT(incomingTransaction(QString,int,CAmount,QString,QString)));
    //}
}

MainView::~MainView()
{
}

//void MainView::setBitcoinGUI(BitcoinGUI *gui)
//{
//    if (gui)
//    {
        // Clicking on a transaction on the overview page simply sends you to transaction history page
        //connect(overviewPage, SIGNAL(transactionClicked(QModelIndex)), gui, SLOT(gotoHistoryPage()));

        // Receive and report messages
        //connect(this, SIGNAL(message(QString,QString,unsigned int)), gui, SLOT(message(QString,QString,unsigned int)));

        // Pass through encryption status changed signals
        //connect(this, SIGNAL(encryptionStatusChanged(int)), gui, SLOT(setEncryptionStatus(int)));

        // Pass through transaction notifications
        //connect(this, SIGNAL(incomingTransaction(QString,int,CAmount,QString,QString)), gui, SLOT(incomingTransaction(QString,int,CAmount,QString,QString)));
//    }
//}

//void MainView::setClientModel(ClientModel *clientModel)
//{
//    this->clientModel = clientModel;
//
//    overviewPage->setClientModel(clientModel);
//    sendCoinsPage->setClientModel(clientModel);
//}

//void MainView::setWalletModel(WalletModel *walletModel)
//{
    //this->walletModel = walletModel;

    // Put transaction list in tabs
    //walletPage->setWalletModel(walletModel);
    //transactionView->setModel(walletModel);
    //overviewPage->setWalletModel(walletModel);
    //receiveCoinsPage->setModel(walletModel);
    //sendCoinsPage->setModel(walletModel);

    //if (walletModel)
    //{
        // Receive and pass through messages from wallet model
        //connect(walletModel, SIGNAL(message(QString,QString,unsigned int)), this, SIGNAL(message(QString,QString,unsigned int)));

        // Handle changes in encryption status
        //connect(walletModel, SIGNAL(encryptionStatusChanged(int)), this, SIGNAL(encryptionStatusChanged(int)));
        //updateEncryptionStatus();

        // Balloon pop-up for new transaction
        //connect(walletModel->getTransactionTableModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
        //        this, SLOT(processNewTransaction(QModelIndex,int,int)));

        // Ask for passphrase if needed
        //connect(walletModel, SIGNAL(requireUnlock()), this, SLOT(unlockWallet()));

        // Show progress dialog
        //connect(walletModel, SIGNAL(showProgress(QString,int)), this, SLOT(showProgress(QString,int)));
    //}
//}

//void MainView::processNewTransaction(const QModelIndex& parent, int start, int /*end*/)
//{
//    // Prevent balloon-spam when initial block download is in progress
//    if (!walletModel || !clientModel || clientModel->inInitialBlockDownload())
//        return;
//
//    TransactionTableModel *ttm = walletModel->getTransactionTableModel();
//    if (!ttm || ttm->processingQueuedTransactions())
//        return;
//
//    QString date = ttm->index(start, TransactionTableModel::Date, parent).data().toString();
//    qint64 amount = ttm->index(start, TransactionTableModel::Amount, parent).data(Qt::EditRole).toULongLong();
//    QString type = ttm->index(start, TransactionTableModel::Type, parent).data().toString();
//    QString address = ttm->index(start, TransactionTableModel::ToAddress, parent).data().toString();
//
//    emit incomingTransaction(date, walletModel->getOptionsModel()->getDisplayUnit(), amount, type, address);
//}
void MainView::gotoWebPage(int nPageID,QUrl url)
{
    LogPrintf("gotowebpage pageid:%i,url:%s",nPageID,url.toString().toStdString());
    //for (std::vector<WebPage*>::iterator it=vWebPages.begin();it!=vWebPages.end();it++){
    //    if (vWebPages*it->nPageID==nPageID){
    for(unsigned int i=0;i<vWebPages.size();i++){
        if (vWebPages[i]->nPageID==nPageID){
            if(url!=QUrl(""))
                vWebPages[i]->setUrl(url);
            setCurrentWidget(vWebPages[i]);
            return;
        }            
    }
    vWebPages.push_back(new WebPage(language,this,jsInterface,url,nPageID));  
    addWidget(*vWebPages.rbegin());
    
    setCurrentWidget(*vWebPages.rbegin());
    //gotoWebPage(nPageID);
    //setCurrentWidget(*vWebPages.rbegin());
    //setCurrentWidget(walletPage);
}
void MainView::closeWebPage(int nPageID){
    for(std::vector<WebPage*>::iterator it=vWebPages.begin();it!=vWebPages.end();it++){
        if((*it)->nPageID==nPageID)
            vWebPages.erase(it);
    }
}
//void MainView::gotoOverviewPage()
//{
//    setCurrentWidget(overviewPage);
//}
//
//void MainView::gotoHistoryPage()
//{
//    setCurrentWidget(transactionsPage);
//}
//
//void MainView::gotoReceiveCoinsPage()
//{
//    setCurrentWidget(receiveCoinsPage);
//}
//
//void MainView::gotoSendCoinsPage(QString addr)
//{
//    setCurrentWidget(sendCoinsPage);
//
//    if (!addr.isEmpty())
//        sendCoinsPage->setAddress(addr);
//}
//
//void MainView::gotoSignMessageTab(QString addr)
//{
//    // calls show() in showTab_SM()
//    SignVerifyMessageDialog *signVerifyMessageDialog = new SignVerifyMessageDialog(this);
//    signVerifyMessageDialog->setAttribute(Qt::WA_DeleteOnClose);
//    signVerifyMessageDialog->setModel(walletModel);
//    signVerifyMessageDialog->showTab_SM(true);
//
//    if (!addr.isEmpty())
//        signVerifyMessageDialog->setAddress_SM(addr);
//}
//
//void MainView::gotoVerifyMessageTab(QString addr)
//{
//    // calls show() in showTab_VM()
//    SignVerifyMessageDialog *signVerifyMessageDialog = new SignVerifyMessageDialog(this);
//    signVerifyMessageDialog->setAttribute(Qt::WA_DeleteOnClose);
//    signVerifyMessageDialog->setModel(walletModel);
//    signVerifyMessageDialog->showTab_VM(true);
//
//    if (!addr.isEmpty())
//        signVerifyMessageDialog->setAddress_VM(addr);
//}
//
//bool MainView::handlePaymentRequest(const SendCoinsRecipient& recipient)
//{
//    return sendCoinsPage->handlePaymentRequest(recipient);
//}

//void MainView::showOutOfSyncWarning(bool fShow)
//{
//    //overviewPage->showOutOfSyncWarning(fShow);
//}

//void MainView::updateEncryptionStatus()
//{
//    emit encryptionStatusChanged(walletModel->getEncryptionStatus());
//}
//
//void MainView::encryptWallet(bool status)
//{
//    if(!walletModel)
//        return;
//    AskPassphraseDialog dlg(status ? AskPassphraseDialog::Encrypt : AskPassphraseDialog::Decrypt, this);
//    dlg.setModel(walletModel);
//    dlg.exec();
//
//    updateEncryptionStatus();
//}
//
//void MainView::backupWallet()
//{
//    QString filename = GUIUtil::getSaveFileName(this,
//        tr("Backup Wallet"), QString(),
//        tr("Wallet Data (*.dat)"), NULL);
//
//    if (filename.isEmpty())
//        return;
//
//    if (!walletModel->backupWallet(filename)) {
//        emit message(tr("Backup Failed"), tr("There was an error trying to save the wallet data to %1.").arg(filename),
//            CClientUIInterface::MSG_ERROR);
//        }
//    else {
//        emit message(tr("Backup Successful"), tr("The wallet data was successfully saved to %1.").arg(filename),
//            CClientUIInterface::MSG_INFORMATION);
//    }
//}
//
//void MainView::changePassphrase()
//{
//    AskPassphraseDialog dlg(AskPassphraseDialog::ChangePass, this);
//    dlg.setModel(walletModel);
//    dlg.exec();
//}
//
//void MainView::unlockWallet()
//{
//    if(!walletModel)
//        return;
//    // Unlock wallet when requested by wallet model
//    if (walletModel->getEncryptionStatus() == WalletModel::Locked)
//    {
//        AskPassphraseDialog dlg(AskPassphraseDialog::Unlock, this);
//        dlg.setModel(walletModel);
//        dlg.exec();
//    }
//}
//
//void MainView::usedSendingAddresses()
//{
//    if(!walletModel)
//        return;
//    AddressBookPage *dlg = new AddressBookPage(AddressBookPage::ForEditing, AddressBookPage::SendingTab, this);
//    dlg->setAttribute(Qt::WA_DeleteOnClose);
//    dlg->setModel(walletModel->getAddressTableModel());
//    dlg->show();
//}
//
//void MainView::usedReceivingAddresses()
//{
//    if(!walletModel)
//        return;
//    AddressBookPage *dlg = new AddressBookPage(AddressBookPage::ForEditing, AddressBookPage::ReceivingTab, this);
//    dlg->setAttribute(Qt::WA_DeleteOnClose);
//    dlg->setModel(walletModel->getAddressTableModel());
//    dlg->show();
//}
//
void MainView::showProgress(const QString &title, int nProgress)
{
    if (nProgress == 0)
    {
        progressDialog = new QProgressDialog(title, "", 0, 100);
        progressDialog->setWindowModality(Qt::ApplicationModal);
        progressDialog->setMinimumDuration(0);
        progressDialog->setCancelButton(0);
        progressDialog->setAutoClose(false);
        progressDialog->setValue(0);
    }
    else if (nProgress == 100)
    {
        if (progressDialog)
        {
            progressDialog->close();
            progressDialog->deleteLater();
        }
    }
    else if (progressDialog)
        progressDialog->setValue(nProgress);
}
