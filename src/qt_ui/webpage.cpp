// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "webpage.h"
#include "jsinterface.h"
#include "browser.h"
#include "mainview.h"
#include "fai/settings.h"
#include "fai/contentutil.h"
//#include "bitcoinunits.h"
//#include "guiconstants.h"
//#include "guiutil.h"
//#include "walletmodel.h"
#include "ui_passworddialog.h"
#include "ui_proxy.h"

#include "featurepermissionbar.h"
#include "downloadmanager.h"
#include "networkaccessmanager.h"
#include <QtWebKitWidgets/QWebView>
#include <QAbstractItemDelegate>
#include <QPainter>
#include <QWebFrame>
#include <QWebElementCollection>
#include <QNetworkDiskCache>
#include <QtGui/QClipboard>
#include <QtNetwork/QAuthenticator>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtGui/QMouseEvent>
//#include <QtUiTools>
//#include <QUiLoader>
//#if defined(QWEBPAGE_HITTESTCONTENT)
#include <QWebHitTestResult>
//#endif

#ifndef QT_NO_UITOOLS
#include <QtUiTools/QUiLoader>
#endif  //QT_NO_UITOOLS

#include <QtCore/QDebug>
#include <QtCore/QBuffer>
#include <QAuthenticator>
#define DECORATION_SIZE 64
#define NUM_ITEMS 3


WebPage::WebPage(QObject *parent)
    : QWebPage(parent)
    , m_keyboardModifiers(Qt::NoModifier)
    , m_pressedButtons(Qt::NoButton)
    , m_openInNewTab(false)
{
    setForwardUnsupportedContent(true);
//#if defined(QWEBPAGE_SETNETWORKACCESSMANAGER)
    QNetworkAccessManager* qnam=BitcoinGUI::networkAccessManager();
    setNetworkAccessManager(qnam);
//#endif
//#if defined(QWEBPAGE_UNSUPPORTEDCONTENT)
    connect(this, SIGNAL(unsupportedContent(QNetworkReply*)),
            this, SLOT(handleUnsupportedContent(QNetworkReply*)));
//#endif
    //connect(this, SIGNAL(authenticationRequired(const QUrl &, QAuthenticator*)),
    //        SLOT(authenticationRequired(const QUrl &, QAuthenticator*)));
    connect(qnam, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(authenticationRequired(QNetworkReply*,QAuthenticator*)));
    //connect(this, SIGNAL(proxyAuthenticationRequired(const QUrl &, QAuthenticator *, const QString &)),
    //        SLOT(proxyAuthenticationRequired(const QUrl &, QAuthenticator *, const QString &)));
    // connect(qnam, SIGNAL(proxyAuthenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(proxyAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
   

}
BitcoinGUI *WebPage::mainWindow()
{
    QObject *w = this->parent();
    while (w) {
        if (BitcoinGUI *mw = qobject_cast<BitcoinGUI*>(w))
            return mw;
        w = w->parent();
    }
    
    return BitcoinApplication::instance()->getWindow();
}
#if defined(QWEBPAGE_ACCEPTNAVIGATIONREQUEST)
bool WebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{
    // ctrl open in new tab
    // ctrl-shift open in new tab and select
    // ctrl-alt open in new window
    if (type == QWebPage::NavigationTypeLinkClicked
        && (m_keyboardModifiers & Qt::ControlModifier
            || m_pressedButtons == Qt::MidButton)) {
        bool newWindow = (m_keyboardModifiers & Qt::AltModifier);
        WebView *webView;
//        if (newWindow) {
//            BrowserApplication::instance()->newMainWindow();
//            BitcoinGUI *newMainWindow = BrowserApplication::instance()->getWindow();
//            webView = newMainWindow->currentTab();
//            newMainWindow->raise();
//            newMainWindow->activateWindow();
//            webView->setFocus();
//        } else {
            bool selectNewTab = (m_keyboardModifiers & Qt::ShiftModifier);
            webView = mainWindow()->getMainView()->newTab(selectNewTab);
        //}
        webView->load(request);
        m_keyboardModifiers = Qt::NoModifier;
        m_pressedButtons = Qt::NoButton;
        return false;
    }
    m_loadingUrl = request.url();
    emit loadingUrl(m_loadingUrl);
}
#endif
QWebPage *WebPage::createWindow(QWebPage::WebWindowType type)
{
   //if (m_openInNewTab || type == QWebPage::WebBrowserTab||type == QWebPage::WebBrowserWindow) {
    //    m_openInNewTab = false;
        return mainWindow()->getMainView()->newTab()->page();
//    }  else {
//        PopupWindow *popup = new PopupWindow(parent()->language);
//        popup->setAttribute(Qt::WA_DeleteOnClose);
//        popup->show();
//        return popup->page();
//    }
}


