// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "mainview.h"

//#include "addressbookpage.h"
//#include "askpassphrasedialog.h"
#include "browser.h"
//#include "clientmodel.h"
#include "guiutil.h"
//#include "optionsmodel.h"
#include "webpage.h"
//#include "overviewpage.h"
///#include "receivecoinsdialog.h"
//#include "sendcoinsdialog.h"
//#include "signverifymessagedialog.h"
//#include "transactiontablemodel.h"
//#include "transactionview.h"
#include "walletmodel.h"
#include "jsinterface.h"
#include "ui_interface.h"
#include "ccc/filepackage.h"
#include "ccc/settings.h"
#include "json/json_spirit_reader_template.h"
#include "util.h"
#include "urllineedit.h"
#include "history.h"
//#include "history.h"
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QProgressDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtWidgets/QStackedWidget>
#include <QtCore/QMimeData>
#include <QtGui/QClipboard>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QListView>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtGui/QDrag>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QStyle>
#include <QtWidgets/QToolButton>
#include <QtCore/QDebug>
using namespace boost::filesystem;

MainView::MainView(QString languageIn,BitcoinGUI *parent,JsInterface *_js):
    QTabWidget(parent),
        language(languageIn),
        jsInterface(_js)
, m_recentlyClosedTabsAction(0)
    , m_newTabAction(0)
    , m_closeTabAction(0)
    , m_nextTabAction(0)
    , m_previousTabAction(0)
    , m_recentlyClosedTabsMenu(0)
    , m_lineEditCompleter(0)
    , m_lineEdits(0)
    , m_tabBar(new TabBar(this))
    //clientModel(0)
    //walletModel(0)
{
    QObject::connect(jsInterface,  SIGNAL(gotoCustomPage(QUrl,int)),
                     this,  SLOT(gotoCustomPage(QUrl,int)));
    setElideMode(Qt::ElideRight);
    connect(m_tabBar, SIGNAL(newTab()), this, SLOT(newTab()));
    connect(m_tabBar, SIGNAL(closeTab(int)), this, SLOT(closeTab(int)));
    connect(m_tabBar, SIGNAL(cloneTab(int)), this, SLOT(cloneTab(int)));
    connect(m_tabBar, SIGNAL(closeOtherTabs(int)), this, SLOT(closeOtherTabs(int)));
    connect(m_tabBar, SIGNAL(reloadTab(int)), this, SLOT(reloadTab(int)));
    connect(m_tabBar, SIGNAL(reloadAllTabs()), this, SLOT(reloadAllTabs()));
    connect(m_tabBar, SIGNAL(tabMoved(int,int)), this, SLOT(moveTab(int,int)));
    setTabBar(m_tabBar);
    setDocumentMode(true);
    setStyleSheet("QTabBar::tab { max-width: 150px;alignment:left; }QTabBar::tab:hover{background:rgb(255, 255, 255, 100);}");    
    // Actions
    m_newTabAction = new QAction(QIcon(QLatin1String(":addtab.png")), tr("New &Tab"), this);
    m_newTabAction->setShortcuts(QKeySequence::AddTab);
    m_newTabAction->setIconVisibleInMenu(false);
    connect(m_newTabAction, SIGNAL(triggered()), this, SLOT(newTab()));

    m_closeTabAction = new QAction(QIcon(QLatin1String(":closetab.png")), tr("&Close Tab"), this);
    m_closeTabAction->setShortcuts(QKeySequence::Close);
    m_closeTabAction->setIconVisibleInMenu(false);
    connect(m_closeTabAction, SIGNAL(triggered()), this, SLOT(closeTab()));

    m_nextTabAction = new QAction(tr("Show Next Tab"), this);
    QList<QKeySequence> shortcuts;
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BraceRight));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_PageDown));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BracketRight));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Less));
    m_nextTabAction->setShortcuts(shortcuts);
    connect(m_nextTabAction, SIGNAL(triggered()), this, SLOT(nextTab()));

    m_previousTabAction = new QAction(tr("Show Previous Tab"), this);
    shortcuts.clear();
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BraceLeft));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_PageUp));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_BracketLeft));
    shortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_Greater));
    m_previousTabAction->setShortcuts(shortcuts);
    connect(m_previousTabAction, SIGNAL(triggered()), this, SLOT(previousTab()));

    m_recentlyClosedTabsMenu = new QMenu(this);
    connect(m_recentlyClosedTabsMenu, SIGNAL(aboutToShow()),
            this, SLOT(aboutToShowRecentTabsMenu()));
    connect(m_recentlyClosedTabsMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(aboutToShowRecentTriggeredAction(QAction*)));
    m_recentlyClosedTabsAction = new QAction(tr("Recently Closed Tabs"), this);
    m_recentlyClosedTabsAction->setMenu(m_recentlyClosedTabsMenu);
    m_recentlyClosedTabsAction->setEnabled(false);

    connect(this, SIGNAL(currentChanged(int)),
            this, SLOT(currentChanged(int)));

    m_lineEdits = new QStackedWidget(this);
}

