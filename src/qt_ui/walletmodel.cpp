// Copyright (c) 2011-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "walletmodel.h"

//#include "addresstablemodel.h"
#include "guiconstants.h"
//#include "recentrequeststablemodel.h"
//#include "transactiontablemodel.h"

#include "base58.h"
#include "keystore.h"
#include "main.h"
#include "ccc/ecminer.h"
#include "sync.h"
#include "ui_interface.h"
#include "wallet.h"
#include "walletdb.h" // for BackupWallet
#include <boost/thread.hpp>
#include <stdint.h>
//#include "key.h"
#include <QDebug>
#include <QSet>
#include <QTimer>

using namespace std;

WalletModel::WalletModel(CWallet *walletIn,QObject *parent) :
    QObject(parent), wallet(walletIn), wallet2(0),fSwitchToAccount(false),strHeaderTarget(""),
    cachedEncryptionStatus(Unencrypted),
    cachedNumBlocks(0)
{
    //fHaveWatchOnly = wallet->HaveWatchOnly();
    //fForceCheckBalanceChanged = false;

    //addressTableModel = new AddressTableModel(wallet, this);
    //transactionTableModel = new TransactionTableModel(wallet, this);
    //recentRequestsTableModel = new RecentRequestsTableModel(wallet, this);

    // This timer will be fired repeatedly to update the balance
    pollTimer = new QTimer(this);
    //connect(pollTimer, SIGNAL(timeout()), this, SLOT(pollBalanceChanged()));
    pollTimer->start(MODEL_UPDATE_DELAY);

    subscribeToCoreSignals();
    updateStatus();
}

WalletModel::~WalletModel()
{
    unsubscribeFromCoreSignals();
}

//CAmount WalletModel::getBalance(const CCoinControl *coinControl) const
//{
//    if (coinControl)
//    {
//        CAmount nBalance = 0;
//        std::vector<COutput> vCoins;
//        wallet->AvailableCoins(vCoins, true, coinControl);
//        BOOST_FOREACH(const COutput& out, vCoins)
//            if(out.fSpendable)
//                nBalance += out.tx->vout[out.i].nValue;
//
//        return nBalance;
//    }
//
//    return wallet->GetBalance();
//}
//
//CAmount WalletModel::getUnconfirmedBalance() const
//{
//    return wallet->GetUnconfirmedBalance();
//}
//
//CAmount WalletModel::getImmatureBalance() const
//{
//    return wallet->GetImmatureBalance();
//}
//


void WalletModel::updateStatus()
{
    EncryptionStatus newEncryptionStatus = getEncryptionStatus();

    if(cachedEncryptionStatus != newEncryptionStatus)
    {
        emit encryptionStatusChanged(newEncryptionStatus);
        cachedEncryptionStatus=newEncryptionStatus;
    }
}





//void WalletModel::updateTransaction()
//{
//    // Balance and number of transactions might have changed
//    fForceCheckBalanceChanged = true;
//}
//
//void WalletModel::updateAddressBook(const QString &address, const QString &label,
//        bool isMine, const QString &purpose, int status)
//{
//    if(addressTableModel)
//        addressTableModel->updateEntry(address, label, isMine, purpose, status);
//}
//
//void WalletModel::updateWatchOnlyFlag(bool fHaveWatchonly)
//{
//    fHaveWatchOnly = fHaveWatchonly;
//    emit notifyWatchonlyChanged(fHaveWatchonly);
//}
//
//bool WalletModel::validateAddress(const QString &address)
//{
//    CBitcoinAddress addressParsed(address.toStdString());
//    return addressParsed.IsValid();
//}
//

//
//OptionsModel *WalletModel::getOptionsModel()
//{
//    return optionsModel;
//}
//
//AddressTableModel *WalletModel::getAddressTableModel()
//{
//    return addressTableModel;
//}
//
//TransactionTableModel *WalletModel::getTransactionTableModel()
//{
//    return transactionTableModel;
//}
//
//RecentRequestsTableModel *WalletModel::getRecentRequestsTableModel()
//{
//    return recentRequestsTableModel;
//}

