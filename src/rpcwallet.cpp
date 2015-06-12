// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "amount.h"
#include "base58.h"
#include "core_io.h"
#include "rpcserver.h"
#include "init.h"
#include "net.h"
#include "netbase.h"
#include "timedata.h"
#include "util.h"
#include "utilmoneystr.h"
#include "wallet.h"
#include "walletdb.h"
#include "utilstrencodings.h"
#include <stdint.h>

#include <boost/assign/list_of.hpp>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"

using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace json_spirit;

int64_t nWalletUnlockTime;
static CCriticalSection cs_nWalletUnlockTime;

std::string HelpRequiringPassphrase()
{
    return pwalletMain && pwalletMain->IsCrypted()
        ? "\nRequires wallet passphrase to be set with walletpassphrase call."
        : "";
}

void EnsureWalletIsUnlocked()
{
    if (pwalletMain->IsLocked())
        throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Error: Please enter the wallet passphrase with walletpassphrase first.");
}

void WalletTxToJSON(const CWalletTx& wtx, Object& entry)
{
    int confirms = wtx.GetDepthInMainChain();
    entry.push_back(Pair("confirmations", confirms));
    if (wtx.IsCoinBase())
        entry.push_back(Pair("generated", true));
    if (confirms > 0)
    {
        entry.push_back(Pair("blockhash", wtx.hashBlock.GetHex()));
        entry.push_back(Pair("blockindex", wtx.nIndex));
        entry.push_back(Pair("blockHeight", wtx.nBlockHeight));
        entry.push_back(Pair("blocktime", mapBlockIndex[wtx.hashBlock]->GetBlockTime()));
    }
    uint256 hash = wtx.GetHash();
    entry.push_back(Pair("txid", hash.GetHex()));
    Array conflicts;
    BOOST_FOREACH(const uint256& conflict, wtx.GetConflicts())
        conflicts.push_back(conflict.GetHex());
    entry.push_back(Pair("walletconflicts", conflicts));
    entry.push_back(Pair("time", wtx.GetTxTime()));
    entry.push_back(Pair("timereceived", (int64_t)wtx.nTimeReceived));
     entry.push_back(Pair("locktime", (uint64_t)wtx.nLockTime));
    BOOST_FOREACH(const PAIRTYPE(string,string)& item, wtx.mapValue)
        entry.push_back(Pair(item.first, item.second));
}

CPubKey AccountFromValue(const Value& value)
{
    string strAccount = value.get_str();
    if (strAccount == "*")
        throw JSONRPCError(RPC_WALLET_INVALID_ACCOUNT_NAME, "Invalid account id");
    CBitcoinAddress add;
    if (!add.SetString(strAccount))
        throw JSONRPCError(RPC_WALLET_INVALID_ACCOUNT_NAME, "Invalid account id");   
    CPubKey pub;
    if(!add.GetKey(pub))
        throw JSONRPCError(RPC_WALLET_INVALID_ACCOUNT_NAME, "Invalid account id");   
    return pub;
}

Value getnewid(const Array& params, bool fHelp)//original getnewaddress
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
            "getnewid ( \"mainid\" )\n"
            "\nReturns a new Cccoin id derived from the main id for receiving payments.\n"
            "If 'mainid' is specified , it is derived from that main id ,if not,it is derived from the default account id\n"
            
            "\nArguments:\n"
            "1. \"mainid\"        (string, optional) The account main id for the new id to be linked to. if not provided, the current main id \"\" is used. It can also be set to the empty string \"\" to represent the default. \n"
            "\nResult:\n"
            "\"cccoinid\"    (string) The new cccoin id\n"
            "\nExamples:\n"
            + HelpExampleCli("getnewid", "")
            + HelpExampleCli("getnewid", "\"\"")
            + HelpExampleCli("getnewid", "\"mainid\"")
            + HelpExampleRpc("getnewid", "\"mainid\"")
        );
    //TODO if main id is provided, derive from the main id    
    CPubKey keyID; 
    if (params.size()==1){
        CPubKey id = AccountFromValue(params[0]);
        CWallet* pwallet=new CWallet(id);
        if (!pwallet->CanExtendKeys())
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,"id file is not enough for extend keys");
        keyID=pwallet->GenerateNewKey();
    }else{
    // Generate a new key that is added to wallet
        if (!pwalletMain->CanExtendKeys())
             throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,"id file is not enough for extend keys");
        keyID=pwalletMain->GenerateNewKey();
    //pwalletMain->SetAddressBook(keyID, "", "receive");
    }
    Value ret;
    ret=CBitcoinAddress(keyID).ToString();
    return ret;
}


CBitcoinAddress GetAccountAddresses(CPubKey id, bool bForceNew=false)
{
//    CWalletDB walletdb(pwalletMain->strWalletFile);
//
//    CAccount account;
//    walletdb.ReadAccount(strAccount, account);
//
//    bool bKeyUsed = false;
//
//    // Check if the current key has been used
//    if (account.vchPubKey.IsValid())
//    {
//        CScript scriptPubKey = GetScriptForDestination(account.vchPubKey.GetID());
//        for (map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin();
//             it != pwalletMain->mapWallet.end() && account.vchPubKey.IsValid();
//             ++it)
//        {
//            const CWalletTx& wtx = (*it).second;
//            BOOST_FOREACH(const CTxOut& txout, wtx.vout)
//                if (txout.scriptPubKey == scriptPubKey)
//                    bKeyUsed = true;
//        }
//    }
//
//    // Generate a new key
//    if (!account.vchPubKey.IsValid() || bForceNew || bKeyUsed)
//    {
//        if (!pwalletMain->GetKeyFromPool(account.vchPubKey))
//            throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
//
//        pwalletMain->SetAddressBook(account.vchPubKey.GetID(), strAccount, "receive");
//        walletdb.WriteAccount(strAccount, account);
//    }

    //return CBitcoinAddress(account.vchPubKey.GetID());
    return NULL;
}

Value getmainid(const Array& params, bool fHelp)
{
    if (fHelp || params.size() >0)
        throw runtime_error(
            "getmainid\n"
            "\nReturns the current Cccoin main id of this account.\n"
            
            "\nResult:\n"
            "\"cccoinid\"   (string) The Cccoin id in base32 format\n"
            "\nExamples:\n"
            + HelpExampleCli("getmainid", "")            
            + HelpExampleRpc("getmainid", "")
        );

    // Parse the account first so we don't generate a key if there's an error
    //CPubKey id = pwalletMain->id;

    Value ret;

    ret = pwalletMain->GetAddress();

    return ret;
}


Value getrawchangeaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 1)
        throw runtime_error(
            "getrawchangeaddress\n"
            "\nReturns a new Cccoin address, for receiving change.\n"
            "This is for use with raw transactions, NOT normal use.\n"
            "\nResult:\n"
            "\"address\"    (string) The address\n"
            "\nExamples:\n"
            + HelpExampleCli("getrawchangeaddress", "")
            + HelpExampleRpc("getrawchangeaddress", "")
       );

    //if (!pwalletMain->IsLocked())
    //    pwalletMain->TopUpKeyPool();

    //CReserveKey reservekey(pwalletMain);
    //CPubKey vchPubKey;
    //if (!reservekey.GetReservedKey(vchPubKey))
    //    throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");

    //reservekey.KeepKey();

    CPubKey keyID=pwalletMain->GenerateNewKey();
    return CBitcoinAddress(keyID).ToString();
}


Value setaccount(const Array& params, bool fHelp)
{
//    if (fHelp || params.size() < 1 || params.size() > 2)
//        throw runtime_error(
//            "setaccount \"cccoinaddress\" \"account\"\n"
//            "\nSets the account associated with the given address.\n"
//            "\nArguments:\n"
//            "1. \"cccoinaddress\"  (string, required) The cccoin address to be associated with an account.\n"
//            "2. \"account\"         (string, required) The account to assign the address to.\n"
//            "\nExamples:\n"
//            + HelpExampleCli("setaccount", "\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\" \"tabby\"")
//            + HelpExampleRpc("setaccount", "\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\", \"tabby\"")
//        );
//
//    CBitcoinAddress address(params[0].get_str());
//    if (!address.IsValid())
//        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Cccoin address");
//
//
//    string strAccount;
//    if (params.size() > 1)
//        strAccount = AccountFromValue(params[1]);
//
//    // Only add the account if the address is yours.
//    if (IsMine(*pwalletMain, address.Get()))
//    {
//        // Detect when changing the account of an address that is the 'unused current key' of another account:
//        if (pwalletMain->mapAddressBook.count(address.Get()))
//        {
//            string strOldAccount = pwalletMain->mapAddressBook[address.Get()].name;
//            if (address == GetAccountAddress(strOldAccount))
//                GetAccountAddress(strOldAccount, true);
//        }
//        pwalletMain->SetAddressBook(address.Get(), strAccount, "receive");
//    }
//    else
//        throw JSONRPCError(RPC_MISC_ERROR, "setaccount can only be used with own address");

    return Value::null;
}


Value getaccount(const Array& params, bool fHelp)
{
//    if (fHelp || params.size() != 1)
//        throw runtime_error(
//            "getaccount \"cccoinaddress\"\n"
//            "\nReturns the account associated with the given address.\n"
//            "\nArguments:\n"
//            "1. \"cccoinaddress\"  (string, required) The cccoin address for account lookup.\n"
//            "\nResult:\n"
//            "\"accountname\"        (string) the account address\n"
//            "\nExamples:\n"
//            + HelpExampleCli("getaccount", "\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\"")
//            + HelpExampleRpc("getaccount", "\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\"")
//        );
//
//    CBitcoinAddress address(params[0].get_str());
//    if (!address.IsValid())
//        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Cccoin address");
//
//    string strAccount;
//    map<CTxDestination, CAddressBookData>::iterator mi = pwalletMain->mapAddressBook.find(address.Get());
//    if (mi != pwalletMain->mapAddressBook.end() && !(*mi).second.name.empty())
//        strAccount = (*mi).second.name;
//    return strAccount;
    return Value::null;
}


Value getidlist(const Array& params, bool fHelp)
{
    if (fHelp || params.size() >1)
        throw runtime_error(
            "getidlist \n"
            "\nReturns the list of ids for the current account.\n"
            "\nArguments:\n"
            "1. \"cccoinid\"  (string, optional) The cccoin main id to get id list.\n"
            "\nResult:\n"
            "[                     (json array of string)\n"
            "  \"cccoinid\"  (string) a cccoin id associated with the current account\n"
            "  ,...\n"
            "]\n"
            "\nExamples:\n"
            + HelpExampleCli("getidlist","")
            + HelpExampleCli("getidlist","mainid")
            + HelpExampleRpc("getidlist", "mainid")
        );
    Array ids;    
    //TODO if main id is provided, get id list from that id
    if (params.size()==1){
        CPubKey id = AccountFromValue(params[0]);        
        LogPrintf("getidlist:mainid:%s\n",params[0].get_str());
        CWallet* pwallet=new CWallet(id);
        //if (!pwallet->pwalletdb->IsWalletExists(id))
        //    throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY,"id does not exist");
        //pwallet->SwitchToAccount(id);
        LogPrintf("getidlist:ids:%u\n",pwallet->mapKeys.size());
        BOOST_FOREACH(const PAIRTYPE(CPubKey, uint64_t)& item, pwallet->mapKeys)
        {
            CBitcoinAddress address;
            address.Set(item.first);
            ids.push_back(address.ToString());
        }   
    }
    
    // Find all addresses that have the given account
    
    else
    {
        LogPrintf("getidlist:no parmas\n");
        BOOST_FOREACH(const PAIRTYPE(CPubKey,uint64_t)& item, pwalletMain->mapKeys)
        {
            CBitcoinAddress address;
            address.Set(item.first);
            ids.push_back(address.ToString());
        }
    }
    Object ret;
    ret.push_back(Pair("ids",ids));
    return ret;

}

