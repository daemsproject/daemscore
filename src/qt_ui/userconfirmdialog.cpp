// Copyright (c) 2011-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "userconfirmdialog.h"
#include "ui_userconfirmdialog.h"

//#include "addresstablemodel.h"
#include "bitcoinunits.h"
//#include "clientmodel.h"
//#include "coincontroldialog.h"
#include "guiutil.h"
//#include "../ccc/content.h"
//#include "optionsmodel.h"
//#include "sendcoinsentry.h"
//#include "walletmodel.h"

//#include "base58.h"
//#include "coincontrol.h"
#include "ui_interface.h"
//#include "wallet.h"
//#include "bitcoingui.h"
#include <QMessageBox>
#include <QScrollBar>
#include <QSettings>
#include <QTextDocument>

UserConfirmDialog::UserConfirmDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserConfirmDialog)
        //gui(parent)
    //clientModel(0),
    //model(0),
    //fNewRecipientAllowed(true),
    //fFeeMinimized(true)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC // Icons on push buttons are very uncommon on Mac
    //ui->addButton->setIcon(QIcon());
    //ui->clearButton->setIcon(QIcon());
    //ui->sendButton->setIcon(QIcon());
#endif

    //GUIUtil::setupAddressWidget(ui->lineEditCoinControlChange, this);

    //addEntry();

    //connect(ui->addButton, SIGNAL(clicked()), this, SLOT(addEntry()));
    //connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clear()));

    // Coin Control
//    connect(ui->pushButtonCoinControl, SIGNAL(clicked()), this, SLOT(coinControlButtonClicked()));
//    connect(ui->checkBoxCoinControlChange, SIGNAL(stateChanged(int)), this, SLOT(coinControlChangeChecked(int)));
//    connect(ui->lineEditCoinControlChange, SIGNAL(textEdited(const QString &)), this, SLOT(coinControlChangeEdited(const QString &)));

    // Coin Control: clipboard actions


   
}


UserConfirmDialog::~UserConfirmDialog()
{

    delete ui;
}
//void UserConfirmDialog::on_sendButton_clicked()
//{}
//void UserConfirmDialog::on_cancelButton_clicked(){};
//QString UserConfirmDialog::getPaymentAlertMessage(CWalletTx tx)
//{
//    
//    // Format confirmation message
//    QStringList formatted;
//    foreach(const CTxOut &rcp, tx.vout)
//    {
//        // generate bold amount string
//        QString amount = "<b>" + BitcoinUnits::formatHtmlWithUnit(0, rcp.nValue);
//        amount.append("</b>");
//        // generate monospace address string
//        string add;
//        ScriptPubKeyToString(rcp.scriptPubKey,add);
//        QString address = "<span style='font-family: monospace;'>" + QString().fromStdString(add);
//        address.append("</span>");
//        
//        QString recipientElement;
//            //if(rcp.label.length() > 0) // label with address
////            {
////                recipientElement = tr("%1 to %2").arg(amount, GUIUtil::HtmlEscape(rcp.label));
////                recipientElement.append(QString(" (%1)").arg(address));
////            }
//            //else // just address
//            {
//                recipientElement = tr("%1 to %2").arg(amount, address);
//            }
////        
////        else if(!rcp.authenticatedMerchant.isEmpty()) // secure payment request
////        {
////            recipientElement = tr("%1 to %2").arg(amount, GUIUtil::HtmlEscape(rcp.authenticatedMerchant));
////        }
////        else // insecure payment request
////        {
////            recipientElement = tr("%1 to %2").arg(amount, address);
////        }
//
//        formatted.append(recipientElement);
//        
//        QString content=QString().fromStdString(CContent(rcp.strContent).ToHumanString());
//        if (content.size()>100)
//            content=content.left(100);
//        if (content.size()>0){
//            recipientElement = tr("    message:%1").arg(content);
//            formatted.append(recipientElement);
//        }
//    }
//    CAmount txFee = tx.GetFee();
//    QString questionString = tr("Are you sure you want to send?");
//    questionString.append("<br /><br />%1");
//
//        // append fee string if a fee is required
//        questionString.append("<hr /><span style='color:#aa0000;'>");
//        questionString.append(BitcoinUnits::formatHtmlWithUnit(0, txFee));
//        questionString.append("</span> ");
//        questionString.append(tr("added as transaction fee"));
//
//        // append transaction size
//        questionString.append(" (" + QString::number((double)tx.GetSerializeSize(SER_NETWORK, CTransaction::CURRENT_VERSION) / 1000) + " kB)");
//    
//
//    // add total amount in all subdivision units
//    questionString.append("<hr />");
//    CAmount totalAmount = tx.GetValueOut() + txFee;
//    
//    
//    questionString.append(tr("Total Amount %1")
//        .arg(BitcoinUnits::formatHtmlWithUnit(0, totalAmount)));
//    return questionString;
//}
//
//
//
//
//bool UserConfirmDialog::HandlePaymentRequest(std::string strToken,PaymentRequest pr, CWalletTx tx,bool fRequestPassword)
//{
//    QString alert=getPaymentAlertMessage(tx);    
//    ui->windowTitle->setText(tr("Confirm Payment"));
//    ui->label_message->setText(alert);
//    if (!fRequestPassword){
//        ui->label_7->hide();
//        ui->passwordEdit->hide();
//    }
//    return true;
//}




