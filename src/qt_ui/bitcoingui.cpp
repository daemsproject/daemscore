// Copyright (c) 2011-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bitcoingui.h"

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
#ifdef ENABLE_WALLET
//#include "mainframe.h"
#include "mainview.h"
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

#include <QAction>
#include <QApplication>
#include <QDateTime>
#include <QDesktopWidget>
#include <QDragEnterEvent>
#include <QIcon>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QProgressBar>
#include <QProgressDialog>
#include <QSettings>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QSettings>
#if QT_VERSION < 0x050000
#include <QTextDocument>
#include <QUrl>
#else
#include <QUrlQuery>
#endif

const QString BitcoinGUI::DEFAULT_WALLET = "~Default";

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
    //overviewAction(0),
    //historyAction(0),
    quitAction(0),
    //sendCoinsAction(0),
    //usedSendingAddressesAction(0),
    //usedReceivingAddressesAction(0),
    //signMessageAction(0),
    //verifyMessageAction(0),
    aboutAction(0),
    //receiveCoinsAction(0),
    //optionsAction(0),
    toggleHideAction(0),
    //encryptWalletAction(0),
    //backupWalletAction(0),
    changePassphraseAction(0),
    //aboutQtAction(0),
    //openRPCConsoleAction(0),
    //openAction(0),
    showHelpMessageAction(0),
    trayIcon(0),
    trayIconMenu(0),
    notificator(0),
    //rpcConsole(0),
    prevBlocks(0),
    spinnerFrame(0)
{
    jsInterface=new JsInterface(this);
    GUIUtil::restoreWindowGeometry("nWindow", QSize(850, 550), this);

    QString windowTitle = tr("Cccoin Browser");
#ifdef ENABLE_WALLET
    /* if compiled with wallet support, -disablewallet can still disable the wallet */
    enableWallet = !GetBoolArg("-disablewallet", false);
#else
    enableWallet = false;
#endif // ENABLE_WALLET
//    if(enableWallet)
//    {
//        windowTitle += tr("Wallet");
//    } else {
//        windowTitle += tr("Node");
//    }
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

    //rpcConsole = new RPCConsole(enableWallet ? this : 0);
//#ifdef ENABLE_WALLET
    //if(enableWallet)
    //{
        /** Create wallet frame and make it the central widget */
    //LogPrintf("bitcoingui: 0 \n");
        mainView = new MainView(language,this,jsInterface);
        connect(mainView, SIGNAL(showNormalIfMinimized()), this, SLOT(showNormalIfMinimized()));
       setCentralWidget(mainView);
    //} else
//#endif // ENABLE_WALLET
//    {
        /* When compiled without wallet or -disablewallet is provided,
         * the central widget is the rpc console.
         */
        //setCentralWidget(rpcConsole);
//    }

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
}

BitcoinGUI::~BitcoinGUI()
{
    // Unsubscribe from notifications from core
    unsubscribeFromCoreSignals();

    GUIUtil::saveWindowGeometry("nWindow", this);
    if(trayIcon) // Hide tray icon, as deleting will let it linger until quit (on Ubuntu)
        trayIcon->hide();
#ifdef Q_OS_MAC
    delete appMenuBar;
    MacDockIconHandler::instance()->setMainWindow(NULL);
    delete jsInterface;
    jsInteface=NULL;
#endif
}

