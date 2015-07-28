
#include <cstdlib>
//#include <QNetworkReply>
//#include <QNetworkRequest>
//#include <QNetworkAccessManager>
//#include <QNetworkDiskCache>
//#include <QtConcurrentMap>
//#include <QWebFrame>
#include "jsinterface.h"
#include "bitcoinunits.h"
#include "util.h"
#include "rpcserver.h"
#include "core_io.h"
#include "random.h"
#include "bitcoingui.h"
#include "wallet.h"
#include "walletmodel.h"

//#include "main.h"
#include "init.h"
#include "ccc/content.h"
#include "ccc/settings.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_utils.h"
using namespace std;
using namespace json_spirit;
JsInterface::JsInterface( BitcoinGUI *_gui)
    : QObject(_gui),walletModel(0)
{
    
    //subscribeToCoreSignals();
    /*  ImageAnalyzer only wants to receive http responses
        for requests that it makes, so that's why it has its own
        QNetworkAccessManager. */
    //m_network = new QNetworkAccessManager(this);
    //m_watcher = new QFutureWatcher<QRgb>(this);
    /*  We want to share a cache with the web browser,
        in case it has some images we want: */
    //m_network->setCache(m_cache);

//    QObject::connect(m_network, SIGNAL(finished(QNetworkReply*)),
//                     this, SLOT(handleReply(QNetworkReply*)));
//    QObject::connect(m_watcher, SIGNAL(finished()),
//                     this, SLOT(doneProcessing()));
//    QObject::connect(m_watcher, SIGNAL(progressValueChanged(int)),
//                     this, SLOT(progressStatus(int)));
    //QObject::connect(gui, SIGNAL(sendMoneyResult(std::string,bool,QString)), this, SLOT(jscallback(std::string,bool,QString)));
    //QObject::connect(this, SIGNAL(requestPayment(std::string,PaymentRequest,CWalletTx,bool)), gui, SLOT(handlePaymentRequest(std::string,PaymentRequest,CWalletTx,bool)));
   
}
JsInterface::~JsInterface(){
    if(walletModel!=NULL)
        delete walletModel;
    //delete(m_watcher);
}
void JsInterface::setWalletModel(WalletModel *walletModelIn)
{
    walletModel=walletModelIn;    
   connect((QObject*)walletModel, SIGNAL(accountSwitched(std::string)), this, SLOT(notifyAccountSwitched(std::string)));   
}
static void NotifyBlockHeight(JsInterface* jsInterface,uint256 blockHash)
{
    QMetaObject::invokeMethod(jsInterface, "notifyBlockHeight", Qt::QueuedConnection,                              
                              Q_ARG(uint256, blockHash));
}
static void NotifyTransactionChanged(JsInterface* jsInterface,const uint256 txid,const uint256 hashBlock){
    QMetaObject::invokeMethod(jsInterface, "notifyTransactionChanged", Qt::QueuedConnection,                              
                              Q_ARG(uint256, txid),Q_ARG(uint256, hashBlock));
    
}
static void NotifyBlockChainFallback(JsInterface* jsInterface,const int nBlockHeight,const uint256 hashBlock){
    QMetaObject::invokeMethod(jsInterface, "notifyBlockChainFallback", Qt::QueuedConnection,                              
                              Q_ARG(int, nBlockHeight),Q_ARG(uint256, hashBlock));
    
}
static void NotifyNewExtendedKey(JsInterface* jsInterface,std::string id)
{
    QMetaObject::invokeMethod(jsInterface, "notifyNewExtendedKey", Qt::QueuedConnection,                              
                              Q_ARG(std::string,id));
}
void JsInterface::subscribeToCoreSignals()
{
    // Connect signals to client
    qRegisterMetaType<uint256>("uint256");
    qRegisterMetaType<std::string>("std::string");
    uiInterface.NotifyBlockTip.connect(boost::bind(NotifyBlockHeight,this,_1));
    pwalletMain->NotifyTransactionChanged.connect(boost::bind(NotifyTransactionChanged,this,_1,_2));
    pwalletMain->NotifyBlockChainFallback.connect(boost::bind(NotifyBlockChainFallback,this,_1,_2));
    pwalletMain->NotifyNewExtendedKey.connect(boost::bind(NotifyNewExtendedKey,this,_1));
    //uiInterface.NotifyNumConnectionsChanged.connect(boost::bind(NotifyNumConnectionsChanged, this, _1));
    //uiInterface.NotifyAlertChanged.connect(boost::bind(NotifyAlertChanged, this, _1, _2));
}

