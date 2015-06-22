// Copyright (c) 2011-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_WALLETMODEL_H
#define BITCOIN_QT_WALLETMODEL_H


#include "allocators.h" /* for SecureString */

#include <map>
#include <vector>

#include <QObject>
#include "pubkey.h"
#include "key.h"
//#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_utils.h"
//#include "json/json_spirit_writer_template.h"
//class OptionsModel;


//class CCoinControl;

class COutPoint;
class COutput;

//class CKey;
class CWallet;
class uint256;
class BitcoinGUI;
class CWalletTx;
class PaymentRequest;
QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE



/** Interface to Bitcoin wallet from Qt view code. */
class WalletModel : public QObject
{
    Q_OBJECT

public:
    explicit WalletModel(CWallet *walletIn,BitcoinGUI *guiIn,QObject *parent = 0);
    ~WalletModel();

    enum StatusCode // Returned by sendCoins
    {
        OK,
        InvalidAmount,
        InvalidAddress,
        AmountExceedsBalance,
        AmountWithFeeExceedsBalance,
        DuplicateAddress,
        TransactionCreationFailed, // Error returned when wallet is still locked
        TransactionCommitFailed,
        InsaneFee
    };

    enum EncryptionStatus
    {
        Unencrypted,  // !wallet->IsCrypted()
        Locked,       // wallet->IsCrypted() && wallet->IsLocked()
        Unlocked      // wallet->IsCrypted() && !wallet->IsLocked()
    };

    //OptionsModel *getOptionsModel();
    //AddressTableModel *getAddressTableModel();
    //TransactionTableModel *getTransactionTableModel();
    //RecentRequestsTableModel *getRecentRequestsTableModel();

    //CAmount getBalance(const CCoinControl *coinControl = NULL) const;
    //CAmount getUnconfirmedBalance() const;
    //CAmount getImmatureBalance() const;

    
    EncryptionStatus getEncryptionStatus() const;

    // Check address for validity
    

    // Return status record for SendCoins, contains error id + information

    // Wallet encryption
    bool setWalletEncrypted(bool encrypted, const SecureString &passphrase);
    // Passphrase only needed when unlocking
    bool setWalletLocked(bool locked, const SecureString &passPhrase=SecureString());
    bool changePassphrase(const SecureString &oldPass, const SecureString &newPass);
    // Wallet backup
    bool backupWallet(const QString &filename);
    double testEccSpeed();
    bool createNewAccount(const QString header,const SecureString &passPhraseNew,bool fSwitchTo);
    bool stopVanityGen();
    QStringList GetAccountList();
    bool switchToAccount(QString ID);
    QString HandlePaymentRequest(const json_spirit::Array arrData);    
    QString HandlePaymentRequest2(const json_spirit::Array arrData);  
    QString DoPayment(const PaymentRequest& pr);
    QString EncryptMessages(json_spirit::Array params);
    //bool handlePaymentRequest(CWalletTx tx,int nOP,string strError,SecureString& ssInput);  
    QString getPaymentAlertMessage(const CWalletTx& tx);
    QString getEncryptMessegeAlert(const std::vector<std::string>& vstrIDsForeign,const bool fEncrypt);
    QString getDomainRegisterAlertMessage(const CWalletTx& tx,const PaymentRequest& pr);
    QString getDomainUpdateAlertMessage(const CWalletTx& tx,const PaymentRequest& pr);
    QString getDomainTransferAlertMessage(const CWalletTx& tx,const PaymentRequest& pr);
    QString getDomainRenewAlertMessage(const CWalletTx& tx,const PaymentRequest& pr);
    QString SendMessage(json_spirit::Array arrData);
    QString RegisterDomain(json_spirit::Array arrData);
    QString UpdateDomain(json_spirit::Array arrData);
    QString RenewDomain(json_spirit::Array arrData);
    QString TransferDomain(json_spirit::Array arrData);
    QString getSMSAlertMessage(const PaymentRequest& pr);
    // RAI object for unlocking wallet, returned by requestUnlock()
    class UnlockContext
    {
    public:
        UnlockContext(WalletModel *wallet, bool valid, bool relock);
        ~UnlockContext();

        bool isValid() const { return valid; }

        // Copy operator and constructor transfer the context
        UnlockContext(const UnlockContext& obj) { CopyFrom(obj); }
        UnlockContext& operator=(const UnlockContext& rhs) { CopyFrom(rhs); return *this; }
    private:
        WalletModel *wallet;
        bool valid;
        mutable bool relock; // mutable, as it can be set to false by copying

        void CopyFrom(const UnlockContext& rhs);
    };

    UnlockContext requestUnlock();
    

private:
    BitcoinGUI *gui;
    CWallet *wallet;
    CWallet *wallet2;
    bool fSwitchToAccount;
    std::string strHeaderTarget;
    SecureString ssPassPhrase;

    // Wallet has an options model for wallet-specific options
    // (transaction fee, for example)
    //OptionsModel *optionsModel;
    EncryptionStatus cachedEncryptionStatus;
    int cachedNumBlocks;

    QTimer *pollTimer;

    void subscribeToCoreSignals();
    void unsubscribeFromCoreSignals();
    //void checkBalanceChanged();

signals:

    

    // Encryption status of wallet changed
    void encryptionStatusChanged(int status);

    // Signal emitted when wallet needs to be unlocked
    // It is valid behaviour for listeners to keep the wallet locked after this signal;
    // this means that the unlocking failed or was cancelled.
    void requireUnlock();
    void accountSwitched(const std::string id);
    // Fired when a message should be reported to the user
    void message(const QString &title, const QString &message, unsigned int style);

    // Coins sent: from wallet, to recipient, in (serialized) transaction:
    //void coinsSent(CWallet* wallet, SendCoinsRecipient recipient, QByteArray transaction);

    // Show progress dialog e.g. for rescan
    //void showProgress(const QString &title, int nProgress);
//    void showTx();
//    void showOverrideTx();
//    viod showHash();
    

    void vanitygenSuccess();
private slots:
    void notifyEcMinerResult(const CPubKey basePub,const CKey stepKey,const std::string strHeader);
    void notifyAccountSwitched(const std::string id);
public slots:
    /* Wallet status might have changed */
    void updateStatus();
    /* New transaction, or transaction changed status */
    //void updateTransaction();
    /* New, updated or removed address book entry */
    //void updateAddressBook(const QString &address, const QString &label, bool isMine, const QString &purpose, int status);
    /* Watch-only added */
    //void updateWatchOnlyFlag(bool fHaveWatchonly);
    /* Current, immature or unconfirmed balance might have changed - emit 'balanceChanged' if so */
    //void pollBalanceChanged();
    
};

#endif // BITCOIN_QT_WALLETMODEL_H
