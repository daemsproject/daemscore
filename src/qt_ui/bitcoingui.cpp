// Copyright (c) 2011-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "browser.h"
#include "mainview.h"
#include "bitcoinunits.h"
#include "clientmodel.h"
#include "guiconstants.h"
#include "guiutil.h"
#include "networkstyle.h"
#include "notificator.h"
//#include "openuridialog.h"
//#include "optionsdialog.h"
//#include "optionsmodel.h"
//#include "rpcconsole.h"
#include "utilitydialog.h"
//#include "rpcserver.h"
#include "userconfirmdialog.h"
#include "ui_userconfirmdialog.h"
#include "accountdialog.h"
#include "webpage.h"
#include "fai/settings.h"
#include "fai/contentutil.h"
#include "toolbarsearch.h"
#include "chasewidget.h"
#include "bookmarks.h"
#include "cookiejar.h"
#include "history.h"
#include "downloadmanager.h"
#include "networkaccessmanager.h"
#include "autosaver.h"
#include "settings2.h"
#include "ui_passworddialog.h"
#include <QtGui/QIcon>
#include <QWebHistory>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QSslSocket>
#include <QtCore/QBuffer>
#include <QtCore/QDir>
#include <QtCore/QLibraryInfo>
#include <QtCore/QSettings>
#include <QtCore/QTextStream>
#include <QtCore/QTranslator>
#include <QtCore/QDebug>
#include <QtGui/QDesktopServices>
#include <QtGui/QFileOpenEvent>
#include <QtWidgets/QMessageBox>
#include <QWebSettings>
#include <QWebFrame>
#include <QtCore/QDebug>
#include <QAction>
#include <QApplication>
#include <QDateTime>

#include <QDragEnterEvent>
#include <QIcon>
#include <QListWidget>


#include <QMimeData>
#include <QProgressBar>
#include <QProgressDialog>
#include <QSettings>
#include <QStackedWidget>

#include <QStyle>
#include <QTimer>

#include <QVBoxLayout>
#include <QSettings>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QPlainTextEdit>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrintPreviewDialog>
#include <QtPrintSupport/QPrinter>
#include <QtWidgets/QInputDialog>
#include <QWidget>

#include <QtWidgets/QDesktopWidget>


#include <QtWidgets/QMenuBar>

#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QToolButton>







#ifdef ENABLE_WALLET
//#include "wallet.h"

#include "walletmodel.h"

#endif // ENABLE_WALLET

#ifdef Q_OS_MAC
#include "macdockiconhandler.h"
#endif

#include "init.h"
#include "ui_interface.h"
#include "util.h"

#include <iostream>
#include <boost/assign.hpp>





#if QT_VERSION < 0x050000
#include <QTextDocument>
#include <QUrl>
#else
#include <QUrlQuery>
#include <bits/stl_pair.h>
#endif

template<typename Arg, typename R, typename C>
struct InvokeWrapper {
    R *receiver;
    void (C::*memberFun)(Arg);
    void operator()(Arg result) {
        (receiver->*memberFun)(result);
    }
};

template<typename Arg, typename R, typename C>
InvokeWrapper<Arg, R, C> invoke(R *receiver, void (C::*memberFun)(Arg))
{
    InvokeWrapper<Arg, R, C> wrapper = {receiver, memberFun};
    return wrapper;
}
//const QString BitcoinGUI::DEFAULT_WALLET = "~Default";
DownloadManager *BitcoinGUI::s_downloadManager = 0;
QNetworkAccessManager *BitcoinGUI::s_networkAccessManager = 0;
HistoryManager *BitcoinGUI::s_historyManager = 0;
BookmarksManager *BitcoinGUI::s_bookmarksManager = 0;
const char *BitcoinGUI::defaultHome = "fai:browser";
BitcoinGUI::BitcoinGUI(const NetworkStyle *networkStyle,QString languageIn,  QWidget *parent) :
    QMainWindow(parent),
    language(languageIn),
    mainView(0),
    clientModel(0),    
    //unitDisplayControl(0),
    labelEncryptionIcon(0),
    labelConnectionsIcon(0),
    labelBlocksIcon(0),
    progressBarLabel(0),
    progressBar(0),
    progressDialog(0),
    toolbar(0),
    appMenuBar(0),
    walletAction(0),
    browserAction(0),
    publisherAction(0),
    messengerAction(0),
    minerAction(0),
    shopAction(0),
    domainNameAction(0),
    newAccountAction(0),
    switchAccountAction(0),
    unlockAccountAction(0),
    encryptAccountAction(0),        
    exportAccountAction(0), 
    importAccountAction(0), 
    downloaderAction(0),
        appsAction(0),
    toolsAction(0),    
    quitAction(0),    
    //usedSendingAddressesAction(0),
    //usedReceivingAddressesAction(0),
    
    aboutAction(0),    
    //optionsAction(0),
    toggleHideAction(0),
    changePassphraseAction(0),
    //aboutQtAction(0),
    //openRPCConsoleAction(0),
    //openAction(0),
    showHideTabBarAction(0),
    showHelpMessageAction(0),
    settingsAction(0),
    helpAction(0),
    serviceManagerAction(0),
    trayIcon(0),
    trayIconMenu(0),
    notificator(0),
    //rpcConsole(0),
    prevBlocks(0),
    spinnerFrame(0),
    m_historyBack(0)
    , m_historyForward(0)
    , m_stop(0)
    , m_reload(0)
    , m_autoSaver(new AutoSaver(this))
{
    jsInterface=new JsInterface(this);
    GUIUtil::restoreWindowGeometry("nWindow", QSize(850, 550), this);

    QString windowTitle = tr("Φ DeskTop Client");
#ifdef ENABLE_WALLET
    /* if compiled with wallet support, -disablewallet can still disable the wallet */
    enableWallet = !GetBoolArg("-disablewallet", false);
#else
    enableWallet = false;
#endif // ENABLE_WALLET

    windowTitle += " " + networkStyle->getTitleAddText();
#ifndef Q_OS_MAC
    QApplication::setWindowIcon(networkStyle->getAppIcon());
    setWindowIcon(networkStyle->getAppIcon());
#else
    MacDockIconHandler::instance()->setIcon(networkStyle->getAppIcon());
#endif
    setWindowTitle(windowTitle);

#if defined(Q_OS_MAC) && QT_VERSION < 0x050000
    // This property is not implemented in Qt 5. Setting it has no effect.
    // A replacement API (QtMacUnifiedToolBar) is available in QtMacExtras.
    setUnifiedTitleAndToolBarOnMac(true);
#endif
#ifndef QT_NO_OPENSSL
    if (!QSslSocket::supportsSsl()) {
    QMessageBox::information(0, "Φ DeskTop Client",
                 "This system does not support OpenSSL. SSL websites will not be available.");
    }
#endif
    QSettings settings;
    settings.beginGroup(QLatin1String("sessions"));
    m_lastSession = settings.value(QLatin1String("lastSession")).toByteArray();
    settings.endGroup();
    //rpcConsole = new RPCConsole(enableWallet ? this : 0);

    setToolButtonStyle(Qt::ToolButtonFollowStyle);
    setAttribute(Qt::WA_DeleteOnClose, true);

    //LogPrintf("bitcoingui: 0 \n");
    mainView = new MainView(language,this,jsInterface);
    connect(mainView, SIGNAL(showNormalIfMinimized()), this, SLOT(showNormalIfMinimized()));

    QWidget *centralWidget = new QWidget(this);
    BookmarksModel *bookmarksModel = BitcoinGUI::bookmarksManager()->bookmarksModel();
    m_bookmarksToolbar = new BookmarksToolBar(bookmarksModel, this);
    connect(m_bookmarksToolbar, SIGNAL(openUrl(QUrl)),
            this, SLOT(loadUrl(QUrl)));
    connect(m_bookmarksToolbar->toggleViewAction(), SIGNAL(toggled(bool)),
            this, SLOT(updateBookmarksToolbarActionText(bool)));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    toolbar = addToolBar(tr("Tabs toolbar"));
#if defined(Q_OS_OSX)
    layout->addWidget(m_bookmarksToolbar);
    layout->addWidget(new QWidget); // <- OS X tab widget style bug
#else
    
   
    addToolBar(m_bookmarksToolbar);
    addToolBarBreak();
     m_navigationBar = addToolBar(tr("Navigation"));
#endif
    layout->addWidget(mainView);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);       
     
    statusBar()->setSizeGripEnabled(true);
    //setupMenu();
    //setupToolBar();

    // Accept D&D of URIs
    setAcceptDrops(true);
//LogPrintf("bitcoingui: 1 \n");
    // Create actions for the toolbar, menu bar and tray/dock icon
    // Needs mainView to be initialized
    createActions(networkStyle);
//LogPrintf("bitcoingui: 2 \n");
// Create application menu bar
    createMenuBar();

    
//LogPrintf("bitcoingui: 3 \n");
    // Create the toolbars
    createToolBars();
//LogPrintf("bitcoingui: 4 \n");
    // Create system tray icon and notification
    createTrayIcon(networkStyle);
//LogPrintf("bitcoingui: 5 \n");
    // Create status bar
    statusBar();
//LogPrintf("bitcoingui: 6 \n");
    // Status bar notification icons
    QFrame *frameBlocks = new QFrame();
    frameBlocks->setContentsMargins(0,0,0,0);
    frameBlocks->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    QHBoxLayout *frameBlocksLayout = new QHBoxLayout(frameBlocks);
    frameBlocksLayout->setContentsMargins(3,0,3,0);
    frameBlocksLayout->setSpacing(3);
    //unitDisplayControl = new UnitDisplayStatusBarControl();
    labelEncryptionIcon = new QLabel();
    labelConnectionsIcon = new QLabel();
    labelBlocksIcon = new QLabel();
    //LogPrintf("bitcoingui: 7 \n");
    if(enableWallet)
    {
        //frameBlocksLayout->addStretch();
        //frameBlocksLayout->addWidget(unitDisplayControl);
        frameBlocksLayout->addStretch();
        frameBlocksLayout->addWidget(labelEncryptionIcon);
    }
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelConnectionsIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelBlocksIcon);
    frameBlocksLayout->addStretch();
//LogPrintf("bitcoingui: 8 \n");
    // Progress bar and label for blocks download
    progressBarLabel = new QLabel();
    progressBarLabel->setVisible(false);
    progressBar = new GUIUtil::ProgressBar();
    progressBar->setAlignment(Qt::AlignCenter);
    progressBar->setVisible(false);
//LogPrintf("bitcoingui: 9 \n");
    // Override style sheet for progress bar for styles that have a segmented progress bar,
    // as they make the text unreadable (workaround for issue #1071)
    // See https://qt-project.org/doc/qt-4.8/gallery.html
    QString curStyle = QApplication::style()->metaObject()->className();
    if(curStyle == "QWindowsStyle" || curStyle == "QWindowsXPStyle")
    {
        progressBar->setStyleSheet("QProgressBar { background-color: #e8e8e8; border: 1px solid grey; border-radius: 7px; padding: 1px; text-align: center; } QProgressBar::chunk { background: QLinearGradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #FF8000, stop: 1 orange); border-radius: 7px; margin: 0px; }");
    }

    statusBar()->addWidget(progressBarLabel);
    statusBar()->addWidget(progressBar);
    statusBar()->addPermanentWidget(frameBlocks);
//LogPrintf("bitcoingui: 10 \n");
    //connect(openRPCConsoleAction, SIGNAL(triggered()), rpcConsole, SLOT(show()));

    // prevents an open debug window from becoming stuck/unusable on client shutdown
    //connect(quitAction, SIGNAL(triggered()), rpcConsole, SLOT(hide()));

    // Install event filter to be able to catch status tip events (QEvent::StatusTip)
    this->installEventFilter(this);
