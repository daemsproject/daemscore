// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_ACCOUNTDIALOG_H
#define BITCOIN_QT_ACCOUNTDIALOG_H



#include <QDialog>
#include <QString>
//#include "bitcoingui.h"
//#include "ui_userconfirmdialog.h"
//class ClientModel;
class WalletModel;
namespace Ui {
    class AccountDialog;
}

//QT_BEGIN_NAMESPACE
//class QUrl;
//QT_END_NAMESPACE

/** Dialog for sending bitcoins */
class AccountDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode {
        CreateNew,
        Switch,
        Encrypt,         /**< Ask passphrase twice and encrypt */        
        Lock,
        Unlock,    
        ChangePass, /**< Ask old passphrase + new passphrase twice */
        Decrypt,
        Export/**< Ask passphrase and decrypt wallet */
    };
    explicit AccountDialog(Mode mode,QWidget *parent,WalletModel* walletModelIn);
    ~AccountDialog();
    Ui::AccountDialog *ui;
    WalletModel* walletModel; 
    Mode mode;
    //QWidget *setupTabChain(QWidget *prev);

    
public slots:
    //void clear();
    //void reject();
    void accept();
    
    //void updateTabsAndLabels();
    

private:
    double eccSpeed;
    void DoAccountAction(bool fSwitchTo);
    //BitcoinGUI* gui;


private slots:
    void textChanged();
    void button1_clicked();
    void button2_clicked();
    void button3_clicked();
    void checkBoxHeaderClicked(bool fChecked);
    void checkBoxEncryptClicked(bool fChecked);
    void headerTextChanged(QString header);
signals:
    // Fired when a message should be reported to the user
    void message(const QString &title, const QString &message, unsigned int style);
};

#endif // BITCOIN_QT_SENDCOINSDIALOG_H