#if !defined(QT_NO_UITOOLS)
QObject *WebPage::createPlugin(const QString &classId, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues)
{
    //LogPrintf("WebPage::createPlugin \n");
    Q_UNUSED(url);
    Q_UNUSED(paramNames);
    Q_UNUSED(paramValues);
    QUiLoader loader;
    return loader.createWidget(classId, view());
}
#endif // !defined(QT_NO_UITOOLS)
//#if defined(QWEBPAGE_UNSUPPORTEDCONTENT)
void WebPage::handleUnsupportedContent(QNetworkReply *reply)
{
    //LogPrintf("WebPage::handleUnsupportedContent \n");
    BitcoinGUI::downloadManager()->handleUnsupportedContent(reply);
//    QString errorString = reply->errorString();
//
//    if (m_loadingUrl != reply->url()) {
//        // sub resource of this page
//        qWarning() << "Resource" << reply->url().toEncoded() << "has unknown Content-Type, will be ignored.";
//        reply->deleteLater();
//        return;
//    }
//
//    if (reply->error() == QNetworkReply::NoError && !reply->header(QNetworkRequest::ContentTypeHeader).isValid()) {
//        errorString = "Unknown Content-Type";
//    }
//
//    QFile file(QLatin1String(":/notfound.html"));
//    bool isOpened = file.open(QIODevice::ReadOnly);
//    Q_ASSERT(isOpened);
//    Q_UNUSED(isOpened)
//
//    QString title = tr("Error loading page: %1").arg(reply->url().toString());
//    QString html = QString(QLatin1String(file.readAll()))
//                        .arg(title)
//                        .arg(errorString)
//                        .arg(reply->url().toString());
//
//    QBuffer imageBuffer;
//    imageBuffer.open(QBuffer::ReadWrite);
//    QIcon icon = view()->style()->standardIcon(QStyle::SP_MessageBoxWarning, 0, view());
//    QPixmap pixmap = icon.pixmap(QSize(32,32));
//    if (pixmap.save(&imageBuffer, "PNG")) {
//        html.replace(QLatin1String("IMAGE_BINARY_DATA_HERE"),
//                     QString(QLatin1String(imageBuffer.buffer().toBase64())));
//    }
//
//    QList<QWebFrame*> frames;
//    frames.append(mainFrame());
//    while (!frames.isEmpty()) {
//        QWebFrame *frame = frames.takeFirst();
//        if (frame->url() == reply->url()) {
//            frame->setHtml(html, reply->url());
//            return;
//        }
//        QList<QWebFrame *> children = frame->childFrames();
//        foreach (QWebFrame *frame, children)
//            frames.append(frame);
//    }
//    if (m_loadingUrl == reply->url()) {
//        mainFrame()->setHtml(html, reply->url());
//    }
}
//#endif
void WebPage::authenticationRequired(QNetworkReply* reply, QAuthenticator *auth)
{
    BitcoinGUI *gui = mainWindow();
    ;
    QDialog dialog(gui);
    dialog.setWindowFlags(Qt::Sheet);

    Ui::PasswordDialog passwordDialog;
    passwordDialog.setupUi(&dialog);

    passwordDialog.iconLabel->setText(QString());
    passwordDialog.iconLabel->setPixmap(gui->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, gui).pixmap(32, 32));

    QString introMessage = tr("<qt>Enter username and password for \"%1\" at %2</qt>");
    introMessage = introMessage.arg(auth->realm()).arg(reply->url().toString().toHtmlEscaped());
    passwordDialog.introLabel->setText(introMessage);
    passwordDialog.introLabel->setWordWrap(true);

    if (dialog.exec() == QDialog::Accepted) {
        auth->setUser(passwordDialog.userNameLineEdit->text());
        auth->setPassword(passwordDialog.passwordLineEdit->text());
    }
}

void WebPage::proxyAuthenticationRequired(QNetworkReply* reply, QAuthenticator *auth, const QString &proxyHost)
{
    Q_UNUSED(reply);
    BitcoinGUI *gui = mainWindow();

    QDialog dialog(gui);
    dialog.setWindowFlags(Qt::Sheet);

    Ui::ProxyDialog proxyDialog;
    proxyDialog.setupUi(&dialog);

    proxyDialog.iconLabel->setText(QString());
    proxyDialog.iconLabel->setPixmap(gui->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, gui).pixmap(32, 32));

    QString introMessage = tr("<qt>Connect to proxy \"%1\" using:</qt>");
    introMessage = introMessage.arg(proxyHost.toHtmlEscaped());
    proxyDialog.introLabel->setText(introMessage);
    proxyDialog.introLabel->setWordWrap(true);

    if (dialog.exec() == QDialog::Accepted) {
        auth->setUser(proxyDialog.userNameLineEdit->text());
        auth->setPassword(proxyDialog.passwordLineEdit->text());
    }
}



