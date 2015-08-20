// Copyright (c) 2011-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_BITCOINGUI_H
#define BITCOIN_QT_BITCOINGUI_H

#if defined(HAVE_CONFIG_H)
#include "config/bitcoin-config.h"
#endif

#include "amount.h"
#include "primitives/transaction.h"
//#include "wallet.h"
#include "jsinterface.h"
#include "allocators.h"
#include <QLabel>
#include <QMainWindow>
#include <QMap>
#include <QMenu>
#include <QPoint>
#include <QSystemTrayIcon>
#include <QtGui/QIcon>


class ClientModel;
class NetworkStyle;
class Notificator;
//class OptionsModel;
//class RPCConsole;

//class UnitDisplayStatusBarControl;
class MainView;
class WebView;
class WalletModel;
//class PaymentRequest;
//class CWalletTx;
//class CWallet;

QT_BEGIN_NAMESPACE
class QAction;
class QProgressBar;
class QProgressDialog;
class QNetworkAccessManager;
class QLocalServer;
class QWebFrame;
QT_END_NAMESPACE
/**
  Bitcoin GUI main class. This class represents the main window of the Bitcoin UI. It communicates with both the client and
  wallet models to give the user an up-to-date view of the current core state.
*/
typedef std::basic_string<char, std::char_traits<char>, secure_allocator<char> > SecureString;


class ToolbarSearch;
class ChaseWidget;
class DownloadManager;
class BookmarksManager;
class CookieJar;
class HistoryManager;
class AutoSaver;
class BookmarksToolBar;

class BitcoinGUI : public QMainWindow
{
    Q_OBJECT

public:
    static const QString DEFAULT_WALLET;

    explicit BitcoinGUI(const NetworkStyle *networkStyle,QString languageIn, QWidget *parent = 0);
    ~BitcoinGUI();
    
    /** Set the client model.
        The client model represents the part of the core that communicates with the P2P network, and is wallet-agnostic.
    */
    void setClientModel(ClientModel *clientModel);
    void installWebPages();
    
    
#ifdef ENABLE_WALLET
    /** Set the wallet model.
        The wallet model represents a bitcoin wallet, and offers access to the list of transactions, address book and sending
        functionality.
    */
    
    bool addWallet(const QString& name,WalletModel *walletModelIn);
    bool setCurrentWallet(const QString& name);
    void removeAllWallets();
    void subscribeToCoreSignalsJs();
#endif // ENABLE_WALLET
    bool enableWallet;
    QString language;
    bool handleUserConfirm(QString title,QString message,int nOP,string& strError,SecureString& ssInput,const int nPageIndex=-1);
    JsInterface* jsInterface;
    
    
    
    
    
    QIcon icon(const QUrl &url) const;
    void saveSession();
    bool canRestoreSession() const;
    MainView *getMainView() const;
    WebView *currentTab() const;
    QIcon defaultIcon() const;
    static HistoryManager *historyManager();
    static CookieJar *cookieJar();
    static DownloadManager *downloadManager();
    static QNetworkAccessManager *networkAccessManager();
    static BookmarksManager *bookmarksManager();
    
    QSize sizeHint() const;
    static const char *defaultHome;
    QByteArray saveState(bool withTabs = true) const;
    bool restoreState(const QByteArray &state);
protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    bool eventFilter(QObject *object, QEvent *event);

private:
    ClientModel *clientModel;
    MainView *mainView;
    WalletModel *walletModel;
    //UnitDisplayStatusBarControl *unitDisplayControl;
    QLabel *labelEncryptionIcon;
    QLabel *labelConnectionsIcon;
    QLabel *labelBlocksIcon;
    QLabel *progressBarLabel;
    QProgressBar *progressBar;
    QProgressDialog *progressDialog;

    QMenuBar *appMenuBar;
    QToolBar *toolbar;
    
    
    
    
    
    QAction *walletAction;
    QAction *browserAction;
    QAction *publisherAction;
    QAction *messengerAction;
    QAction *minerAction;
    QAction *shopAction;
    QAction *domainNameAction;
    QAction *downloaderAction;
    QAction *appsAction;
    QAction *toolsAction;
    QAction *quitAction;
    
    //QAction *usedSendingAddressesAction;
    //QAction *usedReceivingAddressesAction;    /
    QAction *aboutAction;
    