//LogPrintf("bitcoingui: 11 \n");
    // Initially wallet actions should be disabled
    setWalletActionsEnabled(false);
//LogPrintf("bitcoingui: 12 \n");
    // Subscribe to notifications from core
    subscribeToCoreSignals();
    //LogPrintf("bitcoingui: 13 \n");
    
    
    
    
    
    connect(mainView, SIGNAL(tabsChanged()),
            m_autoSaver, SLOT(changeOccurred()));
#if defined(Q_OS_OSX)
    connect(mainView, SIGNAL(lastTabClosed()),
            this, SLOT(close()));
#else
    connect(mainView, SIGNAL(lastTabClosed()),
            mainView, SLOT(newTab()));
#endif
    

    
    
    
    
   

    

    connect(mainView, SIGNAL(loadPage(QString)),
        this, SLOT(loadPage(QString)));
    connect(mainView, SIGNAL(setCurrentTitle(QString)),
        this, SLOT(slotUpdateWindowTitle(QString)));
    connect(mainView, SIGNAL(showStatusBarMessage(QString)),
            statusBar(), SLOT(showMessage(QString)));
    connect(mainView, SIGNAL(linkHovered(QString)),
            statusBar(), SLOT(showMessage(QString)));
    connect(mainView, SIGNAL(loadProgress(int)),
            this, SLOT(slotLoadProgress(int)));
    connect(mainView, SIGNAL(tabsChanged()),
            m_autoSaver, SLOT(changeOccurred()));
    connect(mainView, SIGNAL(geometryChangeRequested(QRect)),
            this, SLOT(geometryChangeRequested(QRect)));
//#if defined(QWEBPAGE_PRINTREQUESTED)
    connect(mainView, SIGNAL(printRequested(QWebFrame*)),
            this, SLOT(printRequested(QWebFrame*)));
//#endif
    connect(mainView, SIGNAL(menuBarVisibilityChangeRequested(bool)),
            menuBar(), SLOT(setVisible(bool)));
    connect(mainView, SIGNAL(statusBarVisibilityChangeRequested(bool)),
            statusBar(), SLOT(setVisible(bool)));
    connect(mainView, SIGNAL(toolBarVisibilityChangeRequested(bool)),
            m_navigationBar, SLOT(setVisible(bool)));
    connect(mainView, SIGNAL(toolBarVisibilityChangeRequested(bool)),
            m_bookmarksToolbar, SLOT(setVisible(bool)));


    slotUpdateWindowTitle();
    loadDefaultState();    
    //mainView->newTab();

    int size = mainView->lineEditStack()->sizeHint().height();
    //m_navigationBar->setIconSize(QSize(size, size));
    BitcoinGUI::historyManager();
}

BitcoinGUI::~BitcoinGUI()
{
    //LogPrintf("~bitcoingui start \n");
    // Unsubscribe from notifications from core
    unsubscribeFromCoreSignals();
    // LogPrintf("~bitcoingui start1 \n");
    delete s_downloadManager;
    // LogPrintf("~bitcoingui start 2\n");
    delete s_networkAccessManager;
   //  LogPrintf("~bitcoingui start 3\n");
    delete s_bookmarksManager;
   //  LogPrintf("~bitcoingui start4 \n");
    GUIUtil::saveWindowGeometry("nWindow", this);
    if(trayIcon) // Hide tray icon, as deleting will let it linger until quit (on Ubuntu)
        trayIcon->hide();
#ifdef Q_OS_MAC
    delete appMenuBar;
    MacDockIconHandler::instance()->setMainWindow(NULL);
    
#endif
     //LogPrintf("~bitcoingui start 5\n");
      //  delete jsInterface;
    jsInterface=NULL;
   // LogPrintf("~bitcoingui done \n");
}

void BitcoinGUI::createActions(const NetworkStyle *networkStyle)
{
    QActionGroup *tabGroup = new QActionGroup(this);
    walletAction = new QAction(QIcon(":/icons/wallet"), tr("Wallet"), this);
    walletAction->setStatusTip(tr("Show wallet page"));
    walletAction->setToolTip(walletAction->statusTip());
    //walletAction->setCheckable(true);
    walletAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_W));
    tabGroup->addAction(walletAction);    
    browserAction = new QAction(QIcon(":/icons/browser"), tr("Browser"), this);
    browserAction->setStatusTip(tr("Show browser page"));
    browserAction->setToolTip(browserAction->statusTip());
    //browserAction->setCheckable(true);
    browserAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_B));
    tabGroup->addAction(browserAction);
    publisherAction = new QAction(QIcon(":/icons/publisher"), tr("Publisher"), this);
    publisherAction->setStatusTip(tr("Show publisher page"));
    publisherAction->setToolTip(publisherAction->statusTip());
    //publisherAction->setCheckable(true);
    publisherAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_P));
    tabGroup->addAction(publisherAction);
    messengerAction = new QAction(QIcon(":/icons/chat"), tr("&Messenger"), this);
    messengerAction->setStatusTip(tr("Show messenger page"));
    messengerAction->setToolTip(messengerAction->statusTip());
   // messengerAction->setCheckable(true);
    messengerAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_C));
    tabGroup->addAction(messengerAction);
    minerAction = new QAction(QIcon(":/icons/tx_mined"), tr("&Miner"), this);
    minerAction->setStatusTip(tr("Show miner page"));
    minerAction->setToolTip(minerAction->statusTip());
   // minerAction->setCheckable(true);
    minerAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_M));
    tabGroup->addAction(minerAction);
    
    shopAction = new QAction(QIcon(":/icons/shop"), tr("&Shop"), this);
    shopAction->setStatusTip(tr("shop"));
   // shopAction->setCheckable(true);
    shopAction->setToolTip(shopAction->statusTip()); 
    shopAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_S));
    tabGroup->addAction(shopAction);
    downloaderAction = new QAction(QIcon(":/icons/receiving_addresses"), tr("&Downloader"), this);
    downloaderAction->setStatusTip(tr("Show downloader page"));
    downloaderAction->setToolTip(downloaderAction->statusTip());    
   // downloaderAction->setCheckable(true);
    downloaderAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_L)); 
    tabGroup->addAction(downloaderAction);
    appsAction = new QAction(QIcon(":/icons/app"), tr("&Apps"), this);
    appsAction->setStatusTip(tr("Apps"));
    appsAction->setToolTip(appsAction->statusTip());     
   // appsAction->setCheckable(true);
    appsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_A)); 
    tabGroup->addAction(appsAction);
    toolsAction = new QAction(QIcon(":/icons/tool"), tr("&Tools"), this);
    toolsAction->setStatusTip(tr("Tools"));
    toolsAction->setToolTip(toolsAction->statusTip());     
   // toolsAction->setCheckable(true);
    toolsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_T)); 
    tabGroup->addAction(toolsAction);
LogPrintf("bitcoingui:createactions 1 \n");
#ifdef ENABLE_WALLET
    // These showNormalIfMinimized are needed because Send Coins and Receive Coins
    // can be triggered from the tray menu, and need to show the GUI to be useful.
    connect(walletAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(walletAction, SIGNAL(triggered()), this, SLOT(gotoWalletPage()));
    connect(publisherAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(publisherAction, SIGNAL(triggered()), this, SLOT(gotoPublisherPage()));
    connect(messengerAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(messengerAction, SIGNAL(triggered()), this, SLOT(gotoMessengerPage()));
    connect(minerAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(minerAction, SIGNAL(triggered()), this, SLOT(gotoMinerPage()));    
    
#endif // ENABLE_WALLET
LogPrintf("bitcoingui:createactions 2 \n");
    quitAction = new QAction(QIcon(":/icons/quit"), tr("E&xit"), this);
    quitAction->setStatusTip(tr("Quit application"));
    quitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
    quitAction->setMenuRole(QAction::QuitRole);
    aboutAction = new QAction(networkStyle->getAppIcon(), tr("&About FAI"), this);
    aboutAction->setStatusTip(tr("Show information about Fai system"));
    aboutAction->setMenuRole(QAction::AboutRole);
#if QT_VERSION < 0x050000
   // aboutQtAction = new QAction(QIcon(":/trolltech/qmessagebox/images/qtlogo-64.png"), tr("About &Qt"), this);
#else
    //aboutQtAction = new QAction(QIcon(":/qt-project.org/qmessagebox/images/qtlogo-64.png"), tr("About &Qt"), this);
#endif
    //aboutQtAction->setStatusTip(tr("Show information about Qt"));
    //aboutQtAction->setMenuRole(QAction::AboutQtRole);
    //optionsAction = new QAction(QIcon(":/icons/options"), tr("&Options..."), this);
    //optionsAction->setStatusTip(tr("Modify configuration options for Faicoin"));
    //optionsAction->setMenuRole(QAction::PreferencesRole);
    toggleHideAction = new QAction(networkStyle->getAppIcon(), tr("&Show / Hide"), this);
    toggleHideAction->setStatusTip(tr("Show or hide the main Window"));
//LogPrintf("bitcoingui:createactions 3 \n");
    encryptAccountAction = new QAction(QIcon(":/icons/lock_closed"), tr("&Encrypt Account..."), this);
    encryptAccountAction->setStatusTip(tr("Encrypt the private keys that belong to your Account"));
    decryptAccountAction = new QAction(QIcon(":/icons/lock_open"), tr("&Decrypt Account..."), this);
    decryptAccountAction->setStatusTip(tr("Decrypt the private keys that belong to your Account"));
    exportAccountAction = new QAction(QIcon(":/icons/filesave"), tr("&Export account..."), this);
    exportAccountAction->setStatusTip(tr("export account to another location"));
//LogPrintf("bitcoingui:createactions 4 \n");     
    changePassphraseAction = new QAction(QIcon(":/icons/key"), tr("&Change Passphrase..."), this);
    changePassphraseAction->setStatusTip(tr("Change the passphrase used for wallet encryption"));
    newAccountAction = new QAction(QIcon(":/icons/key"), tr("&Create new account..."), this);
    newAccountAction->setStatusTip(tr("Create new account..."));
    switchAccountAction = new QAction(QIcon(":/icons/key"), tr("&Switch Account..."), this);
    switchAccountAction->setStatusTip(tr("switch account"));
      unlockAccountAction = new QAction(QIcon(":/icons/lock_open"), tr("&Unlock Account..."), this);
    unlockAccountAction->setStatusTip(tr("unlock account"));
      lockAccountAction = new QAction(QIcon(":/icons/lock_closed"), tr("&Lock Account..."), this);
    lockAccountAction->setStatusTip(tr("Lock account"));
    
    domainNameAction = new QAction(QIcon(":/icons/key"), tr("&Domain name manage..."), this);
    domainNameAction->setStatusTip(tr("Domain name manage"));
    domainNameAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_D)); 
    exportAccountAction = new QAction(QIcon(":/icons/key"), tr("&Export Account"), this);
    exportAccountAction->setStatusTip(tr("Export Account"));
    importAccountAction = new QAction(QIcon(":/icons/key"), tr("&Import Account"), this);
    importAccountAction->setStatusTip(tr("Import Account"));

    //openRPCConsoleAction = new QAction(QIcon(":/icons/debugwindow"), tr("&Debug window"), this);
    //openRPCConsoleAction->setStatusTip(tr("Open debugging and diagnostic console"));

//    usedSendingAddressesAction = new QAction(QIcon(":/icons/address-book"), tr("&Sending addresses..."), this);
//    usedSendingAddressesAction->setStatusTip(tr("Show the list of used sending addresses and labels"));
//    usedReceivingAddressesAction = new QAction(QIcon(":/icons/address-book"), tr("&Receiving addresses..."), this);
//    usedReceivingAddressesAction->setStatusTip(tr("Show the list of used receiving addresses and labels"));

    //openAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_FileIcon), tr("Open &URI..."), this);
    //openAction->setStatusTip(tr("Open a faicoin: URI or payment request"));
//    showHideTabBarAction= new QAction(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation), tr("&Show/Hide tab bar"), this);
//    showHideTabBarAction->setStatusTip(tr("Show/Hide tab bar"));
//    showHideTabBarAction->setCheckable(true);
    showHelpMessageAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation), tr("&Command-line options"), this);
    showHelpMessageAction->setStatusTip(tr("Show the Φ DeskTop Client help message to get a list with possible Fai command-line options"));
    settingsAction = new QAction(QIcon(":/icons/key"), tr("&Settings"), this);
    settingsAction->setStatusTip(tr("Settings"));
    settingsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_S)); 
    helpAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation), tr("&Help"), this);
    helpAction->setStatusTip(tr("Help"));
    helpAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_H)); 
    serviceManagerAction = new QAction(QIcon(":/icons/key"), tr("&Service Manager"), this);
    serviceManagerAction->setStatusTip(tr("Service Manager"));