void JsInterface::notifyBlockHeight(const uint256 blockHash)
{
    BlockMap::iterator mi = mapBlockIndex.find(blockHash);
    Object obj;
    obj.push_back(Pair("type","block"));
    obj.push_back(Pair("blockHash",blockHash.GetHex()));        
    obj.push_back(Pair("blockHeight",Value((int)mi->second->nBlockHeight)));        
    //string result="{\:\"block\",\"blockHash\":\""++"\",\"blockHeight\":"+mi->second->nBlockHeight+"}";
    emit notify(QString().fromStdString(write_string(Value(obj),false)));
    //LogPrintf("jsinterface:notifyblockheight\n");
    //QMetaObject::invokeMethod(JsInterface, "notifyBlockHeight", Qt::QueuedConnection,                              
    //                          Q_ARG(uint256, blockHash));
}
 void JsInterface::notifyBlockChainFallback(const int nBlockHeight,const uint256 hashBlock)
 {
     Object obj;
    obj.push_back(Pair("type","fallback"));
    obj.push_back(Pair("blockHash",hashBlock.GetHex()));        
    obj.push_back(Pair("blockHeight",Value(nBlockHeight)));        
    
    emit notify(QString().fromStdString(write_string(Value(obj),false)));
 }

void JsInterface::notifyTransactionChanged(const uint256 txid,const uint256 hashBlock)
{
    //LogPrintf("jsinterface:notifytx\n");
    //CCoinsViewCache view(pcoinsTip);
    //LogPrintf("jsinterface:notifytx1\n");
    Array arrAddresses;
    CTransaction tx;
    if(hashBlock==0){
        tx=mempool.mapTx[txid].GetTx();
        //LogPrintf("jsinterface:notifytx2\n");
    }
    else{    
        uint256 tmp;
        if (!GetTransaction(txid,tx,tmp))
            return;
    }
    //LogPrintf("jsinterface:notifytx3\n");
    
    //LogPrintf("jsinterface:notifytx5\n");
    Object txJson;
    try{
        TxToJSON(tx, hashBlock, txJson);
    }
    catch (Object& objError)
    {
        LogPrintf("jsinterface:notifytx obj  error\n");
        return;
    }
    catch (std::exception& e)
    {
        LogPrintf("jsinterface:notifytx std error\n");
        return;            
    } 

    Object notifyObj;
    notifyObj.push_back(Pair("type",Value("tx")));
    notifyObj.push_back(Pair("tx",Value(txJson)));
    //notifyObj.push_back(Pair("ids",arrAddresses));
    //LogPrintf("jsinterface:notifytx6\n");
    emit notify(QString().fromStdString(write_string(Value(notifyObj),false)));
    //LogPrintf("jsinterface:notifytx7\n");
}
void JsInterface::notifyAccountSwitched(const std::string id){
    Object notifyObj;
    notifyObj.push_back(Pair("type",Value("accountSwitch")));
    notifyObj.push_back(Pair("id",Value(id)));
    emit notify(QString().fromStdString(write_string(Value(notifyObj),false)));
}
void JsInterface::notifyNewExtendedKey(const std::string id)
{
    Object notifyObj;
    notifyObj.push_back(Pair("type",Value("newID")));
    notifyObj.push_back(Pair("id",Value(id)));
    emit notify(QString().fromStdString(write_string(Value(notifyObj),false)));
}
//enum pageid
//{
//    WALLETPAGE_ID=1,
//    BROWSERPAGE_ID=2,
//    PUBLISHERPAGE_ID=3,
//    MESSENGERPAGE_ID=4,
//    MINERPAGE_ID=5,
//    DOMAINPAGE_ID=6,
//    SETTINGPAGE_ID=7,
//    SERVICEPAGE_ID=8
//};
std::string appNames[9]={"null","wallet","browser","publisher","messenger","miner","domainname","setting","service"};
QString JsInterface::jscall(QString command,QString dataJson,int nPageID){
    json_spirit::Value valData;
    json_spirit::Array arrData;
    json_spirit::Value valResult;
    
    try {
        if (dataJson.size()>0){
            
            if (!json_spirit::read_string(dataJson.toStdString(),valData))
                return QString("{\"error\":\"data is not json format\"}");
            if (valData.type()!=json_spirit::array_type)
                return QString("{\"error\":\"data is not json array\"}");
            arrData=valData.get_array();
            if (command.toStdString()==string("requestpayment"))
                return walletModel->HandlePaymentRequest(arrData);
            if (command.toStdString()==string("requestpayment2"))
                return walletModel->HandlePaymentRequest2(arrData);
            if (command.toStdString()==string("encryptmessages"))
                return walletModel->EncryptMessages(arrData);
            if (command.toStdString()==string("sendmessage"))
                return walletModel->SendMessage(arrData);
            if (command.toStdString()==string("registerdomain"))
                return walletModel->RegisterDomain(arrData);
            if (command.toStdString()==string("renewdomain"))
                return walletModel->RenewDomain(arrData);
            if (command.toStdString()==string("updatedomain"))
                return walletModel->UpdateDomain(arrData);
            if (command.toStdString()==string("transferdomain"))
                return walletModel->TransferDomain(arrData);
            if (command.toStdString()==string("publishproduct"))
                return walletModel->PublishProduct(arrData);
            if (command.toStdString()==string("buyproduct"))
                return walletModel->BuyProduct(arrData);
            if (command.toStdString()==string("writefile")||command.toStdString()==string("readfile")
                    ||command.toStdString()==string("getconf")||command.toStdString()==string("setconf"))
            {
                if(arrData[0].type()!=str_type)
                    throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, expected string");
                std::string appName=arrData[0].get_str();
                if (nPageID==WALLETPAGE_ID)
                    throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid pageID, wallet page setting is forbidden");
                if (nPageID<9&&appNames[nPageID]!=appName)
                    throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid appName, not corresponds to pageid");
            }
                
        }
        //return QString("{\"error\":\"empty data\"}");
        valResult= tableRPC.execute(command.toStdString(),arrData);            
        
    }
    catch (Object& objError)
    {
        LogPrintf("jsinterface:jscall obj  error\n");
        Object reply;    
        reply.push_back(Pair("error", objError));
        valResult=Value(reply);            
    }
    catch (std::exception& e)
    {
        LogPrintf("jsinterface:jscall error\n");
        Object reply;    
        reply.push_back(Pair("error", Value(e.what())));
        valResult=Value(reply);            
    }  
    string str=json_spirit::write_string(valResult,false);
    //LogPrintf("jsinterface:jscall result %s\n",str);
    QString result=QString::fromStdString(str);    
    return result;
}
QString JsInterface::jscallasync(QString command,QString dataJson,QString successfunc,QString errorfunc,int nPageID){
    LogPrintf("jsinterface:jscallasync command %s\n",command.toStdString());
   

    json_spirit::Value valData;
    json_spirit::Array arrData;
    RandAddSeedPerfmon();            
    uint256 token=GetRandHash();            
    string strToken=token.ToString();           
    mapAsync.insert(make_pair(strToken,make_pair(successfunc,errorfunc)));
    try {        
        if (!json_spirit::read_string(dataJson.toStdString(),valData))
            return QString("{\"error\":\"data is not json format\"}");
        if (valData.type()!=json_spirit::array_type)
            return QString("{\"error\":\"data is not json array\"}");
        arrData=valData.get_array();           

    }
    catch (Object& objError)
    {
        LogPrintf("jsinterface:jscall obj  error %s\n",write_string(Value(objError),true));
        return QString().fromStdString(write_string(Value(objError),true));
     }
   
   
    jscallback("0",false,QString().fromStdString("{\"error\":\"command not found\"}"));
    return QString("{\"error\":\"command not found\"}");
   
    //QString result=jscall(command,dataJson);
    
    
    //emit feedback(result,errorfunc);
    //return result;
}
void JsInterface::jscallback(std::string strToken,bool fSuccess,QString dataJson){
    LogPrintf("jsinterface:jscallback token %s,result %s\n",strToken,dataJson.toStdString());
    for(std::map<std::string,std::pair<QString,QString> >::iterator it=mapAsync.begin();it!=mapAsync.end();it++){
        if (it->first==strToken){
            LogPrintf("jsinterface:jscallback token %s found\n",strToken);
            if(fSuccess)
                emit feedback(dataJson,it->second.first);
            else
                emit feedback(dataJson,it->second.second);
            mapAsync.erase(it);
            return;
        }
            
    }
    //emit feedback(dataJson,func);
    //webpage->page()->mainFrame()->evaluateJavaScript("console.log('evaluate js:'"+errorfunc+");var func="+errorfunc+";func("+dataJson+");");
}
//typedef std::basic_string<char, std::char_traits<char>, secure_allocator<char> > SecureString;
//bool DecodeSigs(string ssInput,std::vector<CScript> sigs){
//    return false;
//}