MainView::~MainView()
{
}

void MainView::gotoWebPage(int nPageID,QUrl url,int nFromPageID)
{
    LogPrintf("MainView gotowebpage pageid:%i,url:%s \n",nPageID,url.toString().toStdString());
    //for (std::vector<WebPage*>::iterator it=vWebPages.begin();it!=vWebPages.end();it++){
    //    if (vWebPages*it->nPageID==nPageID){
    for(unsigned int i=0;i<vWebPages.size();i++){
        if (vWebPages[i]->nPageID==nPageID){
            if(url!=QUrl("")&&url!=vWebPages[i]->url())
                vWebPages[i]->setUrl(url);
            setCurrentWidget(vWebPages[i]);
            return;
        }            
    }
    LogPrintf("MainView gotowebpage1 \n");
    WebView* webpage=new WebView(language,this,jsInterface,url,nPageID,nFromPageID);
    LogPrintf("MainView gotowebpage2 \n");
    vWebPages.push_back(webpage);  
    LogPrintf("MainView gotowebpage3 \n");
    //addTab(webpage, tr(GetPageName(nPageID).c_str()));
    LogPrintf("MainView gotowebpage4 \n");
    //addWidget(*vWebPages.rbegin());
    setCurrentWidget(webpage);
    LogPrintf("MainView gotowebpage5 \n");
    //setCurrentWidget(*vWebPages.rbegin());
    //gotoWebPage(nPageID);
    //setCurrentWidget(*vWebPages.rbegin());
    //setCurrentWidget(walletPage);
}
void MainView::gotoCustomPage(QUrl url,int nFromPageID)
{
    //BitcoinGUI *mw=qobject_cast<BitcoinGUI*>(parent());
    QString qstrUrl=url.toString();
    //mw->loadPage(qstrUrl);
    emit loadPage(qstrUrl);
    //gotoWebPage(255,url,nFromPageID);
}
void MainView::loadWebPage(int nPageID)
{
    //QDir dir(QString().fromStdString(GetDataDir().string()));
    //QUrl url= QUrl("file://"+dir.path().toUtf8() + "/appdata/settings/filepackage/settings_en.html"); 
    string strPath;
    GetFilePackageMain(mapPageNames[nPageID],strPath,true);
    QUrl url=QUrl(QString().fromStdString(strPath));
    
    LogPrintf("gotosettings page url:%s \n",url.toString().toStdString());
    gotoWebPage(nPageID,url);
}
//void MainView::closeWebPage(int nPageID,int nSwitchToPageID){
//    for(std::vector<WebPage*>::iterator it=vWebPages.begin();it!=vWebPages.end();it++){
//        if((*it)->nPageID==nPageID)
//        {
//            delete *it;
//            vWebPages.erase(it);
//        }
//    }
//    if (nSwitchToPageID>0&&nSwitchToPageID<=11)
//        gotoWebPage(nSwitchToPageID);
//}

void MainView::showProgress(const QString &title, int nProgress)
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