WebView:: WebView(QString languageIn,QWidget *parent,JsInterface *_js,QUrl urlIn,int nPageIDin,int nFromPageIDIn): 
    QWebView(parent)
,language(languageIn)
,jsInterface(_js)
,nPageID(nPageIDin)
,nFromPageID(nFromPageIDIn)
, m_progress(0)
    , m_page(new WebPage(this))
    , m_iconReply(0)
 {

    setPage(m_page);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    // Signal is emitted before frame loads any web content:
    QObject::connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
                     this, SLOT(addJSObject()));
    QObject::connect(jsInterface,  SIGNAL(feedback(QString,QString)),
                     this,  SIGNAL(feedback(QString,QString)));
    QObject::connect(jsInterface,  SIGNAL(notify(QString)),
                     this,  SIGNAL(notify(QString)));
    
    // Load web content now!
    
    
    //setUrl(urlIn);
//#if defined(QWEBPAGE_STATUSBARMESSAGE)
    connect(page(), SIGNAL(statusBarMessage(QString)),
            SLOT(setStatusBarText(QString)));
//#endif
    connect(this, SIGNAL(loadProgress(int)),
            this, SLOT(setProgress(int)));
    connect(this, SIGNAL(loadFinished(bool)),
            this, SLOT(loadFinished(bool)));
    connect(page(), SIGNAL(loadingUrl(QUrl)),
            this, SIGNAL(urlChanged(QUrl)));
    //connect(page(), SIGNAL(iconUrlChanged(QUrl)),
    //        this, SLOT(onIconUrlChanged(QUrl)));
//#if defined(QWEBPAGE_DOWNLOADREQUESTED)
    connect(page(), SIGNAL(downloadRequested(QNetworkRequest)),
            this, SLOT(downloadRequested(QNetworkRequest)));
//#endif
    connect(page(),&WebPage::featurePermissionRequested, this, &WebView::onFeaturePermissionRequested);
//#if defined(QWEBPAGE_UNSUPPORTEDCONTENT)
    page()->setForwardUnsupportedContent(true);
//#endif
   
 }
WebView::~WebView()
{
   // delete ui;
}
//void WebView::block()
//{
//    fBlocked=true;
//}
void WebView::addJSObject() {    
    
    page()->mainFrame()->addToJavaScriptWindowObject(QString("jsinterface"), this);
}
QString WebView::jscall(QString command,QString dataJson)
{
    if(!fBlocked)
    return jsInterface->jscall(command,dataJson,nPageID,GetTabIndex());
    else
        return  QString().fromStdString("{\"error\":\"page is blocked by user\"}");      
        
}
QString WebView::jscallasync(QString command,QString dataJson,QString successfunc,QString errorfunc)
{
    if(!fBlocked)
    return jsInterface->jscallasync(command,dataJson,successfunc,errorfunc,nPageID,GetTabIndex());
    else
        return QString().fromStdString("{\"error\":\"page is blocked by user\"}"); 
}
int WebView::GetTabIndex()
{
    //LogPrintf("dexgetTabIn\n");
    MainView *mv = m_page->mainWindow()->getMainView();//qobject_cast<MainView*>(parent());
    //LogPrintf("getTabIndex2\n");
    WebView *wv=this;
   // LogPrintf("getTabIndex3\n");
    return mv->webViewIndex(wv);
   //LogPrintf("getTabIndex4\n");
}
void WebView::loadUrl(const QUrl &url)
{
    m_initialUrl = url;
    string strUrlOut=url.toString().toStdString();
    int nPageID;
    ParseUrl(url.toString().toStdString(),strUrlOut,nPageID);
        //url=QUrl(QString().fromStdString(strUrlOut));
    load(QUrl(QString().fromStdString(strUrlOut)));
}
QUrl WebView::url() const
{
    if(nPageID!=255)
        return m_initialUrl;
    QUrl url = QWebView::url();
    if (!url.isEmpty())
        return url;

    return m_initialUrl;
}