WalletModel::EncryptionStatus WalletModel::getEncryptionStatus() const
{
    if(!wallet->IsCrypted())
    {
        return Unencrypted;
    }
    else if(wallet->IsLocked())
    {
        return Locked;
    }
    else
    {
        return Unlocked;
    }
}

bool WalletModel::setWalletEncrypted(bool encrypted, const SecureString &passphrase)
{
    if(encrypted)
    {
        // Encrypt
        return wallet->EncryptWallet(passphrase);
    }
    else
    {
        // Decrypt -- TODO; not supported yet
        return wallet->DecryptWallet(passphrase);        
    }
}

bool WalletModel::setWalletLocked(bool locked, const SecureString &passPhrase)
{
    if(locked)
    {
        // Lock
        return wallet->Lock();
    }
    else
    {
        // Unlock
        return wallet->Unlock(passPhrase);
    }
}

bool WalletModel::changePassphrase(const SecureString &oldPass, const SecureString &newPass)
{
    bool retval;
    {
        LOCK(wallet->cs_wallet);
        wallet->Lock(); // Make sure wallet is locked before attempting pass change
        retval = wallet->ChangeWalletPassphrase(oldPass, newPass);
    }
    return retval;
}

bool WalletModel::backupWallet(const QString &filename)
{
    //return BackupWallet(*wallet, filename.toLocal8Bit().data());
    return false;
}

 //Handlers for core signals
static void NotifyKeyStoreStatusChanged(WalletModel *walletmodel, CCryptoKeyStore *wallet)
{
    qDebug() << "NotifyKeyStoreStatusChanged";
    QMetaObject::invokeMethod(walletmodel, "updateStatus", Qt::QueuedConnection);
}
static void NotifyAccountSwitched(WalletModel *walletmodel, std::string id)
{
    qDebug() << "NotifyAccountSwitched";
    QMetaObject::invokeMethod(walletmodel, "notifyAccountSwitched", Qt::QueuedConnection,Q_ARG(std::string,id));
}
//
//static void NotifyAddressBookChanged(WalletModel *walletmodel, CWallet *wallet,
//        const CTxDestination &address, const std::string &label, bool isMine,
//        const std::string &purpose, ChangeType status)
//{
//    QString strAddress = QString::fromStdString(CBitcoinAddress(address).ToString());
//    QString strLabel = QString::fromStdString(label);
//    QString strPurpose = QString::fromStdString(purpose);
//
//    qDebug() << "NotifyAddressBookChanged : " + strAddress + " " + strLabel + " isMine=" + QString::number(isMine) + " purpose=" + strPurpose + " status=" + QString::number(status);
//    QMetaObject::invokeMethod(walletmodel, "updateAddressBook", Qt::QueuedConnection,
//                              Q_ARG(QString, strAddress),
//                              Q_ARG(QString, strLabel),
//                              Q_ARG(bool, isMine),
//                              Q_ARG(QString, strPurpose),
//                              Q_ARG(int, status));
//}
//
//static void NotifyTransactionChanged(WalletModel *walletmodel, CWallet *wallet, const uint256 &hash, ChangeType status)
//{
//    Q_UNUSED(wallet);
//    Q_UNUSED(hash);
//    Q_UNUSED(status);
//    QMetaObject::invokeMethod(walletmodel, "updateTransaction", Qt::QueuedConnection);
//}

static void ShowProgress(WalletModel *walletmodel, const std::string &title, int nProgress)
{
    // emits signal "showProgress"
    QMetaObject::invokeMethod(walletmodel, "showProgress", Qt::QueuedConnection,
                              Q_ARG(QString, QString::fromStdString(title)),
                              Q_ARG(int, nProgress));
}
//
//static void NotifyWatchonlyChanged(WalletModel *walletmodel, bool fHaveWatchonly)
//{
//    QMetaObject::invokeMethod(walletmodel, "updateWatchOnlyFlag", Qt::QueuedConnection,
//                              Q_ARG(bool, fHaveWatchonly));
//}