void MainView::installWebPage(const string strPageName)
{
    if(CheckFilePackage(strPageName))
        return;
    boost::filesystem::path fpPath=GetDataDir()  / "appdata" / strPageName / "filepackage"  ;  
    // char buf[80];
    //getcwd(buf, sizeof(buf));
    //LogPrintf("current working directory : %s\n", buf);
    //std::string str(buf);
    //boost::filesystem::path fpPath=system_complete(str).parent_path().parent_path().parent_path() / "cccpages" / strPageName; 
    LogPrintf("current working directory : %s\n", fpPath.string());
    boost::filesystem::path fpFile=fpPath / (strPageName+".package.json");
    string filename=fpFile.string();
    boost::filesystem::create_directories(fpPath);
    // boost::filesystem::remove_all(fpPath);
    string str=qrcFileToString(":/"+strPageName+".package.json");
    json_spirit::Array arrFiles;
    std::string strMainFile;
    StringToFile(filename,str);
    ReadFilePackageList(str,strMainFile,arrFiles);
    for(unsigned int i=0;i<arrFiles.size();i++)
            {
                Object obj=arrFiles[i].get_obj();
        
        copyQrcToDisc(obj[0].name_,obj[0].value_.get_str());
    }
}

std::string MainView::qrcFileToString(const std::string fileName)
{
    QFile f(QString().fromStdString(fileName));
        if(!f.open(QIODevice::WriteOnly|QIODevice::Text)){
            LogPrintf("qrcFileToString %s failed \n",fileName);
            return "";
        }        
        QTextStream out(&f);  
        //out< ;
        QString qstr;
        out>>qstr;
        f.close();
        return qstr.toStdString();
}
bool MainView::copyQrcToDisc(string to,string from)
{
    QFile fin(QString().fromStdString(":/"+from));       
    if(!fin.open(QIODevice::ReadOnly))
        return false;
    QDataStream in(&fin);  
    QString qstr;  
    in>>qstr; 
    boost::filesystem::path fpPath=GetDataDir()  / "appdata" / to / "filepackage";  
    std::cout<<"remove filename result:"<<fpPath.remove_filename().string()<<"\n";
    if(boost::filesystem::create_directories(fpPath.remove_filename()))
       return StringToFile(fpPath.string(),qstr.toStdString());
    return false;
}

QWidget *MainView::lineEditStack() const
{
    return m_lineEdits;
}
WebView *MainView::currentWebView() const
{
    return getWebView(currentIndex());
}
QLineEdit *MainView::currentLineEdit() const
{
    return lineEdit(m_lineEdits->currentIndex());
}
QLineEdit *MainView::lineEdit(int index) const
{
    UrlLineEdit *urlLineEdit = qobject_cast<UrlLineEdit*>(m_lineEdits->widget(index));
    if (urlLineEdit)
        return urlLineEdit->lineEdit();
    return 0;
}
QAction *MainView::nextTabAction() const
{
    return m_nextTabAction;
}

QAction *MainView::previousTabAction() const
{
    return m_previousTabAction;
}
void MainView::loadUrlInCurrentTab(const QUrl &url)
{
    WebView *webPage = currentWebView();
    if (webPage) {
        webPage->loadUrl(url);
        webPage->setFocus();
    }
}
WebView *MainView::getWebView(int index) const
{
    QWidget *widget = this->widget(index);
    if (WebView *webPage = qobject_cast<WebView*>(widget)) {
        return webPage;
    } else {
         //optimization to delay creating the first webview
        if (count() == 1) {
            MainView *that = const_cast<MainView*>(this);
            that->setUpdatesEnabled(false);
            that->newTab();
            that->closeTab(0);
            that->setUpdatesEnabled(true);
            return currentWebView();
        }
    }
    return 0;
}
void MainView::addWebAction(QAction *action, QWebPage::WebAction webAction)
{
    if (!action)
        return;
    m_actions.append(new WebActionMapper(action, webAction, this));
}

WebView *MainView::newTab(bool makeCurrent,QUrl url,int nPageID)
{
    LogPrintf("MainView newtab pageid:%i,url:%s \n",nPageID,url.toString().toStdString());
    
    // line edit
    UrlLineEdit *urlLineEdit = new UrlLineEdit;
    QLineEdit *lineEdit = urlLineEdit->lineEdit();
    if (!m_lineEditCompleter && count() > 0) {
        HistoryCompletionModel *completionModel = new HistoryCompletionModel(this);
        completionModel->setSourceModel(BitcoinGUI::historyManager()->historyFilterModel());
        m_lineEditCompleter = new QCompleter(completionModel, this);
        // Should this be in Qt by default?
        QAbstractItemView *popup = m_lineEditCompleter->popup();
        QListView *listView = qobject_cast<QListView*>(popup);
        if (listView)
            listView->setUniformItemSizes(true);
    }
    lineEdit->setCompleter(m_lineEditCompleter);
    connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(lineEditReturnPressed()));
    m_lineEdits->addWidget(urlLineEdit);
    m_lineEdits->setSizePolicy(lineEdit->sizePolicy());

 //    optimization to delay creating the more expensive WebView, history, etc
