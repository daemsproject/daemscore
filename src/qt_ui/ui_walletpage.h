/********************************************************************************
** Form generated from reading UI file 'walletpage.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WALLETPAGE_H
#define UI_WALLETPAGE_H

#include <QtCore/QVariant>
#include <QtWebKitWidgets/QWebView>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_WalletPage
{
public:
    QVBoxLayout *topLayout;
    QWebView *walletWebView;

    void setupUi(QWidget *WalletPage)
    {
        if (WalletPage->objectName().isEmpty())
            WalletPage->setObjectName(QStringLiteral("WalletPage"));
        WalletPage->resize(680, 342);
        topLayout = new QVBoxLayout(WalletPage);
        topLayout->setObjectName(QStringLiteral("topLayout"));
        walletWebView = new QWebView(WalletPage);
        walletWebView->setObjectName(QStringLiteral("walletWebView"));
        walletWebView->setUrl(QUrl(QStringLiteral("about:blank")));

        topLayout->addWidget(walletWebView);


        retranslateUi(WalletPage);

        QMetaObject::connectSlotsByName(WalletPage);
    } // setupUi

    void retranslateUi(QWidget *WalletPage)
    {
        WalletPage->setWindowTitle(QApplication::translate("WalletPage", "Form", 0));
    } // retranslateUi

};

namespace Ui {
    class WalletPage: public Ui_WalletPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WALLETPAGE_H
