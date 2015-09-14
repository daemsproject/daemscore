// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_WALLETVIEW_H
#define BITCOIN_QT_WALLETVIEW_H

#include "amount.h"
#include <QUrl>
#include <QTabWidget>
#include <QWebPage>
#include <QtWidgets/QTabBar>

#include <QtWidgets/QShortcut>
#include "json/json_spirit_value.h"

class BitcoinGUI;
class ClientModel;
class JsInterface;
class WebView;
//sclass HistoryManager;
//class OverviewPage;
//class ReceiveCoinsDialog;
//class SendCoinsDialog;
//class SendCoinsRecipient;
//class TransactionView;
//class WalletModel;

QT_BEGIN_NAMESPACE
class QModelIndex;
class QProgressDialog;
class QCompleter;
class QLineEdit;
class QMenu;
class QStackedWidget;

QT_END_NAMESPACE

/*
  MainView class. This class represents the view to a single wallet.
  It was added to support multiple wallet functionality. Each wallet gets its own MainView instance.
  It communicates with both the client and the wallet models to give the user an up-to-date view of the
  current core state.
*/

class TabBar : public QTabBar
{
    Q_OBJECT

signals:
    void newTab();
    void cloneTab(int index);
    void closeTab(int index);
    void closeOtherTabs(int index);
    void reloadTab(int index);
    void reloadAllTabs();
    void tabMoveRequested(int fromIndex, int toIndex);

public:
    TabBar(QWidget *parent = 0);

protected:
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

private slots:
    void selectTabAction();
    void cloneTab();
    void closeTab();
    void closeOtherTabs();
    void reloadTab();
    void contextMenuRequested(const QPoint &position);

private:
    QList<QShortcut*> m_tabShortcuts;
    friend class MainView;

    QPoint m_dragStartPos;
    int m_dragCurrentIndex;
};
class WebActionMapper : public QObject
{
    Q_OBJECT

public:
    WebActionMapper(QAction *root, QWebPage::WebAction webAction, QObject *parent);
    QWebPage::WebAction webAction() const;
    void addChild(QAction *action);
    void updateCurrent(QWebPage *currentParent);

private slots:
    void rootTriggered();
    void childChanged();
    void rootDestroyed();
    void currentDestroyed();

private:
    QWebPage *m_currentParent;
    QAction *m_root;
    QWebPage::WebAction m_webAction;
};

class MainView : public QTabWidget
{
    Q_OBJECT
signals:
    
    /** Signal that we want to show the main window */
    void showNormalIfMinimized();
    /**  Fired when a message should be reported to the user */
    void message(const QString &title, const QString &message, unsigned int style);
    /** Encryption status of wallet changed */
    //void encryptionStatusChanged(int status);
    /** Notify that a new transaction appeared */
    //void incomingTransaction(const QString& date, int unit, const CAmount& amount, const QString& type, const QString& address);
    void loadPage(const QString &url);
    void tabsChanged();
    void lastTabClosed();

    // current tab signals
    void setCurrentTitle(const QString &url);
    void showStatusBarMessage(const QString &message);
    void linkHovered(const QString &link);
    void loadProgress(int progress);
    void geometryChangeRequested(const QRect &geometry);
    void menuBarVisibilityChangeRequested(bool visible);
    void statusBarVisibilityChangeRequested(bool visible);
    void toolBarVisibilityChangeRequested(bool visible);
//#if defined(QWEBPAGE_PRINTREQUESTED)
    void printRequested(QWebFrame *frame);
//#endif
     
public:
    explicit MainView(QString languageIn,BitcoinGUI *parent,JsInterface *_js=0);
    ~MainView();
    QString language;
    JsInterface *jsInterface;
    std::vector<WebView*> vWebPages;
    //void setBitcoinGUI(BitcoinGUI *gui);
    /** Set the client model.
        The client model represents the part of the core that communicates with the P2P network, and is wallet-agnostic.
    */
    //void setClientModel(ClientModel *clientModel);
    /** Set the wallet model.
        The wallet model represents a bitcoin wallet, and offers access to the list of transactions, address book and sending
        functionality.
    */
    //void setWalletModel(WalletModel *walletModel);

    //bool handlePaymentRequest(const SendCoinsRecipient& recipient);

    //void showOutOfSyncWarning(bool fShow);
    void gotoWebPage(int nPageID,QUrl url=QUrl(""),int nFromPageID=0);
    
    void installWebPage(const std::string strPageName);
    void loadWebPage(int nPageID);
    bool copyQrcToDisc(std::string app,std::string to,std::string from);
    std::string qrcFileToString(const std::string fileName);
    
    
    WebView* currentWebView() const;
    QLineEdit *currentLineEdit() const;
    QWidget *lineEditStack() const;
    QAction *nextTabAction() const;
    QAction *previousTabAction() const;
    QLineEdit *lineEdit(int index) const;
    WebView *getWebView(int index) const;
    void addWebAction(QAction *action, QWebPage::WebAction webAction);
    
    QAction *newTabAction() const;
    QAction *closeTabAction() const;
    QAction *recentlyClosedTabsAction() const;
    int webViewIndex(WebView *webView) const;
    QByteArray saveState() const;
    bool restoreState(const QByteArray &state);
    
protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);    
    
private:
    //ClientModel *clientModel;
    //WalletModel *walletModel;
    QProgressDialog *progressDialog;
    
    QAction *m_recentlyClosedTabsAction;
    QAction *m_newTabAction;
    QAction *m_closeTabAction;
    QAction *m_nextTabAction;
    QAction *m_previousTabAction;

    QMenu *m_recentlyClosedTabsMenu;
    static const int m_recentlyClosedTabsSize = 10;
    QList<QUrl> m_recentlyClosedTabs;
    QList<WebActionMapper*> m_actions;

    QCompleter *m_lineEditCompleter;
    QStackedWidget *m_lineEdits;
    TabBar *m_tabBar;

public slots:
    void gotoCustomPage(QUrl url,int nFromPageID);
    //void closeWebPage(int nPageID,int nSwitchToPageID=0);
    
    //void encryptWallet(bool status);
    /** Backup the wallet */
    //void backupWallet();
    /** Change encrypted wallet passphrase */
    //void changePassphrase();
    /** Ask for passphrase to unlock wallet temporarily */
    //void unlockWallet();

    /** Show used sending addresses */
    //void usedSendingAddresses();
    /** Show used receiving addresses */
    //void usedReceivingAddresses();

    /** Re-emit encryption status signal */
    //void updateEncryptionStatus();

    /** Show progress dialog e.g. for rescan */
    void showProgress(const QString &title, int nProgress);
    void loadUrlInCurrentTab(const QUrl &url);
    WebView *newTab(bool makeCurrent = true,QUrl url=QUrl(),int nPageID=255);
    void cloneTab(int index = -1);
    void closeTab(int index = -1);
    void closeOtherTabs(int index);
    void reloadTab(int index = -1);
    void reloadAllTabs();
    void nextTab();
    void previousTab();
    bool openPageIfNotExists(int nPageIDin);
        
private slots:
    void moveTab(int fromIndex, int toIndex);
    void currentChanged(int index);
    void aboutToShowRecentTabsMenu();
    void aboutToShowRecentTriggeredAction(QAction *action);
    void webViewLoadStarted();
    void webViewIconChanged();
    void webViewTitleChanged(const QString &title);
    void webViewUrlChanged(const QUrl &url);
    void lineEditReturnPressed();
    void windowCloseRequested();
    
};

#endif // BITCOIN_QT_WALLETVIEW_H