    //QAction *optionsAction;
    QAction *toggleHideAction;        
    QAction *changePassphraseAction;
    QAction *newAccountAction;
    QAction *switchAccountAction;
    QAction *unlockAccountAction;
    QAction *lockAccountAction;
    QAction *encryptAccountAction;        
    QAction *decryptAccountAction;        
    QAction *exportAccountAction;
    QAction *importAccountAction;
    //QAction *aboutQtAction;
    //QAction *openRPCConsoleAction;
    //QAction *openAction;
    QAction *showHideTabBarAction;
    QAction *showHelpMessageAction;
    QAction *settingsAction;
    QAction *helpAction;
    QAction *serviceManagerAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    Notificator *notificator;
    //RPCConsole *rpcConsole;

    /** Keep track of previous number of blocks, to detect progress */
    int prevBlocks;
    int spinnerFrame;
    
    
    
    
    mutable QIcon m_defaultIcon;
    static DownloadManager *s_downloadManager;
    static QNetworkAccessManager *s_networkAccessManager;
    static HistoryManager *s_historyManager;
    static BookmarksManager *s_bookmarksManager;    
    QByteArray m_lastSession;    
    
    
    QToolBar *m_navigationBar;
    ToolbarSearch *m_toolbarSearch;
    QAction *m_historyBack;
    QMenu *m_historyBackMenu;
    QAction *m_historyForward;
    QMenu *m_historyForwardMenu;
    QAction *m_stop;
    QAction *m_reload;
    QAction *m_stopReload;
    QIcon m_reloadIcon;
    QIcon m_stopIcon;
    QMenu *m_windowMenu;
    ChaseWidget *m_chaseWidget;
    QAction *m_viewToolbar;
    
    
    BookmarksToolBar *m_bookmarksToolbar;   
    AutoSaver *m_autoSaver;
    
    QAction *m_viewBookmarkBar;
    QAction *m_viewStatusbar;
    QAction *m_restoreLastSession;
    QAction *m_addBookmark;    

    QString m_lastSearch;
    
    /** Create the main UI actions. */
    void createActions(const NetworkStyle *networkStyle);
    /** Create the menu bar and sub-menus. */
    void createMenuBar();
    /** Create the toolbars */
    void createToolBars();
    /** Create system tray icon and notification */
    void createTrayIcon(const NetworkStyle *networkStyle);
    /** Create system tray menu (or setup the dock menu) */
    void createTrayIconMenu();

    /** Enable or disable all wallet-related actions */
    void setWalletActionsEnabled(bool enabled);

    /** Connect core signals to GUI client */
    void subscribeToCoreSignals();
    /** Disconnect core signals from GUI client */
    void unsubscribeFromCoreSignals(); 
    
    
    
    
   void loadDefaultState();
    
    //void setupToolBar();
    void updateStatusbarActionText(bool visible);
    void handleFindTextResult(bool found);
    
signals:
    /** Signal raised when a URI was entered or dragged to the GUI */
    void receivedURI(const QString &uri);
    void sendMoneyResult(std::string str,bool fSuccess,QString data);
    
    
public slots:
    /** Set number of connections shown in the UI */
    void setNumConnections(int count);
    /** Set number of blocks shown in the UI */
    void setNumBlocks(int count);

    /** Notify the user of an event from the core network or transaction handling code.
       @param[in] title     the message box / notification title
       @param[in] message   the displayed text
       @param[in] style     modality and style definitions (icon and used buttons - buttons only for message boxes)
                            @see CClientUIInterface::MessageBoxFlags
       @param[in] ret       pointer to a bool that will be modified to whether Ok was clicked (modal only)
    */
    void message(const QString &title, const QString &message, unsigned int style, bool *ret = NULL);

//#ifdef ENABLE_WALLET
    /** Set the encryption status as shown in the UI.
       @param[in] status            current encryption status
       @see WalletModel::EncryptionStatus
    */
    void setEncryptionStatus(int status);
    
      

    /** Show incoming transaction notification for new transactions. */
    void incomingTransaction(const QString& date, int unit, const CAmount& amount, const QString& type, const QString& address);
    
    
    void loadPage(const QString &url);    
    void slotHome();
   void restoreLastSession();
   
#if defined(Q_OS_OSX)
    void lastWindowClosed();
    void quitBrowser();
#endif
    
//#endif // ENABLE_WALLET

private slots:
#ifdef ENABLE_WALLET
    void gotoWalletPage();    
    void gotoPublisherPage();  
    void gotoMessengerPage();  
    void gotoMinerPage();  
    

    

