/********************************************************************************
** Form generated from reading UI file 'accountdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ACCOUNTDIALOG_H
#define UI_ACCOUNTDIALOG_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_AccountDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *warningLabel;
    QFormLayout *formLayout;
    QCheckBox *checkBoxHeader;
    QLabel *headerLabel;
    QLineEdit *headerEdit;
    QLabel *timeLabel;
    QLabel *timeDisplay;
    QLabel *selectLabel;
    QCheckBox *checkBoxEncrypt;
    QLabel *passLabel1;
    QLineEdit *passEdit1;
    QLabel *passLabel2;
    QLineEdit *passEdit2;
    QLabel *passLabel3;
    QLineEdit *passEdit3;
    QComboBox *selectList;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *Button1;
    QPushButton *Button2;
    QPushButton *Button3;

    void setupUi(QDialog *AccountDialog)
    {
        if (AccountDialog->objectName().isEmpty())
            AccountDialog->setObjectName(QStringLiteral("AccountDialog"));
        AccountDialog->resize(598, 376);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(AccountDialog->sizePolicy().hasHeightForWidth());
        AccountDialog->setSizePolicy(sizePolicy);
        AccountDialog->setMinimumSize(QSize(550, 0));
        verticalLayout = new QVBoxLayout(AccountDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetMinimumSize);
        warningLabel = new QLabel(AccountDialog);
        warningLabel->setObjectName(QStringLiteral("warningLabel"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(warningLabel->sizePolicy().hasHeightForWidth());
        warningLabel->setSizePolicy(sizePolicy1);
        warningLabel->setText(QStringLiteral("Placeholder text"));
        warningLabel->setTextFormat(Qt::RichText);
        warningLabel->setWordWrap(true);

        verticalLayout->addWidget(warningLabel);

        formLayout = new QFormLayout();
        formLayout->setObjectName(QStringLiteral("formLayout"));
        formLayout->setSizeConstraint(QLayout::SetMinimumSize);
        formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        checkBoxHeader = new QCheckBox(AccountDialog);
        checkBoxHeader->setObjectName(QStringLiteral("checkBoxHeader"));

        formLayout->setWidget(0, QFormLayout::LabelRole, checkBoxHeader);

        headerLabel = new QLabel(AccountDialog);
        headerLabel->setObjectName(QStringLiteral("headerLabel"));

        formLayout->setWidget(1, QFormLayout::LabelRole, headerLabel);

        headerEdit = new QLineEdit(AccountDialog);
        headerEdit->setObjectName(QStringLiteral("headerEdit"));

        formLayout->setWidget(1, QFormLayout::FieldRole, headerEdit);

        timeLabel = new QLabel(AccountDialog);
        timeLabel->setObjectName(QStringLiteral("timeLabel"));

        formLayout->setWidget(2, QFormLayout::LabelRole, timeLabel);

        timeDisplay = new QLabel(AccountDialog);
        timeDisplay->setObjectName(QStringLiteral("timeDisplay"));

        formLayout->setWidget(2, QFormLayout::FieldRole, timeDisplay);

        selectLabel = new QLabel(AccountDialog);
        selectLabel->setObjectName(QStringLiteral("selectLabel"));

        formLayout->setWidget(3, QFormLayout::LabelRole, selectLabel);

        checkBoxEncrypt = new QCheckBox(AccountDialog);
        checkBoxEncrypt->setObjectName(QStringLiteral("checkBoxEncrypt"));

        formLayout->setWidget(4, QFormLayout::LabelRole, checkBoxEncrypt);

        passLabel1 = new QLabel(AccountDialog);
        passLabel1->setObjectName(QStringLiteral("passLabel1"));

        formLayout->setWidget(5, QFormLayout::LabelRole, passLabel1);

        passEdit1 = new QLineEdit(AccountDialog);
        passEdit1->setObjectName(QStringLiteral("passEdit1"));
        passEdit1->setEchoMode(QLineEdit::Password);

        formLayout->setWidget(5, QFormLayout::FieldRole, passEdit1);

        passLabel2 = new QLabel(AccountDialog);
        passLabel2->setObjectName(QStringLiteral("passLabel2"));

        formLayout->setWidget(6, QFormLayout::LabelRole, passLabel2);

        passEdit2 = new QLineEdit(AccountDialog);
        passEdit2->setObjectName(QStringLiteral("passEdit2"));
        passEdit2->setEchoMode(QLineEdit::Password);

        formLayout->setWidget(6, QFormLayout::FieldRole, passEdit2);

        passLabel3 = new QLabel(AccountDialog);
        passLabel3->setObjectName(QStringLiteral("passLabel3"));

        formLayout->setWidget(7, QFormLayout::LabelRole, passLabel3);

        passEdit3 = new QLineEdit(AccountDialog);
        passEdit3->setObjectName(QStringLiteral("passEdit3"));
        passEdit3->setEchoMode(QLineEdit::Password);

        formLayout->setWidget(7, QFormLayout::FieldRole, passEdit3);

        selectList = new QComboBox(AccountDialog);
        selectList->setObjectName(QStringLiteral("selectList"));
        selectList->setContextMenuPolicy(Qt::DefaultContextMenu);
        selectList->setLocale(QLocale(QLocale::English, QLocale::UnitedStates));
        selectList->setFrame(true);

        formLayout->setWidget(3, QFormLayout::FieldRole, selectList);


        verticalLayout->addLayout(formLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setSizeConstraint(QLayout::SetMinimumSize);
        horizontalLayout->setContentsMargins(10, 10, -1, 10);
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        Button1 = new QPushButton(AccountDialog);
        Button1->setObjectName(QStringLiteral("Button1"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(Button1->sizePolicy().hasHeightForWidth());
        Button1->setSizePolicy(sizePolicy2);

        horizontalLayout->addWidget(Button1);

        Button2 = new QPushButton(AccountDialog);
        Button2->setObjectName(QStringLiteral("Button2"));
        sizePolicy2.setHeightForWidth(Button2->sizePolicy().hasHeightForWidth());
        Button2->setSizePolicy(sizePolicy2);

        horizontalLayout->addWidget(Button2);

        Button3 = new QPushButton(AccountDialog);
        Button3->setObjectName(QStringLiteral("Button3"));
        sizePolicy2.setHeightForWidth(Button3->sizePolicy().hasHeightForWidth());
        Button3->setSizePolicy(sizePolicy2);
        Button3->setAutoFillBackground(false);
        Button3->setCheckable(false);
        Button3->setChecked(false);
        Button3->setDefault(false);
        Button3->setFlat(false);

        horizontalLayout->addWidget(Button3);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(AccountDialog);

        QMetaObject::connectSlotsByName(AccountDialog);
    } // setupUi

    void retranslateUi(QDialog *AccountDialog)
    {
        AccountDialog->setWindowTitle(QApplication::translate("AccountDialog", "Passphrase Dialog", 0));
        checkBoxHeader->setText(QApplication::translate("AccountDialog", "Preset PubKey Header", 0));
        headerLabel->setText(QApplication::translate("AccountDialog", "PubKey Header", 0));
        timeLabel->setText(QApplication::translate("AccountDialog", "Estimated Calculating time:", 0));
        timeDisplay->setText(QApplication::translate("AccountDialog", "0", 0));
        selectLabel->setText(QApplication::translate("AccountDialog", "Select Account", 0));
        checkBoxEncrypt->setText(QApplication::translate("AccountDialog", "Password Encryption", 0));
        passLabel1->setText(QApplication::translate("AccountDialog", "Enter passphrase", 0));
        passLabel2->setText(QApplication::translate("AccountDialog", "Enter new passphrase", 0));
        passLabel3->setText(QApplication::translate("AccountDialog", "Repeat new passphrase", 0));
        Button1->setText(QApplication::translate("AccountDialog", "Cancel", 0));
        Button2->setText(QApplication::translate("AccountDialog", "Create", 0));
        Button3->setText(QApplication::translate("AccountDialog", "CreateAndSwitchTo", 0));
    } // retranslateUi

};

namespace Ui {
    class AccountDialog: public Ui_AccountDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ACCOUNTDIALOG_H