//    if (count() == 0&&url==QUrl("")) {
//        QWidget *emptyWidget = new QWidget;
//        QPalette p = emptyWidget->palette();
//        p.setColor(QPalette::Window, palette().color(QPalette::Base));
//        emptyWidget->setPalette(p);
//        emptyWidget->setAutoFillBackground(true);
//        disconnect(this, SIGNAL(currentChanged(int)),
//            this, SLOT(currentChanged(int)));
//        addTab(emptyWidget, tr("(Untitled)"));
//        connect(this, SIGNAL(currentChanged(int)),
//            this, SLOT(currentChanged(int)));
//        return 0;
//    }

    // webview
    WebView *webPage = new WebView(language,this,jsInterface,url,nPageID);
    urlLineEdit->setWebView(webPage);
    connect(webPage, SIGNAL(loadStarted()),
            this, SLOT(webViewLoadStarted()));
    connect(webPage, SIGNAL(iconChanged()),
            this, SLOT(webViewIconChanged()));
    connect(webPage, SIGNAL(titleChanged(QString)),
            this, SLOT(webViewTitleChanged(QString)));
    connect(webPage, SIGNAL(urlChanged(QUrl)),
            this, SLOT(webViewUrlChanged(QUrl)));
    connect(webPage->page(), SIGNAL(windowCloseRequested()),
            this, SLOT(windowCloseRequested()));
    connect(webPage->page(), SIGNAL(geometryChangeRequested(QRect)),
            this, SIGNAL(geometryChangeRequested(QRect)));
//#if defined(QWEBPAGE_PRINTREQUESTED)
    connect(webPage->page(), SIGNAL(printRequested(QWebFrame*)),
            this, SIGNAL(printRequested(QWebFrame*)));
//#endif
#if defined(QWEBPAGE_MENUBARVISIBILITYCHANGEREQUESTED)
    connect(webPage->page(), SIGNAL(menuBarVisibilityChangeRequested(bool)),
            this, SIGNAL(menuBarVisibilityChangeRequested(bool)));
#endif
#if defined(QWEBPAGE_STATUSBARVISIBILITYCHANGEREQUESTED)
    connect(webPage->page(), SIGNAL(statusBarVisibilityChangeRequested(bool)),
            this, SIGNAL(statusBarVisibilityChangeRequested(bool)));
#endif
#if defined(QWEBPAGE_TOOLBARVISIBILITYCHANGEREQUESTED)
    connect(webPage->page(), SIGNAL(toolBarVisibilityChangeRequested(bool)),
            this, SIGNAL(toolBarVisibilityChangeRequested(bool)));