    /** Show open dialog */
    void openClicked();
#endif // ENABLE_WALLET
    /** Show configuration dialog */
    //void optionsClicked();
   
   
   void backupWallet();
   void importWallet();
   void encryptWallet();
   void decryptWallet();
    void changePassphrase();
   void newAccountClicked();
   void switchAccountClicked();
   void unlockAccountClicked();
   void lockAccountClicked();
   void domainNameClicked();
    void gotoBrowserPage();  
    void gotoShopPage();
    void gotoDownloaderPage();
    void gotoSettingsPage();
    void gotoAppsPage();
    void gotoToolsPage();
    void gotoHelpPage();
    /** Show about dialog */    
    void aboutClicked();
    void showHideTabBarClicked();
    /** Show help message dialog */
    void showHelpMessageClicked();
#ifndef Q_OS_MAC
    /** Handle tray icon clicked */
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
#endif

    /** Show window if hidden, unminimize when minimized, rise when obscured or show if hidden and fToggleHidden is true */
    void showNormalIfMinimized(bool fToggleHidden = false);
    /** Simply calls showNormalIfMinimized(true) for use in SLOT() macro */
    void toggleHidden();

    /** called by a timer to check if fRequestShutdown has been set **/
    void detectShutdown();

    /** Show progress dialog e.g. for verifychain */
    void showProgress(const QString &title, int nProgress);
    
    
    
    
    void openUrl(const QUrl &url);
    
    void updateToolbarActionText(bool visible);
    void slotAboutToShowBackMenu();
    void slotAboutToShowForwardMenu();
    
    void slotShowWindow();
    void loadUrl(const QUrl &url);
    void save();

    void slotLoadProgress(int);
    void slotUpdateStatusbar(const QString &string);
    void slotUpdateWindowTitle(const QString &title = QString());

    
    void slotPreferences();

    void slotFileNew();
    void slotFileOpen();
    void slotFilePrintPreview();
    void slotFilePrint();
    void slotPrivateBrowsing();
    void slotFileSaveAs();
    void slotEditFind();
    void slotEditFindNext();
    void slotEditFindPrevious();
    void slotShowBookmarksDialog();
    void slotAddBookmark();
    void slotViewZoomIn();
    void slotViewZoomOut();
    void slotViewResetZoom();
    void slotViewToolbar();
    void slotViewBookmarksBar();
    void slotViewStatusbar();
    void slotViewPageSource();
    void slotViewFullScreen(bool enable);

    void slotWebSearch();
    void slotToggleInspector(bool enable);
    void slotAboutApplication();
    void slotDownloadManager();
    void slotSelectLineEdit();
    
    void slotOpenActionUrl(QAction *action);    
    void slotSwapFocus();

//#if defined(QWEBPAGE_PRINT)
    void printRequested(QWebFrame *frame);
//#endif
    void geometryChangeRequested(const QRect &geometry);
    void updateBookmarksToolbarActionText(bool visible);
};

//class UnitDisplayStatusBarControl : public QLabel
//{
//    Q_OBJECT
//
//public:
//    explicit UnitDisplayStatusBarControl();
//    /** Lets the control know about the Options Model (and its signals) */
//    //void setOptionsModel(OptionsModel *optionsModel);
//
//protected:
//    /** So that it responds to left-button clicks */
//    void mousePressEvent(QMouseEvent *event);
//
//private:
//    //OptionsModel *optionsModel;
//    QMenu* menu;
//
//    /** Shows context menu with Display Unit options by the mouse coordinates */
//    void onDisplayUnitsClicked(const QPoint& point);
//    /** Creates context menu, its actions, and wires up all the relevant signals for mouse events. */
//    void createContextMenu();
//
//private slots:
//    /** When Display Units are changed on OptionsModel it will refresh the display text of the control on the status bar */
//    void updateDisplayUnit(int newUnits);
//    /** Tells underlying optionsModel to update its current display unit. */
//    void onMenuSelection(QAction* action);
//};

#endif // BITCOIN_QT_BITCOINGUI_H
