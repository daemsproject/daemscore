/* 
 * File:   jsinterface.cpp
 * Author: alan
 *
 * Created on May 6, 2015, 8:29 PM
 */

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
#include "main.h"
#include "init.h"
#include "ccc/content.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_utils.h"
using namespace std;
using namespace json_spirit;
JsInterface::JsInterface( BitcoinGUI *_gui)
    : QObject(_gui),gui(_gui)
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
    //delete(m_watcher);
}
//static void NotifyBlockHeight(JsInterface* jsInterface,uint256 blockHash)
//{
//    QMetaObject::invokeMethod(jsInterface, "notifyBlockHeight", Qt::QueuedConnection,                              
//                              Q_ARG(uint256, blockHash));
//}
//static void NotifyTransactionChanged(JsInterface* jsInterface,const uint256 txid,const uint256 hashBlock){
//    QMetaObject::invokeMethod(jsInterface, "notifyTransactionChanged", Qt::QueuedConnection,                              
//                              Q_ARG(uint256, txid),Q_ARG(uint256, hashBlock));
//    
//}
void JsInterface::subscribeToCoreSignals()
{
    // Connect signals to client
//    qRegisterMetaType<uint256>("uint256");
//    uiInterface.NotifyBlockTip.connect(boost::bind(NotifyBlockHeight,this,_1));
//    pwalletMain->NotifyTransactionChanged.connect(boost::bind(NotifyTransactionChanged,this,_1,_2));
    //uiInterface.NotifyNumConnectionsChanged.connect(boost::bind(NotifyNumConnectionsChanged, this, _1));
    //uiInterface.NotifyAlertChanged.connect(boost::bind(NotifyAlertChanged, this, _1, _2));
}

//void JsInterface::notifyBlockHeight(const uint256 blockHash)
//{
//    BlockMap::iterator mi = mapBlockIndex.find(blockHash);
//    Object obj;
//    obj.push_back(Pair("type","block"));
//    obj.push_back(Pair("blockHash",blockHash.GetHex()));        
//    obj.push_back(Pair("blockHeight",Value((int)mi->second->nBlockHeight)));        
//    //string result="{\:\"block\",\"blockHash\":\""++"\",\"blockHeight\":"+mi->second->nBlockHeight+"}";
//    emit notify(QString().fromStdString(write_string(Value(obj),false)));
//    LogPrintf("jsinterface:notifyblockheight\n");
//    //QMetaObject::invokeMethod(JsInterface, "notifyBlockHeight", Qt::QueuedConnection,                              
//    //                          Q_ARG(uint256, blockHash));
//}


//void JsInterface::notifyTransactionChanged(const uint256 txid,const uint256 hashBlock)
//{
//    
//    CCoinsViewCache view(pcoinsTip);
//    
//    Array arrAddresses;
//    CTransaction tx;
//    if(hashBlock==0){
//        tx=mempool.mapTx[txid].GetTx();
//    
//    }
//    else{    
//        uint256 tmp;
//        if (!GetTransaction(txid,tx,tmp))
//            return;
//    }
//    
//    if(!tx.IsCoinBase()){        
//      BOOST_FOREACH(const CTxIn &txin, tx.vin) {                  
//            const COutPoint &prevout = txin.prevout;
//            const CCoins *coins = view.AccessCoins(prevout.hash);  
//            if (coins==NULL){
//                LogPrintf("NotifyTransactionChanged null coin\n");
//                continue;
//            }           
//            string add;
//            ScriptPubKeyToString(coins->vout[prevout.n].scriptPubKey,add);
//            arrAddresses.push_back(Value(add));            
//      }
//    }
//    
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
//    
//    Object txJson;
//    TxToJSON(tx, hashBlock, txJson);
//    
//    Object notifyObj;
//    notifyObj.push_back(Pair("type",Value("tx")));
//    notifyObj.push_back(Pair("tx",Value(txJson)));
//    notifyObj.push_back(Pair("ids",arrAddresses));
//    
//    emit notify(QString().fromStdString(write_string(Value(notifyObj),false)));
//    
//}
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
//            if (command.toStdString()==string("requestpayment"))
//                return HandlePaymentReqeust(arrData);
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
    LogPrintf("jsinterface:jscall result %s\n",str.substr(0,100));
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
QString JsInterface::HandlePaymentReqeust(Array arrData)
{
    
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
        questionString.append(" (" + QString::number((double)tx.GetSerializeSize(SER_NETWORK, CTransaction::CURRENT_VERSION) / 1000) + " kB)");
    

    // add total amount in all subdivision units
    questionString.append("<hr />");
    CAmount totalAmount = tx.GetValueOut() + txFee;
    
    
    questionString.append(tr("Total Amount %1")
        .arg(BitcoinUnits::formatHtmlWithUnit(0, totalAmount)));
    //questionString.arg();
    return questionString;
}

bool DecodeSigs(string ssInput,std::vector<CScript> sigs){
    return false;
}
/*
 * 
 */