#endif
    addTab(webPage, tr("(Untitled)"));
    if (makeCurrent)
        setCurrentWidget(webPage);

    // webview actions
    for (int i = 0; i < m_actions.count(); ++i) {
        WebActionMapper *mapper = m_actions[i];
        mapper->addChild(webPage->page()->action(mapper->webAction()));
    }

    if (count() == 1)
        currentChanged(currentIndex());
    emit tabsChanged();
    webPage->loadUrl(url);
    return webPage;
}
void MainView::closeTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    bool hasFocus = false;
    if (WebView *tab = getWebView(index)) {
#if defined(QWEBPAGE_ISMODIFIED)
        if (tab->isModified()) {
            QMessageBox closeConfirmation(tab);
            closeConfirmation.setWindowFlags(Qt::Sheet);
            closeConfirmation.setWindowTitle(tr("Do you really want to close this page?"));
            closeConfirmation.setInformativeText(tr("You have modified this page and when closing it you would lose the modification.\n"
                                                     "Do you really want to close this page?\n"));
            closeConfirmation.setIcon(QMessageBox::Question);
            closeConfirmation.addButton(QMessageBox::Yes);
            closeConfirmation.addButton(QMessageBox::No);
            closeConfirmation.setEscapeButton(QMessageBox::No);
            if (closeConfirmation.exec() == QMessageBox::No)
                return;
        }
#endif
        hasFocus = tab->hasFocus();

#if defined(QTWEB_PRIVATEBROWSING)
        QWebSettings *globalSettings = QWebSettings::globalSettings();
        if (!globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
#endif
        {
            m_recentlyClosedTabsAction->setEnabled(true);
            m_recentlyClosedTabs.prepend(tab->url());
            if (m_recentlyClosedTabs.size() >= MainView::m_recentlyClosedTabsSize)
                m_recentlyClosedTabs.removeLast();
        }
    }
    QWidget *lineEdit = m_lineEdits->widget(index);
    m_lineEdits->removeWidget(lineEdit);
    lineEdit->deleteLater();
    QWidget *webPage = widget(index);
    removeTab(index);
    webPage->deleteLater();
    emit tabsChanged();
    if (hasFocus && count() > 0)
        currentWebView()->setFocus();
    LogPrintf("close tab,tabs left:%i \n",count());
    if (count() == 0)
        emit lastTabClosed();
}
void MainView::moveTab(int fromIndex, int toIndex)
{
    //LogPrintf("MainView::moveTab(int fromIndex %i, int toIndex%i  \n)",fromIndex,toIndex);    
        QWidget *lineEdit = m_lineEdits->widget(fromIndex);
        //LogPrintf("MainView::moveTab lineEdit%i  \n)",lineEdit); 
        //QWidget *lineEdit2 = m_lineEdits->widget(toIndex);
        //LogPrintf("MainView::moveTablineEdit2%i  \n)",lineEdit2); 
        if(lineEdit!=NULL)
        {
            m_lineEdits->removeWidget(lineEdit);
           // LogPrintf("MainView::moveTab(int fromIndex %i, int toIndex%i  \n)",fromIndex,toIndex); 
        //if(lineEdit2!=NULL)
        //    m_lineEdits->removeWidget(lineEdit2);
        //LogPrintf("MainView::moveTab(int fromIndex %i, int toIndex%i  \n)",fromIndex,toIndex); 
            m_lineEdits->insertWidget(toIndex, lineEdit);
        }
    //LogPrintf("MainView::moveTab(int fromIndex %i, int toIndex%i  \n)",fromIndex,toIndex); 
    //m_lineEdits->insertWidget(fromIndex, lineEdit2);
}
// When index is -1 index chooses the current tab
void MainView::reloadTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;

    QWidget *widget = this->widget(index);
    if (WebView *tab = qobject_cast<WebView*>(widget))
        tab->reload();
}
void MainView::cloneTab(int index)
{
    if (index < 0)
        index = currentIndex();
    if (index < 0 || index >= count())
        return;
    WebView *tab = newTab(false);
    tab->setUrl(getWebView(index)->url());
}
void MainView::closeOtherTabs(int index)
{
    if (-1 == index)
        return;
    for (int i = count() - 1; i > index; --i)
        closeTab(i);
    for (int i = index - 1; i >= 0; --i)
        closeTab(i);
}
void MainView::reloadAllTabs()
{
    for (int i = 0; i < count(); ++i) {
        QWidget *tabWidget = widget(i);
        if (WebView *tab = qobject_cast<WebView*>(tabWidget)) {
            tab->reload();
        }
    }
}
void MainView::nextTab()
{
    int next = currentIndex() + 1;
    if (next == count())
        next = 0;
    setCurrentIndex(next);
}

void MainView::previousTab()
{
    int next = currentIndex() - 1;
    if (next < 0)
        next = count() - 1;
    setCurrentIndex(next);
}
QAction *MainView::newTabAction() const
{
    return m_newTabAction;
}

QAction *MainView::closeTabAction() const
{
    return m_closeTabAction;
}

QAction *MainView::recentlyClosedTabsAction() const
{
    return m_recentlyClosedTabsAction;
}
void MainView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!childAt(event->pos())
            // Remove the line below when QTabWidget does not have a one pixel frame
            && event->pos().y() < (tabBar()->y() + tabBar()->height())) {
        newTab();
        return;
    }
    QTabWidget::mouseDoubleClickEvent(event);
}