//LogPrintf("bitcoingui:createactions 5 \n");
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(aboutClicked()));
    connect(browserAction, SIGNAL(triggered()), this, SLOT(gotoBrowserPage()));    
    connect(shopAction, SIGNAL(triggered()), this, SLOT(gotoShopPage()));    
    connect(downloaderAction, SIGNAL(triggered()), this, SLOT(gotoDownloaderPage()));   
    connect(appsAction, SIGNAL(triggered()), this, SLOT(gotoAppsPage()));    
    connect(toolsAction, SIGNAL(triggered()), this, SLOT(gotoToolsPage()));    
    //connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    //connect(optionsAction, SIGNAL(triggered()), this, SLOT(optionsClicked()));
    connect(toggleHideAction, SIGNAL(triggered()), this, SLOT(toggleHidden()));
    //connect(showHideTabBarAction, SIGNAL(triggered()), this, SLOT(showHideTabBarClicked()));
    connect(showHelpMessageAction, SIGNAL(triggered()), this, SLOT(showHelpMessageClicked()));
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(gotoSettingsPage()));
    connect(helpAction, SIGNAL(triggered()), this, SLOT(gotoHelpPage()));
    connect(serviceManagerAction, SIGNAL(triggered()), this, SLOT(gotoSettingsPage()));
    //LogPrintf("bitcoingui:createactions 6 \n");
#ifdef ENABLE_WALLET
    
        connect(encryptAccountAction, SIGNAL(triggered()), this, SLOT(encryptWallet()));
        connect(decryptAccountAction, SIGNAL(triggered()), this, SLOT(decryptWallet()));
        //LogPrintf("bitcoingui:createactions 7 \n");
        connect(exportAccountAction, SIGNAL(triggered()), this, SLOT(backupWallet()));
        connect(importAccountAction, SIGNAL(triggered()), this, SLOT(importWallet()));
        //LogPrintf("bitcoingui:createactions 8 \n");
        connect(changePassphraseAction, SIGNAL(triggered()), this, SLOT(changePassphrase()));
        //LogPrintf("bitcoingui:createactions 9 \n");
         connect(newAccountAction, SIGNAL(triggered()), this, SLOT(newAccountClicked()));
         //LogPrintf("bitcoingui:createactions 10 \n");
          connect(switchAccountAction, SIGNAL(triggered()), this, SLOT(switchAccountClicked()));
          connect(lockAccountAction, SIGNAL(triggered()), this, SLOT(lockAccountClicked()));
          connect(unlockAccountAction, SIGNAL(triggered()), this, SLOT(unlockAccountClicked()));
          //LogPrintf("bitcoingui:createactions 11 \n");
           connect(domainNameAction, SIGNAL(triggered()), this, SLOT(domainNameClicked()));
           //LogPrintf("bitcoingui:createactions 12 \n");
    
#endif // ENABLE_WALLET
}

void BitcoinGUI::createMenuBar()
{
    new QShortcut(QKeySequence(Qt::Key_F6), this, SLOT(slotSwapFocus()));
#ifdef Q_OS_MAC
    // Create a decoupled menu bar on Mac which stays even if the window is closed
    appMenuBar = new QMenuBar();
#else
    // Get the main window's menu bar on other platforms
    appMenuBar = menuBar();
#endif

    // Configure the menus
    
    QMenu *fileMenu = appMenuBar->addMenu(tr("&File"));  
        //file->addAction(openAction);
        //file->addAction(backupWalletAction);        
        //file->addSeparator();
 //   #if defined(QWEBPAGE_PRINT)
    fileMenu->addAction(tr("P&rint Preview..."), this, SLOT(slotFilePrintPreview()));
    fileMenu->addAction(tr("&Print..."), this, SLOT(slotFilePrint()), QKeySequence::Print);
    fileMenu->addSeparator();
//#endif
    fileMenu->addAction(tr("&Open File..."), this, SLOT(slotFileOpen()), QKeySequence::Open);
    fileMenu->addAction(tr("Open &Location..."), this,
                SLOT(slotSelectLineEdit()), QKeySequence(Qt::ControlModifier + Qt::Key_L));
    fileMenu->addSeparator();
    //#if defined(QWEBPAGE_SETNETWORKACCESSMANAGER)
    fileMenu->addAction(tr("&Save As..."), this,
                SLOT(slotFileSaveAs()), QKeySequence(QKeySequence::Save));
    fileMenu->addAction(tr("Download List"), this,
                SLOT(slotDownloadManager()));
    fileMenu->addSeparator();
//#endif
    
    fileMenu->addAction(quitAction);
    
    QMenu *editMenu = appMenuBar->addMenu(tr("&Edit"));  
    QAction *m_undo = editMenu->addAction(tr("&Undo"));
    m_undo->setShortcuts(QKeySequence::Undo);
    mainView->addWebAction(m_undo, QWebPage::Undo);
    QAction *m_redo = editMenu->addAction(tr("&Redo"));
    m_redo->setShortcuts(QKeySequence::Redo);
    mainView->addWebAction(m_redo, QWebPage::Redo);
    editMenu->addSeparator();
    QAction *m_cut = editMenu->addAction(tr("Cu&t"));
    m_cut->setShortcuts(QKeySequence::Cut);
    mainView->addWebAction(m_cut, QWebPage::Cut);
    QAction *m_copy = editMenu->addAction(tr("&Copy"));
    m_copy->setShortcuts(QKeySequence::Copy);
    mainView->addWebAction(m_copy, QWebPage::Copy);
    QAction *m_paste = editMenu->addAction(tr("&Paste"));
    m_paste->setShortcuts(QKeySequence::Paste);
    mainView->addWebAction(m_paste, QWebPage::Paste);
    editMenu->addSeparator();
    QAction *m_find = editMenu->addAction(tr("&Find"));
    m_find->setShortcuts(QKeySequence::Find);
    connect(m_find, SIGNAL(triggered()), this, SLOT(slotEditFind()));
    new QShortcut(QKeySequence(Qt::Key_Slash), this, SLOT(slotEditFind()));
    QAction *m_findNext = editMenu->addAction(tr("&Find Next"));
    m_findNext->setShortcuts(QKeySequence::FindNext);
    connect(m_findNext, SIGNAL(triggered()), this, SLOT(slotEditFindNext()));
    QAction *m_findPrevious = editMenu->addAction(tr("&Find Previous"));
    m_findPrevious->setShortcuts(QKeySequence::FindPrevious);
    connect(m_findPrevious, SIGNAL(triggered()), this, SLOT(slotEditFindPrevious()));
    editMenu->addSeparator();
    
    
    QMenu *viewMenu = appMenuBar->addMenu(tr("&View"));  
    
    m_viewBookmarkBar = new QAction(this);
    updateBookmarksToolbarActionText(true);
    m_viewBookmarkBar->setShortcut(tr("Shift+Ctrl+B"));
    connect(m_viewBookmarkBar, SIGNAL(triggered()), this, SLOT(slotViewBookmarksBar()));
    viewMenu->addAction(m_viewBookmarkBar);

    m_viewToolbar = new QAction(this);
    updateToolbarActionText(true);
    m_viewToolbar->setShortcut(tr("Ctrl+|"));
    connect(m_viewToolbar, SIGNAL(triggered()), this, SLOT(slotViewToolbar()));
    viewMenu->addAction(m_viewToolbar);

    m_viewStatusbar = new QAction(this);
    updateStatusbarActionText(true);
    m_viewStatusbar->setShortcut(tr("Ctrl+/"));
    connect(m_viewStatusbar, SIGNAL(triggered()), this, SLOT(slotViewStatusbar()));
    viewMenu->addAction(m_viewStatusbar);
    
    
    showHideTabBarAction=toolbar->toggleViewAction();
    viewMenu->addAction(showHideTabBarAction);
    viewMenu->addSeparator();

    m_stop = viewMenu->addAction(tr("&Stop"));
    QList<QKeySequence> shortcuts;
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Period));
    shortcuts.append(Qt::Key_Escape);
    m_stop->setShortcuts(shortcuts);
    mainView->addWebAction(m_stop, QWebPage::Stop);

    m_reload = viewMenu->addAction(tr("Reload Page"));
    m_reload->setShortcuts(QKeySequence::Refresh);
    mainView->addWebAction(m_reload, QWebPage::Reload);

    viewMenu->addAction(tr("Zoom &In"), this, SLOT(slotViewZoomIn()), QKeySequence(Qt::CTRL | Qt::Key_Equal));
    viewMenu->addAction(tr("Zoom &Out"), this, SLOT(slotViewZoomOut()), QKeySequence(Qt::CTRL | Qt::Key_Minus));
    viewMenu->addAction(tr("Reset &Zoom"), this, SLOT(slotViewResetZoom()), QKeySequence(Qt::CTRL | Qt::Key_0));

    viewMenu->addSeparator();
    viewMenu->addAction(tr("Page S&ource"), this, SLOT(slotViewPageSource()), tr("Ctrl+Alt+U"));
    QAction *a = viewMenu->addAction(tr("&Full Screen"), this, SLOT(slotViewFullScreen(bool)),  Qt::Key_F11);
    a->setCheckable(true);
    viewMenu->addSeparator();
    viewMenu->addAction(mainView->newTabAction());     
    viewMenu->addAction(mainView->closeTabAction());
    viewMenu->addSeparator();    

    

#if defined(QTWEB_PRIVATEBROWSING)
    QAction *action = viewMenu->addAction(tr("Private &Browsing..."), this, SLOT(slotPrivateBrowsing()));
    action->setCheckable(true);
    viewMenu->addSeparator();