void BitcoinGUI::createActions(const NetworkStyle *networkStyle)
{
    QActionGroup *tabGroup = new QActionGroup(this);
    walletAction = new QAction(QIcon(":/icons/wallet"), tr("&Wallet"), this);
    walletAction->setStatusTip(tr("Show wallet page"));
    walletAction->setToolTip(walletAction->statusTip());
    walletAction->setCheckable(true);
    walletAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_1));
    tabGroup->addAction(walletAction);    
    browserAction = new QAction(QIcon(":/icons/browser"), tr("&Browser"), this);
    browserAction->setStatusTip(tr("Show browser page"));
    browserAction->setToolTip(browserAction->statusTip());
    browserAction->setCheckable(true);
    browserAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_2));
    tabGroup->addAction(browserAction);
    publisherAction = new QAction(QIcon(":/icons/publisher"), tr("&Publisher"), this);
    publisherAction->setStatusTip(tr("Show publisher page"));
    publisherAction->setToolTip(publisherAction->statusTip());
    publisherAction->setCheckable(true);
    publisherAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_3));
    tabGroup->addAction(publisherAction);
    messengerAction = new QAction(QIcon(":/icons/chat"), tr("&Messenger"), this);
    messengerAction->setStatusTip(tr("Show messenger page"));
    messengerAction->setToolTip(messengerAction->statusTip());
    messengerAction->setCheckable(true);
    messengerAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_C));
    tabGroup->addAction(messengerAction);
    minerAction = new QAction(QIcon(":/icons/tx_mined"), tr("&Miner"), this);
    minerAction->setStatusTip(tr("Show miner page"));
    minerAction->setToolTip(minerAction->statusTip());
    minerAction->setCheckable(true);
    minerAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_M));
    tabGroup->addAction(minerAction);
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
    aboutAction = new QAction(networkStyle->getAppIcon(), tr("&About Cccoin Browser"), this);
    aboutAction->setStatusTip(tr("Show information about Cccoin Browser"));
    aboutAction->setMenuRole(QAction::AboutRole);
#if QT_VERSION < 0x050000
   // aboutQtAction = new QAction(QIcon(":/trolltech/qmessagebox/images/qtlogo-64.png"), tr("About &Qt"), this);
#else
    //aboutQtAction = new QAction(QIcon(":/qt-project.org/qmessagebox/images/qtlogo-64.png"), tr("About &Qt"), this);
#endif
    //aboutQtAction->setStatusTip(tr("Show information about Qt"));
    //aboutQtAction->setMenuRole(QAction::AboutQtRole);
    //optionsAction = new QAction(QIcon(":/icons/options"), tr("&Options..."), this);
    //optionsAction->setStatusTip(tr("Modify configuration options for Cccoin"));
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
    exportAccountAction = new QAction(QIcon(":/icons/key"), tr("&Export Account"), this);
    exportAccountAction->setStatusTip(tr("Export Account"));
    importAccountAction = new QAction(QIcon(":/icons/key"), tr("&Import Account"), this);
    importAccountAction->setStatusTip(tr("Import Account"));
    shopAction = new QAction(QIcon(":/icons/key"), tr("&Shop"), this);
    shopAction->setStatusTip(tr("shop"));
    //signMessageAction = new QAction(QIcon(":/icons/edit"), tr("Sign &message..."), this);
    //signMessageAction->setStatusTip(tr("Sign messages with your Cccoin addresses to prove you own them"));
    //verifyMessageAction = new QAction(QIcon(":/icons/transaction_0"), tr("&Verify message..."), this);
    //verifyMessageAction->setStatusTip(tr("Verify messages to ensure they were signed with specified Cccoin addresses"));

    //openRPCConsoleAction = new QAction(QIcon(":/icons/debugwindow"), tr("&Debug window"), this);
    //openRPCConsoleAction->setStatusTip(tr("Open debugging and diagnostic console"));

//    usedSendingAddressesAction = new QAction(QIcon(":/icons/address-book"), tr("&Sending addresses..."), this);
//    usedSendingAddressesAction->setStatusTip(tr("Show the list of used sending addresses and labels"));
//    usedReceivingAddressesAction = new QAction(QIcon(":/icons/address-book"), tr("&Receiving addresses..."), this);
//    usedReceivingAddressesAction->setStatusTip(tr("Show the list of used receiving addresses and labels"));

    //openAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_FileIcon), tr("Open &URI..."), this);
    //openAction->setStatusTip(tr("Open a cccoin: URI or payment request"));

    showHelpMessageAction = new QAction(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation), tr("&Command-line options"), this);
    showHelpMessageAction->setStatusTip(tr("Show the Cccoin Core help message to get a list with possible Cccoin command-line options"));