void SendMoney(const CTxDestination &address, CAmount nValue, CWalletTx& wtxNew)
{
    // Check amount
    if (nValue <= 0)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid amount");

    if (nValue > pwalletMain->GetBalance())
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Insufficient funds");

    string strError;
    if (pwalletMain->IsLocked())
    {
        strError = "Error: Wallet locked, unable to create transaction!";
        LogPrintf("SendMoney() : %s", strError);
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }

    // Parse Bitcoin address
    CScript scriptPubKey = GetScriptForDestination(address);

    // Create and send the transaction
    CReserveKey reservekey(pwalletMain);
    CAmount nFeeRequired;
    if (!pwalletMain->CreateTransaction(scriptPubKey, nValue, wtxNew, reservekey, nFeeRequired, strError))
    {
        if (nValue + nFeeRequired > pwalletMain->GetBalance())
            strError = strprintf("Error: This transaction requires a transaction fee of at least %s because of its amount, complexity, or use of recently received funds!", FormatMoney(nFeeRequired));
        LogPrintf("SendMoney() : %s\n", strError);
        throw JSONRPCError(RPC_WALLET_ERROR, strError);
    }
    if (!pwalletMain->CommitTransaction(wtxNew, reservekey))
        throw JSONRPCError(RPC_WALLET_ERROR, "Error: The transaction was rejected! This might happen if some of the coins in your wallet were already spent, such as if you used a copy of wallet.dat and coins were spent in the copy but not marked as spent here.");
}

Value sendtoaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 4)
        throw runtime_error(
            "sendtoaddress \"cccoinaddress\" amount ( \"comment\" \"comment-to\" )\n"
            "\nSend an amount to a given address. The amount is a real and is rounded to the nearest 0.00000001\n"
            + HelpRequiringPassphrase() +
            "\nArguments:\n"
            "1. \"cccoinaddress\"  (string, required) The cccoin address to send to.\n"
            "2. \"amount\"      (numeric, required) The amount in ltc to send. eg 0.1\n"
            "3. \"comment\"     (string, optional) A comment used to store what the transaction is for. \n"
            "                             This is not part of the transaction, just kept in your wallet.\n"
            "4. \"comment-to\"  (string, optional) A comment to store the name of the person or organization \n"
            "                             to which you're sending the transaction. This is not part of the \n"
            "                             transaction, just kept in your wallet.\n"
            "\nResult:\n"
            "\"transactionid\"  (string) The transaction id.\n"
            "\nExamples:\n"
            + HelpExampleCli("sendtoaddress", "\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\" 0.1")
            + HelpExampleCli("sendtoaddress", "\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\" 0.1 \"donation\" \"seans outpost\"")
            + HelpExampleRpc("sendtoaddress", "\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\", 0.1, \"donation\", \"seans outpost\"")
        );

    CBitcoinAddress address(params[0].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Cccoin address");

    // Amount
    CAmount nAmount = AmountFromValue(params[1]);

    // Wallet comments
    CWalletTx wtx;
    if (params.size() > 2 && params[2].type() != null_type && !params[2].get_str().empty())
        wtx.mapValue["comment"] = params[2].get_str();
    if (params.size() > 3 && params[3].type() != null_type && !params[3].get_str().empty())
        wtx.mapValue["to"]      = params[3].get_str();

    EnsureWalletIsUnlocked();

    SendMoney(address.Get(), nAmount, wtx);

    return wtx.GetHash().GetHex();
}

Value listaddressgroupings(const Array& params, bool fHelp)
{
    if (fHelp)
        throw runtime_error(
            "listaddressgroupings\n"
            "\nLists groups of addresses which have had their common ownership\n"
            "made public by common use as inputs or as the resulting change\n"
            "in past transactions\n"
            "\nResult:\n"
            "[\n"
            "  [\n"
            "    [\n"
            "      \"cccoinaddress\",     (string) The cccoin address\n"
            "      amount,                 (numeric) The amount in ltc\n"
            "      \"account\"             (string, optional) The account\n"
            "    ]\n"
            "    ,...\n"
            "  ]\n"
            "  ,...\n"
            "]\n"
            "\nExamples:\n"
            + HelpExampleCli("listaddressgroupings", "")
            + HelpExampleRpc("listaddressgroupings", "")
        );

    Array jsonGroupings;
    map<CTxDestination, CAmount> balances = pwalletMain->GetAddressBalances();
    BOOST_FOREACH(set<CTxDestination> grouping, pwalletMain->GetAddressGroupings())
    {
        Array jsonGrouping;
        BOOST_FOREACH(CTxDestination address, grouping)
        {
            Array addressInfo;
            addressInfo.push_back(CBitcoinAddress(address).ToString());
            addressInfo.push_back(ValueFromAmount(balances[address]));
            {
                LOCK(pwalletMain->cs_wallet);
                if (pwalletMain->mapAddressBook.find(CBitcoinAddress(address).Get()) != pwalletMain->mapAddressBook.end())
                    addressInfo.push_back(pwalletMain->mapAddressBook.find(CBitcoinAddress(address).Get())->second.name);
            }
            jsonGrouping.push_back(addressInfo);
        }
        jsonGroupings.push_back(jsonGrouping);
    }
    return jsonGroupings;
}

Value signmessage(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 2)
        throw runtime_error(
            "signmessage \"cccoinaddress\" \"message\"\n"
            "\nSign a message with the private key of an address"
            + HelpRequiringPassphrase() + "\n"
            "\nArguments:\n"
            "1. \"cccoinaddress\"  (string, required) The cccoin address to use for the private key.\n"
            "2. \"message\"         (string, required) The message to create a signature of.\n"
            "\nResult:\n"
            "\"signature\"          (string) The signature of the message encoded in base 64\n"
            "\nExamples:\n"
            "\nUnlock the wallet for 30 seconds\n"
            + HelpExampleCli("walletpassphrase", "\"mypassphrase\" 30") +
            "\nCreate the signature\n"
            + HelpExampleCli("signmessage", "\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\" \"my message\"") +
            "\nVerify the signature\n"
            + HelpExampleCli("verifymessage", "\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\" \"signature\" \"my message\"") +
            "\nAs json rpc\n"
            + HelpExampleRpc("signmessage", "\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\", \"my message\"")
        );

    EnsureWalletIsUnlocked();

    string strAddress = params[0].get_str();
    string strMessage = params[1].get_str();

    CBitcoinAddress addr(strAddress);
    if (!addr.IsValid())
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");

    CPubKey keyID;
    if(!addr.GetKey(keyID))        
        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid address");
    CKey key;
    if (!pwalletMain->GetKey(keyID, key))
        throw JSONRPCError(RPC_WALLET_ERROR, "Private key not available");

    CHashWriter ss(SER_GETHASH, 0);
    ss << strMessageMagic;
    ss << strMessage;

    vector<unsigned char> vchSig;
    if (!key.SignCompact(ss.GetHash(), vchSig))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Sign failed");

    return EncodeBase64(&vchSig[0], vchSig.size());
}

Value getreceivedbyaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "getreceivedbyaddress \"cccoinaddress\" ( minconf )\n"
            "\nReturns the total amount received by the given cccoinaddress in transactions with at least minconf confirmations.\n"
            "\nArguments:\n"
            "1. \"cccoinaddress\"  (string, required) The cccoin address for transactions.\n"
            "2. minconf             (numeric, optional, default=1) Only include transactions confirmed at least this many times.\n"
            "\nResult:\n"
            "amount   (numeric) The total amount in ltc received at this address.\n"
            "\nExamples:\n"
            "\nThe amount from transactions with at least 1 confirmation\n"
            + HelpExampleCli("getreceivedbyaddress", "\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\"") +
            "\nThe amount including unconfirmed transactions, zero confirmations\n"
            + HelpExampleCli("getreceivedbyaddress", "\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\" 0") +
            "\nThe amount with at least 6 confirmation, very safe\n"
            + HelpExampleCli("getreceivedbyaddress", "\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\" 6") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("getreceivedbyaddress", "\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\", 6")
       );

    // Bitcoin address
    CBitcoinAddress address = CBitcoinAddress(params[0].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Cccoin address");
    CScript scriptPubKey = GetScriptForDestination(address.Get());
//    if (!IsMine(*pwalletMain,scriptPubKey))
//        return (double)0.0;

    // Minimum confirmations
    int nMinDepth = 1;
    if (params.size() > 1)
        nMinDepth = params[1].get_int();

    // Tally
    CAmount nAmount = 0;
    std::vector<CPubKey> vIds;
    CPubKey id;
    if(!address.GetKey(id))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Cccoin address");
    vIds.push_back(id);
    map<uint256, CWalletTx> mapTxs=pwalletMain->GetWalletTxs(vIds);
    for (map<uint256, CWalletTx>::iterator it = mapTxs.begin(); it != mapTxs.end(); ++it)
    {
        const CWalletTx& wtx = (*it).second;
        if (wtx.IsCoinBase() || !IsFinalTx(wtx))
            continue;

        BOOST_FOREACH(const CTxOut& txout, wtx.vout)
            if (txout.scriptPubKey == scriptPubKey)
                if (wtx.GetDepthInMainChain() >= nMinDepth)
                    nAmount += txout.nValue;
    }

    return  ValueFromAmount(nAmount);
}

/**browser: the definition account is a series of ids from a basekey*/
Value getreceivedbyaccount(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "getreceivedbyaccount \"account\" ( minconf )\n"
            "\nReturns the total amount received by addresses with <account> in transactions with at least [minconf] confirmations.\n"
            "\nArguments:\n"
            "1. \"account\"      (string, required) The selected account, may be the default account using \"\".\n"
            "2. minconf          (numeric, optional, default=1) Only include transactions confirmed at least this many times.\n"
            "\nResult:\n"
            "amount              (numeric) The total amount in ltc received for this account.\n"
            "\nExamples:\n"
            "\nAmount received by the default account with at least 1 confirmation\n"
            + HelpExampleCli("getreceivedbyaccount", "\"\"") +
            "\nAmount received at the tabby account including unconfirmed amounts with zero confirmations\n"
            + HelpExampleCli("getreceivedbyaccount", "\"tabby\" 0") +
            "\nThe amount with at least 6 confirmation, very safe\n"
            + HelpExampleCli("getreceivedbyaccount", "\"tabby\" 6") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("getreceivedbyaccount", "\"tabby\", 6")
        );

    // Minimum confirmations
    int nMinDepth = 1;
    if (params.size() > 1)
        nMinDepth = params[1].get_int();

    // Get the set of pub keys assigned to account
    CPubKey id = AccountFromValue(params[0]);
    CWallet wallet(id);
    wallet.LoadTxs();
    // Tally
    CAmount nAmount = 0;
    for (map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); ++it)
    {
        const CWalletTx& wtx = (*it).second;
        if (wtx.IsCoinBase() || !IsFinalTx(wtx))
            continue;
        BOOST_FOREACH(const CTxOut& txout, wtx.vout)
        {
            CTxDestination address;
            if (ExtractDestination(txout.scriptPubKey, address) && IsMine(*pwalletMain, address))
                if (wtx.GetDepthInMainChain() >= nMinDepth)
                    nAmount += txout.nValue;
        }
    }

    return (double)nAmount / (double)COIN;
}


