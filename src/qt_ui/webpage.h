// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BROWSER_QT_WEBPAGE_H
#define BROWSER_QT_WEBPAGE_H

//#include "amount.h"
#include <QObject>
#include <QWidget>
#include <QWebView>
//#include <QBoxLayout>
//#include <QLineEdit>
//QT_BEGIN_NAMESPACE
//class QModelIndex;
class JsInterface;
class BitcoinGUI;
//class QNetworkDiskCache;
//class QNetworkAccessManager;
//QT_END_NAMESPACE
//class BitcoinGUI;
class QAuthenticator;
class QMouseEvent;
class QNetworkProxy;
class QNetworkReply;
class QSslError;

class WebPage : public QWebPage {
    Q_OBJECT

signals:
    void loadingUrl(const QUrl &url);

public:
    WebPage(QObject *parent = 0);
    BitcoinGUI *mainWindow();

protected:
#if defined(QWEBPAGE_ACCEPTNAVIGATIONREQUEST)
 //   bool acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type);
#endif
    QWebPage *createWindow(QWebPage::WebWindowType type);
#if !defined(QT_NO_UITOOLS)
    QObject *createPlugin(const QString &classId, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues);
#endif
    //virtual bool certificateError(const QWebCertificateError &error) Q_DECL_OVERRIDE;

private slots:
/////#if defined(QWEBPAGE_UNSUPPORTEDCONTENT)
    void handleUnsupportedContent(QNetworkReply *reply);
//#endif
    void authenticationRequired(QNetworkReply* reply, QAuthenticator *auth);
    void proxyAuthenticationRequired(QNetworkReply* reply, QAuthenticator *auth, const QString &proxyHost);

private:
    friend class WebView;

    // set the webview mousepressedevent
    Qt::KeyboardModifiers m_keyboardModifiers;
    Qt::MouseButtons m_pressedButtons;
    bool m_openInNewTab;
    QUrl m_loadingUrl;
};

class WebView:public QWebView
{
        Q_OBJECT

public:
    explicit WebView(QString languageIn,QWidget *parent = 0,JsInterface *_js=0,QUrl urlIn=QUrl(""),int nPageIDin=255,int nFromPageIDIn=0);
    ~WebView();
    QString language;
    int nPageID;    
    int nFromPageID;   
    WebPage *webPage() const { return m_page; }

    void loadUrl(const QUrl &url);
    QUrl url() const;
    QIcon icon() const;

    QString lastStatusBarText() const;
    inline int progress() const { return m_progress; }

private slots:    
    void addJSObject();
    void setProgress(int progress);
    void loadFinished(bool success);
    void setStatusBarText(const QString &string);
    void downloadRequested(const QNetworkRequest &request);
    void openLinkInNewTab();
    void onFeaturePermissionRequested(QWebFrame* frame, QWebPage::Feature);
    void onIconUrlChanged(const QUrl &url);
    void iconLoaded();
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void wheelEvent(QWheelEvent *event);
private:    
    JsInterface* jsInterface;    
    QString m_statusBarText;
    QUrl m_initialUrl;
    int m_progress;
    WebPage *m_page;
    QIcon m_icon;
    QNetworkReply *m_iconReply;
    //WalletModel *walletModel;
    //QNetworkAccessManager * m_network;
    //QNetworkDiskCache * m_cache;
public slots:    
    Q_INVOKABLE    
    QString jscall(QString command,QString dataJson);
    QString jscallasync(QString command,QString dataJson,QString successfunc,QString errorfunc);
signals:
    //Q_SIGNAL
    void feedback(QString str,QString func);    
    void notify(QString result);
    void iconChanged();
    
};
//class PopupWindow : public QWidget {
//    Q_OBJECT
//public:
//    PopupWindow(QString language)
//        : m_addressBar(new QLineEdit(this))
//        , m_view(new WebView(language,this))
//    {
//        QVBoxLayout *layout = new QVBoxLayout;
//        layout->setMargin(0);
//        setLayout(layout);
//        layout->addWidget(m_addressBar);
//        layout->addWidget(m_view);
//        m_view->setFocus();
//
//        connect(m_view, &WebView::titleChanged, this, &QWidget::setWindowTitle);
//        connect(m_view, &WebView::urlChanged, this, &PopupWindow::setUrl);
//        connect(page(), &WebPage::geometryChangeRequested, this, &PopupWindow::adjustGeometry);
//        connect(page(), &WebPage::windowCloseRequested, this, &QWidget::close);
//    }
//
//    QWebPage* page() const { return m_view->page(); }
//
//private Q_SLOTS:
//    void setUrl(const QUrl &url)
//    {
//        m_addressBar->setText(url.toString());
//    }
//
//    void adjustGeometry(const QRect &newGeometry)
//    {
//        const int x1 = frameGeometry().left() - geometry().left();
//        const int y1 = frameGeometry().top() - geometry().top();
//        const int x2 = frameGeometry().right() - geometry().right();
//        const int y2 = frameGeometry().bottom() - geometry().bottom();
//
//        setGeometry(newGeometry.adjusted(x1, y1 - m_addressBar->height(), x2, y2));
//    }
//
//private:
//    QLineEdit *m_addressBar;
//    WebView *m_view;
//
//};
#endif // BROWSER_QT_WEBPAGE_H