void MainView::contextMenuEvent(QContextMenuEvent *event)
{
    if (!childAt(event->pos())) {
        m_tabBar->contextMenuRequested(event->pos());
        return;
    }
    QTabWidget::contextMenuEvent(event);
}

void MainView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MidButton && !childAt(event->pos())
            // Remove the line below when QTabWidget does not have a one pixel frame
            && event->pos().y() < (tabBar()->y() + tabBar()->height())) {
        QUrl url(QApplication::clipboard()->text(QClipboard::Selection));
        if (!url.isEmpty() && url.isValid() && !url.scheme().isEmpty()) {
            WebView *webView = newTab();
            webView->setUrl(url);
        }
    }
}
void MainView::currentChanged(int index)
{
    WebView *webView = this->getWebView(index);
    if (!webView)
        return;
    //TODO reactive this assert
    //Q_ASSERT(m_lineEdits->count() == count());

    WebView *oldWebView = this->getWebView(m_lineEdits->currentIndex());
    if (oldWebView) {
//#if defined(QWEBEVIEW_STATUSBARMESSAGE)
        disconnect(oldWebView, SIGNAL(statusBarMessage(QString)),
                this, SIGNAL(showStatusBarMessage(QString)));
//#endif
        disconnect(oldWebView->page(), SIGNAL(linkHovered(const QString&,const QString&,const QString&)),
                this, SIGNAL(linkHovered(const QString&)));
        disconnect(oldWebView, SIGNAL(loadProgress(int)),
                this, SIGNAL(loadProgress(int)));
    }

//#if defined(QWEBVIEW_STATUSBARMESSAGE)
    connect(webView, SIGNAL(statusBarMessage(QString)),
            this, SIGNAL(showStatusBarMessage(QString)));
//#endif
    connect(webView->page(), SIGNAL(linkHovered(const QString&,const QString&,const QString&)),
            this, SIGNAL(linkHovered(const QString&)));
    connect(webView, SIGNAL(loadProgress(int)),
            this, SIGNAL(loadProgress(int)));

    for (int i = 0; i < m_actions.count(); ++i) {
        WebActionMapper *mapper = m_actions[i];
        mapper->updateCurrent(webView->page());
    }
    emit setCurrentTitle(webView->title());
    m_lineEdits->setCurrentIndex(index);
    emit loadProgress(webView->progress());
    emit showStatusBarMessage(webView->lastStatusBarText());
    if (webView->url().isEmpty())
        m_lineEdits->currentWidget()->setFocus();
    else
        webView->setFocus();
}
void MainView::aboutToShowRecentTabsMenu()
{
    m_recentlyClosedTabsMenu->clear();
    for (int i = 0; i < m_recentlyClosedTabs.count(); ++i) {
        QAction *action = new QAction(m_recentlyClosedTabsMenu);
        action->setData(m_recentlyClosedTabs.at(i));
        QIcon icon = BitcoinApplication::instance()->getWindow()->icon(m_recentlyClosedTabs.at(i));
        action->setIcon(icon);
        action->setText(m_recentlyClosedTabs.at(i).toString());
        m_recentlyClosedTabsMenu->addAction(action);
    }
}

void MainView::aboutToShowRecentTriggeredAction(QAction *action)
{
    QUrl url = action->data().toUrl();
    loadUrlInCurrentTab(url);
}
void MainView::webViewLoadStarted()
{
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index) {
        QIcon icon(QLatin1String(":loading.gif"));
        setTabIcon(index, icon);
    }
}

void MainView::webViewIconChanged()
{
    
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    //LogPrintf("MainView::webViewIconChanged:index%s \n",index);
    if (-1 != index) {
        QIcon icon = webView->icon();
        setTabIcon(index, icon);
    }
}

void MainView::webViewTitleChanged(const QString &title)
{
    //LogPrintf("slot triggered:MainView::webViewTitleChanged\n");
    QString title1=title;//.left(20);            
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index) {
        setTabText(index, title1);
    }
    if (currentIndex() == index)
        emit setCurrentTitle(title1);
    //LogPrintf("MainView::webViewTitleChanged %s \n",title1.toStdString().c_str());
    BitcoinGUI::historyManager()->updateHistoryItem(webView->url(), title1);
}