void WalletModel::subscribeToCoreSignals()
{
             qRegisterMetaType<CPubKey>("CPubKey");
        
         qRegisterMetaType<CKey>("CKey");
         
         qRegisterMetaType<std::string>("std::string");
    // Connect signals to wallet
    wallet->NotifyStatusChanged.connect(boost::bind(&NotifyKeyStoreStatusChanged, this, _1));
    wallet->NotifyAccountSwitched.connect(boost::bind(&NotifyAccountSwitched, this, _1));
    //wallet->NotifyAddressBookChanged.connect(boost::bind(NotifyAddressBookChanged, this, _1, _2, _3, _4, _5, _6));
    //wallet->NotifyTransactionChanged.connect(boost::bind(NotifyTransactionChanged, this, _1, _2, _3));
    
    wallet->ShowProgress.connect(boost::bind(ShowProgress, this, _1, _2));
    //wallet->NotifyWatchonlyChanged.connect(boost::bind(NotifyWatchonlyChanged, this, _1));
}

void WalletModel::unsubscribeFromCoreSignals()
{
    // Disconnect signals from wallet
    wallet->NotifyStatusChanged.disconnect(boost::bind(&NotifyKeyStoreStatusChanged, this, _1));
    wallet->NotifyAccountSwitched.disconnect(boost::bind(&NotifyAccountSwitched, this, _1));
    //wallet->NotifyAddressBookChanged.disconnect(boost::bind(NotifyAddressBookChanged, this, _1, _2, _3, _4, _5, _6));
    //wallet->NotifyTransactionChanged.disconnect(boost::bind(NotifyTransactionChanged, this, _1, _2, _3));
    wallet->ShowProgress.disconnect(boost::bind(ShowProgress, this, _1, _2));
    //wallet->NotifyWatchonlyChanged.disconnect(boost::bind(NotifyWatchonlyChanged, this, _1));
}

// WalletModel::UnlockContext implementation
WalletModel::UnlockContext WalletModel::requestUnlock()
{
    bool was_locked = getEncryptionStatus() == Locked;
    if(was_locked)
    {
        // Request UI to unlock wallet
        emit requireUnlock();
    }
    // If wallet is still locked, unlock was failed or cancelled, mark context as invalid
    bool valid = getEncryptionStatus() != Locked;

    return UnlockContext(this, valid, was_locked);
}

WalletModel::UnlockContext::UnlockContext(WalletModel *wallet, bool valid, bool relock):
        wallet(wallet),
        valid(valid),
        relock(relock)
{
}

WalletModel::UnlockContext::~UnlockContext()
{
    if(valid && relock)
    {
        wallet->setWalletLocked(true);
    }
}

void WalletModel::UnlockContext::CopyFrom(const UnlockContext& rhs)
{
    // Transfer context; old object no longer relocks wallet
    *this = rhs;
    rhs.relock = false;
}

