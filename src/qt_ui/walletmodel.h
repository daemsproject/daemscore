// Copyright (c) 2011-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_WALLETMODEL_H
#define BITCOIN_QT_WALLETMODEL_H

//#include "paymentrequestplus.h"
//#include "walletmodeltransaction.h"

#include "allocators.h" /* for SecureString */

#include <map>
#include <vector>

#include <QObject>


class OptionsModel;
class WalletModelTransaction;

//class CCoinControl;
//class CKeyID;
//class COutPoint;
//class COutput;
//class CPubKey;
class CWallet;
class uint256;

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE



/** Interface to Bitcoin wallet from Qt view code. */
class WalletModel : public QObject
{
    Q_OBJECT

public:
    explicit WalletModel(CWallet *wallet, OptionsModel *optionsModel, QObject *parent = 0);
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

    OptionsModel *getOptionsModel();
    //AddressTableModel *getAddressTableModel();
    //TransactionTableModel *getTransactionTableModel();
    //RecentRequestsTableModel *getRecentRequestsTableModel();

    //CAmount getBalance(const CCoinControl *coinControl = NULL) const;
    //CAmount getUnconfirmedBalance() const;
    //CAmount getImmatureBalance() const;
    //bool haveWatchOnly() const;
    //CAmount getWatchBalance() const;
    //CAmount getWatchUnconfirmedBalance() const;
    //CAmount getWatchImmatureBalance() const;
    EncryptionStatus getEncryptionStatus() const;

    // Check address for validity
    //bool validateAddress(const QString &address);

    // Return status record for SendCoins, contains error id + information
    struct SendCoinsReturn
    {
        SendCoinsReturn(StatusCode status = OK):
            status(status) {}
        StatusCode status;
    };

    // prepare transaction for getting txfee before sending coins
    //SendCoinsReturn prepareTransaction(WalletModelTransaction &transaction, const CCoinControl *coinControl = NULL);

    // Send coins to a list of recipients
    //SendCoinsReturn sendCoins(WalletModelTransaction &transaction);

    // Wallet encryption
    bool setWalletEncrypted(bool encrypted, const SecureString &passphrase);
    // Passphrase only needed when unlocking
    bool setWalletLocked(bool locked, const SecureString &passPhrase=SecureString());
    bool changePassphrase(const SecureString &oldPass, const SecureString &newPass);
    // Wallet backup
    bool backupWallet(const QString &filename);

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

    //bool getPubKey(const CKeyID &address, CPubKey& vchPubKeyOut) const;
    //void getOutputs(const std::vector<COutPoint>& vOutpoints, std::vector<COutput>& vOutputs);
    //bool isSpent(const COutPoint& outpoint) const;
    //void listCoins(std::map<QString, std::vector<COutput> >& mapCoins) const;

    //bool isLockedCoin(uint256 hash, unsigned int n,CAmount value) const;
    //void lockCoin(COutPoint& output);
    //void unlockCoin(COutPoint& output);
    //void listLockedCoins(std::vector<COutPoint>& vOutpts);

    //void loadReceiveRequests(std::vector<std::string>& vReceiveRequests);
    //bool saveReceiveRequest(const std::string &sAddress, const int64_t nId, const std::string &sRequest);

private:
    CWallet *wallet;
    //bool fHaveWatchOnly;
    //bool fForceCheckBalanceChanged;

    // Wallet has an options model for wallet-specific options
    // (transaction fee, for example)
    //OptionsModel *optionsModel;

//    AddressTableModel *addressTableModel;
//    TransactionTableModel *transactionTableModel;
//    RecentRequestsTableModel *recentRequestsTableModel;

    // Cache some values to be able to detect changes
    //CAmount cachedBalance;
    //CAmount cachedUnconfirmedBalance;
    //CAmount cachedImmatureBalance;
   // CAmount cachedWatchOnlyBalance;
    //CAmount cachedWatchUnconfBalance;
    //CAmount cachedWatchImmatureBalance;
    EncryptionStatus cachedEncryptionStatus;
    int cachedNumBlocks;

    QTimer *pollTimer;

    void subscribeToCoreSignals();
    void unsubscribeFromCoreSignals();
    //void checkBalanceChanged();

signals:
    // Signal that balance in wallet changed
    //void balanceChanged(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance,
    //                    const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance);

    // Encryption status of wallet changed
    void encryptionStatusChanged(int status);

    // Signal emitted when wallet needs to be unlocked
    // It is valid behaviour for listeners to keep the wallet locked after this signal;
    // this means that the unlocking failed or was cancelled.
    void requireUnlock();

    // Fired when a message should be reported to the user
    void message(const QString &title, const QString &message, unsigned int style);

    // Coins sent: from wallet, to recipient, in (serialized) transaction:
    //void coinsSent(CWallet* wallet, SendCoinsRecipient recipient, QByteArray transaction);

    // Show progress dialog e.g. for rescan
    void showProgress(const QString &title, int nProgress);
//    void showTx();
//    void showOverrideTx();
//    viod showHash();
    // Watch-only address added
    //void notifyWatchonlyChanged(bool fHaveWatchonly);

public slots:
    /* Wallet status might have changed */
    //void updateStatus();
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