void MainView::webViewUrlChanged(const QUrl &url)
{
    //LogPrintf("slot triggered:MainView::webViewTitleChanged\n");
    WebView *webView = qobject_cast<WebView*>(sender());
    int index = webViewIndex(webView);
    if (-1 != index) {
        m_tabBar->setTabData(index, url);
    }
    emit tabsChanged();
}
void MainView::lineEditReturnPressed()
{
    if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(sender())) {
        emit loadPage(lineEdit->text());
        if (m_lineEdits->currentWidget() == lineEdit)
            currentWebView()->setFocus();
    }
}

void MainView::windowCloseRequested()
{
    WebPage *webPage = qobject_cast<WebPage*>(sender());
    WebView *webView = qobject_cast<WebView*>(webPage->view());
    int index = webViewIndex(webView);
    if (index >= 0) {
        if (count() == 1)
            webView->webPage()->mainWindow()->close();
        else
            closeTab(index);
    }
}
static const qint32 TabWidgetMagic = 0xaa;
QByteArray MainView::saveState() const
{
    int version = 1;
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << qint32(TabWidgetMagic);
    stream << qint32(version);

    QStringList tabs;
    for (int i = 0; i < count(); ++i) {
        if (WebView *tab = qobject_cast<WebView*>(widget(i))) {
            tabs.append(tab->url().toString());
        } else {
            tabs.append(QString::null);
        }
    }
    stream << tabs;
    stream << currentIndex();
    return data;
}

bool MainView::restoreState(const QByteArray &state)
{
    int version = 1;
    QByteArray sd = state;
    QDataStream stream(&sd, QIODevice::ReadOnly);
    if (stream.atEnd())
        return false;

    qint32 marker;
    qint32 v;
    stream >> marker;
    stream >> v;
    if (marker != TabWidgetMagic || v != version)
        return false;

    QStringList openTabs;
    stream >> openTabs;

    for (int i = 0; i < openTabs.count(); ++i) {
        if (i != 0)
            newTab();
        loadPage(openTabs.at(i));
    }

    int currentTab;
    stream >> currentTab;
    setCurrentIndex(currentTab);

    return true;
}
int MainView::webViewIndex(WebView *webView) const
{
    int index = indexOf(webView);
    return index;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WebActionMapper::WebActionMapper(QAction *root, QWebPage::WebAction webAction, QObject *parent)
    : QObject(parent)
    , m_currentParent(0)
    , m_root(root)
    , m_webAction(webAction)
{
    if (!m_root)
        return;
    connect(m_root, SIGNAL(triggered()), this, SLOT(rootTriggered()));
    connect(root, SIGNAL(destroyed(QObject*)), this, SLOT(rootDestroyed()));
    root->setEnabled(false);
}
void WebActionMapper::rootDestroyed()
{
    //LogPrintf("slot triggered:WebActionMapper::rootDestroyed \n");
    m_root = 0;
}

void WebActionMapper::currentDestroyed()
{
    //LogPrintf("slot triggered:WebActionMapper::currentDestroyed \n");
    updateCurrent(0);
}

void WebActionMapper::addChild(QAction *action)
{
    if (!action)
        return;
    connect(action, SIGNAL(changed()), this, SLOT(childChanged()));
}

QWebPage::WebAction WebActionMapper::webAction() const
{
    return m_webAction;
}

void WebActionMapper::rootTriggered()
{
   // LogPrintf("slot triggered:WebActionMapper::rootTriggered \n");
    if (m_currentParent) {
        QAction *gotoAction = m_currentParent->action(m_webAction);
        gotoAction->trigger();
    }
}

void WebActionMapper::childChanged()
{
    //LogPrintf("slot triggered:WebActionMapper::childChanged \n");
    if (QAction *source = qobject_cast<QAction*>(sender())) {
        if (m_root
            && m_currentParent
            && source->parent() == m_currentParent) {
            m_root->setChecked(source->isChecked());
            m_root->setEnabled(source->isEnabled());
        }
    }
}

void WebActionMapper::updateCurrent(QWebPage *currentParent)
{
    if (m_currentParent)
        disconnect(m_currentParent, SIGNAL(destroyed(QObject*)),
                   this, SLOT(currentDestroyed()));

    m_currentParent = currentParent;
    if (!m_root)
        return;
    if (!m_currentParent) {
        m_root->setEnabled(false);
        m_root->setChecked(false);
        return;
    }
    QAction *source = m_currentParent->action(m_webAction);
    m_root->setChecked(source->isChecked());
    m_root->setEnabled(source->isEnabled());
    connect(m_currentParent, SIGNAL(destroyed(QObject*)),
            this, SLOT(currentDestroyed()));
}


//////////////////////////////////////////////////////////////////////////////////////////////////////

void TabBar::reloadTab()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        int index = action->data().toInt();
        emit reloadTab(index);
    }
}
TabBar::TabBar(QWidget *parent)
    : QTabBar(parent)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setAcceptDrops(true);    
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(contextMenuRequested(QPoint)));

    QString ctrl = QLatin1String("Ctrl+%1");
    for (int i = 1; i <= 10; ++i) {
        int key = i;
        if (key == 10)
            key = 0;
        QShortcut *shortCut = new QShortcut(ctrl.arg(key), this);
        m_tabShortcuts.append(shortCut);
        connect(shortCut, SIGNAL(activated()), this, SLOT(selectTabAction()));
    }
    setTabsClosable(true);
    connect(this, SIGNAL(tabCloseRequested(int)),
            this, SIGNAL(closeTab(int)));
    setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
    setMovable(true);
}

