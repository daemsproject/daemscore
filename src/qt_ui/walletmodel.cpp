// Copyright (c) 2011-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "walletmodel.h"
#include "bitcoingui.h"

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
#include "rpcserver.h"
#include "core_io.h"
#include "random.h"
#include "txdb.h"
#include "bitcoinunits.h"
#include <boost/thread.hpp>
#include <stdint.h>
//#include "key.h"
#include <QDebug>
#include <QSet>
#include <QTimer>
#include "json/json_spirit_value.h"
#include "json/json_spirit_utils.h"
using namespace std;
using namespace json_spirit;

WalletModel::WalletModel(CWallet *walletIn,BitcoinGUI *guiIn,QObject *parent) :
    QObject(parent), wallet(walletIn), wallet2(0),gui(guiIn),fSwitchToAccount(false),strHeaderTarget(""),
    cachedEncryptionStatus(Unencrypted),cachedNumBlocks(0)
{
    
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


void WalletModel::updateStatus()
{
    EncryptionStatus newEncryptionStatus = getEncryptionStatus();

    //if(cachedEncryptionStatus != newEncryptionStatus)
    {
        emit encryptionStatusChanged(newEncryptionStatus);
        cachedEncryptionStatus=newEncryptionStatus;
    }
}


//void WalletModel::updateAddressBook(const QString &address, const QString &label,
//        bool isMine, const QString &purpose, int status)
//{
//    if(addressTableModel)
//        addressTableModel->updateEntry(address, label, isMine, purpose, status);
//}

//
//OptionsModel *WalletModel::getOptionsModel()
//{
//    return optionsModel;
//}
//


WalletModel::EncryptionStatus WalletModel::getEncryptionStatus() const
{
    if(!wallet->IsCrypted())
        return Unencrypted;    
    else if(wallet->IsLocked())   
        return Locked;    
    else  
        return Unlocked;
}

bool WalletModel::setWalletEncrypted(bool encrypted, const SecureString &passphrase)
{
    if(encrypted)  
        // Encrypt
        return wallet->EncryptWallet(passphrase);   
    else
        return wallet->DecryptWallet(passphrase);        
   
}

bool WalletModel::setWalletLocked(bool locked, const SecureString &passPhrase)
{
    if(locked)   
        // Lock
        return wallet->Lock();
   
    else
        // Unlock
        return wallet->Unlock(passPhrase);   
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
}

void WalletModel::unsubscribeFromCoreSignals()
{
    // Disconnect signals from wallet
    wallet->NotifyStatusChanged.disconnect(boost::bind(&NotifyKeyStoreStatusChanged, this, _1));
    wallet->NotifyAccountSwitched.disconnect(boost::bind(&NotifyAccountSwitched, this, _1));
    //wallet->NotifyAddressBookChanged.disconnect(boost::bind(NotifyAddressBookChanged, this, _1, _2, _3, _4, _5, _6));
    //wallet->NotifyTransactionChanged.disconnect(boost::bind(NotifyTransactionChanged, this, _1, _2, _3));
    wallet->ShowProgress.disconnect(boost::bind(ShowProgress, this, _1, _2));   
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
    fSwitchToAccount=fSwitchTo;
    if(header.size()>0)
    {
        delete wallet2;
        wallet2= new CWallet();
        wallet2->CreateNew("",false);
        //LogPrintf("WalletModel createNewAccount1 \n"); 
        std::vector<std::string> vstr;
        
        //LogPrintf("WalletModel createNewAccount11 \n"); 
        strHeaderTarget=header.toStdString();
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
        if (fSwitchTo)
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
    LogPrintf("WalletModel notifyEcMinerResult basepubvs wallet2id:%b,strHeader vs target:%b \n",wallet2->GetID()==basePub,B32Equal(strHeader,strHeaderTarget));
    if(wallet2->GetID()==basePub&&B32Equal(strHeader,strHeaderTarget))
    {
        CKey resultKey;
        CKey baseKey;
        //LogPrintf("WalletModel notifyEcMinerResult2 \n");
        wallet2->GetBaseKey(baseKey);
        //LogPrintf("WalletModel notifyEcMinerResult3 \n");     
        baseKey.AddSteps(stepKey,1,resultKey);
        CPubKey resultPub;
        resultKey.GetPubKey(resultPub);
        std::string add=CBitcoinAddress(resultPub).ToString();        
        if(B32Equal(add.substr(0,strHeaderTarget.size()),strHeaderTarget))
        { 
            std::string resultAdd=strHeaderTarget.append(add.substr(strHeaderTarget.size()));
            LogPrintf("WalletModel notifyEcMinerResult4 result:%s\n",resultAdd);
            CKey originalStepKey;
            wallet2->GetStepKey(originalStepKey);      
        //LogPrintf("WalletModel notifyEcMinerResult5 \n");
            wallet2->Set(resultKey,originalStepKey,ssPassPhrase,resultAdd,true);        
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
    if(CompareBase32(ID.toStdString(),CBitcoinAddress(wallet->GetID()).ToString())==0)
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

QString WalletModel::HandlePaymentRequest(const Array arrData)
{
    CPaymentOrder pr=ParseJsonPaymentRequest(arrData[0],0);
    return DoPayment(pr); 
}
QString WalletModel::DoPayment(const CPaymentOrder& pr)
{
    CWalletTx tx;
    string strError;    
    CPubKey id;    
    CTxDestination address;
    if(!ExtractDestination(pr.vFrom[0],address))        
            return QString().fromStdString("{\"error\":\"wrong idfrom\"}");
    CBitcoinAddress pub;
    pub.Set(address);
    pub.GetKey(id);
    //memcpy(&id, &pr.vFrom[0][1], 20);
    CWallet* pwallet;
    //LogPrintf("jsinterface:hadlepaymentrequest:id,pwalletmain id:%i",HexStr(id.begin(),id.end()),HexStr(wallet->id.begin(),wallet->id.end()));
    if(id==wallet->GetID())
        pwallet=wallet;
    else
        pwallet=new CWallet(id);    
    int nOP=0;//0::unencrypted,1::encrypted,2::offline
    if (!pwallet->HavePriv())
        nOP=2;
    else if (pwallet->IsLocked())
        nOP=1;
    bool fDelete=(pwallet!=wallet);
    if (!pwallet->CreateTransactionUnsigned(pr,tx,strError)){
        if(fDelete)
                delete pwallet;
                return QString().fromStdString("{\"error\":\""+strError+"\"}"); 
    }
    //tx=CreateRawTransaction(pr,fRequestPassword,pwallet);
    LogPrintf("walletmodel:dopayment:tx created,pwallet:%i",pwallet);
    SecureString ssInput;
    QString alert; 
    QString title;
    switch (pr.nRequestType)
    {
        case PR_PUBLISH:
            alert = getPaymentAlertMessage(tx);
            title = QString(tr("Publish Content"));
            break;
        case PR_DOMAIN_REGISTER:
            alert = getDomainRegisterAlertMessage(tx,pr);
            title = QString(tr("Register Domain"));
        case PR_DOMAIN_RENEW:
            alert = getDomainRegisterAlertMessage(tx,pr);
            title = QString(tr("Renew Domain"));
            break;
        case PR_DOMAIN_UPDATE:
            alert = getDomainUpdateAlertMessage(tx,pr);
            title = QString(tr("Update Domain"));
            break;
        case PR_DOMAIN_TRANSFER:
            alert = getDomainTransferAlertMessage(tx,pr);
            title = QString(tr("Transfer Domain"));
            break;
        default:
        alert=getPaymentAlertMessage(tx); 
        title=QString(tr("Request Payment"));
    }
    if (!gui->handleUserConfirm(title,alert,nOP,strError,ssInput)){
        if(fDelete)
        delete pwallet;
        return QString().fromStdString("{\"error\":\""+strError+"\"}");             
    }  
    std::string result;
    CWalletTx wtxSigned; 
    SignAndSendTx(pwallet,tx,pr.nSigType,nOP,ssInput,fDelete,wtxSigned,result);
    return QString().fromStdString(result);
}
QString WalletModel::HandlePaymentRequest2(const Array arrData)
{
    std::vector<unsigned char> raw = ParseHexV(arrData[2], "parameter 3");
    CContent ctt(raw);
    CPaymentOrder pr = GetPublisherPaymentRequest(arrData[0].get_str(), arrData[1].get_str(), ctt);
    return DoPayment(pr);    
}

QString WalletModel::getPaymentAlertMessage(const CWalletTx& tx)
{
    
    // Format confirmation message
    QStringList formatted;
    foreach(const CTxOut &rcp, tx.vout)
    {
        // generate bold amount string
        QString amount = "<b>" + BitcoinUnits::formatHtmlWithUnit(0, rcp.nValue);
        amount.append("</b>");
        //LogPrintf(amount.toStdString());
        // generate monospace address string
        string add;
        ScriptPubKeyToString(rcp.scriptPubKey,add);
        QString address = "<span style='font-family: monospace;'>" + QString().fromStdString(add);
        address.append("</span>");
        //LogPrintf(address.toStdString());
        QString recipientElement;
          
            {
                recipientElement = tr("%1 to %2").arg(amount, address);
            }

        formatted.append(recipientElement);
        
        QString content=QString().fromStdString(CContent(rcp.strContent).ToHumanString());
        if (content.size()>100)
            content=content.left(100);
        if (content.size()>0){
            recipientElement = tr("    message:%1").arg(content);
            formatted.append(recipientElement);
        }
    }
    CAmount txFee = tx.GetFee();
    QString questionString = tr("Are you sure you want to send?");
    questionString.append("<br /><br />");
    questionString.append(formatted.join("<br />"));
        // append fee string if a fee is required
        questionString.append("<hr /><span style='color:#aa0000;'>");
        questionString.append(BitcoinUnits::formatHtmlWithUnit(0, txFee));
        questionString.append("</span> ");
        questionString.append(tr("added as transaction fee"));

        // append transaction size
        questionString.append(" (" + QString::number((double)(tx.CTransaction::GetSerializeSize(SER_NETWORK, CTransaction::CURRENT_VERSION)+tx.vin.size()*67) / 1000) + " kB)");
    

    // add total amount in all subdivision units
    questionString.append("<hr />");
    CAmount totalAmount = tx.GetValueOut() + txFee;
    
    
    questionString.append(tr("Total Amount %1")
        .arg(BitcoinUnits::formatHtmlWithUnit(0, totalAmount)));
    //questionString.arg();
    return questionString;
}
QString WalletModel::getEncryptMessegeAlert(const std::vector<string>& vstrIDsForeign,const bool fEncrypt)
{
    
    // Format confirmation message
    QStringList formatted;
    foreach(const string ID, vstrIDsForeign)
    {
        QString address = "<span style='font-family: monospace;'>" + QString().fromStdString(ID);
        address.append("</span>");        
        formatted.append(address);                
    }    
    
    
    QString questionString = tr("Are you sure you want to decrypt messages related to IDs below?");
    if(fEncrypt)
        questionString = tr("Are you sure you want to encrypt messages related to IDs below?");
    questionString.append("<br /><br />");
    questionString.append(formatted.join("<br />"));

    questionString.append("<hr />");
    
    return questionString;
}
QString WalletModel::getDomainUpdateAlertMessage(const CWalletTx& tx,const CPaymentOrder& pr)
{
    QStringList formatted;
    //string str1="domain";
    std::map<string,string> p=pr.info;
    QString domain = "<span style='font-family: monospace;'>"+tr("Domain name:")+QString().fromStdString(p["domain"]);
    domain.append("</span>");        
    formatted.append(domain);
    string add;
    ScriptPubKeyToString(pr.vFrom[0],add);
    QString address = "<span style='font-family: monospace;'>" +tr("Registered by:") +QString().fromStdString(add);
    address.append("</span>");
    formatted.append(address);  
    
    QString questionString = tr("Please confirm to update domain info:");    
    questionString.append("<br /><br />");
    questionString.append(formatted.join("<br />"));
    questionString.append("<hr /><span style='color:#aa0000;'>");
        questionString.append(BitcoinUnits::formatHtmlWithUnit(0, tx.GetFee()));
        questionString.append("</span> ");
        questionString.append(tr("added as transaction fee"));

        // append transaction size
        questionString.append(" (" + QString::number((double)(tx.CTransaction::GetSerializeSize(SER_NETWORK, CTransaction::CURRENT_VERSION)+tx.vin.size()*67) / 1000) + " kB)");
    
    questionString.append("<hr />");  
    
    return questionString;
}
QString WalletModel::getDomainTransferAlertMessage(const CWalletTx& tx,const CPaymentOrder& pr)
{
    //string str1="domain";
    std::map<string,string> p=pr.info;
    QStringList formatted;
    QString domain = "<span style='font-family: monospace;'>"+tr("Domain name:")+QString().fromStdString(p["domain"]);
    domain.append("</span>");        
    formatted.append(domain);
    string add;
    ScriptPubKeyToString(pr.vFrom[0],add);
    QString address = "<span style='font-family: monospace;'>" +tr("Registered by:") +QString().fromStdString(add);
    address.append("</span>");
    formatted.append(address);  
    //string str2="transfer";
    QString address1 = "<span style='font-family: monospace;'>" +tr("Transfer to:") +QString().fromStdString(p["transfer"]);
    address1.append("</span>");
    formatted.append(address1); 
    QString questionString = tr("Please confirm to transfer domain:");    
    questionString.append("<br /><br />");
    questionString.append(formatted.join("<br />"));
    questionString.append("<hr /><span style='color:#aa0000;'>");
        questionString.append(BitcoinUnits::formatHtmlWithUnit(0, tx.GetFee()));
        questionString.append("</span> ");
        questionString.append(tr("added as transaction fee"));

        // append transaction size
        questionString.append(" (" + QString::number((double)(tx.CTransaction::GetSerializeSize(SER_NETWORK, CTransaction::CURRENT_VERSION)+tx.vin.size()*67) / 1000) + " kB)");
    
    questionString.append("<hr />");  
    
    return questionString;
}
QString WalletModel::getDomainRegisterAlertMessage(const CWalletTx& tx,const CPaymentOrder& pr)
{
    //string str1="domain";
    std::map<string,string> p=pr.info;
    QStringList formatted;
    QString domain = "<span style='font-family: monospace;'>"+tr("Domain name:")+QString().fromStdString(p["domain"]);
    domain.append("</span>");        
    formatted.append(domain);
    string add;
    ScriptPubKeyToString(pr.vFrom[0],add);
    QString address = "<span style='font-family: monospace;'>" +tr("Register by:") +QString().fromStdString(add);
    address.append("</span>");
    formatted.append(address);  
    QString amount = "<span style='font-family: monospace;color:#aa0000;'>" +tr("Locked value:") +"<b>" + BitcoinUnits::formatHtmlWithUnit(0, tx.GetValueOut());
    amount.append("</b></span>");
    formatted.append(amount);
    QString timelasting = "<span style='font-family: monospace;color:#aa0000;'>" +tr("Lock for:") +QString().fromStdString(num2str((GetLockLasting(tx.vout[0].nLockTime)/3600/24))) +tr("days");
    timelasting.append("</b></span>");
    formatted.append(timelasting);
    QString questionString = tr("Please check domain registration details:");    
    questionString.append("<br /><br />");
    questionString.append(formatted.join("<br />"));
    questionString.append("<hr />");    
    return questionString;
}
QString WalletModel::EncryptMessages(Array params)
{
    
    if (params.size() <2)
        throw runtime_error("Wrong number of parameters");
    if (params[0].type() != str_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter1, expected string");
    CPubKey IDLocal=AccountFromValue(params[0]);
    //string strIDLocal=params[0].get_str();
    if (params[1].type() != array_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter2, expected array");    
    bool fEncrypt=true;
    if(params.size()>2)
    {
        if (params[2].type()!=bool_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter3, expected boolean");    
        fEncrypt=params[2].get_bool();
    }
    Array arrIDMsg=params[1].get_array();  
    //id-(pubkey,alias)
    std::map<string,std::vector<string> > mapMessages;
    std::vector<string> vstrIDsForeign;
    bool fHasAllSharedKeys=true;    
    bool fIsWalletMain;
    CWallet* pwallet;
    //LogPrintf("jsinterface:hadlepaymentrequest:id%s,pwalletmain id:%s size:%i\n",HexStr(id.begin(),id.end()),HexStr(wallet->GetID().begin(),wallet->GetID().end()),wallet->GetID().size());
    if(IDLocal==wallet->GetID())
    {
        pwallet=wallet;
        fIsWalletMain=true;
    }
    else
        pwallet=new CWallet(IDLocal,false); 
    for(unsigned int i=0;i<arrIDMsg.size();i++){
        if(arrIDMsg[i].type()!=obj_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter contact, expected object");
        Object obj=arrIDMsg[i].get_obj();        
        Value tmp;
        tmp=find_value(obj, "idForeign");
        if (tmp.type()!=str_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter id, expected string");        
        vstrIDsForeign.push_back(tmp.get_str());
        //CPubKey IDForeign=AccountFromValue(tmp.get_str());
        string IDForeign=tmp.get_str();
        CPubKey pubForeign=AccountFromValue(tmp);
        fHasAllSharedKeys=pwallet->HasSharedKey(IDLocal,pubForeign);        
        tmp=find_value(obj, "messages");
        if (tmp.type()!=array_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter messages, expected array");      
        Array arrMsg=tmp.get_array();
        std::vector<string> vMsg;
        for(unsigned int i=0;i<arrMsg.size();i++){
            if(arrMsg[i].type()!=array_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter message, expected array");
            CContent cc;
            if(!cc.SetJson(arrMsg[i].get_array())){
                LogPrintf("jsinterface:encryptMessages fail:%s\n",HexStr(cc.begin(),cc.end()));
                
            }
            vMsg.push_back(cc);
            //LogPrintf("jsinterface:encryptMessages:content:%s\n",HexStr(cc.begin(),cc.end()));
        }
        mapMessages.insert(make_pair(IDForeign,vMsg));  
    }
    
    int nOP=0;//0::unencrypted,1::encrypted,2::offline
    SecureString ssInput;    
    string strError;
    
         
    //LogPrintf("jsinterface:encryptMessages:pwallet:%i\n",pwallet);
    if(!fHasAllSharedKeys)
    {
        if (!pwallet->HavePriv())
            nOP=2;
        else if (pwallet->IsCrypted())
            nOP=1;   
        QString alert=getEncryptMessegeAlert(vstrIDsForeign,fEncrypt); 
        QString title=QString(tr("Confirm encrypt messages"));
        if(!fEncrypt)
            title=QString(tr("Confirm decrypt messages"));
        if (!gui->handleUserConfirm(title,alert,nOP,strError,ssInput)){
            if(!fIsWalletMain)
            delete pwallet;
            return QString().fromStdString("{\"error\":\""+strError+"\"}");             
        } 
    }
    if(nOP==1)
        if(!pwallet->SetPassword(ssInput)){
            if(!fIsWalletMain)
            delete pwallet;
            return QString().fromStdString("{\"error\":\"wrong password\"}");
        }
    std::map<string,std::vector<string> > mapMsgOut;
    if(nOP==2){
        //TODO show json of messages to get decoded,and collect decoded messages from signer
        return QString().fromStdString("{\"error\":\"encryption failed\"}");    
    }
    else if(!pwallet->EncryptMessages(mapMessages, mapMsgOut,fEncrypt)){
        if(!fIsWalletMain)
        delete pwallet;
        return QString().fromStdString("{\"error\":\"encryption failed\"}");            
    }
    //Value result;
    Array arrResult;
    for(std::map<string,std::vector<string> >::iterator it=mapMsgOut.begin();it!=mapMsgOut.end();it++)
    {
        Object objMsg;
        objMsg.push_back(Pair("id",it->first));
        Array arrMsg;
        for(unsigned int i=0;i<it->second.size();i++)
        {
            arrMsg.push_back(CContent(it->second[i]).ToJson(STR_FORMAT_B64));
        }
        objMsg.push_back(Pair("messages",arrMsg));
        arrResult.push_back(objMsg);
    }
    if(!fIsWalletMain)
    delete pwallet;
    return QString().fromStdString(write_string(Value(arrResult),false));  
}


QString WalletModel::SendMessage(Array arrData)
{
    if ( arrData.size() !=3)
        throw runtime_error("");
    string idLocal=arrData[0].get_str();
    string idForeign=arrData[1].get_str();
    CContent msg=_create_text_content(arrData[2].get_str());    
    CPaymentOrder pr=MessageRequestToPaymentRequest(idLocal,idForeign,msg);
    CWalletTx tx;
    string strError;    
    CPubKey pub;
    if(!CBitcoinAddress(idLocal).GetKey(pub))    
        return QString().fromStdString("{\"error\":\"wrong idlocal\"}");
    CPubKey pubForeign;         
    if(!CBitcoinAddress(idForeign).GetKey(pubForeign))    
        return QString().fromStdString("{\"error\":\"wrong idforeign\"}");
    bool fIsWalletMain;
    CWallet* pwallet;
    //LogPrintf("jsinterface:hadlepaymentrequest:id%s,pwalletmain id:%s size:%i\n",HexStr(id.begin(),id.end()),HexStr(wallet->GetID().begin(),wallet->GetID().end()),wallet->GetID().size());
    if(pub==wallet->GetID())
    {
        pwallet=wallet;
        fIsWalletMain=true;
    }
    else
        pwallet=new CWallet(pub,false);    
    int nOP=0;//0::unencrypted,1::encrypted,2::offline
    SecureString ssInput;
    if (!pwallet->HavePriv())
            nOP=2;
        else if (pwallet->IsLocked())
            nOP=1;
    if(!pwallet->HasSharedKey(pub,pubForeign)||nOP>0||msg.size()>1000)
    {
        QString alert=getSMSAlertMessage(pr);  
        QString title=QString(tr("Request Send Message"));
        if (!gui->handleUserConfirm(title,alert,nOP,strError,ssInput)){
            if(!fIsWalletMain)
                delete pwallet;
            return QString().fromStdString("{\"error\":\""+strError+"\"}");             
        } 
    }
    if(nOP==1)
        if(!pwallet->SetPassword(ssInput)){
            if(!fIsWalletMain)
            delete pwallet;
            return QString().fromStdString("{\"error\":\"wrong password\"}");
        }
    CWalletTx wtxSigned; 
    if(nOP==2){
        std::vector<CScript> sigs;
        if (!DecodeSigs(string(ssInput.begin(),ssInput.end()),sigs)){
            if(!fIsWalletMain)
                delete pwallet;
            return QString().fromStdString("{\"error\":\"invalid signatures\"}");
        }
        
        CMutableTransaction mtx=CMutableTransaction(tx);        
        for(unsigned int i=0;i<mtx.vin.size();i++)
            mtx.vin[i].scriptSig=sigs[i];
        *static_cast<CTransaction*>(&wtxSigned) = CTransaction(mtx);
    }
    else
    {
        std::map<string,std::vector<string> > mapMsgOut;
        std::map<string,std::vector<string> > mapMessages;
        std::vector<string> vmsg;
        vmsg.push_back(msg);
        mapMessages[idForeign]=vmsg;
    
        if(!pwallet->EncryptMessages(mapMessages, mapMsgOut,true))
        {
            LogPrintf("jsinterface:SendMessage:encryption failed\n");
            if(!fIsWalletMain)
                delete pwallet;
            return QString().fromStdString("{\"error\":\"encryption failed\"}");;
        }
        //LogPrintf("jsinterface:SendMessage:encryption done:%s\n",mapMsgOut[idForeign].size());
        if(mapMsgOut[idForeign].size()>0)
         pr.vout[0].strContent=mapMsgOut[idForeign][0];
     
        if (!pwallet->CreateTransactionUnsigned(pr,tx,strError))
        {   
            if(!fIsWalletMain)        
                delete pwallet;
            return QString().fromStdString("{\"error\":\""+strError+"\"}"); 
        }    
        //LogPrintf("jsinterface:SendMessage:createtransaction done\n");
        if(!pwallet->SignTransaction(tx, wtxSigned,pr.nSigType))
        {
            if(!fIsWalletMain)        
                delete pwallet;
            return QString().fromStdString("{\"error\":\"sign transaction failed\"}");  
        }
    }
     //LogPrintf("jsinterface:SendMessage:signOK\n");
    if (!wtxSigned.AcceptToMemoryPool(false))
        {            
            LogPrintf("jsinterface:SendMessage:sendtx : Error: Transaction not valid\n");
            if(!fIsWalletMain)    
                delete pwallet;
            return QString().fromStdString("{\"error\":\"tx rejected\"}");;
        }
     //LogPrintf("jsinterface:SendMessage:acceptedto mempool\n");
     RelayTransaction(wtxSigned);
     LogPrintf("jsinterface:SendMessage:sendtx :%s\n",EncodeHexTx(CTransaction(wtxSigned)));
     if(!fIsWalletMain)
        delete pwallet;
    return QString("{\"success\":\"tx sent\"}");
            
}
QString WalletModel::getSMSAlertMessage(const CPaymentOrder& pr)
{    
    // Format confirmation message
    QStringList formatted;
    foreach(const CTxOut &rcp, pr.vout)
    {
        string add;
        ScriptPubKeyToString(rcp.scriptPubKey,add);
        QString address = "<span style='font-family: monospace;'>" + QString().fromStdString(add);
        address.append("</span>");
        //LogPrintf(address.toStdString());
        QString recipientElement;
        QString content=QString().fromStdString(CContent(rcp.strContent).ToHumanString());
        if (content.size()>100)
            content=content.left(100);
        recipientElement = tr("%1 to %2").arg(content, address);           
        formatted.append(recipientElement);
    }
    CAmount txFee = ::minRelayTxFee.GetFee(pr.vout[0].strContent.size()+180);
    QString questionString = tr("Are you sure you want to send message?");
    questionString.append("<br /><br />");
    questionString.append(formatted.join("<br />"));
        // append fee string if a fee is required
        questionString.append("<hr /><span style='color:#aa0000;'>");
        questionString.append(BitcoinUnits::formatHtmlWithUnit(0, txFee));
        questionString.append("</span> ");
        questionString.append(tr("transaction fee"));
    return questionString;
}
QString WalletModel::RegisterDomain(json_spirit::Array arrData)
{
    LogPrintf("walletmodel:RegisterDomain");
    if ( arrData.size() !=3)
        throw runtime_error("");
    CPubKey id=AccountFromValue(arrData[0]);
    CScript scriptPubKey;
    StringToScriptPubKey(arrData[0].get_str(),scriptPubKey);
    string strDomain=arrData[1].get_str();
    uint32_t nLockTime=(uint32_t)arrData[2].get_int();    
    CPaymentOrder pr = GetRegisterDomainPaymentRequest(arrData[0].get_str(), strDomain, nLockTime);   
    
    CDomain cdomain;
    if(pDomainDBView->GetDomainByName(strDomain,cdomain)&&(GetLockLasting(cdomain.nExpireTime)>0))
        return QString().fromStdString("{\"error:domain already registered\"}");  
    if(IsLevel2Domain(strDomain))
    {
        LogPrintf("walletmodel:RegisterDomain level1domain:%s",GetLevel1Domain(strDomain));
        if(!pDomainDBView->GetDomainByName(GetLevel1Domain(strDomain),cdomain)||(GetLockLasting(cdomain.nExpireTime)==0))
            return QString().fromStdString("{\"error: level1 domain not exists\"}");  
        if(cdomain.owner!=scriptPubKey)
            return QString().fromStdString("{\"error: level1 domain not belonged to sending id\"}");  
    }
    else
    {
        
    }
    return DoPayment(pr);
}
QString WalletModel::UpdateDomain(json_spirit::Array arrData)
{
    LogPrintf("walletmodel:UpdateDomain \n");
    if ( arrData.size() !=3)
        throw runtime_error("");
    CPubKey id=AccountFromValue(arrData[0]);
    CScript scriptPubKey;
    StringToScriptPubKey(arrData[0].get_str(),scriptPubKey);
    LogPrintf("walletmodel:UpdateDomain scriptPubkey:s% \n",scriptPubKey.ToString());
    string strDomain=arrData[1].get_str();
    Object objInfo=arrData[2].get_obj();    
    CPaymentOrder pr = GetUpdateDomainPaymentRequest(arrData);       
    CDomain cdomain;
    if(!pDomainDBView->GetDomainByName(strDomain,cdomain)||(GetLockLasting(cdomain.nExpireTime)==0))
        return QString().fromStdString("{\"error:domain not registered\"}");  
    if(IsLevel2Domain(strDomain))
    {
        if(!pDomainDBView->GetDomainByName(GetLevel1Domain(strDomain),cdomain)||(GetLockLasting(cdomain.nExpireTime)==0))
            return QString().fromStdString("{\"error: level1 domain not exists\"}");  
        if(cdomain.owner!=scriptPubKey)
            return QString().fromStdString("{\"error: level1 domain not belonged to sending id\"}");  
    }
    else
    {        
        LogPrintf("walletmodel:UpdateDomain scriptPubkey:%s \n",scriptPubKey.ToString());
        if(cdomain.owner!=scriptPubKey)
            return QString().fromStdString("{\"error:domain not belonged to sending id\"}");  
    }
    return DoPayment(pr);      
    
}
QString WalletModel::RenewDomain(json_spirit::Array arrData)
{
      LogPrintf("walletmodel:RenewDomain \n");
    if ( arrData.size() !=3)
        throw runtime_error("");
    CPubKey id=AccountFromValue(arrData[0]);
    CScript scriptPubKey;
    StringToScriptPubKey(arrData[0].get_str(),scriptPubKey);    
    LogPrintf("walletmodel:RenewDomain scriptPubkey:s% \n",scriptPubKey.ToString());
    string strDomain=arrData[1].get_str();    
    uint32_t nLockTime=arrData[2].get_int64();
    CPaymentOrder pr = GetRegisterDomainPaymentRequest(arrData[0].get_str(), strDomain, nLockTime); 
    
    CDomain cdomain;
    if(IsLevel2Domain(strDomain))
        {
            if(!pDomainDBView->GetDomainByName(GetLevel1Domain(strDomain),cdomain)||(GetLockLasting(cdomain.nExpireTime)==0))
                return QString().fromStdString("{\"error: level1 domain not exists\"}");  
            if(cdomain.owner!=scriptPubKey)
                return QString().fromStdString("{\"error: level1 domain not belonged to sending id\"}");  
        }
    if(pDomainDBView->GetDomainByName(strDomain,cdomain)&&(GetLockLasting(cdomain.nExpireTime)>0))
    {
        if(cdomain.owner!=scriptPubKey)
                return QString().fromStdString("{\"error:domain not belonged to sending id\"}");  
        if(LockTimeToTime(nLockTime)<LockTimeToTime(cdomain.nExpireTime))
            return QString().fromStdString("{\"error:domain renew time earlier than expire time\"}");  
        pr.nRequestType=PR_DOMAIN_RENEW;
    }
    return DoPayment(pr);    
}
QString WalletModel::TransferDomain(json_spirit::Array arrData)
{
     LogPrintf("walletmodel:TransferDomain \n");
    if ( arrData.size() !=3)
        throw runtime_error("");
    //CPubKey id=AccountFromValue(arrData[0]);
    CScript scriptPubKey;
    StringToScriptPubKey(arrData[0].get_str(),scriptPubKey);    
    LogPrintf("walletmodel:TransferDomain scriptPubkey:s% \n",scriptPubKey.ToString());    
    string strDomain=arrData[1].get_str();    
    string idTo=arrData[2].get_str();
    CScript scriptPubKey2;         
    if(!StringToScriptPubKey(idTo,scriptPubKey2))        
        return QString().fromStdString("{\"error: tranfer target id invalid\"}");  
    Object objInfo; 
    objInfo.push_back(Pair("transfer",idTo));
    Array arr=arrData;
    arr[2]=Value(objInfo);
    CPaymentOrder pr = GetUpdateDomainPaymentRequest(arr); 
    pr.nRequestType=PR_DOMAIN_TRANSFER;
    pr.info["transfer"]=arrData[2].get_str();
    CDomain cdomain;
    if(!pDomainDBView->GetDomainByName(strDomain,cdomain)||(GetLockLasting(cdomain.nExpireTime)==0))
        return QString().fromStdString("{\"error:domain not registered\"}");  
    if(IsLevel2Domain(strDomain))
    {
        if(!pDomainDBView->GetDomainByName(GetLevel1Domain(strDomain),cdomain)||(GetLockLasting(cdomain.nExpireTime)==0))
            return QString().fromStdString("{\"error: level1 domain not exists\"}");  
        if(cdomain.owner!=scriptPubKey)
            return QString().fromStdString("{\"error: level1 domain not belonged to sending id\"}");  
    }
    else
    {        
        LogPrintf("walletmodel:TransferDomain scriptPubkey:s% \n",scriptPubKey.ToString());
        if(cdomain.owner!=scriptPubKey)
            return QString().fromStdString("{\"error:domain not belonged to sending id\"}");  
    }
    return DoPayment(pr);     
}
QString WalletModel::PublishProduct(json_spirit::Array arrData)
{     
    
    CPaymentOrder pr = GetPublishProductPaymentRequest(arrData); 
    return DoPayment(pr); 
}
QString WalletModel::BuyProduct(json_spirit::Array arrData)
{     
    
    CPaymentOrder po = GetPublishProductPaymentRequest(arrData); 
    return DoPayment(po); 
}