#endif
    
    QMenu *account = appMenuBar->addMenu(tr("&Account"));
        //TODO account actions import, export
        account->addAction(newAccountAction);
        account->addAction(switchAccountAction);
        account->addAction(encryptAccountAction);
        account->addAction(unlockAccountAction);
        account->addAction(decryptAccountAction);
        account->addAction(lockAccountAction);        
        account->addAction(changePassphraseAction);
        account->addAction(domainNameAction);
        account->addAction(exportAccountAction);
        account->addAction(importAccountAction);
    
    QMenu *applications = appMenuBar->addMenu(tr("&Applications"));
    applications->addAction(browserAction);
    applications->addAction(walletAction);
    applications->addAction(publisherAction);
    applications->addAction(messengerAction);
    applications->addAction(minerAction);
    applications->addAction(shopAction);
    applications->addAction(downloaderAction);
    applications->addAction(appsAction);
    applications->addAction(toolsAction);
    
    //QMenu *pageMenu = appMenuBar->addMenu(tr("&Pages"));
     
    
    // History
    HistoryMenu *historyMenu = new HistoryMenu(this);
    connect(historyMenu, SIGNAL(openUrl(QUrl)),
            this, SLOT(loadUrl(QUrl)));
    connect(historyMenu, SIGNAL(hovered(QString)), this,
            SLOT(slotUpdateStatusbar(QString)));
    historyMenu->setTitle(tr("&History"));
    appMenuBar->addMenu(historyMenu);
    QList<QAction*> historyActions;

    m_historyBack = new QAction(tr("&Back"), this);
    mainView->addWebAction(m_historyBack, QWebPage::Back);
    m_historyBack->setShortcuts(QKeySequence::Back);
    m_historyBack->setIconVisibleInMenu(false);
    historyActions.append(m_historyBack);

    m_historyForward = new QAction(tr("&Forward"), this);
    mainView->addWebAction(m_historyForward, QWebPage::Forward);
    m_historyForward->setShortcuts(QKeySequence::Forward);
    m_historyForward->setIconVisibleInMenu(false);
    historyActions.append(m_historyForward);

    QAction *m_historyHome = new QAction(tr("H&ome"), this);
    connect(m_historyHome, SIGNAL(triggered()), this, SLOT(slotHome()));
    m_historyHome->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_H));
    historyActions.append(m_historyHome);

//#if defined(QWEBHISTORY_RESTORESESSION)
    m_restoreLastSession = new QAction(tr("&Restore Last Session"), this);
    connect(m_restoreLastSession, SIGNAL(triggered()), this, SLOT(restoreLastSession()));
    m_restoreLastSession->setEnabled(canRestoreSession());
    historyActions.append(mainView->recentlyClosedTabsAction());
    historyActions.append(m_restoreLastSession);
//#endif

    historyMenu->setInitialActions(historyActions);
    // Bookmarks
    BookmarksMenu *bookmarksMenu = new BookmarksMenu(this);
    connect(bookmarksMenu, SIGNAL(openUrl(QUrl)),
            this, SLOT(loadUrl(QUrl)));
    connect(bookmarksMenu, SIGNAL(hovered(QString)),
            this, SLOT(slotUpdateStatusbar(QString)));
    bookmarksMenu->setTitle(tr("&Bookmarks"));
    appMenuBar->addMenu(bookmarksMenu);

    QList<QAction*> bookmarksActions;

    QAction *showAllBookmarksAction = new QAction(tr("&Show All Bookmarks"), this);
    connect(showAllBookmarksAction, SIGNAL(triggered()), this, SLOT(slotShowBookmarksDialog()));
    m_addBookmark = new QAction(QIcon(QLatin1String(":addbookmark.png")), tr("Add Bookmark..."), this);
    m_addBookmark->setIconVisibleInMenu(false);

    connect(m_addBookmark, SIGNAL(triggered()), this, SLOT(slotAddBookmark()));
    m_addBookmark->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));

    bookmarksActions.append(showAllBookmarksAction);
    bookmarksActions.append(m_addBookmark);
    bookmarksMenu->setInitialActions(bookmarksActions);
    
    //settings
    QMenu *settings = appMenuBar->addMenu(tr("&Settings"));
    settings->addAction(settingsAction);
    settings->addAction(tr("&Preferences"), this, SLOT(slotPreferences()), tr("Ctrl+,"));
//#if defined(QWEBINSPECTOR)
    a = viewMenu->addAction(tr("Enable Web &Inspector"), this, SLOT(slotToggleInspector(bool)));
    a->setCheckable(true);
//#endif
    
    
    
    
    //services->addAction(icqServiceAction);    
    //services->addAction(miningpoolServiceAction);
        
        

    QMenu *help = appMenuBar->addMenu(tr("&Help"));
    
    {
        //help->addAction(openRPCConsoleAction);
    }
    //help->addAction(showHelpMessageAction);
    //help->addSeparator();
    help->addAction(aboutAction);
    help->addAction(helpAction);
       
    
}

void BitcoinGUI::createToolBars()
{
    if(mainView)
    {
        
        toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        toolbar->setMaximumHeight(30);
        toolbar->addAction(browserAction);
        browserAction->setEnabled(true);
        browserAction->setChecked(true);
        toolbar->addAction(walletAction);                
        //toolbar->addAction(publisherAction);
        toolbar->addAction(messengerAction);
        toolbar->addAction(minerAction);  
        toolbar->addAction(shopAction);  
        toolbar->addAction(appsAction);  
        //toolbar->addAction(downloaderAction);
        
        
           //LogPrintf("bitcoingui createToolBars: 1 \n");    
        //m_navigationBar = addToolBar(tr("Navigation"));
        m_navigationBar->setMaximumHeight(30);
        m_navigationBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
         //LogPrintf("bitcoingui createToolBars: 2 \n"); 
        connect(m_navigationBar->toggleViewAction(), SIGNAL(toggled(bool)),
            this, SLOT(updateToolbarActionText(bool)));
         //LogPrintf("bitcoingui createToolBars: 3 \n"); 
        m_historyBack->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowBack, 0, this));
        m_historyBackMenu = new QMenu(this);
        m_historyBack->setMenu(m_historyBackMenu);
         //LogPrintf("bitcoingui createToolBars: 4 \n"); 
        connect(m_historyBackMenu, SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowBackMenu()));
         //LogPrintf("bitcoingui createToolBars: 5 \n"); 
        connect(m_historyBackMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOpenActionUrl(QAction*)));
        //LogPrintf("bitcoingui createToolBars: 6 \n"); 
    m_navigationBar->addAction(m_historyBack);
    
    m_historyForward->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowForward, 0, this));
    m_historyForward->setIconVisibleInMenu(true);
    m_historyForwardMenu = new QMenu(this);
    // LogPrintf("bitcoingui createToolBars: 7 \n"); 
    connect(m_historyForwardMenu, SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowForwardMenu()));
    // LogPrintf("bitcoingui createToolBars: 8 \n"); 
    connect(m_historyForwardMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOpenActionUrl(QAction*)));
    // LogPrintf("bitcoingui createToolBars: 9 \n"); 
    m_historyForward->setMenu(m_historyForwardMenu);
    m_navigationBar->addAction(m_historyForward);
 //LogPrintf("bitcoingui createToolBars: 10 \n"); 
    m_stopReload = new QAction(this);
    m_historyBack->setIconVisibleInMenu(true);
    m_reloadIcon = QApplication::style()->standardIcon(QStyle::SP_BrowserReload);
    m_stopReload->setIcon(m_reloadIcon);
 //LogPrintf("bitcoingui createToolBars: 11 \n"); 
    m_navigationBar->addAction(m_stopReload);
 //LogPrintf("bitcoingui createToolBars: 12 \n"); 
    m_navigationBar->addWidget(mainView->lineEditStack());
 //LogPrintf("bitcoingui createToolBars: 13 \n"); 
   // m_toolbarSearch = new ToolbarSearch(m_navigationBar);
     //LogPrintf("bitcoingui createToolBars: 14 \n"); 
   // m_navigationBar->addWidget(m_toolbarSearch);
     //LogPrintf("bitcoingui createToolBars: 15 \n"); 
    //connect(m_toolbarSearch, SIGNAL(search(QUrl)), SLOT(loadUrl(QUrl)));
 //LogPrintf("bitcoingui createToolBars: 16 \n"); 
    m_chaseWidget = new ChaseWidget(this);
    // LogPrintf("bitcoingui createToolBars: 17 \n"); 
    m_navigationBar->addWidget(m_chaseWidget);
   //  LogPrintf("bitcoingui createToolBars: 18 \n"); 
    }
}

void BitcoinGUI::setClientModel(ClientModel *clientModel)
{
    this->clientModel = clientModel;
    if(clientModel)
    {
        // Create system tray menu (or setup the dock menu) that late to prevent users from calling actions,
        // while the client has not yet fully loaded
        createTrayIconMenu();

        // Keep up to date with client
        setNumConnections(clientModel->getNumConnections());
        connect(clientModel, SIGNAL(numConnectionsChanged(int)), this, SLOT(setNumConnections(int)));

        setNumBlocks(clientModel->getNumBlocks());
        connect(clientModel, SIGNAL(numBlocksChanged(int)), this, SLOT(setNumBlocks(int)));

        // Receive and report messages from client model
        connect(clientModel, SIGNAL(message(QString,QString,unsigned int)), this, SLOT(message(QString,QString,unsigned int)));

        // Show progress dialog
        connect(clientModel, SIGNAL(showProgress(QString,int)), this, SLOT(showProgress(QString,int)));

        //rpcConsole->setClientModel(clientModel);
#ifdef ENABLE_WALLET
        //if(mainView)
        //{
        //    mainView->setClientModel(clientModel);
        //}
#endif // ENABLE_WALLET
        //unitDisplayControl->setOptionsModel(clientModel->getOptionsModel());
    } else {
        // Disable possibility to show main window via action
        toggleHideAction->setEnabled(false);
        if(trayIconMenu)
        {
            // Disable context menu on tray icon
            trayIconMenu->clear();
        }
    }
}

#ifdef ENABLE_WALLET
bool BitcoinGUI::addWallet(const QString& name, WalletModel *walletModelIn)
{
    LogPrintf("bitcoingui addwallet \n");
    walletModel=walletModelIn;
    jsInterface->setWalletModel(walletModel);
    setWalletActionsEnabled(true);
    connect(walletModel, SIGNAL(encryptionStatusChanged(int)), this, SLOT(setEncryptionStatus(int)));
    walletModel->updateStatus();
    if(!mainView)
        return false;
    
    //LogPrintf("bitcoingui addwallet2 \n");
    //QUrl walletUrl= QUrl("file://"+QDir::currentPath().toUtf8() + "/res/html/wallet_en.html"); 
   // mainView->gotoWebPage(1,walletUrl);//, walletModel);    
    //LogPrintf("bitcoingui addwallet3 \n");
    mainView->newTab(true,QUrl("fai:browser"),2);
     return true;
}

bool BitcoinGUI::setCurrentWallet(const QString& name)
{
    //if(!walletFrame)
        return false;
   // return walletFrame->setCurrentWallet(name);
}

void BitcoinGUI::removeAllWallets()
{
    //if(!walletFrame)
        return;
    //setWalletActionsEnabled(false);
    //walletFrame->removeAllWallets();
}
#endif // ENABLE_WALLET

void BitcoinGUI::setWalletActionsEnabled(bool enabled)
{
    
    encryptAccountAction->setEnabled(enabled);
    decryptAccountAction->setEnabled(enabled);
    exportAccountAction->setEnabled(enabled);
    unlockAccountAction->setEnabled(enabled);
    lockAccountAction->setEnabled(enabled);
    changePassphraseAction->setEnabled(enabled);    
    //signMessageAction->setEnabled(enabled);
    //verifyMessageAction->setEnabled(enabled);
    //usedSendingAddressesAction->setEnabled(enabled);
    //usedReceivingAddressesAction->setEnabled(enabled);
    //openAction->setEnabled(enabled);
}

void BitcoinGUI::createTrayIcon(const NetworkStyle *networkStyle)
{
#ifndef Q_OS_MAC
    trayIcon = new QSystemTrayIcon(this);
    QString toolTip = tr("Φ DeskTop Client") + " " + networkStyle->getTitleAddText();
    trayIcon->setToolTip(toolTip);
    trayIcon->setIcon(networkStyle->getAppIcon());
    trayIcon->show();
#endif

    notificator = new Notificator(QApplication::applicationName(), trayIcon, this);
}