//LogPrintf("bitcoingui:createactions 5 \n");
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(aboutClicked()));
    connect(browserAction, SIGNAL(triggered()), this, SLOT(gotoBrowserPage()));
    //connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    //connect(optionsAction, SIGNAL(triggered()), this, SLOT(optionsClicked()));
    connect(toggleHideAction, SIGNAL(triggered()), this, SLOT(toggleHidden()));
    connect(showHelpMessageAction, SIGNAL(triggered()), this, SLOT(showHelpMessageClicked()));
    //LogPrintf("bitcoingui:createactions 6 \n");
#ifdef ENABLE_WALLET
    
        connect(encryptAccountAction, SIGNAL(triggered()), this, SLOT(encryptWallet()));
        connect(decryptAccountAction, SIGNAL(triggered()), this, SLOT(decryptWallet()));
        //LogPrintf("bitcoingui:createactions 7 \n");
        connect(exportAccountAction, SIGNAL(triggered()), this, SLOT(backupWallet()));
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
            
            
        //connect(signMessageAction, SIGNAL(triggered()), this, SLOT(gotoSignMessageTab()));
        //connect(verifyMessageAction, SIGNAL(triggered()), this, SLOT(gotoVerifyMessageTab()));
        //connect(usedSendingAddressesAction, SIGNAL(triggered()), walletFrame, SLOT(usedSendingAddresses()));
        //connect(usedReceivingAddressesAction, SIGNAL(triggered()), walletFrame, SLOT(usedReceivingAddresses()));
        //connect(openAction, SIGNAL(triggered()), this, SLOT(openClicked()));
       
    
#endif // ENABLE_WALLET
}

void BitcoinGUI::createMenuBar()
{
#ifdef Q_OS_MAC
    // Create a decoupled menu bar on Mac which stays even if the window is closed
    appMenuBar = new QMenuBar();
#else
    // Get the main window's menu bar on other platforms
    appMenuBar = menuBar();
#endif

    // Configure the menus
    
    QMenu *file = appMenuBar->addMenu(tr("&File"));  
        //file->addAction(openAction);
        //file->addAction(backupWalletAction);
        //file->addAction(signMessageAction);
        //file->addAction(verifyMessageAction);
        //file->addSeparator();
        //file->addAction(usedSendingAddressesAction);
        //file->addAction(usedReceivingAddressesAction);
        //file->addSeparator();
    file->addAction(quitAction);
    QMenu *account = appMenuBar->addMenu(tr("&Account"));
    
        
        //TODO account actions like create new, view switch, import, export,encrypt, decrypt 
        account->addAction(newAccountAction);
        account->addAction(switchAccountAction);
        account->addAction(encryptAccountAction);
        account->addAction(unlockAccountAction);
        account->addAction(decryptAccountAction);
        account->addAction(lockAccountAction);
        //account->addAction(decryptAccountAction);
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
    
    QMenu *settings = appMenuBar->addMenu(tr("&Settings"));
    QMenu *services = appMenuBar->addMenu(tr("&Services"));
    
    //services->addAction(icqServiceAction);    
    //services->addAction(miningpoolServiceAction);
        
        //settings->addSeparator();
    
    //settings->addAction(optionsAction);

    QMenu *help = appMenuBar->addMenu(tr("&Help"));
    
    {
        //help->addAction(openRPCConsoleAction);
    }
    help->addAction(showHelpMessageAction);
    help->addSeparator();
    help->addAction(aboutAction);
    //help->addAction(aboutQtAction);
}

