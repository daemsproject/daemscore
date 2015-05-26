/********************************************************************************
** Form generated from reading UI file 'userconfirmdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_USERCONFIRMDIALOG_H
#define UI_USERCONFIRMDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_UserConfirmDialog
{
public:
    QDialogButtonBox *buttonBox;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *layout_password;
    QLabel *label_7;
    QLineEdit *passwordEdit;
    QLabel *label_message;

    void setupUi(QDialog *UserConfirmDialog)
    {
        if (UserConfirmDialog->objectName().isEmpty())
            UserConfirmDialog->setObjectName(QStringLiteral("UserConfirmDialog"));
        UserConfirmDialog->resize(650, 300);
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(UserConfirmDialog->sizePolicy().hasHeightForWidth());
        UserConfirmDialog->setSizePolicy(sizePolicy);
        UserConfirmDialog->setSizeGripEnabled(false);
        buttonBox = new QDialogButtonBox(UserConfirmDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setGeometry(QRect(20, 260, 601, 31));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        buttonBox->setCenterButtons(true);
        horizontalLayoutWidget = new QWidget(UserConfirmDialog);
        horizontalLayoutWidget->setObjectName(QStringLiteral("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(120, 220, 411, 31));
        layout_password = new QHBoxLayout(horizontalLayoutWidget);
        layout_password->setObjectName(QStringLiteral("layout_password"));
        layout_password->setContentsMargins(0, 0, 0, 0);
        label_7 = new QLabel(horizontalLayoutWidget);
        label_7->setObjectName(QStringLiteral("label_7"));

        layout_password->addWidget(label_7);

        passwordEdit = new QLineEdit(horizontalLayoutWidget);
        passwordEdit->setObjectName(QStringLiteral("passwordEdit"));
        passwordEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);

        layout_password->addWidget(passwordEdit);

        label_message = new QLabel(UserConfirmDialog);
        label_message->setObjectName(QStringLiteral("label_message"));
        label_message->setGeometry(QRect(30, 10, 571, 171));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_message->sizePolicy().hasHeightForWidth());
        label_message->setSizePolicy(sizePolicy1);
        label_message->setTextFormat(Qt::RichText);
        label_message->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        buttonBox->raise();
        horizontalLayoutWidget->raise();
        label_7->raise();
        label_message->raise();

        retranslateUi(UserConfirmDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), UserConfirmDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), UserConfirmDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(UserConfirmDialog);
    } // setupUi

    void retranslateUi(QDialog *UserConfirmDialog)
    {
        UserConfirmDialog->setWindowTitle(QApplication::translate("UserConfirmDialog", "Dialog", 0));
        label_7->setText(QApplication::translate("UserConfirmDialog", "Please input password:", 0));
        label_message->setText(QApplication::translate("UserConfirmDialog", "TextLabel", 0));
    } // retranslateUi

};

namespace Ui {
    class UserConfirmDialog: public Ui_UserConfirmDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_USERCONFIRMDIALOG_H