//CAmount GetAccountBalance(CWalletDB& walletdb, const string& strAccount, int nMinDepth, const isminefilter& filter)
//{
//    CAmount nBalance = 0;

//    // Tally wallet transactions
//    for (map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); ++it)
//    {
//        const CWalletTx& wtx = (*it).second;
//        if (!IsFinalTx(wtx) || wtx.GetBlocksToMaturity() > 0 || wtx.GetDepthInMainChain() < 0)
//            continue;
//
//        CAmount nReceived, nSent, nFee;
//        wtx.GetAccountAmounts(strAccount, nReceived, nSent, nFee, filter);
//
//        if (nReceived != 0 && wtx.GetDepthInMainChain() >= nMinDepth)
//            nBalance += nReceived;
//        nBalance -= nSent + nFee;
//    }
//
//    // Tally internal accounting entries
//    nBalance += walletdb.GetAccountCreditDebit(strAccount);

//    return nBalance;
//}

CAmount GetAccountBalance(const CPubKey& id, int nMinDepth, const isminefilter& filter)
{
    //CBitcoinAddress add;
    //if (!add.SetString(strAccount))
    //    throw JSONRPCError(RPC_WALLET_INVALID_ACCOUNT_NAME, "Invalid account id");    
    //CPubKey id=add.GetKeyID();
    CWallet wallet(id);
    wallet.LoadTxs();
    return wallet.GetBalance();
}


Value getbalance(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 3)
        throw runtime_error(
            "getbalance ( \"account\" minconf includeWatchonly )\n"
            "\nIf account is not specified, returns the server's total available balance.\n"
            "If account is specified, returns the balance in the account.\n"
            "Note that the account \"\" is not the same as leaving the parameter out.\n"
            "The server total may be different to the balance in the default \"\" account.\n"
            "\nArguments:\n"
            "1. \"account\"      (string, optional) The selected account, or \"*\" for entire wallet. It may be the default account using \"\".\n"
            "2. minconf          (numeric, optional, default=1) Only include transactions confirmed at least this many times.\n"
            "3. includeWatchonly (bool, optional, default=false) Also include balance in watchonly addresses (see 'importaddress')\n"
            "\nResult:\n"
            "amount              (numeric) The total amount in ltc received for this account.\n"
            "\nExamples:\n"
            "\nThe total amount in the server across all accounts\n"
            + HelpExampleCli("getbalance", "") +
            "\nThe total amount in the server across all accounts, with at least 5 confirmations\n"
            + HelpExampleCli("getbalance", "\"*\" 6") +
            "\nThe total amount in the default account with at least 1 confirmation\n"
            + HelpExampleCli("getbalance", "\"\"") +
            "\nThe total amount in the account named tabby with at least 6 confirmations\n"
            + HelpExampleCli("getbalance", "\"tabby\" 6") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("getbalance", "\"tabby\", 6")
        );

    if (params.size() == 0)
        return  ValueFromAmount(pwalletMain->GetBalance());

    int nMinDepth = 1;
    if (params.size() > 1)
        nMinDepth = params[1].get_int();
    isminefilter filter = ISMINE_SPENDABLE;
    if(params.size() > 2)
        if(params[2].get_bool())
            filter = filter | ISMINE_WATCH_ONLY;

    if (params[0].get_str() == "*") {
        // Calculate total balance a different way from GetBalance()
        // (GetBalance() sums up all unspent TxOuts)
        // getbalance and getbalance '*' 0 should return the same number
        pwalletMain->LoadTxs();
        CAmount nBalance = 0;
        for (map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); ++it)
        {
            const CWalletTx& wtx = (*it).second;
            if (!wtx.IsTrusted() || wtx.GetBlocksToMaturity() > 0)
                continue;

            CAmount allFee;
            string strSentAccount;
            list<COutputEntry> listReceived;
            list<COutputEntry> listSent;
            wtx.GetAmounts(listReceived, listSent, allFee, strSentAccount, filter);
            if (wtx.GetDepthInMainChain() >= nMinDepth)
            {
                BOOST_FOREACH(const COutputEntry& r, listReceived)
                    nBalance += r.amount;
            }
            BOOST_FOREACH(const COutputEntry& s, listSent)
                nBalance -= s.amount;
            nBalance -= allFee;
        }
        return  ValueFromAmount(nBalance);
    }

    CPubKey id = AccountFromValue(params[0]);

    CAmount nBalance = GetAccountBalance(id, nMinDepth, filter);

    return ValueFromAmount(nBalance);
}

Value getunconfirmedbalance(const Array &params, bool fHelp)
{
    if (fHelp || params.size() > 0)
        throw runtime_error(
                "getunconfirmedbalance\n"
                "Returns the server's total unconfirmed balance\n");
    return ValueFromAmount(pwalletMain->GetUnconfirmedBalance());
}


Value movecmd(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 3 || params.size() > 5)
        throw runtime_error(
            "move \"fromaccount\" \"toaccount\" amount ( minconf \"comment\" )\n"
            "\nMove a specified amount from one account in your wallet to another.\n"
            "\nArguments:\n"
            "1. \"fromaccount\"   (string, required) The name of the account to move funds from. May be the default account using \"\".\n"
            "2. \"toaccount\"     (string, required) The name of the account to move funds to. May be the default account using \"\".\n"
            "3. minconf           (numeric, optional, default=1) Only use funds with at least this many confirmations.\n"
            "4. \"comment\"       (string, optional) An optional comment, stored in the wallet only.\n"
            "\nResult:\n"
            "true|false           (boolean) true if successfull.\n"
            "\nExamples:\n"
            "\nMove 0.01 ltc from the default account to the account named tabby\n"
            + HelpExampleCli("move", "\"\" \"tabby\" 0.01") +
            "\nMove 0.01 ltc timotei to akiko with a comment and funds have 6 confirmations\n"
            + HelpExampleCli("move", "\"timotei\" \"akiko\" 0.01 6 \"happy birthday!\"") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("move", "\"timotei\", \"akiko\", 0.01, 6, \"happy birthday!\"")
        );

//    string strFrom = AccountFromValue(params[0]);
//    string strTo = AccountFromValue(params[1]);
//    CAmount nAmount = AmountFromValue(params[2]);
//    if (params.size() > 3)
//        // unused parameter, used to be nMinDepth, keep type-checking it though
//        (void)params[3].get_int();
//    string strComment;
//    if (params.size() > 4)
//        strComment = params[4].get_str();
//
//    CWalletDB walletdb();
//    //if (!walletdb.TxnBegin())
//    //    throw JSONRPCError(RPC_DATABASE_ERROR, "database error");
//
//    int64_t nNow = GetAdjustedTime();
//
//    // Debit
//    CAccountingEntry debit;
//    debit.nOrderPos = pwalletMain->IncOrderPosNext(&walletdb);
//    debit.strAccount = strFrom;
//    debit.nCreditDebit = -nAmount;
//    debit.nTime = nNow;
//    debit.strOtherAccount = strTo;
//    debit.strComment = strComment;
//    walletdb.WriteAccountingEntry(debit);
//
//    // Credit
//    CAccountingEntry credit;
//    credit.nOrderPos = pwalletMain->IncOrderPosNext(&walletdb);
//    credit.strAccount = strTo;
//    credit.nCreditDebit = nAmount;
//    credit.nTime = nNow;
//    credit.strOtherAccount = strFrom;
//    credit.strComment = strComment;
//    walletdb.WriteAccountingEntry(credit);

    //if (!walletdb.TxnCommit())
    //    throw JSONRPCError(RPC_DATABASE_ERROR, "database error");

    return false;
}


Value sendfrom(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 3 || params.size() > 6)
        throw runtime_error(
            "sendfrom \"fromaccount\" \"tocccoinaddress\" amount ( minconf \"comment\" \"comment-to\" )\n"
            "\nSent an amount from an account to a cccoin address.\n"
            "The amount is a real and is rounded to the nearest 0.00000001."
            + HelpRequiringPassphrase() + "\n"
            "\nArguments:\n"
            "1. \"fromaccount\"       (string, required) The name of the account to send funds from. May be the default account using \"\".\n"
            "2. \"tocccoinaddress\"  (string, required) The cccoin address to send funds to.\n"
            "3. amount                (numeric, required) The amount in ltc. (transaction fee is added on top).\n"
            "4. minconf               (numeric, optional, default=1) Only use funds with at least this many confirmations.\n"
            "5. \"comment\"           (string, optional) A comment used to store what the transaction is for. \n"
            "                                     This is not part of the transaction, just kept in your wallet.\n"
            "6. \"comment-to\"        (string, optional) An optional comment to store the name of the person or organization \n"
            "                                     to which you're sending the transaction. This is not part of the transaction, \n"
            "                                     it is just kept in your wallet.\n"
            "\nResult:\n"
            "\"transactionid\"        (string) The transaction id.\n"
            "\nExamples:\n"
            "\nSend 0.01 ltc from the default account to the address, must have at least 1 confirmation\n"
            + HelpExampleCli("sendfrom", "\"\" \"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\" 0.01") +
            "\nSend 0.01 from the tabby account to the given address, funds must have at least 6 confirmations\n"
            + HelpExampleCli("sendfrom", "\"tabby\" \"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\" 0.01 6 \"donation\" \"seans outpost\"") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("sendfrom", "\"tabby\", \"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\", 0.01, 6, \"donation\", \"seans outpost\"")
        );

    CPubKey id = AccountFromValue(params[0]);
    CBitcoinAddress address(params[1].get_str());
    if (!address.IsValid())
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid Cccoin address");
    CAmount nAmount = AmountFromValue(params[2]);
    int nMinDepth = 1;
    if (params.size() > 3)
        nMinDepth = params[3].get_int();

    CWalletTx wtx;
    CBitcoinAddress address0;
    address0.Set(id);
    wtx.strFromAccount = address0.ToString();
    if (params.size() > 4 && params[4].type() != null_type && !params[4].get_str().empty())
        wtx.mapValue["comment"] = params[4].get_str();
    if (params.size() > 5 && params[5].type() != null_type && !params[5].get_str().empty())
        wtx.mapValue["to"]      = params[5].get_str();

    EnsureWalletIsUnlocked();

    // Check funds
    CAmount nBalance = GetAccountBalance(id, nMinDepth, ISMINE_SPENDABLE);
    if (nAmount > nBalance)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Account has insufficient funds");

    SendMoney(address.Get(), nAmount, wtx);

    return wtx.GetHash().GetHex();
}