void BitcoinGUI::createToolBars()
{
    if(mainView)
    {
        QToolBar *toolbar = addToolBar(tr("Tabs toolbar"));
        toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        
        toolbar->addAction(browserAction);
        toolbar->addAction(walletAction);
        walletAction->setEnabled(true);
        walletAction->setChecked(true);        
        toolbar->addAction(publisherAction);
        toolbar->addAction(messengerAction);
        toolbar->addAction(minerAction);        
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
    QUrl walletUrl= QUrl("file://"+QDir::currentPath().toUtf8() + "/res/html/wallet_en.html"); 
    mainView->gotoWebPage(1,walletUrl);//, walletModel);    
    //LogPrintf("bitcoingui addwallet3 \n");
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
    QString toolTip = tr("Cccoin browser client") + " " + networkStyle->getTitleAddText();
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
enum pageid
{
    WALLETPAGE_ID=1,
    BROWSERPAGE_ID=2,
    PUBLISHERPAGE_ID=3,
    MESSENGERPAGE_ID=4,
    MINERPAGE_ID=5,
    DOMAINPAGE_ID=6,
    SETTINGPAGE_ID=7,
    SERVICEPAGE_ID=8
};
void BitcoinGUI::gotoWalletPage()
{
    walletAction->setChecked(true);
    if (mainView) mainView->gotoWebPage(WALLETPAGE_ID);
}
void BitcoinGUI::gotoBrowserPage()
{
    QUrl browserUrl= QUrl("file://"+QDir::currentPath().toUtf8() + "/res/html/browser_en.html"); 
    browserAction->setChecked(true);
    if (mainView) mainView->gotoWebPage(BROWSERPAGE_ID,browserUrl);
}
void BitcoinGUI::gotoPublisherPage()
{
    QUrl url=QUrl("file://"+QDir::currentPath().toUtf8() + "/res/html/publisher_en.html"); 
    publisherAction->setChecked(true);
    if (mainView) mainView->gotoWebPage(PUBLISHERPAGE_ID,url);
}
void BitcoinGUI::gotoMessengerPage()
{
    QUrl messengerUrl= QUrl("file://"+QDir::currentPath().toUtf8() + "/res/html/messenger_en.html"); 
    messengerAction->setChecked(true);
    if (mainView) mainView->gotoWebPage(MESSENGERPAGE_ID,messengerUrl);
}
void BitcoinGUI::gotoMinerPage()
{
    minerAction->setChecked(true);
    QUrl minerUrl= QUrl("file://"+QDir::currentPath().toUtf8() + "/res/html/miner_en.html"); 
    if (mainView) mainView->gotoWebPage(MINERPAGE_ID,minerUrl);
}


//void BitcoinGUI::gotoSignMessageTab(QString addr)
//{
//    if (walletFrame) walletFrame->gotoSignMessageTab(addr);
//}
//
//void BitcoinGUI::gotoVerifyMessageTab(QString addr)
//{
//    if (walletFrame) walletFrame->gotoVerifyMessageTab(addr);
//}
#endif // ENABLE_WALLET

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
    labelConnectionsIcon->setToolTip(tr("%n active connection(s) to Cccoin network", "", count));
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
    QString strTitle = tr("Cccoin"); // default title
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
#ifndef Q_OS_MAC // Ignored on Mac
    if(clientModel)// && clientModel->getOptionsModel())
    {
        //if(!clientModel->getOptionsModel()->getMinimizeToTray() &&
        //   !clientModel->getOptionsModel()->getMinimizeOnClose())
        {
            QApplication::quit();
        }
    }
#endif
    QMainWindow::closeEvent(event);
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
bool BitcoinGUI::handleUserConfirm(QString title,QString message,int nOP,string& strError,SecureString& ssInput){
    LogPrintf("bitcoingui handleUserConfirm received: \n");    
        UserConfirmDialog dlg(this);
        dlg.setWindowTitle(title);
        dlg.ui->label_message->setText(message);
        if (nOP!=1){
            dlg.ui->label_7->hide();
            dlg.ui->passwordEdit->hide();
        }
        
        if(dlg.exec()){
            if(nOP==1){
                ssInput.reserve(MAX_PASSPHRASE_SIZE);    
                ssInput.assign(dlg.ui->passwordEdit->text().toStdString().c_str());           
            }
            LogPrintf("bitcoingui handleUserConfirm ok pressed, \n");             
            return true;//QString("{\"success\":\"tx sent\"}");
            //emit sendMoneyResult(strToken,true,dlg.ui->passwordEdit->text());
        }else{
            LogPrintf("bitcoingui handleUserConfirm cancel pressed \n"); 
            strError="user cancelled";
            return false;//QString("{\"error\":\"payment request cancelled\"}");
            //emit sendMoneyResult(strToken,false,QString().fromStdString("{\"result\":\"dialogue opened\"}"));
        }
}
void BitcoinGUI::subscribeToCoreSignalsJs()
{
    jsInterface->subscribeToCoreSignals();
}