/* 
 * File:   jsinterface.h
 * Author: alan
 *
 * Created on May 6, 2015, 8:28 PM
 */

#ifndef JSINTERFACE_H
#define	JSINTERFACE_H
#include <QFutureWatcher>
#include <QtWidgets>

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkDiskCache;
class JsInterface: public QObject
{
    Q_OBJECT
public:
    JsInterface(QObject * parent=0);
    ~JsInterface();

public slots:
    /*! initiates analysis of all the urls in the list */
    Q_INVOKABLE
    void test();
    QString jscall(QString command,QString dataJson);
    QString jscallasync(QString command,QString dataJson,QString successfunc,QString errorfunc);
signals:
    //Q_SIGNAL
    void feedback(QString str);
    

private slots:
    


    

private:
    QNetworkAccessManager* m_network;
    QNetworkDiskCache* m_cache;
    QStringList m_URLQueue;
    QList<QImage> m_imageQueue;
    int m_outstandingFetches;
    QFutureWatcher<QRgb> * m_watcher;
    
};


#endif	/* JSINTERFACE_H */