void BitcoinGUI::createTrayIconMenu()
{
#ifndef Q_OS_MAC
    // return if trayIcon is unset (only on non-Mac OSes)
    if (!trayIcon)
        return;

    trayIconMenu = new QMenu(this);
    trayIcon->setContextMenu(trayIconMenu);

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
#else
    // Note: On Mac, the dock icon is used to provide the tray's functionality.
    MacDockIconHandler *dockIconHandler = MacDockIconHandler::instance();
    dockIconHandler->setMainWindow((QMainWindow *)this);
    trayIconMenu = dockIconHandler->dockMenu();
#endif

    // Configuration of the tray icon (or dock icon) icon menu
    trayIconMenu->addAction(toggleHideAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(browserAction);
    trayIconMenu->addAction(walletAction);
    trayIconMenu->addAction(publisherAction);
    trayIconMenu->addAction(messengerAction);    
    trayIconMenu->addAction(minerAction);    
    trayIconMenu->addSeparator();
    //trayIconMenu->addAction(signMessageAction);
    //trayIconMenu->addAction(verifyMessageAction);
    trayIconMenu->addSeparator();
    //trayIconMenu->addAction(optionsAction);
    //trayIconMenu->addAction(openRPCConsoleAction);
#ifndef Q_OS_MAC // This is built-in on Mac
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
#endif
}

#ifndef Q_OS_MAC
void BitcoinGUI::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    //LogPrintf("trayIconActivated reason:%i \n",reason);
    if(reason == QSystemTrayIcon::Trigger)
    {
        // Click on system tray icon triggers show/hide of the main window
        toggleHidden();
    }
}
#endif

//void BitcoinGUI::optionsClicked()
//{
//    if(!clientModel || !clientModel->getOptionsModel())
//        return;
//
//    OptionsDialog dlg(this, enableWallet);
//    dlg.setModel(clientModel->getOptionsModel());
//    dlg.exec();
//}

void BitcoinGUI::aboutClicked()
{
    if(!clientModel)
        return;

    HelpMessageDialog dlg(this, true);
    dlg.exec();
}
void BitcoinGUI::showHideTabBarClicked()
{
    QAction * act=toolbar->toggleViewAction() ;
    if(act->isChecked())
        act->setChecked(false);
    else
        act->setChecked(true);
    //act->setVisible(false);
    toolbar->setVisible(false);
}
void BitcoinGUI::showHelpMessageClicked()
{
    HelpMessageDialog *help = new HelpMessageDialog(this, false);
    help->setAttribute(Qt::WA_DeleteOnClose);
    help->show();
}

#ifdef ENABLE_WALLET
void BitcoinGUI::openClicked()
{
//    OpenURIDialog dlg(this);
//    if(dlg.exec())
//    {
//        emit receivedURI(dlg.getURI());
//    }
}
//enum ACCOUNT_OPS
//{
//    ACCOUNT_CREATE_NEW,
//    ACCOUNT_SWITCH,
//    ACCOUNT_CHANGE_PWD,
//    ACCOUNT_ENCRYPT
//};
void BitcoinGUI::newAccountClicked()
{
    if(!walletModel)
        return;
    AccountDialog dlg(AccountDialog::CreateNew,this, walletModel);    
    
        if(dlg.exec()){
        }
}
void BitcoinGUI::switchAccountClicked()
{
    if(!walletModel)
        return;
        AccountDialog dlg(AccountDialog::Switch,this, walletModel);    
    
        if(dlg.exec()){
            
        }
}
void BitcoinGUI::backupWallet()
{
    if(!walletModel)
        return;
        AccountDialog dlg(AccountDialog::Export,this, walletModel);    
    
        if(dlg.exec()){
        }
}
void BitcoinGUI::importWallet()
{
    if(!walletModel)
        return;
        walletModel->importAccount();
}
void BitcoinGUI::unlockAccountClicked()
{
    if(!walletModel)
        return;
        AccountDialog dlg(AccountDialog::Unlock,this, walletModel);    
    
        if(dlg.exec()){
        }
}
void BitcoinGUI::lockAccountClicked()
{
    if(!walletModel)
        return;
        AccountDialog dlg(AccountDialog::Lock,this, walletModel);    
    
        if(dlg.exec()){
        }
}
void BitcoinGUI::encryptWallet()
{
    if(!walletModel)
        return;
        AccountDialog dlg(AccountDialog::Encrypt,this, walletModel);    
    
        if(dlg.exec()){
        }
}
void BitcoinGUI::decryptWallet()
{
    if(!walletModel)
        return;
        AccountDialog dlg(AccountDialog::Decrypt,this, walletModel);    
    
        if(dlg.exec()){
        }
}
void BitcoinGUI::changePassphrase()
{
    if(!walletModel)
        return;
        AccountDialog dlg(AccountDialog::ChangePass,this, walletModel);    
    
        if(dlg.exec()){
        }
}
//enum pageid
//{
//    WALLETPAGE_ID=1,
//    BROWSERPAGE_ID=2,
//    PUBLISHERPAGE_ID=3,
//    MESSENGERPAGE_ID=4,
//    MINERPAGE_ID=5,
//    DOMAINPAGE_ID=6,
//    SETTINGPAGE_ID=7,
//    SERVICEPAGE_ID=8,
//    SHOPPAGE_ID=9,
//    TOOLSPAGE_ID=10,
//    DOWNLOADERPAGE_ID=11
//};
//static std::map<int,std::string> mapPageNames=boost::assign::map_list_of
//(WALLETPAGE_ID,"wallet")
//(BROWSERPAGE_ID,"browser")
//(PUBLISHERPAGE_ID,"publisher")
//(MESSENGERPAGE_ID,"messenger")
//(DOMAINPAGE_ID,"domain")
//(SETTINGPAGE_ID,"settings")
//(SERVICEPAGE_ID,"service")
//(SHOPPAGE_ID,"shop")
//(TOOLSPAGE_ID,"tools")
//(DOWNLOADERPAGE_ID,"downloader")
//;
void BitcoinGUI::gotoWalletPage()
{

    loadUrl(QUrl("fai:wallet"));
}
void BitcoinGUI::gotoBrowserPage()
{
//    QDir dir(QDir::currentPath());
//    dir.cdUp();
//    dir.cdUp();
//    dir.cdUp();
//    dir.cd(QString().fromStdString("faipages"));
//    QUrl url= QUrl("file://"+dir.path().toUtf8() + "/html/browser_en.html"); 
//    LogPrintf("gotobrowser page url:%s \n",url.toString().toStdString());
//    browserAction->setChecked(true);
   // if (mainView) mainView->gotoWebPage(BROWSERPAGE_ID,url);
    
        loadUrl(QUrl("fai:browser"));
    
}
void BitcoinGUI::gotoPublisherPage()
{  
    loadUrl(QUrl("fai:publisher"));
}
void BitcoinGUI::gotoMessengerPage()
{
   loadUrl(QUrl("fai:messenger"));
}
void BitcoinGUI::gotoMinerPage()
{
    loadUrl(QUrl("fai:miner"));
}
void BitcoinGUI::gotoShopPage()
{
    loadUrl(QUrl("fai:shop"));
}
void BitcoinGUI::gotoDownloaderPage()
{
  //  downloaderAction->setChecked(true);
//    QUrl url= QUrl("file://"+QDir::currentPath().toUtf8() + "/res/html/downloader_en.html"); 
//    if (mainView) mainView->gotoWebPage(DOWNLOADERPAGE_ID,url);
   // if (mainView) mainView->loadWebPage(DOWNLOADERPAGE_ID);
    loadUrl(QUrl("fai:downloader"));
}

void BitcoinGUI::gotoToolsPage()
{    
    loadUrl(QUrl("fai:tools"));
    
}
void BitcoinGUI::domainNameClicked()
{
    loadUrl(QUrl("fai:domain"));
}

#endif // ENABLE_WALLET
void BitcoinGUI::gotoSettingsPage()
{    
   loadUrl(QUrl("fai:settings"));
    
}
void BitcoinGUI::gotoHelpPage()
{    
   loadUrl(QUrl("fai:help"));
    
}
void BitcoinGUI::gotoAppsPage()
{    
   loadUrl(QUrl("fai:apps"));
    
}
void BitcoinGUI::installWebPages()
{
    for(int i=1;i<=HELPPAGE_ID;i++)
    {
       if(mainView) mainView->installWebPage(mapPageNames[i]);
    }
}

void BitcoinGUI::setNumConnections(int count)
{
    QString icon;
    switch(count)
    {
    case 0: icon = ":/icons/connect_0"; break;
    case 1: case 2: case 3: icon = ":/icons/connect_1"; break;
    case 4: case 5: case 6: icon = ":/icons/connect_2"; break;
    case 7: case 8: case 9: icon = ":/icons/connect_3"; break;
    default: icon = ":/icons/connect_4"; break;
    }
    labelConnectionsIcon->setPixmap(QIcon(icon).pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
    labelConnectionsIcon->setToolTip(tr("%n active connection(s) to Φ network", "", count));
}

void BitcoinGUI::setNumBlocks(int count)
{
    if(!clientModel)
        return;

    // Prevent orphan statusbar messages (e.g. hover Quit in main menu, wait until chain-sync starts -> garbelled text)
    statusBar()->clearMessage();

    // Acquire current block source
    enum BlockSource blockSource = clientModel->getBlockSource();
    switch (blockSource) {
        case BLOCK_SOURCE_NETWORK:
            progressBarLabel->setText(tr("Synchronizing with network..."));
            break;
        case BLOCK_SOURCE_DISK:
            progressBarLabel->setText(tr("Importing blocks from disk..."));
            break;
        case BLOCK_SOURCE_REINDEX:
            progressBarLabel->setText(tr("Reindexing blocks on disk..."));
            break;
        case BLOCK_SOURCE_NONE:
            // Case: not Importing, not Reindexing and no network connection
            progressBarLabel->setText(tr("No block source available..."));
            break;
    }

    QString tooltip;

    QDateTime lastBlockDate = clientModel->getLastBlockDate();
    QDateTime currentDate = QDateTime::currentDateTime();
    int secs = lastBlockDate.secsTo(currentDate);

    tooltip = tr("Processed %n blocks of transaction history.", "", count);

    // Set icon state: spinning if catching up, tick otherwise
    if(secs < 90*60)
    {
        tooltip = tr("Up to date") + QString(".<br>") + tooltip;
        labelBlocksIcon->setPixmap(QIcon(":/icons/synced").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));

#ifdef ENABLE_WALLET
        //if(mainView)
        //    mainView->showOutOfSyncWarning(false);
#endif // ENABLE_WALLET

        progressBarLabel->setVisible(false);
        progressBar->setVisible(false);
    }
    else
    {
        // Represent time from last generated block in human readable text
        QString timeBehindText;
        const int HOUR_IN_SECONDS = 60*60;
        const int DAY_IN_SECONDS = 24*60*60;
        const int WEEK_IN_SECONDS = 7*24*60*60;
        const int YEAR_IN_SECONDS = 31556952; // Average length of year in Gregorian calendar
        if(secs < 2*DAY_IN_SECONDS)
        {
            timeBehindText = tr("%n hour(s)","",secs/HOUR_IN_SECONDS);
        }
        else if(secs < 2*WEEK_IN_SECONDS)
        {
            timeBehindText = tr("%n day(s)","",secs/DAY_IN_SECONDS);
        }
        else if(secs < YEAR_IN_SECONDS)
        {
            timeBehindText = tr("%n week(s)","",secs/WEEK_IN_SECONDS);
        }
        else
        {
            int years = secs / YEAR_IN_SECONDS;
            int remainder = secs % YEAR_IN_SECONDS;
            timeBehindText = tr("%1 and %2").arg(tr("%n year(s)", "", years)).arg(tr("%n week(s)","", remainder/WEEK_IN_SECONDS));
        }

        progressBarLabel->setVisible(true);
        progressBar->setFormat(tr("%1 behind").arg(timeBehindText));
        progressBar->setMaximum(1000000000);
        progressBar->setValue(clientModel->getVerificationProgress() * 1000000000.0 + 0.5);
        progressBar->setVisible(true);

        tooltip = tr("Catching up...") + QString("<br>") + tooltip;
        if(count != prevBlocks)
        {
            labelBlocksIcon->setPixmap(QIcon(QString(
                ":/movies/spinner-%1").arg(spinnerFrame, 3, 10, QChar('0')))
                .pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));
            spinnerFrame = (spinnerFrame + 1) % SPINNER_FRAMES;
        }
        prevBlocks = count;

#ifdef ENABLE_WALLET
       // if(mainView)
       //     mainView->showOutOfSyncWarning(true);
#endif // ENABLE_WALLET

        tooltip += QString("<br>");
        tooltip += tr("Last received block was generated %1 ago.").arg(timeBehindText);
        tooltip += QString("<br>");
        tooltip += tr("Transactions after this will not yet be visible.");
    }

    // Don't word-wrap this (fixed-width) tooltip
    tooltip = QString("<nobr>") + tooltip + QString("</nobr>");

    labelBlocksIcon->setToolTip(tooltip);
    progressBarLabel->setToolTip(tooltip);
    progressBar->setToolTip(tooltip);
}

