/* 
 * File:   jsinterface.cpp
 * Author: alan
 *
 * Created on May 6, 2015, 8:29 PM
 */

#include <cstdlib>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QtConcurrentMap>
#include "jsinterface.h"
#include "util.h"
#include "rpcserver.h"
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_utils.h"
#include "json/json_spirit_writer_template.h"
using namespace std;
using namespace json_spirit;
JsInterface::JsInterface( QObject* parent)
    : QObject(parent)
{
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
}
JsInterface::~JsInterface(){
    delete(m_watcher);
}
void JsInterface::test(){
    LogPrintf("jsinterface:test called\n");
    QString str=QString("test passed");    
    emit feedback(str);//(str);
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
        }
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
    LogPrintf("jsinterface:jscall result %s\n",str);
    QString result=QString::fromStdString(str);    
    return result;
}
QString JsInterface::jscallasync(QString command,QString dataJson,QString successfunc,QString errorfunc){
    QString result=jscall(command,dataJson);
    return result;
}

/*
 * 
 */