QIcon WebView::icon() const
{
    if (!m_icon.isNull())
        return m_icon;
    return m_page->mainWindow()->defaultIcon();
}
QString WebView::lastStatusBarText() const
{
    return m_statusBarText;
}
void WebView::wheelEvent(QWheelEvent *event)
{
//#if defined(QWEBPAGE_SETTEXTSIZEMULTIPLIER)
    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        int numDegrees = event->delta() / 8;
        int numSteps = numDegrees / 15;
        setTextSizeMultiplier(textSizeMultiplier() + numSteps * 0.1);
        event->accept();
        return;
    }
//#endif
    QWebView::wheelEvent(event);
}
void WebView::mousePressEvent(QMouseEvent *event)
{
    m_page->m_pressedButtons = event->buttons();
    m_page->m_keyboardModifiers = event->modifiers();
    QWebView::mousePressEvent(event);
}

void WebView::mouseReleaseEvent(QMouseEvent *event)
{
    QWebView::mouseReleaseEvent(event);
    if (!event->isAccepted() && (m_page->m_pressedButtons & Qt::MidButton)) {
        QUrl url(QApplication::clipboard()->text(QClipboard::Selection));
        if (!url.isEmpty() && url.isValid() && !url.scheme().isEmpty()) {
            setUrl(url);
        }
    }
}
void WebView::contextMenuEvent(QContextMenuEvent *event)
{
//#if defined(QWEBPAGE_HITTESTCONTENT)
    QWebHitTestResult r = page()->mainFrame()->hitTestContent(event->pos());
    if (!r.linkUrl().isEmpty()) {
        QMenu menu(this);
        menu.addAction(pageAction(QWebPage::OpenLinkInNewWindow));
        menu.addAction(tr("Open in New Tab"), this, SLOT(openLinkInNewTab()));
        menu.addSeparator();
        menu.addAction(pageAction(QWebPage::DownloadLinkToDisk));
        // Add link to bookmarks...
        menu.addSeparator();
        menu.addAction(pageAction(QWebPage::CopyLinkToClipboard));
        if (page()->settings()->testAttribute(QWebSettings::DeveloperExtrasEnabled))
            menu.addAction(pageAction(QWebPage::InspectElement));
        menu.exec(mapToGlobal(event->pos()));
        return;
    }
//#endif
    QWebView::contextMenuEvent(event);
}
void WebView::setProgress(int progress)

{
   //LogPrintf("slot triggered:WebView::setProgress %i",progress);
    m_progress = progress;
}

void WebView::loadFinished(bool success)
{
    ///LogPrintf("slot triggered:WebView::loadFinished \n");
    if (success && 100 != m_progress) {
        qWarning() << "Received finished signal while progress is still:" << progress()
                   << "Url:" << url();
    }
    m_progress = 0;
}

void WebView::setStatusBarText(const QString &string)
{
    //LogPrintf("slot triggered:WebView::setStatusBarText %s",string.toStdString());
    m_statusBarText = string;
}

void WebView::downloadRequested(const QNetworkRequest &request)
{
    BitcoinGUI::downloadManager()->download(request);
}
void WebView::openLinkInNewTab()
{
//#if defined(QWEBPAGE_WEBACTION_OPENLINKINNEWWINDOW)
    m_page->m_openInNewTab = true;
    pageAction(QWebPage::OpenLinkInNewWindow)->trigger();
//#endif
}
void WebView::onFeaturePermissionRequested(QWebFrame* frame, QWebPage::Feature feature)
{
    //LogPrintf("slot triggered:WebView::onFeaturePermissionRequested \n");
    FeaturePermissionBar *permissionBar = new FeaturePermissionBar(this);
    connect(permissionBar, SIGNAL(&FeaturePermissionBar::featurePermissionProvided), page(), SLOT(&QWebPage::setFeaturePermission));

    // Discard the bar on new loads (if we navigate away or reload).
    connect(page(), &QWebPage::loadStarted, permissionBar, &QObject::deleteLater);

    permissionBar->requestPermission(frame, feature);
}
void WebView::onIconUrlChanged(const QUrl &url)
{
    //LogPrintf("onIconUrlChanged %s",url.toString().toStdString());
    QNetworkRequest iconRequest(url);
    m_iconReply = BitcoinGUI::networkAccessManager()->get(iconRequest);
    m_iconReply->setParent(this);
    connect(m_iconReply, SIGNAL(finished()), this, SLOT(iconLoaded()));
}

void WebView::iconLoaded()
{
    //LogPrintf("slot triggered:WebView::iconLoaded \n");
    m_icon = QIcon();
    if (m_iconReply) {
        QByteArray data = m_iconReply->readAll();
        QPixmap pixmap;
        pixmap.loadFromData(data);
        m_icon.addPixmap(pixmap);
        m_iconReply->deleteLater();
        m_iconReply = 0;
    }
    emit iconChanged();
}


