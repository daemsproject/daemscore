// Copyright (c) 2011-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "accountdialog.h"
#include "ui_accountdialog.h"
#include "bitcoinunits.h"
#include "guiconstants.h"
#include "guiutil.h"
#include <QValidator>
//#include "base58.h"
#include "util.h"
#include "ui_interface.h"
#include "walletmodel.h"
//#include "wallet.h"
//#include "bitcoingui.h"
#include "allocators.h"
#include <QMessageBox>
#include <QScrollBar>
#include <QSettings>
#include <QTextDocument>
#include <math.h> 
AccountDialog::AccountDialog(Mode mode,QWidget *parent,WalletModel* walletModelIn) :
    QDialog(parent),
    ui(new Ui::AccountDialog),
        walletModel(walletModelIn),
        mode(mode)
        //gui(parent)
    //clientModel(0),
    //model(0),
    //fNewRecipientAllowed(true),
    //fFeeMinimized(true)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC // Icons on push buttons are very uncommon on Mac
    ui->Button1->setIcon(QIcon());
    ui->Button2->setIcon(QIcon());
    ui->Button3->setIcon(QIcon());
#endif
    ui->passEdit1->setMinimumSize(ui->passEdit1->sizeHint());
    ui->passEdit2->setMinimumSize(ui->passEdit2->sizeHint());
    ui->passEdit3->setMinimumSize(ui->passEdit3->sizeHint());
    eccSpeed=walletModel->testEccSpeed();
    ui->headerEdit->setMaxLength((int)(log(eccSpeed)/log(32)+4.5));//MAX_PUBKEY_HEADER_SIZE);
    ui->passEdit1->setMaxLength(MAX_PASSPHRASE_SIZE);
    ui->passEdit2->setMaxLength(MAX_PASSPHRASE_SIZE);
    ui->passEdit3->setMaxLength(MAX_PASSPHRASE_SIZE);
    QRegExp rx("^[A-Za-z0-9]+$");
    QRegExpValidator *pRevalidotor = new QRegExpValidator(rx, this);
    ui->headerEdit->setValidator(new QRegExpValidator(rx, this)); 
    // Setup Caps Lock detection.
    ui->passEdit1->installEventFilter(this);
    ui->passEdit2->installEventFilter(this);
    ui->passEdit3->installEventFilter(this);
    //GUIUtil::setupAddressWidget(ui->lineEditCoinControlChange, this);

    //addEntry();

    connect(ui->Button1, SIGNAL(clicked()), this, SLOT(button1_clicked()));
    connect(ui->Button2, SIGNAL(clicked()), this, SLOT(button2_clicked()));
    connect(ui->Button3, SIGNAL(clicked()), this, SLOT(button3_clicked()));
    connect(ui->checkBoxHeader, SIGNAL(clicked(bool)), this, SLOT(checkBoxHeaderClicked(bool)));
    connect(ui->checkBoxEncrypt, SIGNAL(clicked(bool)), this, SLOT(checkBoxEncryptClicked(bool)));
    connect(walletModel,SIGNAL(vanitygenSuccess()),this,SLOT(accept()));
    //connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clear()));
    switch(mode)
    {
        case Encrypt: // Ask passphrase x2
            
            ui->warningLabel->setText(tr("Enter the new passphrase to the wallet.<br/>Please use a passphrase of <b>ten or more random characters</b>, or <b>eight or more words</b>."));
            ui->passLabel1->hide();
            ui->passEdit1->hide();
            ui->headerLabel->hide();
            ui->headerEdit->hide();            
            ui->timeLabel->hide();
            ui->timeDisplay->hide();
            ui->checkBoxHeader->hide();
            ui->selectLabel->hide();
            ui->selectList->hide();
            ui->checkBoxEncrypt->hide();
            ui->Button3->hide();
            ui->Button2->setText("OK");
            setWindowTitle(tr("Encrypt wallet"));
            break;
        case Unlock: // Ask passphrase
            ui->warningLabel->setText(tr("This operation needs your wallet passphrase to unlock the wallet."));
            ui->passLabel1->hide();
            ui->passEdit1->hide();
            ui->passLabel2->hide();
            ui->passEdit2->hide();
            ui->passLabel3->hide();
            ui->passEdit3->hide();
            ui->headerLabel->hide();
            ui->headerEdit->hide();            
            ui->timeLabel->hide();
            ui->timeDisplay->hide();
            ui->checkBoxHeader->hide();
            ui->selectLabel->hide();
            ui->selectList->hide();
            ui->checkBoxEncrypt->hide();
            ui->Button3->hide();
            ui->Button2->setText("OK");
            setWindowTitle(tr("Lock wallet"));
            break;
        case Lock: // Ask passphrase
            ui->warningLabel->setText(tr("This operation will lock the wallet."));
            ui->passLabel2->hide();
            ui->passEdit2->hide();
            ui->passLabel3->hide();
            ui->passEdit3->hide();
            ui->headerLabel->hide();
            ui->headerEdit->hide();            
            ui->timeLabel->hide();
            ui->timeDisplay->hide();
            ui->checkBoxHeader->hide();
            ui->selectLabel->hide();
            ui->selectList->hide();
            ui->checkBoxEncrypt->hide();
            ui->Button3->hide();
            ui->Button2->setText("OK");
            setWindowTitle(tr("Unlock wallet"));
            break;
        case Decrypt:   // Ask passphrase
            ui->warningLabel->setText(tr("This operation needs your wallet passphrase to decrypt the wallet."));
            ui->headerLabel->hide();
            ui->headerEdit->hide();            
            ui->timeLabel->hide();
            ui->timeDisplay->hide();
            ui->checkBoxHeader->hide();
            ui->passLabel2->hide();
            ui->passEdit2->hide();
            ui->passLabel3->hide();
            ui->passEdit3->hide();
            ui->selectLabel->hide();
            ui->selectList->hide();
            ui->checkBoxEncrypt->hide();
            ui->Button3->hide();
            ui->Button2->setText("OK");
            setWindowTitle(tr("Decrypt wallet"));
            break;
        case ChangePass: // Ask old passphrase + new passphrase x2
            setWindowTitle(tr("Change passphrase"));
            ui->headerLabel->hide();
            ui->headerEdit->hide();            
            ui->timeLabel->hide();
            ui->timeDisplay->hide();
            ui->checkBoxHeader->hide();
            ui->selectLabel->hide();
            ui->selectList->hide();
            ui->checkBoxEncrypt->hide();
            ui->Button3->hide();
            ui->Button2->setText("OK");
            ui->warningLabel->setText(tr("Enter the old and new passphrase to the wallet."));
            break;
        case CreateNew:
            setWindowTitle(tr("Create New Account"));
            ui->passLabel1->hide();
            ui->passEdit1->hide();
            ui->passLabel2->hide();
            ui->passEdit2->hide();
            ui->passLabel3->hide();
            ui->passEdit3->hide();
            ui->headerLabel->hide();
            ui->headerEdit->hide();            
            ui->timeLabel->hide();
            ui->timeDisplay->hide();
            ui->selectLabel->hide();
            ui->selectList->hide();
            ui->warningLabel->setText(tr("Please fill in the form to create a new account."));
        
        
            break;
        case Switch:
            setWindowTitle(tr("Switch Account"));
            ui->passLabel1->hide();
            ui->passEdit1->hide();
            ui->passLabel2->hide();
            ui->passEdit2->hide();
            ui->passLabel3->hide();
            ui->passEdit3->hide();
            ui->headerLabel->hide();
            ui->headerEdit->hide();            
            ui->timeLabel->hide();
            ui->timeDisplay->hide();
            ui->checkBoxHeader->hide();
            ui->checkBoxEncrypt->hide();
            ui->Button3->hide();
            ui->Button2->setText("OK");
            ui->warningLabel->setText(tr("Please choose an account to switch to."));
            QStringList idList=walletModel->GetAccountList();
            LogPrintf("switchto list size:%i \n",idList.size());
            ui->selectList->addItems(idList);
            break;
    }
    textChanged();
    connect(ui->passEdit1, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));
    connect(ui->passEdit2, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));
    connect(ui->passEdit3, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));
    connect(ui->headerEdit, SIGNAL(textChanged(QString)), this, SLOT(headerTextChanged(QString)));
    connect(ui->checkBoxEncrypt, SIGNAL(clicked()), this, SLOT(textChanged()));
    //connect(ui->checkBoxHeader, SIGNAL(clicked()), this, SLOT(textChanged()));
   
}


