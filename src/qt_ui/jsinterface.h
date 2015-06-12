
#ifndef JSINTERFACE_H
#define	JSINTERFACE_H
#include <QFutureWatcher>
#include <QtWidgets>
#include <QWebView>
#include "primitives/transaction.h"
#include "wallet.h"
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_utils.h"
#include "json/json_spirit_writer_template.h"

class BitcoinGUI;
class PaymentRequest;
class CWalletTx;
class WalletModel;
class JsInterface: public QObject
{
    Q_OBJECT
public:
    JsInterface(BitcoinGUI *_gui=0);
    ~JsInterface();
    //BitcoinGUI *gui;
    void subscribeToCoreSignals();
    void setWalletModel(WalletModel *walletModelIn);
public slots:    
    void notifyAccountSwitched(std::string id);
    Q_INVOKABLE    
    QString jscall(QString command,QString dataJson,int nPageID=0);
    QString jscallasync(QString command,QString dataJson,QString successfunc,QString errorfunc,int nPageID=0);
    void jscallback(std::string strToken,bool fSuccess,QString dataJson);
    
signals:
    //Q_SIGNAL
    void feedback(QString str,QString func);
    //void requestPayment(std::string strToken,PaymentRequest pr, CWalletTx tx,bool fRequestPassword);
    void notify(QString result);
private slots:
    void notifyBlockHeight(const uint256 blockHash);
    void notifyTransactionChanged(const uint256 txid,const uint256 hashBlock);


    

private:
    std::map<std::string,std::pair<QString,QString> > mapAsync;

    //CWallet *wallet;
    WalletModel *walletModel;
    
};
bool DecodeSigs(string ssInput,std::vector<CScript> sigs);

#endif	/* JSINTERFACE_H */