Value sendmany(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 4)
        throw runtime_error(
            "sendmany \"fromaccount\" {\"address\":amount,...} ( minconf \"comment\" )\n"
            "\nSend multiple times. Amounts are double-precision floating point numbers."
            + HelpRequiringPassphrase() + "\n"
            "\nArguments:\n"
            "1. \"fromaccount\"         (string, required) The account to send the funds from, can be \"\" for the default account\n"
            "2. \"amounts\"             (string, required) A json object with addresses and amounts\n"
            "    {\n"
            "      \"address\":amount   (numeric) The cccoin address is the key, the numeric amount in ltc is the value\n"
            "      ,...\n"
            "    }\n"
            "3. minconf                 (numeric, optional, default=1) Only use the balance confirmed at least this many times.\n"
            "4. \"comment\"             (string, optional) A comment\n"
            "\nResult:\n"
            "\"transactionid\"          (string) The transaction id for the send. Only 1 transaction is created regardless of \n"
            "                                    the number of addresses.\n"
            "\nExamples:\n"
            "\nSend two amounts to two different addresses:\n"
            + HelpExampleCli("sendmany", "\"tabby\" \"{\\\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\\\":0.01,\\\"LbhhnRHHVfP1eUJp1tDNiyeeVsNhFN9Fcw\\\":0.02}\"") +
            "\nSend two amounts to two different addresses setting the confirmation and comment:\n"
            + HelpExampleCli("sendmany", "\"tabby\" \"{\\\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\\\":0.01,\\\"LbhhnRHHVfP1eUJp1tDNiyeeVsNhFN9Fcw\\\":0.02}\" 6 \"testing\"") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("sendmany", "\"tabby\", \"{\\\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\\\":0.01,\\\"LbhhnRHHVfP1eUJp1tDNiyeeVsNhFN9Fcw\\\":0.02}\", 6, \"testing\"")
        );

    CPubKey id = AccountFromValue(params[0]);
    Object sendTo = params[1].get_obj();
    int nMinDepth = 1;
    if (params.size() > 2)
        nMinDepth = params[2].get_int();

    CWalletTx wtx;
    CBitcoinAddress address0;
    address0.Set(id);
    wtx.strFromAccount = address0.ToString();
    if (params.size() > 3 && params[3].type() != null_type && !params[3].get_str().empty())
        wtx.mapValue["comment"] = params[3].get_str();

    set<CBitcoinAddress> setAddress;
    vector<pair<CScript, CAmount> > vecSend;

    CAmount totalAmount = 0;
    BOOST_FOREACH(const Pair& s, sendTo)
    {
        CBitcoinAddress address(s.name_);
        if (!address.IsValid())
            throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, string("Invalid Cccoin address: ")+s.name_);

        if (setAddress.count(address))
            throw JSONRPCError(RPC_INVALID_PARAMETER, string("Invalid parameter, duplicated address: ")+s.name_);
        setAddress.insert(address);

        CScript scriptPubKey = GetScriptForDestination(address.Get());
        CAmount nAmount = AmountFromValue(s.value_);
        totalAmount += nAmount;

        vecSend.push_back(make_pair(scriptPubKey, nAmount));
    }

    EnsureWalletIsUnlocked();

    // Check funds
    CAmount nBalance = GetAccountBalance(id, nMinDepth, ISMINE_SPENDABLE);
    if (totalAmount > nBalance)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, "Account has insufficient funds");

    // Send
    CReserveKey keyChange(pwalletMain);
    CAmount nFeeRequired = 0;
    string strFailReason;
    bool fCreated = pwalletMain->CreateTransaction(vecSend, wtx, keyChange, nFeeRequired, strFailReason);
    if (!fCreated)
        throw JSONRPCError(RPC_WALLET_INSUFFICIENT_FUNDS, strFailReason);
    if (!pwalletMain->CommitTransaction(wtx, keyChange))
        throw JSONRPCError(RPC_WALLET_ERROR, "Transaction commit failed");

    return wtx.GetHash().GetHex();
}

// Defined in rpcmisc.cpp
extern CScript _createmultisig_redeemScript(const Array& params);

Value addmultisigaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 3)
    {
        string msg = "addmultisigaddress nrequired [\"key\",...] ( \"account\" )\n"
            "\nAdd a nrequired-to-sign multisignature address to the wallet.\n"
            "Each key is a Cccoin address or hex-encoded public key.\n"
            "If 'account' is specified, assign address to that account.\n"

            "\nArguments:\n"
            "1. nrequired        (numeric, required) The number of required signatures out of the n keys or addresses.\n"
            "2. \"keysobject\"   (string, required) A json array of cccoin addresses or hex-encoded public keys\n"
            "     [\n"
            "       \"address\"  (string) cccoin address or hex-encoded public key\n"
            "       ...,\n"
            "     ]\n"
            "3. \"account\"      (string, optional) An account to assign the addresses to.\n"

            "\nResult:\n"
            "\"cccoinaddress\"  (string) A cccoin address associated with the keys.\n"

            "\nExamples:\n"
            "\nAdd a multisig address from 2 addresses\n"
            + HelpExampleCli("addmultisigaddress", "2 \"[\\\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\\\",\\\"LbhhnRHHVfP1eUJp1tDNiyeeVsNhFN9Fcw\\\"]\"") +
            "\nAs json rpc call\n"
            + HelpExampleRpc("addmultisigaddress", "2, \"[\\\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\\\",\\\"LbhhnRHHVfP1eUJp1tDNiyeeVsNhFN9Fcw\\\"]\"")
        ;
        throw runtime_error(msg);
    }

    string strAccount;
   // if (params.size() > 2)
   //     strAccount = AccountFromValue(params[2]);

    // Construct using pay-to-script-hash:
    CScript inner = _createmultisig_redeemScript(params);
    CScriptID innerID(inner);
    pwalletMain->AddCScript(inner);

    pwalletMain->SetAddressBook(innerID, strAccount, "send");
    return CBitcoinAddress(innerID).ToString();
}


struct tallyitem
{
    CAmount nAmount;
    int nConf;
    vector<uint256> txids;
    bool fIsWatchonly;
    tallyitem()
    {
        nAmount = 0;
        nConf = std::numeric_limits<int>::max();
        fIsWatchonly = false;
    }
};

Value ListReceived(const Array& params, bool fByAccounts)
{
    // Minimum confirmations
    int nMinDepth = 1;
    if (params.size() > 0)
        nMinDepth = params[0].get_int();

    // Whether to include empty accounts
    bool fIncludeEmpty = false;
    if (params.size() > 1)
        fIncludeEmpty = params[1].get_bool();

    isminefilter filter = ISMINE_SPENDABLE;
    if(params.size() > 2)
        if(params[2].get_bool())
            filter = filter | ISMINE_WATCH_ONLY;

    // Tally
    map<CBitcoinAddress, tallyitem> mapTally;
    pwalletMain->LoadTxs();
    for (map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); ++it)
    {
        const CWalletTx& wtx = (*it).second;

        if (wtx.IsCoinBase() || !IsFinalTx(wtx))
            continue;

        int nDepth = wtx.GetDepthInMainChain();
        if (nDepth < nMinDepth)
            continue;

        BOOST_FOREACH(const CTxOut& txout, wtx.vout)
        {
            CTxDestination address;
            if (!ExtractDestination(txout.scriptPubKey, address))
                continue;

            isminefilter mine = IsMine(*pwalletMain, address);
            if(!(mine & filter))
                continue;

            tallyitem& item = mapTally[address];
            item.nAmount += txout.nValue;
            item.nConf = min(item.nConf, nDepth);
            item.txids.push_back(wtx.GetHash());
            if (mine & ISMINE_WATCH_ONLY)
                item.fIsWatchonly = true;
        }
    }

    // Reply
    Array ret;
    map<string, tallyitem> mapAccountTally;
    BOOST_FOREACH(const PAIRTYPE(CBitcoinAddress, CAddressBookData)& item, pwalletMain->mapAddressBook)
    {
        const CBitcoinAddress& address = item.first;
        const string& strAccount = item.second.name;
        map<CBitcoinAddress, tallyitem>::iterator it = mapTally.find(address);
        if (it == mapTally.end() && !fIncludeEmpty)
            continue;

        CAmount nAmount = 0;
        int nConf = std::numeric_limits<int>::max();
        bool fIsWatchonly = false;
        if (it != mapTally.end())
        {
            nAmount = (*it).second.nAmount;
            nConf = (*it).second.nConf;
            fIsWatchonly = (*it).second.fIsWatchonly;
        }

        if (fByAccounts)
        {
            tallyitem& item = mapAccountTally[strAccount];
            item.nAmount += nAmount;
            item.nConf = min(item.nConf, nConf);
            item.fIsWatchonly = fIsWatchonly;
        }
        else
        {
            Object obj;
            if(fIsWatchonly)
                obj.push_back(Pair("involvesWatchonly", true));
            obj.push_back(Pair("address",       address.ToString()));
            obj.push_back(Pair("account",       strAccount));
            obj.push_back(Pair("amount",        ValueFromAmount(nAmount)));
            obj.push_back(Pair("confirmations", (nConf == std::numeric_limits<int>::max() ? 0 : nConf)));
            Array transactions;
            if (it != mapTally.end())
            {
                BOOST_FOREACH(const uint256& item, (*it).second.txids)
                {
                    transactions.push_back(item.GetHex());
                }
            }
            obj.push_back(Pair("txids", transactions));
            ret.push_back(obj);
        }
    }

    if (fByAccounts)
    {
        for (map<string, tallyitem>::iterator it = mapAccountTally.begin(); it != mapAccountTally.end(); ++it)
        {
            CAmount nAmount = (*it).second.nAmount;
            int nConf = (*it).second.nConf;
            Object obj;
            if((*it).second.fIsWatchonly)
                obj.push_back(Pair("involvesWatchonly", true));
            obj.push_back(Pair("account",       (*it).first));
            obj.push_back(Pair("amount",        ValueFromAmount(nAmount)));
            obj.push_back(Pair("confirmations", (nConf == std::numeric_limits<int>::max() ? 0 : nConf)));
            ret.push_back(obj);
        }
    }

    return ret;
}

Value listreceivedbyaddress(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 3)
        throw runtime_error(
            "listreceivedbyaddress ( minconf includeempty includeWatchonly)\n"
            "\nList balances by receiving address.\n"
            "\nArguments:\n"
            "1. minconf       (numeric, optional, default=1) The minimum number of confirmations before payments are included.\n"
            "2. includeempty  (numeric, optional, default=false) Whether to include addresses that haven't received any payments.\n"
            "3. includeWatchonly (bool, optional, default=false) Whether to include watchonly addresses (see 'importaddress').\n"

            "\nResult:\n"
            "[\n"
            "  {\n"
            "    \"involvesWatchonly\" : \"true\",    (bool) Only returned if imported addresses were involved in transaction\n"
            "    \"address\" : \"receivingaddress\",  (string) The receiving address\n"
            "    \"account\" : \"accountname\",       (string) The account of the receiving address. The default account is \"\".\n"
            "    \"amount\" : x.xxx,                  (numeric) The total amount in ltc received by the address\n"
            "    \"confirmations\" : n                (numeric) The number of confirmations of the most recent transaction included\n"
            "  }\n"
            "  ,...\n"
            "]\n"

            "\nExamples:\n"
            + HelpExampleCli("listreceivedbyaddress", "")
            + HelpExampleCli("listreceivedbyaddress", "6 true")
            + HelpExampleRpc("listreceivedbyaddress", "6, true, true")
        );
    
    return ListReceived(params, false);
}

