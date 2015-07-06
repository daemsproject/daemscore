// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_USERTCONFIRMDIALOG_H
#define BITCOIN_QT_USERTCONFIRMDIALOG_H



#include <QDialog>
#include <QString>
//#include "bitcoingui.h"
//#include "ui_userconfirmdialog.h"
class ClientModel;
//class OptionsModel;
//class SendCoinsEntry;
//class SendCoinsRecipient;

//class CPaymentOrder;
//class CWalletTx;
namespace Ui {
    class UserConfirmDialog;
}

//QT_BEGIN_NAMESPACE
//class QUrl;
//QT_END_NAMESPACE

/** Dialog for sending bitcoins */
class UserConfirmDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserConfirmDialog(QWidget *parent );
    ~UserConfirmDialog();
    Ui::UserConfirmDialog *ui;
    //QWidget *setupTabChain(QWidget *prev);

    //bool HandlePaymentRequest(std::string strToken,CPaymentOrder pr, CWalletTx tx,bool fRequestPassword);
    //QString getPaymentAlertMessage(CWalletTx tx);
public slots:
    //void clear();
    //void reject();
    //void accept();
    //SendCoinsEntry *addEntry();
    //void updateTabsAndLabels();
    //void setBalance(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance,
    //                const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance);

private:
    
    //BitcoinGUI* gui;


private slots:
    //void on_sendButton_clicked();
    //void on_cancelButton_clicked();


signals:
    // Fired when a message should be reported to the user
    void message(const QString &title, const QString &message, unsigned int style);
};

#endif // BITCOIN_QT_SENDCOINSDIALOG_H
