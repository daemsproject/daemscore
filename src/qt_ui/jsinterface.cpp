
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
//#include "main.h"
#include "init.h"
#include "ccc/content.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_utils.h"
using namespace std;
using namespace json_spirit;
JsInterface::JsInterface( BitcoinGUI *_gui)
    : QObject(_gui),gui(_gui),walletModel(0)
{
    LogPrintf("jsinterface:gui: %i \n",gui);
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
void JsInterface::subscribeToCoreSignals()
{
    // Connect signals to client
    qRegisterMetaType<uint256>("uint256");
    uiInterface.NotifyBlockTip.connect(boost::bind(NotifyBlockHeight,this,_1));
    pwalletMain->NotifyTransactionChanged.connect(boost::bind(NotifyTransactionChanged,this,_1,_2));
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
    LogPrintf("jsinterface:notifyblockheight\n");
    //QMetaObject::invokeMethod(JsInterface, "notifyBlockHeight", Qt::QueuedConnection,                              
    //                          Q_ARG(uint256, blockHash));
}


void JsInterface::notifyTransactionChanged(const uint256 txid,const uint256 hashBlock)
{
    LogPrintf("jsinterface:notifytx\n");
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
//    if(!tx.IsCoinBase()){        
//      BOOST_FOREACH(const CTxIn &txin, tx.vin) {                  
//            const COutPoint &prevout = txin.prevout;
//             CTransaction prevTx;
//            uint256 tmphash;
//            if (!GetTransaction(prevout.hash, prevTx, tmphash, true)){
//                   LogPrintf("NotifyTransactionChanged null coin\n");
//            continue;
//            }
//            LogPrintf("jsinterface:notifytx31\n");
////            if (coins==NULL){
////                LogPrintf("NotifyTransactionChanged null coin\n");
////                continue;
////            }           
//            string add;
//            ScriptPubKeyToString(prevTx.vout[prevout.n].scriptPubKey,add);
//            LogPrintf("jsinterface:notifytx32\n");
//            arrAddresses.push_back(Value(add));            
//      }
//    }
//    LogPrintf("jsinterface:notifytx4\n");
//    BOOST_FOREACH(const CTxOut &txout, tx.vout) {
//        //if address is empty don't record it
//        if (txout.scriptPubKey.size()==0)
//            continue;     
//        string addr;
//        ScriptPubKeyToString(txout.scriptPubKey,addr);
//        Value add=Value(addr);
//        if (find(arrAddresses.begin(),arrAddresses.end(),add)==arrAddresses.end()){
//            arrAddresses.push_back(add);
//        }
//    }
    //LogPrintf("jsinterface:notifytx4\n");
    Object notifyObj;
    notifyObj.push_back(Pair("type",Value("tx")));
    notifyObj.push_back(Pair("tx",Value(txJson)));
    //notifyObj.push_back(Pair("ids",arrAddresses));
    //LogPrintf("jsinterface:notifytx6\n");
    emit notify(QString().fromStdString(write_string(Value(notifyObj),false)));
    //LogPrintf("jsinterface:notifytx7\n");
}
void JsInterface::notifyAccountSwitched(std::string id){
    Object notifyObj;
    notifyObj.push_back(Pair("type",Value("accountSwitch")));
    notifyObj.push_back(Pair("id",Value(id)));
    emit notify(QString().fromStdString(write_string(Value(notifyObj),false)));
}
void JsInterface::test(){
    LogPrintf("jsinterface:test called\n");
    QString str=QString("test passed");    
    //emit feedback(str);//(str);
} 
QString JsInterface::jscall(QString command,QString dataJson){
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
                return HandlePaymentRequest(arrData);
            if (command.toStdString()==string("decryptmessages"))
                return EncryptMessages(arrData);
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
QString JsInterface::jscallasync(QString command,QString dataJson,QString successfunc,QString errorfunc){
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
//        if (command.toStdString()==string("requestpayment"))
//            {
//            
//            PaymentRequest pr=ParsePaymentRequest(arrData[0]);
//            CWalletTx tx;
//            string strError;
//            bool fRequestPassword;
//            tx=CreateRawTransaction(pr,fRequestPassword);
//            return gui->handlePaymentRequest(strToken,pr,tx,fRequestPassword);  
//            //emit requestPayment(strToken,pr,tx,fRequestPassword);            
//            //Sreturn QString().fromStdString("{\"token\":"+strToken+"\"}"); 
//         }
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
QString JsInterface::HandlePaymentRequest(Array arrData)
{
    PaymentRequest pr=ParsePaymentRequest(arrData[0]);
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
    //LogPrintf("jsinterface:hadlepaymentrequest:id,pwalletmain id:%i",HexStr(id.begin(),id.end()),HexStr(pwalletMain->id.begin(),pwalletMain->id.end()));
    if(id==pwalletMain->GetID())
        pwallet=pwalletMain;
    else
        pwallet=new CWallet(id);    
    int nOP=0;//0::unencrypted,1::encrypted,2::offline
    if (!pwallet->HavePriv())
        nOP=2;
    else if (pwallet->IsLocked())
        nOP=1;
    if (!pwallet->CreateTransactionUnsigned(pr,tx,strError)){
        if(pwallet!=pwalletMain)
                delete pwallet;
                return QString().fromStdString("{\"error\":\""+strError+"\"}"); 
    }
    //tx=CreateRawTransaction(pr,fRequestPassword,pwallet);
    LogPrintf("jsinterface:hadlepaymentrequest:tx created,pwallet:%i",pwallet);
    SecureString ssInput;
    QString alert=getPaymentAlertMessage(tx); 
    QString title=QString(tr("Request Payment"));
    if (!gui->handleUserConfirm(title,alert,nOP,strError,ssInput)){
        if(pwallet!=pwalletMain)
        delete pwallet;
        return QString().fromStdString("{\"error\":\""+strError+"\"}");             
        }    
    if(nOP==1)
        if(!pwallet->SetPassword(ssInput)){
            if(pwallet!=pwalletMain)
            delete pwallet;
            return QString().fromStdString("{\"error\":\"wrong password\"}");
        }
    CWalletTx wtxSigned; 
    if(nOP==2){
        std::vector<CScript> sigs;
        if (!DecodeSigs(string(ssInput.begin(),ssInput.end()),sigs)){
            if(pwallet!=pwalletMain)
            delete pwallet;
            return QString().fromStdString("{\"error\":\"invalid signatures\"}");
        }
        
        CMutableTransaction mtx=CMutableTransaction(tx);        
        for(unsigned int i=0;i<mtx.vin.size();i++)
            mtx.vin[i].scriptSig=sigs[i];
        *static_cast<CTransaction*>(&wtxSigned) = CTransaction(mtx);
    }
    else if(!pwallet->SignTransaction(tx, wtxSigned,pr.nSigType)){
        if(pwallet!=pwalletMain)
        delete pwallet;
        return QString().fromStdString("{\"error\":\"sign transaction failed\"}");            
    }
     LogPrintf("jsinterface:hadlepaymentrequest:signOK\n");
    if (!wtxSigned.AcceptToMemoryPool(false))
        {            
            LogPrintf("jsinterface:hadlepaymentrequest:sendtx : Error: Transaction not valid\n");
            delete pwallet;
            return QString().fromStdString("{\"error\":\"tx rejected\"}");;
        }
     LogPrintf("jsinterface:hadlepaymentrequest:acceptedto mempool\n");
     RelayTransaction(wtxSigned);
     LogPrintf("jsinterface:hadlepaymentrequest:sendtx :%s\n",EncodeHexTx(CTransaction(wtxSigned)));
     if(pwallet!=pwalletMain)
        delete pwallet;
    return QString("{\"success\":\"tx sent\"}");
            
}
QString JsInterface::getPaymentAlertMessage(CWalletTx tx)
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
            //if(rcp.label.length() > 0) // label with address
//            {
//                recipientElement = tr("%1 to %2").arg(amount, GUIUtil::HtmlEscape(rcp.label));
//                recipientElement.append(QString(" (%1)").arg(address));
//            }
            //else // just address
            {
                recipientElement = tr("%1 to %2").arg(amount, address);
            }
//        
//        else if(!rcp.authenticatedMerchant.isEmpty()) // secure payment request
//        {
//            recipientElement = tr("%1 to %2").arg(amount, GUIUtil::HtmlEscape(rcp.authenticatedMerchant));
//        }
//        else // insecure payment request
//        {
//            recipientElement = tr("%1 to %2").arg(amount, address);
//        }
        //LogPrintf(recipientElement.toStdString());
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
QString JsInterface::getEncryptMessegeAlert(std::vector<string> vstrIDsForeign,bool fEncrypt)
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
QString JsInterface::EncryptMessages(Array params)
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
        tmp=find_value(obj, "messages");
        if (tmp.type()!=array_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter messages, expected array");      
        Array arrMsg=tmp.get_array();
        std::vector<string> vMsg;
        for(unsigned int i=0;i<arrMsg.size();i++){
            if(arrMsg[i].type()!=array_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter message, expected string");
            CContent cc;
            cc.SetJson(arrMsg[i].get_array());
            vMsg.push_back(cc);
        }
        mapMessages.insert(make_pair(IDForeign,vMsg));     
        
    }
    CWallet* pwallet =new CWallet(IDLocal,false);    
    int nOP=0;//0::unencrypted,1::encrypted,2::offline
    if (!pwallet->HavePriv())
        nOP=2;
    else if (pwallet->IsCrypted())
        nOP=1;        
    LogPrintf("jsinterface:DecryptMessages:pwallet:%i",pwallet);
    SecureString ssInput;    
    string strError;
    QString alert=getEncryptMessegeAlert(vstrIDsForeign,fEncrypt); 
    QString title=QString(tr("Confirm encrypt messages"));
    if(!fEncrypt)
        title=QString(tr("Confirm decrypt messages"));
    if (!gui->handleUserConfirm(title,alert,nOP,strError,ssInput)){
        delete pwallet;
        return QString().fromStdString("{\"error\":\""+strError+"\"}");             
    } 
    if(nOP==1)
        if(!pwallet->SetPassword(ssInput)){
            delete pwallet;
            return QString().fromStdString("{\"error\":\"wrong password\"}");
        }
    std::map<string,std::vector<string> > mapMsgOut;
    if(nOP==2){
        //TODO show json of messages to get decoded,and collect decoded messages from signer
        return QString().fromStdString("{\"error\":\"encryption failed\"}");    
    }
    else if(!pwallet->EncryptMessages(mapMessages, mapMsgOut,fEncrypt)){
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
            arrMsg.push_back(it->second[i]);
        }
        objMsg.push_back(Pair("messages",arrMsg));
        arrResult.push_back(objMsg);
    }
    delete pwallet;
    return QString().fromStdString(write_string(Value(arrResult),false));  
}
bool DecodeSigs(string ssInput,std::vector<CScript> sigs){
    return false;
}
/*
 * 
 */