Value listreceivedbyaccount(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 3)
        throw runtime_error(
            "listreceivedbyaccount ( minconf includeempty includeWatchonly)\n"
            "\nList balances by account.\n"
            "\nArguments:\n"
            "1. minconf      (numeric, optional, default=1) The minimum number of confirmations before payments are included.\n"
            "2. includeempty (boolean, optional, default=false) Whether to include accounts that haven't received any payments.\n"
            "3. includeWatchonly (bool, optional, default=false) Whether to include watchonly addresses (see 'importaddress').\n"

            "\nResult:\n"
            "[\n"
            "  {\n"
            "    \"involvesWatchonly\" : \"true\",    (bool) Only returned if imported addresses were involved in transaction\n"
            "    \"account\" : \"accountname\",  (string) The account name of the receiving account\n"
            "    \"amount\" : x.xxx,             (numeric) The total amount received by addresses with this account\n"
            "    \"confirmations\" : n           (numeric) The number of confirmations of the most recent transaction included\n"
            "  }\n"
            "  ,...\n"
            "]\n"

            "\nExamples:\n"
            + HelpExampleCli("listreceivedbyaccount", "")
            + HelpExampleCli("listreceivedbyaccount", "6 true")
            + HelpExampleRpc("listreceivedbyaccount", "6, true, true")
        );

    return ListReceived(params, true);
}

static void MaybePushAddress(Object & entry, const CTxDestination &dest)
{
    CBitcoinAddress addr;
    if (addr.Set(dest))
        entry.push_back(Pair("address", addr.ToString()));
}

void ListTransactions(const CWalletTx& wtx, const string& strAccount, int nMinDepth, bool fLong, Array& ret, const isminefilter& filter)
{
    CAmount nFee;
    string strSentAccount;
    list<COutputEntry> listReceived;
    list<COutputEntry> listSent;

    wtx.GetAmounts(listReceived, listSent, nFee, strSentAccount, filter);
    //LogPrintf("rpcwallet listtxs listreceived:%u,listsent:%u,%i\n",listReceived.size(),listSent.size(),nFee);
    bool fAllAccounts = true;//(strAccount == string("*"));
    bool involvesWatchonly = wtx.IsFromMe(ISMINE_WATCH_ONLY);

    // Sent
    if ((!listSent.empty() || nFee != 0) && (fAllAccounts || strAccount == strSentAccount))
    {
        BOOST_FOREACH(const COutputEntry& s, listSent)
        {
            Object entry;
            if(involvesWatchonly || (::IsMine(*pwalletMain, s.destination) & ISMINE_WATCH_ONLY))
                entry.push_back(Pair("involvesWatchonly", true));
            entry.push_back(Pair("account", strSentAccount));
            MaybePushAddress(entry, s.destination);
            entry.push_back(Pair("category", "send"));
            entry.push_back(Pair("amount", ValueFromAmount(-s.amount)));
            entry.push_back(Pair("vout", s.vout));
            entry.push_back(Pair("fee", ValueFromAmount(-nFee)));
           
            if (fLong)
                WalletTxToJSON(wtx, entry);
            ret.push_back(entry);
            //LogPrintf("rpcwallet listsent %i\n",-s.amount);
        }
    }
    //LogPrintf("rpcwallet listtxs GetDepthInMainChain %i\n",wtx.GetDepthInMainChain());
    // Received
    if (listReceived.size() > 0 && wtx.GetDepthInMainChain() >= nMinDepth)
    {
        BOOST_FOREACH(const COutputEntry& r, listReceived)
        {
            string account;
            if (pwalletMain->mapAddressBook.count(r.destination))
                account = pwalletMain->mapAddressBook[r.destination].name;
            if (fAllAccounts || (account == strAccount))
            {
                Object entry;
                if(involvesWatchonly || (::IsMine(*pwalletMain, r.destination) & ISMINE_WATCH_ONLY))
                    entry.push_back(Pair("involvesWatchonly", true));
                entry.push_back(Pair("account", account));
                MaybePushAddress(entry, r.destination);
                if (wtx.IsCoinBase())
                {
                    if (wtx.GetDepthInMainChain() < 1)
                        entry.push_back(Pair("category", "orphan"));
                    else if (wtx.GetBlocksToMaturity() > 0)
                        entry.push_back(Pair("category", "immature"));
                    else
                        entry.push_back(Pair("category", "generate"));
                }
                else
                {
                    entry.push_back(Pair("category", "receive"));
                }
                entry.push_back(Pair("amount", ValueFromAmount(r.amount)));
                entry.push_back(Pair("vout", r.vout));                
                if (fLong)
                    WalletTxToJSON(wtx, entry);
                ret.push_back(entry);
            }
        }
    }
    //LogPrintf("rpcwallet ret size %i\n",ret.size());
}

void AcentryToJSON(const CAccountingEntry& acentry, const string& strAccount, Array& ret)
{
    bool fAllAccounts = (strAccount == string("*"));

    if (fAllAccounts || acentry.strAccount == strAccount)
    {
        Object entry;
        entry.push_back(Pair("account", acentry.strAccount));
        entry.push_back(Pair("category", "move"));
        entry.push_back(Pair("time", acentry.nTime));
        entry.push_back(Pair("amount", ValueFromAmount(acentry.nCreditDebit)));
        entry.push_back(Pair("otheraccount", acentry.strOtherAccount));
        entry.push_back(Pair("comment", acentry.strComment));
        ret.push_back(entry);
    }
}

Value listtransactions(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 4)
        throw runtime_error(
            "listtransactions ( \"id\" count from includeextendedids)\n"
            "\nReturns up to 'count' most recent transactions skipping the first 'from' transactions for mainid  'main id'.\n"
            "\nArguments:\n"
            "1. \"id\"    (string, optional) The main id. If not included, it will list all transactions for current accounts.\n"            
            "2. count          (numeric, optional, default=100) The number of transactions to return\n"
            "3. from           (numeric, optional, default=0) The number of transactions to skip\n"
            "4. includeextendedids (bool, optional, default=true) Include transactions from extended ids\n"
            "\nResult:\n"
            "{\"txs\":\n"    
            "[\n"
            "  {\n"            
            "    \"address\":\"cccoinaddress\",    (string) The cccoin address of the transaction. Not present for \n"
            "                                                move transactions (category = move).\n"
            "    \"category\":\"send|receive|move\", (string) The transaction category. 'move' is a local (off blockchain)\n"
            "                                                transaction between ids, and not associated with an id,\n"
            "                                                transaction id or block. 'send' and 'receive' transactions are \n"
            "                                                associated with an account id, transaction id and block details\n"
            "    \"amount\": x.xxx,          (numeric) The amount in ccc. This is negative for the 'send' category, and for the\n"
            "                                         'move' category for moves outbound. It is positive for the 'receive' category,\n"
            "                                         and for the 'move' category for inbound funds.\n"
            "    \"vout\" : n,               (numeric) the vout value\n"
            "    \"fee\": x.xxx,             (numeric) The amount of the fee in ccc. This is negative and only available for the \n"
            "                                         'send' category of transactions.\n"
            "    \"confirmations\": n,       (numeric) The number of confirmations for the transaction. Available for 'send' and \n"
            "                                         'receive' category of transactions.\n"
            "    \"blockhash\": \"hashvalue\", (string) The block hash containing the transaction. Available for 'send' and 'receive'\n"
            "                                          category of transactions.\n"
            "    \"blockindex\": n,          (numeric) The block index containing the transaction. Available for 'send' and 'receive'\n"
            "                                          category of transactions.\n"
            "    \"txid\": \"transactionid\", (string) The transaction id. Available for 'send' and 'receive' category of transactions.\n"
            "    \"time\": xxx,              (numeric) The transaction time in seconds since epoch (midnight Jan 1 1970 GMT).\n"
            "    \"timereceived\": xxx,      (numeric) The time received in seconds since epoch (midnight Jan 1 1970 GMT). Available \n"
            "                                          for 'send' and 'receive' category of transactions.\n"
            "    \"content\": \"...\",       (string) If a content is associated with the transaction.\n"
            "  }\n"
            "]\n"
            "\"balances\":\n"
            "  {\n"
                "\"balance_available\":x.xxx ,(numeric) available balance\n"
                "\"balance_unconfirmed\":x.xxx ,(numeric) unconfirmed balance\n"
                "\"balance_locked\":x.xxx ,(numeric) locked balance\n"
                "\"balance_total\":x.xxx ,(numeric) total balance\n"
            "  }\n"                
            "}\n"
            "\nExamples:\n"
            "\nList the most recent 100 transactions in the default account\n"
            + HelpExampleCli("listtransactions", "") +
            "\nList the most recent 100 transactions for the account QN5BF73O35EMQ2G5NZAQ554YHLERLDVMAAPIQBI6 \n"
            + HelpExampleCli("listtransactions", "\"QN5BF73O35EMQ2G5NZAQ554YHLERLDVMAAPIQBI6 \"") +
            "\nList transactions 100 to 120 from the QN5BF73O35EMQ2G5NZAQ554YHLERLDVMAAPIQBI6  account\n"
            + HelpExampleCli("listtransactions", "\"QN5BF73O35EMQ2G5NZAQ554YHLERLDVMAAPIQBI6 \" 20 100") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("listtransactions", "\"QN5BF73O35EMQ2G5NZAQ554YHLERLDVMAAPIQBI6 \", 20, 100")
        );
    CWallet* pwallet;
    string strAccount="*";
    if (params.size() > 0){        
         strAccount= params[0].get_str();
        if (strAccount==""){
            pwallet=pwalletMain;
            pwalletMain->LoadTxs();
        }
        else{
            CPubKey id;
            id=AccountFromValue(params[0]);
            LogPrintf("rpcwallet listtxs new pwallet id:%s \n",HexStr(id.begin(),id.end()));
            pwallet=new CWallet(id);
        }
            
    }else{
        pwallet=pwalletMain;
            pwallet->LoadTxs();
    }
    int nCount = 100;
    if (params.size() > 1)
        nCount = params[1].get_int();
    int nFrom = 0;
    if (params.size() > 2)
        nFrom = params[2].get_int();
    isminefilter filter = ISMINE_ALL;
    bool fIncludeExtendedKeys=true;
    if(params.size() > 3)
        if(!params[3].get_bool())
            fIncludeExtendedKeys=false;
    LogPrintf("rpcwallet listtxs params:%u,%s %i %i %b\n",params.size(),strAccount,nCount,nFrom,fIncludeExtendedKeys);
    if (nCount < 0)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative count");
    if (nFrom < 0)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Negative from");

    Array ret;

    CWallet::TxItems txOrdered = pwallet->OrderedTxItems(fIncludeExtendedKeys);

    // iterate backwards until we have nCount items to return:
    for (CWallet::TxItems::iterator it = txOrdered.begin(); it != txOrdered.end(); it++)
    {
        CWalletTx *const pwtx = (*it).second.first;
        //LogPrintf("rpcwallet listtxs pwtx :%i\n",pwtx);
        if (pwtx != 0)
            ListTransactions(*pwtx, strAccount, 0, true, ret, filter);
        CAccountingEntry *const pacentry = (*it).second.second;
        if (pacentry != 0)
            AcentryToJSON(*pacentry, strAccount, ret);
        //LogPrintf("rpcwallet listtxs ntx :%u\n",ret.size());
        if ((int)ret.size() >= (nCount+nFrom)) break;
    }
    // ret is newest to oldest

    if (nFrom > (int)ret.size())
        nFrom = ret.size();
    if ((nFrom + nCount) > (int)ret.size())
        nCount = ret.size() - nFrom;
    Array::iterator first = ret.begin();
    std::advance(first, nFrom);
    Array::iterator last = ret.begin();
    std::advance(last, nFrom+nCount);

    if (last != ret.end()) ret.erase(last, ret.end());
    if (first != ret.begin()) ret.erase(ret.begin(), first);

    //std::reverse(ret.begin(), ret.end()); // Return oldest to newest
    Object balance;
    CAmount balance_available=pwallet->GetBalance(false);
    CAmount balance_unconfirmed=pwallet->GetUnconfirmedBalance(false);
    CAmount balance_locked=pwallet->GetImmatureBalance(false);
    CAmount balance_total=balance_available+balance_unconfirmed+balance_locked;
    balance.push_back(Pair("balance_available",ValueFromAmount(balance_available)));
    balance.push_back(Pair("balance_unconfirmed",ValueFromAmount(balance_unconfirmed)));
    balance.push_back(Pair("balance_locked",ValueFromAmount(balance_locked)));
    balance.push_back(Pair("balance_total",ValueFromAmount(balance_total)));
    Object result;
    LogPrintf("rpcwallet ret size %i\n",ret.size());
    result.push_back(Pair("txs",ret));
    result.push_back(Pair("balance",balance));
    result.push_back(Pair("currentblockheight",chainActive.Height()));
    if (pwallet!=pwalletMain)
        delete pwallet;
    return result;
}