AccountDialog::~AccountDialog()
{

    delete ui;
}
void AccountDialog::textChanged()
{
    // Validate input, set Ok button to enabled when acceptable
    bool acceptable = false;
    switch(mode)
    {
    case Encrypt: // New passphrase x2
        acceptable = !ui->passEdit2->text().isEmpty() && !ui->passEdit3->text().isEmpty();
        break;
    case Unlock: // Old passphrase x1        
    case Decrypt:
        acceptable = !ui->passEdit1->text().isEmpty();
        break;
    case ChangePass: // Old passphrase x1, new passphrase x2
        acceptable = !ui->passEdit1->text().isEmpty() && !ui->passEdit2->text().isEmpty() && !ui->passEdit3->text().isEmpty()&&(ui->passEdit2->text()==ui->passEdit3->text());
        break;
    case CreateNew:
            //LogPrintf("checkboxencrypt checkstate:%i \n",ui->checkBoxEncrypt->checkState());
            acceptable=(ui->checkBoxEncrypt->checkState()==Qt::Unchecked)||(!ui->passEdit2->text().isEmpty() && !ui->passEdit3->text().isEmpty()&&(ui->passEdit2->text()==ui->passEdit3->text()));            
        break;
    case Switch:
            acceptable=true;
            break;
    }
    ui->Button2->setEnabled(acceptable);
    ui->Button3->setEnabled(acceptable);
    //ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(acceptable);
}
void AccountDialog::headerTextChanged(QString header)
{
    if(eccSpeed==0)
        return;
    double eccTime=(double)(1<<(header.size()*5))/eccSpeed;
    if(header.size()>6)
        eccTime=(double)(1<<30)/eccSpeed*(1<<((header.size()-6)*5));
    std::string strTime;
    if(eccTime<60)
        strTime=num2str(eccTime)+"s";
    else if (eccTime<3600)
        strTime=num2str(eccTime/60)+"mins";
    else if (eccTime<3600*24)
        strTime=num2str(eccTime/3600)+"hours";
    else if(eccTime<3600*24*356)
        strTime=num2str(eccTime/3600/24)+"days";
    else
        strTime=num2str(eccTime/3600/24/365)+"years";
    ui->timeDisplay->setText(QString().fromStdString(strTime));
}