void BitcoinGUI::message(const QString &title, const QString &message, unsigned int style, bool *ret)
{
    QString strTitle = tr("Φ DeskTop Client"); // default title
    // Default to information icon
    int nMBoxIcon = QMessageBox::Information;
    int nNotifyIcon = Notificator::Information;

    QString msgType;

    // Prefer supplied title over style based title
    if (!title.isEmpty()) {
        msgType = title;
    }
    else {
        switch (style) {
        case CClientUIInterface::MSG_ERROR:
            msgType = tr("Error");
            break;
        case CClientUIInterface::MSG_WARNING:
            msgType = tr("Warning");
            break;
        case CClientUIInterface::MSG_INFORMATION:
            msgType = tr("Information");
            break;
        default:
            break;
        }
    }
    // Append title to "Bitcoin - "
    if (!msgType.isEmpty())
        strTitle += " - " + msgType;

    // Check for error/warning icon
    if (style & CClientUIInterface::ICON_ERROR) {
        nMBoxIcon = QMessageBox::Critical;
        nNotifyIcon = Notificator::Critical;
    }
    else if (style & CClientUIInterface::ICON_WARNING) {
        nMBoxIcon = QMessageBox::Warning;
        nNotifyIcon = Notificator::Warning;
    }

    // Display message
    if (style & CClientUIInterface::MODAL) {
        // Check for buttons, use OK as default, if none was supplied
        QMessageBox::StandardButton buttons;
        if (!(buttons = (QMessageBox::StandardButton)(style & CClientUIInterface::BTN_MASK)))
            buttons = QMessageBox::Ok;

        showNormalIfMinimized();
        QMessageBox mBox((QMessageBox::Icon)nMBoxIcon, strTitle, message, buttons, this);
        int r = mBox.exec();
        if (ret != NULL)
            *ret = r == QMessageBox::Ok;
    }
    else
        notificator->notify((Notificator::Class)nNotifyIcon, strTitle, message);
}

void BitcoinGUI::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
#ifndef Q_OS_MAC // Ignored on Mac
    if(e->type() == QEvent::WindowStateChange)
    {
        if(clientModel)// && clientModel->getOptionsModel() && clientModel->getOptionsModel()->getMinimizeToTray())
        {
            QWindowStateChangeEvent *wsevt = static_cast<QWindowStateChangeEvent*>(e);
            if(!(wsevt->oldState() & Qt::WindowMinimized) && isMinimized())
            {
                QTimer::singleShot(0, this, SLOT(hide()));
                e->ignore();
            }
        }
    }
#endif
}

void BitcoinGUI::closeEvent(QCloseEvent *event)
{
    if (mainView->count() > 1) {
        int ret = QMessageBox::warning(this, QString(),
                           tr("Are you sure you want to close the window?"
                              "  There are %1 tabs open").arg(mainView->count()),
                           QMessageBox::Yes | QMessageBox::No,
                           QMessageBox::No);
        if (ret == QMessageBox::No) {
            event->ignore();
            return;
        }
    }
    //event->accept();
    //deleteLater();
#ifndef Q_OS_MAC // Ignored on Mac
//    if(clientModel)// && clientModel->getOptionsModel())
//    {
//        //if(!clientModel->getOptionsModel()->getMinimizeToTray() &&
//        //   !clientModel->getOptionsModel()->getMinimizeOnClose())
//        {
//            QApplication::quit();
//        }
//    }
#endif
    //QMainWindow::closeEvent(event);
    StartShutdown();
    event->ignore();
}

#ifdef ENABLE_WALLET
void BitcoinGUI::incomingTransaction(const QString& date, int unit, const CAmount& amount, const QString& type, const QString& address)
{
    // On new transaction, make an info balloon
    message((amount)<0 ? tr("Sent transaction") : tr("Incoming transaction"),
             tr("Date: %1\n"
                "Amount: %2\n"
                "Type: %3\n"
                "Address: %4\n")
                  .arg(date)
                  .arg(BitcoinUnits::formatWithUnit(unit, amount, true))
                  .arg(type)
                  .arg(address), CClientUIInterface::MSG_INFORMATION);
}
#endif // ENABLE_WALLET

void BitcoinGUI::dragEnterEvent(QDragEnterEvent *event)
{
    // Accept only URIs
    if(event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void BitcoinGUI::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        foreach(const QUrl &uri, event->mimeData()->urls())
        {
            emit receivedURI(uri.toString());
        }
    }
    event->acceptProposedAction();
}

bool BitcoinGUI::eventFilter(QObject *object, QEvent *event)
{
    // Catch status tip events
    if (event->type() == QEvent::StatusTip)
    {
        // Prevent adding text from setStatusTip(), if we currently use the status bar for displaying other stuff
        if (progressBarLabel->isVisible() || progressBar->isVisible())
            return true;
    }
    return QMainWindow::eventFilter(object, event);
}

#ifdef ENABLE_WALLET