Value listaccounts(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw runtime_error(
            "listaccounts ( minconf includeWatchonly)\n"
            "\nReturns Object that has account names as keys, account balances as values.\n"
            "\nArguments:\n"
            "1. minconf          (numeric, optional, default=1) Only include transactions with at least this many confirmations\n"
            "2. includeWatchonly (bool, optional, default=false) Include balances in watchonly addresses (see 'importaddress')\n"
            "\nResult:\n"
            "{                      (json object where keys are account names, and values are numeric balances\n"
            "  \"account\": x.xxx,  (numeric) The property name is the account name, and the value is the total balance for the account.\n"
            "  ...\n"
            "}\n"
            "\nExamples:\n"
            "\nList account balances where there at least 1 confirmation\n"
            + HelpExampleCli("listaccounts", "") +
            "\nList account balances including zero confirmation transactions\n"
            + HelpExampleCli("listaccounts", "0") +
            "\nList account balances for 6 or more confirmations\n"
            + HelpExampleCli("listaccounts", "6") +
            "\nAs json rpc call\n"
            + HelpExampleRpc("listaccounts", "6")
        );

    int nMinDepth = 1;
    if (params.size() > 0)
        nMinDepth = params[0].get_int();
    isminefilter includeWatchonly = ISMINE_SPENDABLE;
    if(params.size() > 1)
        if(params[1].get_bool())
            includeWatchonly = includeWatchonly | ISMINE_WATCH_ONLY;

    map<string, CAmount> mapAccountBalances;
    BOOST_FOREACH(const PAIRTYPE(CTxDestination, CAddressBookData)& entry, pwalletMain->mapAddressBook) {
        if (IsMine(*pwalletMain, entry.first) & includeWatchonly) // This address belongs to me
            mapAccountBalances[entry.second.name] = 0;
    }

    for (map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); ++it)
    {
        const CWalletTx& wtx = (*it).second;
        CAmount nFee;
        string strSentAccount;
        list<COutputEntry> listReceived;
        list<COutputEntry> listSent;
        int nDepth = wtx.GetDepthInMainChain();
        if (wtx.GetBlocksToMaturity() > 0 || nDepth < 0)
            continue;
        wtx.GetAmounts(listReceived, listSent, nFee, strSentAccount, includeWatchonly);
        mapAccountBalances[strSentAccount] -= nFee;
        BOOST_FOREACH(const COutputEntry& s, listSent)
            mapAccountBalances[strSentAccount] -= s.amount;
        if (nDepth >= nMinDepth)
        {
            BOOST_FOREACH(const COutputEntry& r, listReceived)
                if (pwalletMain->mapAddressBook.count(r.destination))
                    mapAccountBalances[pwalletMain->mapAddressBook[r.destination].name] += r.amount;
                else
                    mapAccountBalances[""] += r.amount;
        }
    }

    list<CAccountingEntry> acentries;
    //CWalletDB().ListAccountCreditDebit("*", acentries);
    BOOST_FOREACH(const CAccountingEntry& entry, acentries)
        mapAccountBalances[entry.strAccount] += entry.nCreditDebit;

    Object ret;
    BOOST_FOREACH(const PAIRTYPE(string, CAmount)& accountBalance, mapAccountBalances) {
        ret.push_back(Pair(accountBalance.first, ValueFromAmount(accountBalance.second)));
    }
    return ret;
}

Value listsinceblock(const Array& params, bool fHelp)
{
    if (fHelp)
        throw runtime_error(
            "listsinceblock ( \"blockhash\" target-confirmations includeWatchonly)\n"
            "\nGet all transactions in blocks since block [blockhash], or all transactions if omitted\n"
            "\nArguments:\n"
            "1. \"blockhash\"   (string, optional) The block hash to list transactions since\n"
            "2. target-confirmations:    (numeric, optional) The confirmations required, must be 1 or more\n"
            "3. includeWatchonly:        (bool, optional, default=false) Include transactions to watchonly addresses (see 'importaddress')"
            "\nResult:\n"
            "{\n"
            "  \"transactions\": [\n"
            "    \"account\":\"accountname\",       (string) The account name associated with the transaction. Will be \"\" for the default account.\n"
            "    \"address\":\"cccoinaddress\",    (string) The cccoin address of the transaction. Not present for move transactions (category = move).\n"
            "    \"category\":\"send|receive\",     (string) The transaction category. 'send' has negative amounts, 'receive' has positive amounts.\n"
            "    \"amount\": x.xxx,          (numeric) The amount in ltc. This is negative for the 'send' category, and for the 'move' category for moves \n"
            "                                          outbound. It is positive for the 'receive' category, and for the 'move' category for inbound funds.\n"
            "    \"vout\" : n,               (numeric) the vout value\n"
            "    \"fee\": x.xxx,             (numeric) The amount of the fee in ltc. This is negative and only available for the 'send' category of transactions.\n"
            "    \"confirmations\": n,       (numeric) The number of confirmations for the transaction. Available for 'send' and 'receive' category of transactions.\n"
            "    \"blockhash\": \"hashvalue\",     (string) The block hash containing the transaction. Available for 'send' and 'receive' category of transactions.\n"
            "    \"blockindex\": n,          (numeric) The block index containing the transaction. Available for 'send' and 'receive' category of transactions.\n"
            "    \"blocktime\": xxx,         (numeric) The block time in seconds since epoch (1 Jan 1970 GMT).\n"
            "    \"txid\": \"transactionid\",  (string) The transaction id. Available for 'send' and 'receive' category of transactions.\n"
            "    \"time\": xxx,              (numeric) The transaction time in seconds since epoch (Jan 1 1970 GMT).\n"
            "    \"timereceived\": xxx,      (numeric) The time received in seconds since epoch (Jan 1 1970 GMT). Available for 'send' and 'receive' category of transactions.\n"
            "    \"comment\": \"...\",       (string) If a comment is associated with the transaction.\n"
            "    \"to\": \"...\",            (string) If a comment to is associated with the transaction.\n"
             "  ],\n"
            "  \"lastblock\": \"lastblockhash\"     (string) The hash of the last block\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("listsinceblock", "")
            + HelpExampleCli("listsinceblock", "\"0c3b2c31c8aa025e5ae7a87dfe63d1795a061b95e7b00aee61e5384338a26739\" 6")
            + HelpExampleRpc("listsinceblock", "\"0c3b2c31c8aa025e5ae7a87dfe63d1795a061b95e7b00aee61e5384338a26739\", 6")
        );

    CBlockIndex *pindex = NULL;
    int target_confirms = 1;
    isminefilter filter = ISMINE_SPENDABLE;

    if (params.size() > 0)
    {
        uint256 blockId = 0;

        blockId.SetHex(params[0].get_str());
        BlockMap::iterator it = mapBlockIndex.find(blockId);
        if (it != mapBlockIndex.end())
            pindex = it->second;
    }

    if (params.size() > 1)
    {
        target_confirms = params[1].get_int();

        if (target_confirms < 1)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter");
    }

    if(params.size() > 2)
        if(params[2].get_bool())
            filter = filter | ISMINE_WATCH_ONLY;

    int depth = pindex ? (1 + chainActive.Height() - pindex->nHeight) : -1;

    Array transactions;

    for (map<uint256, CWalletTx>::iterator it = pwalletMain->mapWallet.begin(); it != pwalletMain->mapWallet.end(); it++)
    {
        CWalletTx tx = (*it).second;

        if (depth == -1 || tx.GetDepthInMainChain() < depth)
            ListTransactions(tx, "*", 0, true, transactions, filter);
    }

    CBlockIndex *pblockLast = chainActive[chainActive.Height() + 1 - target_confirms];
    uint256 lastblock = pblockLast ? pblockLast->GetBlockHash() : 0;

    Object ret;
    ret.push_back(Pair("transactions", transactions));
    ret.push_back(Pair("lastblock", lastblock.GetHex()));

    return ret;
}