void AccountDialog::checkBoxHeaderClicked(bool fChecked)
{
    if(fChecked){
        ui->headerLabel->show();
            ui->headerEdit->show();            
            ui->timeLabel->show();
            ui->timeDisplay->show();
            //eccSpeed=walletModel->testEccSpeed();
            //ui->timeDisplay->setText(QString(eccSpeed).append(QString("ecc/s")));
    }else
    {
        ui->headerLabel->hide();
            ui->headerEdit->hide();            
            ui->timeLabel->hide();
            ui->timeDisplay->hide();
    }
}
void AccountDialog::checkBoxEncryptClicked(bool fChecked)
{
    if(fChecked){
        ui->passLabel2->show();
            ui->passEdit2->show();
            ui->passLabel3->show();
            ui->passEdit3->show();
    }else
    {
        ui->passLabel2->hide();
            ui->passEdit2->hide();
            ui->passLabel3->hide();
            ui->passEdit3->hide();
    }
}
void AccountDialog::button1_clicked()
{
    switch(mode)
    {   
        case CreateNew:
            walletModel->stopVanityGen();
    }
    
    QDialog::reject();
}
void AccountDialog::button2_clicked()
{
    DoAccountAction(false);
    
}
void AccountDialog::button3_clicked()
{
 DoAccountAction(true);
}
void AccountDialog::DoAccountAction(bool fSwitchTo)
{
    QString header;
   SecureString oldpass, newpass1, newpass2;
    if(!walletModel)
        return;
    oldpass.reserve(MAX_PASSPHRASE_SIZE);
    newpass1.reserve(MAX_PASSPHRASE_SIZE);
    newpass2.reserve(MAX_PASSPHRASE_SIZE);
    // TODO: get rid of this .c_str() by implementing SecureString::operator=(std::string)
    // Alternately, find a way to make this input mlock()'d to begin with.
    //oldpass.assign(ui->passEdit1->text().toStdString().c_str());
    //newpass1.assign(ui->passEdit2->text().toStdString().c_str());
    //newpass2.assign(ui->passEdit3->text().toStdString().c_str());
     switch(mode)
    {   
        case CreateNew:
            ui->Button2->setEnabled(false);
            ui->Button3->setEnabled(false);
            if(ui->checkBoxHeader->checkState()==Qt::Checked)   
                header=ui->headerEdit->text();    
            if(ui->checkBoxEncrypt->checkState()==Qt::Checked)
                newpass1.assign(ui->passEdit2->text().toStdString().c_str());
            if(walletModel->createNewAccount(header,newpass1,fSwitchTo))
            {
               if(header.size()==0)
                    QDialog::accept();    
               else
               {
                   ui->warningLabel->setText(tr("Calculating NameKey header,please wait..."));
                   
                   //fEcMinerOn=true;
               }
            }
        break;
         case Encrypt:
            newpass1.assign(ui->passEdit2->text().toStdString().c_str());
            walletModel->setWalletEncrypted(true,newpass1);
            QDialog::accept();
            break;    
        case Switch:
            walletModel->switchToAccount(ui->selectList->currentText());
            QDialog::accept(); 
             break;
         case Unlock:
             oldpass.assign(ui->passEdit1->text().toStdString().c_str());
             walletModel->setWalletLocked(false, oldpass);
             QDialog::accept(); 
             break;
         case Lock:
             walletModel->setWalletLocked(true, oldpass);
             QDialog::accept(); 
             break;
         case Decrypt:
             oldpass.assign(ui->passEdit1->text().toStdString().c_str());
             walletModel->setWalletEncrypted(false, oldpass);
             QDialog::accept(); 
             break;
         case ChangePass:
             oldpass.assign(ui->passEdit1->text().toStdString().c_str());
             newpass1.assign(ui->passEdit2->text().toStdString().c_str());
             walletModel->changePassphrase(oldpass,newpass1);
         default:
             break;
    }
}
void AccountDialog::accept()
{
    QDialog::accept();
}