void BitcoinGUI::setEncryptionStatus(int status)
{
    switch(status)
    {
    case WalletModel::Unencrypted:
        labelEncryptionIcon->hide();
        changePassphraseAction->setVisible(false);
        unlockAccountAction->setVisible(false);
        lockAccountAction->setVisible(false);
        encryptAccountAction->setVisible(true);
        decryptAccountAction->setVisible(false);
        break;
    case WalletModel::Unlocked:
        labelEncryptionIcon->show();
        labelEncryptionIcon->setPixmap(QIcon(":/icons/lock_open").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>unlocked</b>"));
        
        changePassphraseAction->setVisible(true);
        decryptAccountAction->setVisible(true);
        encryptAccountAction->setVisible(false);
        unlockAccountAction->setVisible(false);
        lockAccountAction->setVisible(true);
        break;
    case WalletModel::Locked:
        labelEncryptionIcon->show();
        labelEncryptionIcon->setPixmap(QIcon(":/icons/lock_closed").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>locked</b>"));
        changePassphraseAction->setVisible(true);
        decryptAccountAction->setVisible(true);
        encryptAccountAction->setVisible(false);
        unlockAccountAction->setVisible(true);
        lockAccountAction->setVisible(false);
        break;
    }
}



#endif // ENABLE_WALLET

void BitcoinGUI::showNormalIfMinimized(bool fToggleHidden)
{
    //LogPrintf("showNormalIfMinimized called clientmodel %i,isHidden %b,isMinimized:%b \n",clientModel,isHidden(),isMinimized());
    if(!clientModel)
        return;

    // activateWindow() (sometimes) helps with keyboard focus on Windows
    if (isHidden())
    {
        show();
        activateWindow();
    }
    else if (isMinimized())
    {
        showNormal();
        activateWindow();
    }
    else if (GUIUtil::isObscured(this))
    {
        raise();
        activateWindow();
    }
    else if(fToggleHidden)
        hide();
}

void BitcoinGUI::toggleHidden()
{
    //LogPrintf("toggleHidden called \n");
    showNormalIfMinimized(true);
}

void BitcoinGUI::detectShutdown()
{
    if (ShutdownRequested())
    {
//        if(rpcConsole)
//            rpcConsole->hide();
        qApp->quit();
    }
}

void BitcoinGUI::showProgress(const QString &title, int nProgress)
{
    if (nProgress == 0)
    {
        progressDialog = new QProgressDialog(title, "", 0, 100);
        progressDialog->setWindowModality(Qt::ApplicationModal);
        progressDialog->setMinimumDuration(0);
        progressDialog->setCancelButton(0);
        progressDialog->setAutoClose(false);
        progressDialog->setValue(0);
    }
    else if (nProgress == 100)
    {
        if (progressDialog)
        {
            progressDialog->close();
            progressDialog->deleteLater();
        }
    }
    else if (progressDialog)
        progressDialog->setValue(nProgress);
}

static bool ThreadSafeMessageBox(BitcoinGUI *gui, const std::string& message, const std::string& caption, unsigned int style)
{
    bool modal = (style & CClientUIInterface::MODAL);
    // The SECURE flag has no effect in the Qt GUI.
     bool secure = (style & CClientUIInterface::SECURE);
    style &= ~CClientUIInterface::SECURE;
    bool ret = false;
    // In case of modal message, use blocking connection to wait for user to click a button
    QMetaObject::invokeMethod(gui, "message",
                               modal ? GUIUtil::blockingGUIThreadConnection() : Qt::QueuedConnection,
                               Q_ARG(QString, QString::fromStdString(caption)),
                               Q_ARG(QString, QString::fromStdString(message)),
                               Q_ARG(unsigned int, style),
                               Q_ARG(bool*, &ret));
    return ret;
}

void BitcoinGUI::subscribeToCoreSignals()
{
    // Connect signals to client
    uiInterface.ThreadSafeMessageBox.connect(boost::bind(ThreadSafeMessageBox, this, _1, _2, _3));
}

void BitcoinGUI::unsubscribeFromCoreSignals()
{
    // Disconnect signals from client
    uiInterface.ThreadSafeMessageBox.disconnect(boost::bind(ThreadSafeMessageBox, this, _1, _2, _3));
}

//UnitDisplayStatusBarControl::UnitDisplayStatusBarControl() :
//    //optionsModel(0),
//    menu(0)
//{
//    createContextMenu();
//    setToolTip(tr("Unit to show amounts in. Click to select another unit."));
//}
//
///** So that it responds to button clicks */
//void UnitDisplayStatusBarControl::mousePressEvent(QMouseEvent *event)
//{
//    onDisplayUnitsClicked(event->pos());
//}
//
///** Creates context menu, its actions, and wires up all the relevant signals for mouse events. */
//void UnitDisplayStatusBarControl::createContextMenu()
//{
//    menu = new QMenu();
//    foreach(BitcoinUnits::Unit u, BitcoinUnits::availableUnits())
//    {
//        QAction *menuAction = new QAction(QString(BitcoinUnits::name(u)), this);
//        menuAction->setData(QVariant(u));
//        menu->addAction(menuAction);
//    }
//    connect(menu,SIGNAL(triggered(QAction*)),this,SLOT(onMenuSelection(QAction*)));
//}

/** Lets the control know about the Options Model (and its signals) */
//void UnitDisplayStatusBarControl::setOptionsModel(OptionsModel *optionsModel)
//{
//    if (optionsModel)
//    {
//        this->optionsModel = optionsModel;
//
//        // be aware of a display unit change reported by the OptionsModel object.
//        connect(optionsModel,SIGNAL(displayUnitChanged(int)),this,SLOT(updateDisplayUnit(int)));
//
//        // initialize the display units label with the current value in the model.
//        updateDisplayUnit(optionsModel->getDisplayUnit());
//    }
//}

/** When Display Units are changed on OptionsModel it will refresh the display text of the control on the status bar */
//void UnitDisplayStatusBarControl::updateDisplayUnit(int newUnits)
//{
//    setPixmap(QIcon(":/icons/unit_" + BitcoinUnits::id(newUnits)).pixmap(31,STATUSBAR_ICONSIZE));
//}
//
///** Shows context menu with Display Unit options by the mouse coordinates */
//void UnitDisplayStatusBarControl::onDisplayUnitsClicked(const QPoint& point)
//{
//    QPoint globalPos = mapToGlobal(point);
//    menu->exec(globalPos);
//}
//
///** Tells underlying optionsModel to update its current display unit. */
//void UnitDisplayStatusBarControl::onMenuSelection(QAction* action)
//{
//    if (action)
//    {
//        //optionsModel->setDisplayUnit(action->data());
//    }
//}
bool BitcoinGUI::handleUserConfirm(QString title,QString message,int nOP,string& strError,SecureString& ssInput,const int nPageIndex){
    //LogPrintf("bitcoingui handleUserConfirm received: \n");    
        UserConfirmDialog *dlg=new UserConfirmDialog(this,nPageIndex);
        dlg->setWindowTitle(title);
        dlg->ui->label_message->setText(message);
        dlg->ui->label_message->setWordWrap(true);
        if (nOP!=1){
            dlg->ui->label_7->hide();
            dlg->ui->passwordEdit->hide();
        }        
        //connect(dlg,SIGNAL(killPage(int)),mainView,SLOT(closeTab(int)));
        bool fConfirm=dlg->exec();        
        if(dlg->nPageIndex==-2)            
            mainView->getWebView(nPageIndex)->fBlocked=true;        
        if(fConfirm){
            //disconnect(dlg,SIGNAL(killPage(int)),mainView,SLOT(closeTab(int)));
            if(nOP==1){
                ssInput.reserve(MAX_PASSPHRASE_SIZE);    
                ssInput.assign(dlg->ui->passwordEdit->text().toStdString().c_str());           
            }
            //LogPrintf("bitcoingui handleUserConfirm ok pressed, \n");  
             delete dlg;
            return true;//QString("{\"success\":\"tx sent\"}");
            //emit sendMoneyResult(strToken,true,dlg.ui->passwordEdit->text());
        }else{
            //LogPrintf("bitcoingui handleUserConfirm return value,%i \n",dlg->nPageIndex); 
            //disconnect(dlg,SIGNAL(killPage(int)),mainView,SLOT(closeTab(int)));
            //LogPrintf("bitcoingui handleUserConfirm cancel pressed \n"); 
            strError="user cancelled";     
            delete dlg;
            return false;//QString("{\"error\":\"payment request cancelled\"}");
            //emit sendMoneyResult(strToken,false,QString().fromStdString("{\"result\":\"dialogue opened\"}"));
        }
       
}
void BitcoinGUI::subscribeToCoreSignalsJs()
{
    jsInterface->subscribeToCoreSignals();
}
void BitcoinGUI::updateToolbarActionText(bool visible)
{
    m_viewToolbar->setText(!visible ? tr("Show Navigate bar") : tr("Hide Navigate bar"));
}
void BitcoinGUI::slotAboutToShowBackMenu()
{
    m_historyBackMenu->clear();
    if (!currentTab())
        return;
    QWebHistory *history = currentTab()->history();
    int historyCount = history->count();
    for (int i = history->backItems(historyCount).count() - 1; i >= 0; --i) {
        QWebHistoryItem item = history->backItems(history->count()).at(i);
        QAction *action = new QAction(this);
        action->setData(-1*(historyCount-i-1));
        QIcon icon = this->icon(item.url());
        action->setIcon(icon);
        action->setText(item.title());
        m_historyBackMenu->addAction(action);
    }
}

void BitcoinGUI::slotAboutToShowForwardMenu()
{
    m_historyForwardMenu->clear();
    if (!currentTab())
        return;
    QWebHistory *history = currentTab()->history();
    int historyCount = history->count();
    for (int i = 0; i < history->forwardItems(history->count()).count(); ++i) {
        QWebHistoryItem item = history->forwardItems(historyCount).at(i);
        QAction *action = new QAction(this);
        action->setData(historyCount-i);
        QIcon icon = this->icon(item.url());
        action->setIcon(icon);
        action->setText(item.title());
        m_historyForwardMenu->addAction(action);
    }
}

void BitcoinGUI::slotShowWindow()
{
//    if (QAction *action = qobject_cast<QAction*>(sender())) {
//        QVariant v = action->data();
//        if (v.canConvert<int>()) {
//            int offset = qvariant_cast<int>(v);
//            QList<BitcoinGUI*> windows = BitcoinApplication::instance()->mainWindows();
//            windows.at(offset)->activateWindow();
//            windows.at(offset)->currentTab()->setFocus();
//        }
//    }
    this->activateWindow();
    this->currentTab()->setFocus();
}
void BitcoinGUI::loadUrl(const QUrl &url)
{
    //LogPrintf("bitcoingui loadUrl: 1\n"); 
    string strUrlOut;
    int nPageID;
    if(!ParseUrl(url.toString().toStdString(),strUrlOut,nPageID))
        return ;
    QUrl urlOut(QString().fromStdString(strUrlOut));
     //LogPrintf("bitcoingui loadUrl urlout: %s\n",strUrlOut);
     mainView->currentLineEdit()->setText(mainView->currentWebView()->url().toString());   
     for(unsigned i=0;i<mainView->count();i++)
     {
         WebView *tab = mainView->getWebView(i);
         if (tab->url()==url)
         {
             mainView->setCurrentWidget(tab);
            return;
         }
     }
    if (!currentTab() || !urlOut.isValid())
        return;
     //LogPrintf("bitcoingui loadUrl: 2\n"); 
     if(currentTab()->nPageID==255&&nPageID==255)
     {
        mainView->currentLineEdit()->setText(QString::fromUtf8(url.toEncoded()));
        //LogPrintf("bitcoingui loadUrl: 3\n"); 
        mainView->loadUrlInCurrentTab(url);
       // LogPrintf("bitcoingui loadUrl: 4\n"); 
     }
     else
     {      
      WebView* page= mainView->newTab(true,url,nPageID); 
       //page->nPageID=nPageID;
        //mainView->loadUrlInCurrentTab(url);        
        mainView->currentLineEdit()->setText(QString::fromUtf8(url.toEncoded()));
       // mainView->gotoWebPage(nPageID,url);
       // LogPrintf("bitcoingui loadUrl: 5\n"); 
       
     }
}
MainView *BitcoinGUI::getMainView() const
{
    return mainView;
}

WebView *BitcoinGUI::currentTab() const
{
    return mainView->currentWebView();
}
QIcon BitcoinGUI::defaultIcon() const
{
    if (m_defaultIcon.isNull())
        m_defaultIcon = QIcon(QLatin1String(":defaulticon.png"));
    return m_defaultIcon;
}
DownloadManager *BitcoinGUI::downloadManager()
{
    if (!s_downloadManager) {
        s_downloadManager = new DownloadManager();
    }
    return s_downloadManager;
}

QNetworkAccessManager *BitcoinGUI::networkAccessManager()
{
    //LogPrintf("bitcoingui networkAccessManager\n"); 
//#if defined(QWEBPAGE_SETNETWORKACCESSMANAGER)
    if (!s_networkAccessManager) {
        s_networkAccessManager = new NetworkAccessManager();
        s_networkAccessManager->setCookieJar(new CookieJar);
    }
    return s_networkAccessManager;
//#else
//    if (!s_networkAccessManager) {
//        s_networkAccessManager = new QNetworkAccessManager();
//    }
//    return s_networkAccessManager;
//#endif
}
HistoryManager *BitcoinGUI::historyManager()
{
    if (!s_historyManager)
        s_historyManager = new HistoryManager();
    return s_historyManager;
}

BookmarksManager *BitcoinGUI::bookmarksManager()
{
    if (!s_bookmarksManager) {
        s_bookmarksManager = new BookmarksManager;
    }
    return s_bookmarksManager;
}
void BitcoinGUI::saveSession()
{
#if defined(QTWEB_PRIVATEBROWSING)
    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        return;
#endif

    //clean();

    QSettings settings;
    settings.beginGroup(QLatin1String("sessions"));

    QByteArray data;
    QBuffer buffer(&data);
    QDataStream stream(&buffer);
    buffer.open(QIODevice::ReadWrite);

    stream << this->saveState();
    settings.setValue(QLatin1String("lastSession"), data);
    settings.endGroup();
}
bool BitcoinGUI::canRestoreSession() const
{
    return !m_lastSession.isEmpty();
}
void BitcoinGUI::restoreLastSession()
{
    QBuffer buffer(&m_lastSession);
    QDataStream stream(&buffer);
    buffer.open(QIODevice::ReadOnly);
        QByteArray windowState;
        stream >> windowState;
        this->restoreState(windowState);
}
void BitcoinGUI::openUrl(const QUrl &url)
{
    loadPage(url.toString());
}
void BitcoinGUI::loadPage(const QString &page)
{
    //QUrl url = QUrl::fromUserInput(page);
    QUrl url = QUrl(page);
    loadUrl(url);
}
CookieJar *BitcoinGUI::cookieJar()
{
//#if defined(QWEBPAGE_SETNETWORKACCESSMANAGER)
    return (CookieJar*)networkAccessManager()->cookieJar();
//#else
//    return 0;
//#endif
}
QIcon BitcoinGUI::icon(const QUrl &url) const
{
//#if defined(QTWEB_ICONDATABASE)
    QIcon icon = QWebSettings::iconForUrl(url);
    if (!icon.isNull())
        return icon.pixmap(16, 16);
//#else
//    Q_UNUSED(url);
//#endif
    return defaultIcon();
}
QSize BitcoinGUI::sizeHint() const
{
    QRect desktopRect = QApplication::desktop()->screenGeometry();
    QSize size = desktopRect.size() * qreal(0.9);
    return size;
}

static const qint32 BrowserMainWindowMagic = 0xba;
QByteArray BitcoinGUI::saveState(bool withTabs) const
{
    int version = 2;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << qint32(BrowserMainWindowMagic);
    stream << qint32(version);

    stream << size();
    stream << !m_navigationBar->isHidden();
    stream << !m_bookmarksToolbar->isHidden();
    stream << !statusBar()->isHidden();
    if (withTabs)
        stream << getMainView()->saveState();
    else
        stream << QByteArray();
    return data;
}

bool BitcoinGUI::restoreState(const QByteArray &state)
{
    int version = 2;
    QByteArray sd = state;
    QDataStream stream(&sd, QIODevice::ReadOnly);
    if (stream.atEnd())
        return false;

    qint32 marker;
    qint32 v;
    stream >> marker;
    stream >> v;
    if (marker != BrowserMainWindowMagic || v != version)
        return false;

    QSize size;
    bool showToolbar;
    bool showBookmarksBar;
    bool showStatusbar;
    QByteArray tabState;

    stream >> size;
    stream >> showToolbar;
    stream >> showBookmarksBar;
    stream >> showStatusbar;
    stream >> tabState;

    resize(size);

    m_navigationBar->setVisible(showToolbar);
    updateToolbarActionText(showToolbar);

    m_bookmarksToolbar->setVisible(showBookmarksBar);
    updateBookmarksToolbarActionText(showBookmarksBar);

    statusBar()->setVisible(showStatusbar);
    updateStatusbarActionText(showStatusbar);

    if (!getMainView()->restoreState(tabState))
        return false;

    return true;
}
void BitcoinGUI::slotHome()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("MainWindow"));
    QString home = settings.value(QLatin1String("home"), QLatin1String(defaultHome)).toString();
    loadPage(home);
}
void BitcoinGUI::slotLoadProgress(int progress)
{
    //LogPrintf("slot triggered:BitcoinGUI::slotLoadProgress \n");
    if (progress < 100 && progress > 0) {
        m_chaseWidget->setAnimated(true);
        disconnect(m_stopReload, SIGNAL(triggered()), m_reload, SLOT(trigger()));
        if (m_stopIcon.isNull())
            m_stopIcon = style()->standardIcon(QStyle::SP_BrowserStop);
        m_stopReload->setIcon(m_stopIcon);
        connect(m_stopReload, SIGNAL(triggered()), m_stop, SLOT(trigger()));
        m_stopReload->setToolTip(tr("Stop loading the current page"));
    } else {
        m_chaseWidget->setAnimated(false);
        disconnect(m_stopReload, SIGNAL(triggered()), m_stop, SLOT(trigger()));
        m_stopReload->setIcon(m_reloadIcon);
        connect(m_stopReload, SIGNAL(triggered()), m_reload, SLOT(trigger()));
        m_stopReload->setToolTip(tr("Reload the current page"));
    }
    //LogPrintf("BitcoinGUI::slotLoadProgress done\n");
}
void BitcoinGUI::slotUpdateStatusbar(const QString &string)
{
    //LogPrintf("slot triggered:BitcoinGUI::slotUpdateStatusbar \n");
    statusBar()->showMessage(string, 2000);
}
void BitcoinGUI::slotUpdateWindowTitle(const QString &title)
{
    //LogPrintf("slot triggered:BitcoinGUI::slotUpdateWindowTitle \n");
    if (title.isEmpty()) {
        setWindowTitle(tr("Φ DeskTop Client"));
    } else {
#if defined(Q_OS_OSX)
        setWindowTitle(title);
#else
        setWindowTitle(tr("%1 - Φ DeskTop Client", "Page title and Browser name").arg(title));
#endif
    }
}
void BitcoinGUI::slotPreferences()
{
    SettingsDialog *s = new SettingsDialog(this);
    s->show();
}
void BitcoinGUI::slotFileNew()
{    
    BitcoinGUI *mw = BitcoinApplication::instance()->getWindow();
    mw->slotHome();
}