Value gettransaction(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "gettransaction \"txid\" ( includeWatchonly )\n"
            "\nGet detailed information about in-wallet transaction <txid>\n"
            "\nArguments:\n"
            "1. \"txid\"    (string, required) The transaction id\n"
            "2. \"includeWatchonly\"    (bool, optional, default=false) Whether to include watchonly addresses in balance calculation and details[]\n"
            "\nResult:\n"
            "{\n"
            "  \"amount\" : x.xxx,        (numeric) The transaction amount in ltc\n"
            "  \"confirmations\" : n,     (numeric) The number of confirmations\n"
            "  \"blockhash\" : \"hash\",  (string) The block hash\n"
            "  \"blockindex\" : xx,       (numeric) The block index\n"
            "  \"blocktime\" : ttt,       (numeric) The time in seconds since epoch (1 Jan 1970 GMT)\n"
            "  \"txid\" : \"transactionid\",   (string) The transaction id.\n"
            "  \"time\" : ttt,            (numeric) The transaction time in seconds since epoch (1 Jan 1970 GMT)\n"
            "  \"timereceived\" : ttt,    (numeric) The time received in seconds since epoch (1 Jan 1970 GMT)\n"
            "  \"details\" : [\n"
            "    {\n"
            "      \"account\" : \"accountname\",  (string) The account name involved in the transaction, can be \"\" for the default account.\n"
            "      \"address\" : \"cccoinaddress\",   (string) The cccoin address involved in the transaction\n"
            "      \"category\" : \"send|receive\",    (string) The category, either 'send' or 'receive'\n"
            "      \"amount\" : x.xxx                  (numeric) The amount in ltc\n"
            "      \"vout\" : n,                       (numeric) the vout value\n"
            "    }\n"
            "    ,...\n"
            "  ],\n"
            "  \"hex\" : \"data\"         (string) Raw data for transaction\n"
            "}\n"

            "\nExamples:\n"
            + HelpExampleCli("gettransaction", "\"cdf21f8cfaee3544ada266ff2da8554e8374fcd7efc6a0444f7ac75d7948884e\"")
            + HelpExampleCli("gettransaction", "\"cdf21f8cfaee3544ada266ff2da8554e8374fcd7efc6a0444f7ac75d7948884e\" true")
            + HelpExampleRpc("gettransaction", "\"cdf21f8cfaee3544ada266ff2da8554e8374fcd7efc6a0444f7ac75d7948884e\"")
        );

    uint256 hash;
    hash.SetHex(params[0].get_str());

    isminefilter filter = ISMINE_SPENDABLE;
    if(params.size() > 1)
        if(params[1].get_bool())
            filter = filter | ISMINE_WATCH_ONLY;

    Object entry;
    if (!pwalletMain->mapWallet.count(hash))
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Invalid or non-wallet transaction id");
    const CWalletTx& wtx = pwalletMain->mapWallet[hash];

    CAmount nCredit = wtx.GetCredit(filter);
    CAmount nDebit = wtx.GetDebit(filter);
    CAmount nNet = nCredit - nDebit;
    CAmount nFee = (wtx.IsFromMe(filter) ? wtx.GetValueOut() - nDebit : 0);

    entry.push_back(Pair("amount", ValueFromAmount(nNet - nFee)));
    if (wtx.IsFromMe(filter))
        entry.push_back(Pair("fee", ValueFromAmount(nFee)));

    WalletTxToJSON(wtx, entry);

    Array details;
    ListTransactions(wtx, "*", 0, false, details, filter);
    entry.push_back(Pair("details", details));

    string strHex = EncodeHexTx(static_cast<CTransaction>(wtx));
    entry.push_back(Pair("hex", strHex));

    return entry;
}


Value backupwallet(const Array& params, bool fHelp)
{
//    if (fHelp || params.size() != 1)
//        throw runtime_error(
//            "backupwallet \"destination\"\n"
//            "\nSafely copies wallet.dat to destination, which can be a directory or a path with filename.\n"
//            "\nArguments:\n"
//            "1. \"destination\"   (string) The destination directory or file\n"
//            "\nExamples:\n"
//            + HelpExampleCli("backupwallet", "\"backup.dat\"")
//            + HelpExampleRpc("backupwallet", "\"backup.dat\"")
//        );
//
//    string strDest = params[0].get_str();
//    if (!BackupWallet(*pwalletMain, strDest))
//        throw JSONRPCError(RPC_WALLET_ERROR, "Error: Wallet backup failed!");

    return Value::null;
}


Value keypoolrefill(const Array& params, bool fHelp)
{
//    if (fHelp || params.size() > 1)
//        throw runtime_error(
//            "keypoolrefill ( newsize )\n"
//            "\nFills the keypool."
//            + HelpRequiringPassphrase() + "\n"
//            "\nArguments\n"
//            "1. newsize     (numeric, optional, default=100) The new keypool size\n"
//            "\nExamples:\n"
//            + HelpExampleCli("keypoolrefill", "")
//            + HelpExampleRpc("keypoolrefill", "")
//        );
//
//    // 0 is interpreted by TopUpKeyPool() as the default keypool size given by -keypool
//    unsigned int kpSize = 0;
//    if (params.size() > 0) {
//        if (params[0].get_int() < 0)
//            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, expected valid size.");
//        kpSize = (unsigned int)params[0].get_int();
//    }
//
//    EnsureWalletIsUnlocked();
//    pwalletMain->TopUpKeyPool(kpSize);
//
//    if (pwalletMain->GetKeyPoolSize() < kpSize)
//        throw JSONRPCError(RPC_WALLET_ERROR, "Error refreshing keypool.");

    return Value::null;
}


static void LockWallet(CWallet* pWallet)
{
//    LOCK(cs_nWalletUnlockTime);
//    nWalletUnlockTime = 0;
//    pWallet->Lock();
}

Value walletpassphrase(const Array& params, bool fHelp)
{
    if (pwalletMain->IsCrypted() && (fHelp || params.size() != 2))
        throw runtime_error(
            "walletpassphrase \"passphrase\" timeout\n"
            "\nStores the wallet decryption key in memory for 'timeout' seconds.\n"
            "This is needed prior to performing transactions related to private keys such as sending cccoins\n"
            "\nArguments:\n"
            "1. \"passphrase\"     (string, required) The wallet passphrase\n"
            "2. timeout            (numeric, required) The time to keep the decryption key in seconds.\n"
            "\nNote:\n"
            "Issuing the walletpassphrase command while the wallet is already unlocked will set a new unlock\n"
            "time that overrides the old one.\n"
            "\nExamples:\n"
            "\nunlock the wallet for 60 seconds\n"
            + HelpExampleCli("walletpassphrase", "\"my pass phrase\" 60") +
            "\nLock the wallet again (before 60 seconds)\n"
            + HelpExampleCli("walletlock", "") +
            "\nAs json rpc call\n"
            + HelpExampleRpc("walletpassphrase", "\"my pass phrase\", 60")
        );

    if (fHelp)
        return true;
    if (!pwalletMain->IsCrypted())
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an unencrypted wallet, but walletpassphrase was called.");

    // Note that the walletpassphrase is stored in params[0] which is not mlock()ed
    SecureString strWalletPass;
    strWalletPass.reserve(100);
    // TODO: get rid of this .c_str() by implementing SecureString::operator=(std::string)
    // Alternately, find a way to make params[0] mlock()'d to begin with.
    strWalletPass = params[0].get_str().c_str();

    if (strWalletPass.length() > 0)
    {
        if (!pwalletMain->Unlock(strWalletPass))
            throw JSONRPCError(RPC_WALLET_PASSPHRASE_INCORRECT, "Error: The wallet passphrase entered was incorrect.");
    }
    else
        throw runtime_error(
            "walletpassphrase <passphrase> <timeout>\n"
            "Stores the wallet decryption key in memory for <timeout> seconds.");

    //pwalletMain->TopUpKeyPool();

    int64_t nSleepTime = params[1].get_int64();
    LOCK(cs_nWalletUnlockTime);
    nWalletUnlockTime = GetTime() + nSleepTime;
    RPCRunLater("lockwallet", boost::bind(LockWallet, pwalletMain), nSleepTime);

    return Value::null;
}


Value walletpassphrasechange(const Array& params, bool fHelp)
{
    if (pwalletMain->IsCrypted() && (fHelp || params.size() != 2))
        throw runtime_error(
            "walletpassphrasechange \"oldpassphrase\" \"newpassphrase\"\n"
            "\nChanges the wallet passphrase from 'oldpassphrase' to 'newpassphrase'.\n"
            "\nArguments:\n"
            "1. \"oldpassphrase\"      (string) The current passphrase\n"
            "2. \"newpassphrase\"      (string) The new passphrase\n"
            "\nExamples:\n"
            + HelpExampleCli("walletpassphrasechange", "\"old one\" \"new one\"")
            + HelpExampleRpc("walletpassphrasechange", "\"old one\", \"new one\"")
        );

    if (fHelp)
        return true;
    if (!pwalletMain->IsCrypted())
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an unencrypted wallet, but walletpassphrasechange was called.");

    // TODO: get rid of these .c_str() calls by implementing SecureString::operator=(std::string)
    // Alternately, find a way to make params[0] mlock()'d to begin with.
    SecureString strOldWalletPass;
    strOldWalletPass.reserve(100);
    strOldWalletPass = params[0].get_str().c_str();

    SecureString strNewWalletPass;
    strNewWalletPass.reserve(100);
    strNewWalletPass = params[1].get_str().c_str();

    if (strOldWalletPass.length() < 1 || strNewWalletPass.length() < 1)
        throw runtime_error(
            "walletpassphrasechange <oldpassphrase> <newpassphrase>\n"
            "Changes the wallet passphrase from <oldpassphrase> to <newpassphrase>.");

    if (!pwalletMain->ChangeWalletPassphrase(strOldWalletPass, strNewWalletPass))
        throw JSONRPCError(RPC_WALLET_PASSPHRASE_INCORRECT, "Error: The wallet passphrase entered was incorrect.");

    return Value::null;
}


Value walletlock(const Array& params, bool fHelp)
{
    if (pwalletMain->IsCrypted() && (fHelp || params.size() != 0))
        throw runtime_error(
            "walletlock\n"
            "\nRemoves the wallet encryption key from memory, locking the wallet.\n"
            "After calling this method, you will need to call walletpassphrase again\n"
            "before being able to call any methods which require the wallet to be unlocked.\n"
            "\nExamples:\n"
            "\nSet the passphrase for 2 minutes to perform a transaction\n"
            + HelpExampleCli("walletpassphrase", "\"my pass phrase\" 120") +
            "\nPerform a send (requires passphrase set)\n"
            + HelpExampleCli("sendtoaddress", "\"Ler4HNAEfwYhBmGXcFP2Po1NpRUEiK8km2\" 1.0") +
            "\nClear the passphrase since we are done before 2 minutes is up\n"
            + HelpExampleCli("walletlock", "") +
            "\nAs json rpc call\n"
            + HelpExampleRpc("walletlock", "")
        );

    if (fHelp)
        return true;
    if (!pwalletMain->IsCrypted())
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an unencrypted wallet, but walletlock was called.");

    {
        LOCK(cs_nWalletUnlockTime);
        pwalletMain->Lock();
        nWalletUnlockTime = 0;
    }

    return Value::null;
}


Value encryptwallet(const Array& params, bool fHelp)
{
    if (!pwalletMain->IsCrypted() && (fHelp || params.size() != 1))
        throw runtime_error(
            "encryptwallet \"passphrase\"\n"
            "\nEncrypts the wallet with 'passphrase'. This is for first time encryption.\n"
            "After this, any calls that interact with private keys such as sending or signing \n"
            "will require the passphrase to be set prior the making these calls.\n"
            "Use the walletpassphrase call for this, and then walletlock call.\n"
            "If the wallet is already encrypted, use the walletpassphrasechange call.\n"
            "Note that this will shutdown the server.\n"
            "\nArguments:\n"
            "1. \"passphrase\"    (string) The pass phrase to encrypt the wallet with. It must be at least 1 character, but should be long.\n"
            "\nExamples:\n"
            "\nEncrypt you wallet\n"
            + HelpExampleCli("encryptwallet", "\"my pass phrase\"") +
            "\nNow set the passphrase to use the wallet, such as for signing or sending cccoin\n"
            + HelpExampleCli("walletpassphrase", "\"my pass phrase\"") +
            "\nNow we can so something like sign\n"
            + HelpExampleCli("signmessage", "\"cccoinaddress\" \"test message\"") +
            "\nNow lock the wallet again by removing the passphrase\n"
            + HelpExampleCli("walletlock", "") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("encryptwallet", "\"my pass phrase\"")
        );

    if (fHelp)
        return true;
    if (pwalletMain->IsCrypted())
        throw JSONRPCError(RPC_WALLET_WRONG_ENC_STATE, "Error: running with an encrypted wallet, but encryptwallet was called.");

    // TODO: get rid of this .c_str() by implementing SecureString::operator=(std::string)
    // Alternately, find a way to make params[0] mlock()'d to begin with.
    SecureString strWalletPass;
    strWalletPass.reserve(100);
    strWalletPass = params[0].get_str().c_str();

    if (strWalletPass.length() < 1)
        throw runtime_error(
            "encryptwallet <passphrase>\n"
            "Encrypts the wallet with <passphrase>.");

    if (!pwalletMain->EncryptWallet(strWalletPass))
        throw JSONRPCError(RPC_WALLET_ENCRYPTION_FAILED, "Error: Failed to encrypt the wallet.");

    // BDB seems to have a bad habit of writing old data into
    // slack space in .dat files; that is bad if the old data is
    // unencrypted private keys. So:
    StartShutdown();
    return "wallet encrypted; Cccoin server stopping, restart to run with encrypted wallet. The keypool has been flushed, you need to make a new backup.";
}