double WalletModel::testEccSpeed()
{
     int nKernels=boost::thread::hardware_concurrency();
     
     //uint256* temphash=new uint256(111);
     CKey tempKey;
     tempKey.MakeNewKey(false);
     CPubKey pub;
     tempKey.GetPubKey(pub);
    //LogPrintf("time:%d \n",startTime);
     double startTime=clock();
    for (int i=0;i<1000;i++){
        pub.AddSteps(pub,1);
    }
    double timeUsed=clock()-startTime;
    //LogPrintf("time used:%d \n",timeUsed);    
    double kernelRate=1000*1000000/timeUsed;
    return kernelRate*nKernels;
}
static void NotifyEcMinerResult(WalletModel* walletModel,const CPubKey basePub,const CKey stepKey,const std::string strHeader){
    //LogPrintf("WalletModel NotifyEcMinerResult \n");
    QMetaObject::invokeMethod(walletModel, "notifyEcMinerResult", Qt::QueuedConnection,                              
                              Q_ARG(CPubKey, basePub),Q_ARG(CKey, stepKey),Q_ARG(std::string,strHeader));
    
}
bool WalletModel::createNewAccount(const QString header,const SecureString &passPhraseNew,bool fSwitchTo)
{
    ssPassPhrase=passPhraseNew;
    if(header.size()>0)
    {
        delete wallet2;
        wallet2= new CWallet();
        wallet2->CreateNew("",false);
        //LogPrintf("WalletModel createNewAccount1 \n"); 
        std::vector<std::string> vstr;
        
        //LogPrintf("WalletModel createNewAccount11 \n"); 
        strHeaderTarget=ToStandardB32String(header.toStdString());
        vstr.push_back(strHeaderTarget); 
        //LogPrintf("WalletModel createNewAccount2 \n"); 
         //qRegisterMetaType<uint256>("uint256");

         //LogPrintf("WalletModel createNewAccount5 \n"); 
        wallet2->NotifyEcMinerResult.connect(boost::bind(NotifyEcMinerResult,this,_1,_2,_3));
        //LogPrintf("WalletModel createNewAccount6 \n"); 
        //CKey baseKey;
        //wallet2->GetBaseKey(baseKey);
        CPubKey basePub=wallet2->GetID();
        //LogPrintf("WalletModel createNewAccount7 \n"); 
         return VanityGen(true,wallet2,vstr,basePub);        
         //LogPrintf("WalletModel createNewAccount8 \n"); 
        //return true;
    }else
    {
        delete wallet2;
        wallet2= new CWallet();
        wallet2->CreateNew(passPhraseNew);
        wallet->SwitchToAccount(wallet2->GetID());      
        return true;
    }
 return false;   
}
bool WalletModel::stopVanityGen(){
    if(wallet2)
    {
        CPubKey pub=wallet2->GetID();
        VanityGen(false,wallet2,std::vector<std::string>(0),pub);
        delete wallet2;
        wallet2=NULL;
    }
}
void WalletModel::notifyEcMinerResult(const CPubKey basePub,const CKey stepKey,const std::string strHeader)
{
    LogPrintf("WalletModel notifyEcMinerResult basepub:%s,wallet2id:%s,strHeader:%s,target:%s \n",CBitcoinAddress(basePub).ToString(),CBitcoinAddress(wallet2->GetID()).ToString(),strHeader,strHeaderTarget);    
    LogPrintf("WalletModel notifyEcMinerResult basepubvs wallet2id:%b,strHeader vs target:%b \n",wallet2->GetID()==basePub,strHeader==strHeaderTarget);
    if(wallet2->GetID()==basePub&&strHeader==strHeaderTarget)
    {
        CKey resultKey;
        CKey baseKey;
        //LogPrintf("WalletModel notifyEcMinerResult2 \n");
        wallet2->GetBaseKey(baseKey);
        //LogPrintf("WalletModel notifyEcMinerResult3 \n");     
        baseKey.AddSteps(stepKey,1,resultKey);
        CPubKey tmppub;
        resultKey.GetPubKey(tmppub);
        LogPrintf("WalletModel notifyEcMinerResult4 result:%s\n",CBitcoinAddress(tmppub).ToString());
        CKey originalStepKey;
        wallet2->GetStepKey(originalStepKey);      
        //LogPrintf("WalletModel notifyEcMinerResult5 \n");
        wallet2->Set(resultKey,originalStepKey,ssPassPhrase,true);        
        //LogPrintf("WalletModel notifyEcMinerResult6 \n");
        if(fSwitchToAccount)
        {
            wallet->SwitchToAccount(wallet2->GetID(),true);        
            //emit(accountChanged());
            //updateStatus();
        }
        //LogPrintf("WalletModel notifyEcMinerResult7 \n");
        emit vanitygenSuccess();
    }
    //LogPrintf("WalletModel notifyEcMinerResult8 \n");
    delete wallet2;
    wallet2=NULL;
}
QStringList WalletModel::GetAccountList()
{
    std::vector<std::string> idlist=wallet->GetAccountList();
    QStringList qlist;
    for(unsigned int i=0;i<idlist.size();i++){
        //LogPrintf("WalletModel GetAccountList:%s \n",idlist[i]);
        qlist.append(QString().fromStdString(idlist[i]));
    }
    return qlist;
}
bool WalletModel::switchToAccount(QString ID)
{
    if(ID.toStdString()==CBitcoinAddress(wallet->GetID()).ToString())
        return false;
    CPubKey pub;
    if(!CBitcoinAddress(ID.toStdString()).GetKey(pub))
        return false;
     wallet->SwitchToAccount(pub,true);
     
     return true;
}
void WalletModel::notifyAccountSwitched(const std::string id)
{
    emit accountSwitched(id);
     updateStatus();
}