void BitcoinGUI::slotFileOpen()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Open Web Resource"), QString(),
            tr("Web Resources (*.html *.htm *.svg *.png *.gif *.svgz);;All files (*.*)"));

    if (file.isEmpty())
        return;

    loadPage(file);
}

void BitcoinGUI::slotFilePrintPreview()
{
//#ifndef QT_NO_PRINTPREVIEWDIALOG
    if (!currentTab())
        return;
    QPrintPreviewDialog *dialog = new QPrintPreviewDialog(this);
    connect(dialog, SIGNAL(paintRequested(QPrinter*)),
            currentTab(), SLOT(print(QPrinter*)));
    dialog->exec();
//#endif
}

void BitcoinGUI::slotFilePrint()
{
//#if defined(QWEBPAGE_PRINT)
    if (!currentTab())
        return;
    printRequested(currentTab()->page()->mainFrame());
//#endif
}
void BitcoinGUI::slotPrivateBrowsing()
{
    //LogPrintf("slot triggered:BitcoinGUI::slotPrivateBrowsing \n");
#if defined(QTWEB_PRIVATEBROWSING)
    QWebSettings *settings = QWebSettings::globalSettings();
    bool pb = settings->testAttribute(QWebSettings::PrivateBrowsingEnabled);
    if (!pb) {
        QString title = tr("Are you sure you want to turn on private browsing?");
        QString text = tr("<b>%1</b><br><br>When private browsing in turned on,"
            " webpages are not added to the history,"
            " items are automatically removed from the Downloads window," \
            " new cookies are not stored, current cookies can't be accessed," \
            " site icons wont be stored, session wont be saved, " \
            " and searches are not added to the pop-up menu in the Google search box." \
            "  Until you close the window, you can still click the Back and Forward buttons" \
            " to return to the webpages you have opened.").arg(title);

        QMessageBox::StandardButton button = QMessageBox::question(this, QString(), text,
                               QMessageBox::Ok | QMessageBox::Cancel,
                               QMessageBox::Ok);
        if (button == QMessageBox::Ok) {
            settings->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
        }
    } else {
        settings->setAttribute(QWebSettings::PrivateBrowsingEnabled, false);            
            m_lastSearch = QString::null;
            mainView->clear();
        }
    }
#endif
}
void BitcoinGUI::slotFileSaveAs()
{
    BitcoinGUI::downloadManager()->download(currentTab()->url(), true);
}
void BitcoinGUI::slotEditFind()
{
    if (!currentTab())
        return;
    bool ok;
    QString search = QInputDialog::getText(this, tr("Find"),
                                          tr("Text:"), QLineEdit::Normal,
                                          m_lastSearch, &ok);
    if (ok && !search.isEmpty()) {
        m_lastSearch = search;        
        handleFindTextResult(currentTab()->findText(m_lastSearch, 0));
    }
}

void BitcoinGUI::slotEditFindNext()
{
    if (!currentTab() && !m_lastSearch.isEmpty())
        return;
    currentTab()->findText(m_lastSearch);
}

void BitcoinGUI::slotEditFindPrevious()
{
    if (!currentTab() && !m_lastSearch.isEmpty())
        return;
    currentTab()->findText(m_lastSearch, QWebPage::FindBackward);
}
void BitcoinGUI::slotShowBookmarksDialog()
{
    BookmarksDialog *dialog = new BookmarksDialog(this);
    connect(dialog, SIGNAL(openUrl(QUrl)),
            this, SLOT(loadUrl(QUrl)));
    dialog->show();
}
void BitcoinGUI::slotAddBookmark()
{
    WebView *webView = currentTab();
    QString url = webView->url().toString();
    QString title = webView->title();
    AddBookmarkDialog dialog(url, title);
    dialog.exec();
}
void BitcoinGUI::slotViewZoomIn()
{
    if (!currentTab())
        return;
    currentTab()->setZoomFactor(currentTab()->zoomFactor() + 0.1);
}

void BitcoinGUI::slotViewZoomOut()
{
    if (!currentTab())
        return;
    currentTab()->setZoomFactor(currentTab()->zoomFactor() - 0.1);
}

void BitcoinGUI::slotViewResetZoom()
{
    if (!currentTab())
        return;
    currentTab()->setZoomFactor(1.0);
}

void BitcoinGUI::slotViewFullScreen(bool makeFullScreen)
{
    if (makeFullScreen) {
        showFullScreen();
    } else {
        if (isMinimized())
            showMinimized();
        else if (isMaximized())
            showMaximized();
        else showNormal();
    }
}
void BitcoinGUI::slotViewToolbar()
{
    if (m_navigationBar->isVisible()) {
        updateToolbarActionText(false);
        m_navigationBar->close();
    } else {
        updateToolbarActionText(true);
        m_navigationBar->show();
    }
    m_autoSaver->changeOccurred();
}
void BitcoinGUI::slotViewBookmarksBar()
{
    if (m_bookmarksToolbar->isVisible()) {
        updateBookmarksToolbarActionText(false);
        m_bookmarksToolbar->close();
    } else {
        updateBookmarksToolbarActionText(true);
        m_bookmarksToolbar->show();
    }
    m_autoSaver->changeOccurred();
}
void BitcoinGUI::slotViewStatusbar()
{
    if (statusBar()->isVisible()) {
        updateStatusbarActionText(false);
        statusBar()->close();
    } else {
        updateStatusbarActionText(true);
        statusBar()->show();
    }
    m_autoSaver->changeOccurred();
}
void BitcoinGUI::slotViewPageSource()
{
    if (!currentTab())
        return;

    QPlainTextEdit *view = new QPlainTextEdit;
    view->setWindowTitle(tr("Page Source of %1").arg(currentTab()->title()));
    view->setMinimumWidth(640);
    view->setAttribute(Qt::WA_DeleteOnClose);
    view->show();
    view->setPlainText(currentTab()->page()->currentFrame()->toHtml());
    //currentTab()->page()->currentFrame()->toHtml(invoke(view, &QPlainTextEdit::setPlainText));
}
void BitcoinGUI::slotWebSearch()
{
    m_toolbarSearch->lineEdit()->selectAll();
    m_toolbarSearch->lineEdit()->setFocus();
}
void BitcoinGUI::slotToggleInspector(bool enable)
{
//#if defined(QWEBINSPECTOR)
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, enable);
    if (enable) {
        int result = QMessageBox::question(this, tr("Web Inspector"),
                                           tr("The web inspector will only work correctly for pages that were loaded after enabling.\n"
                                           "Do you want to reload all pages?"),
                                           QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::Yes) {
            mainView->reloadAllTabs();
        }
    }
//#else
//    Q_UNUSED(enable);
//#endif
}
void BitcoinGUI::slotAboutApplication()
{
    QMessageBox::about(this, tr("About"), tr(
        "Version %1"
        "Φ DeskTop Client"
        ).arg(QCoreApplication::applicationVersion()));
}
void BitcoinGUI::slotDownloadManager()
{
    BitcoinGUI::downloadManager()->show();
}
void BitcoinGUI::slotSelectLineEdit()
{
   // LogPrintf("slot triggered:BitcoinGUI::slotSelectLineEdit \n");
    mainView->currentLineEdit()->selectAll();
    mainView->currentLineEdit()->setFocus();
}
void BitcoinGUI::slotOpenActionUrl(QAction *action)
{
   // LogPrintf("slot triggered:BitcoinGUI::slotOpenActionUrl \n");
    int offset = action->data().toInt();
    QWebHistory *history = currentTab()->history();
    if (offset < 0)
        history->goToItem(history->backItems(-1*offset).first()); // back
    else if (offset > 0)
        history->goToItem(history->forwardItems(history->count() - offset + 1).back()); // forward
}
void BitcoinGUI::slotSwapFocus()
{
    if (currentTab()->hasFocus())
        mainView->currentLineEdit()->setFocus();
    else
        currentTab()->setFocus();
}
//#if defined(QWEBPAGE_PRINT)
void BitcoinGUI::printRequested(QWebFrame *frame)
{
#ifndef QT_NO_PRINTDIALOG
    QPrinter printer;
    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle(tr("Print Document"));
    if (dialog->exec() != QDialog::Accepted)
        return;
    frame->print(&printer);
#endif
}
//#endif
void BitcoinGUI::geometryChangeRequested(const QRect &geometry)
{
    setGeometry(geometry);
}
void BitcoinGUI::updateBookmarksToolbarActionText(bool visible)
{
    m_viewBookmarkBar->setText(!visible ? tr("Show Bookmarks bar") : tr("Hide Bookmarks bar"));
}
void BitcoinGUI::loadDefaultState()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("BrowserMainWindow"));
    QByteArray data = settings.value(QLatin1String("defaultState")).toByteArray();
    restoreState(data);
    settings.endGroup();
}
void BitcoinGUI::updateStatusbarActionText(bool visible)
{
    m_viewStatusbar->setText(!visible ? tr("Show Status Bar") : tr("Hide Status Bar"));
}
void BitcoinGUI::handleFindTextResult(bool found)
{
    if (!found)
        slotUpdateStatusbar(tr("\"%1\" not found.").arg(m_lastSearch));
}
void BitcoinGUI::save()
{
    saveSession();

    QSettings settings;
    settings.beginGroup(QLatin1String("BrowserMainWindow"));
    QByteArray data = saveState(false);
    settings.setValue(QLatin1String("defaultState"), data);
    settings.endGroup();
}