Value lockunspent(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "lockunspent unlock [{\"txid\":\"txid\",\"vout\":n},...]\n"
            "\nUpdates list of temporarily unspendable outputs.\n"
            "Temporarily lock (unlock=false) or unlock (unlock=true) specified transaction outputs.\n"
            "A locked transaction output will not be chosen by automatic coin selection, when spending cccoins.\n"
            "Locks are stored in memory only. Nodes start with zero locked outputs, and the locked output list\n"
            "is always cleared (by virtue of process exit) when a node stops or fails.\n"
            "Also see the listunspent call\n"
            "\nArguments:\n"
            "1. unlock            (boolean, required) Whether to unlock (true) or lock (false) the specified transactions\n"
            "2. \"transactions\"  (string, required) A json array of objects. Each object the txid (string) vout (numeric)\n"
            "     [           (json array of json objects)\n"
            "       {\n"
            "         \"txid\":\"id\",    (string) The transaction id\n"
            "         \"vout\": n         (numeric) The output number\n"
            "       }\n"
            "       ,...\n"
            "     ]\n"

            "\nResult:\n"
            "true|false    (boolean) Whether the command was successful or not\n"

            "\nExamples:\n"
            "\nList the unspent transactions\n"
            + HelpExampleCli("listunspent", "") +
            "\nLock an unspent transaction\n"
            + HelpExampleCli("lockunspent", "false \"[{\\\"txid\\\":\\\"cdf21f8cfaee3544ada266ff2da8554e8374fcd7efc6a0444f7ac75d7948884e\\\",\\\"vout\\\":1}]\"") +
            "\nList the locked transactions\n"
            + HelpExampleCli("listlockunspent", "") +
            "\nUnlock the transaction again\n"
            + HelpExampleCli("lockunspent", "true \"[{\\\"txid\\\":\\\"cdf21f8cfaee3544ada266ff2da8554e8374fcd7efc6a0444f7ac75d7948884e\\\",\\\"vout\\\":1}]\"") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("lockunspent", "false, \"[{\\\"txid\\\":\\\"cdf21f8cfaee3544ada266ff2da8554e8374fcd7efc6a0444f7ac75d7948884e\\\",\\\"vout\\\":1}]\"")
        );

    if (params.size() == 1)
        RPCTypeCheck(params, list_of(bool_type));
    else
        RPCTypeCheck(params, list_of(bool_type)(array_type));

    bool fUnlock = params[0].get_bool();

    if (params.size() == 1) {
        if (fUnlock)
            pwalletMain->UnlockAllCoins();
        return true;
    }

    Array outputs = params[1].get_array();
    BOOST_FOREACH(Value& output, outputs)
    {
        if (output.type() != obj_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, expected object");
        const Object& o = output.get_obj();

        RPCTypeCheck(o, map_list_of("txid", str_type)("vout", int_type));

        string txid = find_value(o, "txid").get_str();
        if (!IsHex(txid))
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, expected hex txid");

        int nOutput = find_value(o, "vout").get_int();
        if (nOutput < 0)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter, vout must be positive");

        COutPoint outpt(uint256(txid), nOutput,0);

        if (fUnlock)
            pwalletMain->UnlockCoin(outpt);
        else
            pwalletMain->LockCoin(outpt);
    }

    return true;
}

Value listlockunspent(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 0)
        throw runtime_error(
            "listlockunspent\n"
            "\nReturns list of temporarily unspendable outputs.\n"
            "See the lockunspent call to lock and unlock transactions for spending.\n"
            "\nResult:\n"
            "[\n"
            "  {\n"
            "    \"txid\" : \"transactionid\",     (string) The transaction id locked\n"
            "    \"vout\" : n                      (numeric) The vout value\n"
            "  }\n"
            "  ,...\n"
            "]\n"
            "\nExamples:\n"
            "\nList the unspent transactions\n"
            + HelpExampleCli("listunspent", "") +
            "\nLock an unspent transaction\n"
            + HelpExampleCli("lockunspent", "false \"[{\\\"txid\\\":\\\"cdf21f8cfaee3544ada266ff2da8554e8374fcd7efc6a0444f7ac75d7948884e\\\",\\\"vout\\\":1}]\"") +
            "\nList the locked transactions\n"
            + HelpExampleCli("listlockunspent", "") +
            "\nUnlock the transaction again\n"
            + HelpExampleCli("lockunspent", "true \"[{\\\"txid\\\":\\\"cdf21f8cfaee3544ada266ff2da8554e8374fcd7efc6a0444f7ac75d7948884e\\\",\\\"vout\\\":1}]\"") +
            "\nAs a json rpc call\n"
            + HelpExampleRpc("listlockunspent", "")
        );

    vector<COutPoint> vOutpts;
    pwalletMain->ListLockedCoins(vOutpts);

    Array ret;

    BOOST_FOREACH(COutPoint &outpt, vOutpts) {
        Object o;

        o.push_back(Pair("txid", outpt.hash.GetHex()));
        o.push_back(Pair("vout", (int)outpt.n));
        ret.push_back(o);
    }

    return ret;
}

Value settxfee(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 1)
        throw runtime_error(
            "settxfee amount\n"
            "\nSet the transaction fee per kB.\n"
            "\nArguments:\n"
            "1. amount         (numeric, required) The transaction fee in CCC/kB rounded to the nearest 0.00000001\n"
            "\nResult\n"
            "true|false        (boolean) Returns true if successful\n"
            "\nExamples:\n"
            + HelpExampleCli("settxfee", "0.00001")
            + HelpExampleRpc("settxfee", "0.00001")
        );

    // Amount
    CAmount nAmount = 0;
    if (params[0].get_real() != 0.0)
        nAmount = AmountFromValue(params[0]);        // rejects 0.0 amounts

    payTxFee = CFeeRate(nAmount, 1);
    return true;
}

Value getwalletinfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getwalletinfo\n"
            "Returns an object containing various wallet state info.\n"
            "\nResult:\n"
            "{\n"
            "  \"walletversion\": xxxxx,     (numeric) the wallet version\n"
            "  \"balance\": xxxxxxx,         (numeric) the total cccoin balance of the wallet\n"
            "  \"txcount\": xxxxxxx,         (numeric) the total number of transactions in the wallet\n"
            "  \"keypoololdest\": xxxxxx,    (numeric) the timestamp (seconds since GMT epoch) of the oldest pre-generated key in the key pool\n"
            "  \"keypoolsize\": xxxx,        (numeric) how many new keys are pre-generated\n"
            "  \"unlocked_until\": ttt,      (numeric) the timestamp in seconds since epoch (midnight Jan 1 1970 GMT) that the wallet is unlocked for transfers, or 0 if the wallet is locked\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("getwalletinfo", "")
            + HelpExampleRpc("getwalletinfo", "")
        );

    Object obj;
    obj.push_back(Pair("walletversion", pwalletMain->GetVersion()));
    obj.push_back(Pair("balance",       ValueFromAmount(pwalletMain->GetBalance())));
    obj.push_back(Pair("txcount",       (int)pwalletMain->mapWallet.size()));
    //obj.push_back(Pair("keypoololdest", pwalletMain->GetOldestKeyPoolTime()));
    //obj.push_back(Pair("keypoolsize",   (int)pwalletMain->GetKeyPoolSize()));
    if (pwalletMain->IsCrypted())
        obj.push_back(Pair("unlocked_until", nWalletUnlockTime));
    return obj;
}
Value getcontacts(const json_spirit::Array& params, bool fHelp){
    return Value::null;
}
Value addcontacts(const json_spirit::Array& params, bool fHelp){
    if (fHelp || params.size() !=2)
        throw runtime_error("Wrong number of parameters");
    if (params[0].type() != str_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter1, expected string");
    CPubKey ID=AccountFromValue(params[0].get_str());
    LogPrintf("addcontacts 1\n");
    if (params[1].type() != array_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter2, expected array");
    Array arrContact=params[1].get_array();  
    //id-(pubkey,alias)
    std::map<string,Object> mapContact;
    for(unsigned int i=0;i<arrContact.size();i++){
        if(arrContact[i].type()!=obj_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter contact, expected object");
        Object obj=arrContact[i].get_obj();        
        Value tmp;
        tmp=find_value(obj, "id");
        if (tmp.type()!=str_type)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter id, expected string");        
       
        string strContactID=tmp.get_str();
        LogPrintf("addcontacts id:%s\n",strContactID);
        tmp=find_value(obj, "alias");
        Object objInfo;        
//        string strAlias="";
        if (tmp.type()!=null_type)
        {
            if (tmp.type()!=str_type)
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid parameter alias, expected string");        
//            strAlias=tmp.get_str();
            objInfo.push_back(Pair("alias",tmp));
        }
        mapContact.insert(make_pair(strContactID,objInfo));  
        //TODO add time
    }
    LogPrintf("addcontacts mapontact size:%u\n",mapContact.size());
    CWallet* pwallet=new CWallet(ID,false);
    
//    for(std::map<string,Object > ::iterator it=mapContact.begin();it!=mapContact.end();it++)
//    {
//        CPubKey pub;
//        if(pwallet->GetContactPubKey(it->first,pub))
//        {
//            LogPrintf("addcontacts pubkey:%s\n",CBitcoinAddress(pub).ToString());
//            it->second.push_back(Pair("pubkey",Value(CBitcoinAddress(pub).ToString())));                     
//            continue;
//        }
            
//         CScript script;
//        StringToScriptPubKey(it->first,script);        
        //if(!GetPubKeyFromBlockChain(script,pub))
        //     throw JSONRPCError(RPC_INVALID_PARAMETER, "pubkey not found"); 
        //LogPrintf("addcontacts pubkey:%s\n",CBitcoinAddress(pub).ToString());
        //it->second.push_back(Pair("pubkey",Value(CBitcoinAddress(pub).ToString())));                    
    //}
    pwallet->AddContacts(mapContact);
    Array result;
    for(std::map<string,Object>::iterator it=mapContact.begin();it!=mapContact.end();it++)
    {
        //Object contact;
        //contact.push_back(Pair("id",it->first));
        //contact.push_back(Pair("pubkey",find_value(it->second,"pubkey")));
        result.push_back(Value(it->first));
    }
    delete pwallet;
    return Value(result);
}