void TabBar::selectTabAction()
{
    if (QShortcut *shortCut = qobject_cast<QShortcut*>(sender())) {
        int index = m_tabShortcuts.indexOf(shortCut);
        if (index == 0)
            index = 10;
        setCurrentIndex(index);
    }
}

void TabBar::contextMenuRequested(const QPoint &position)
{
    QMenu menu;
    menu.addAction(tr("New &Tab"), this, SIGNAL(newTab()), QKeySequence::AddTab);
    int index = tabAt(position);
    if (-1 != index) {
        QAction *action = menu.addAction(tr("Clone Tab"),
                this, SLOT(cloneTab()));
        action->setData(index);

        menu.addSeparator();

        action = menu.addAction(tr("&Close Tab"),
                this, SLOT(closeTab()), QKeySequence::Close);
        action->setData(index);

        action = menu.addAction(tr("Close &Other Tabs"),
                this, SLOT(closeOtherTabs()));
        action->setData(index);

        menu.addSeparator();

        action = menu.addAction(tr("Reload Tab"),
                this, SLOT(reloadTab()), QKeySequence::Refresh);
        action->setData(index);
    } else {
        menu.addSeparator();
    }
    menu.addAction(tr("Reload All Tabs"), this, SIGNAL(reloadAllTabs()));
    menu.exec(QCursor::pos());
}

void TabBar::cloneTab()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        int index = action->data().toInt();
        emit cloneTab(index);
    }
}

void TabBar::closeTab()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        int index = action->data().toInt();
        emit closeTab(index);
    }
}

void TabBar::closeOtherTabs()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        int index = action->data().toInt();
        emit closeOtherTabs(index);
    }
}

void TabBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_dragStartPos = event->pos();
    QTabBar::mousePressEvent(event);
}

void TabBar::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        int diffX = event->pos().x() - m_dragStartPos.x();
        int diffY = event->pos().y() - m_dragStartPos.y();
        if ((event->pos() - m_dragStartPos).manhattanLength() > QApplication::startDragDistance()
            && diffX < 3 && diffX > -3
            && diffY < -10) {
            QDrag *drag = new QDrag(this);
            QMimeData *mimeData = new QMimeData;
            QList<QUrl> urls;
            int index = tabAt(event->pos());
            QUrl url = tabData(index).toUrl();
            urls.append(url);
            mimeData->setUrls(urls);
            mimeData->setText(tabText(index));
            mimeData->setData(QLatin1String("action"), "tab-reordering");
            drag->setMimeData(mimeData);
            drag->exec();
        }
    }
    QTabBar::mouseMoveEvent(event);
}
