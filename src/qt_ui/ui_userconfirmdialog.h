/********************************************************************************
** Form generated from reading UI file 'userconfirmdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
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
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_UserConfirmDialog
{
public:
    QWidget *verticalLayoutWidget_2;
    QVBoxLayout *verticalLayout_3;
    QLabel *label_message;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *layout_password;
    QLabel *label_7;
    QLineEdit *passwordEdit;
    QHBoxLayout *horizontalLayout;
    QPushButton *btn_blockpage;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *UserConfirmDialog)
    {
        if (UserConfirmDialog->objectName().isEmpty())
            UserConfirmDialog->setObjectName(QStringLiteral("UserConfirmDialog"));
        UserConfirmDialog->resize(721, 439);
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(UserConfirmDialog->sizePolicy().hasHeightForWidth());
        UserConfirmDialog->setSizePolicy(sizePolicy);
        UserConfirmDialog->setSizeGripEnabled(false);
        verticalLayoutWidget_2 = new QWidget(UserConfirmDialog);
        verticalLayoutWidget_2->setObjectName(QStringLiteral("verticalLayoutWidget_2"));
        verticalLayoutWidget_2->setGeometry(QRect(10, 10, 701, 421));
        verticalLayout_3 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        label_message = new QLabel(verticalLayoutWidget_2);
        label_message->setObjectName(QStringLiteral("label_message"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_message->sizePolicy().hasHeightForWidth());
        label_message->setSizePolicy(sizePolicy1);
        label_message->setTextFormat(Qt::RichText);
        label_message->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        label_message->setWordWrap(true);

        verticalLayout_3->addWidget(label_message);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

        verticalLayout_3->addItem(verticalSpacer);

        layout_password = new QHBoxLayout();
        layout_password->setObjectName(QStringLiteral("layout_password"));
        layout_password->setSizeConstraint(QLayout::SetMinimumSize);
        label_7 = new QLabel(verticalLayoutWidget_2);
        label_7->setObjectName(QStringLiteral("label_7"));
        sizePolicy1.setHeightForWidth(label_7->sizePolicy().hasHeightForWidth());
        label_7->setSizePolicy(sizePolicy1);
        label_7->setMaximumSize(QSize(16777215, 173));
        label_7->setBaseSize(QSize(0, 40));

        layout_password->addWidget(label_7);

        passwordEdit = new QLineEdit(verticalLayoutWidget_2);
        passwordEdit->setObjectName(QStringLiteral("passwordEdit"));
        passwordEdit->setEchoMode(QLineEdit::Password);

        layout_password->addWidget(passwordEdit);


        verticalLayout_3->addLayout(layout_password);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        btn_blockpage = new QPushButton(verticalLayoutWidget_2);
        btn_blockpage->setObjectName(QStringLiteral("btn_blockpage"));
        btn_blockpage->setAutoDefault(false);

        horizontalLayout->addWidget(btn_blockpage);

        buttonBox = new QDialogButtonBox(verticalLayoutWidget_2);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        buttonBox->setCenterButtons(true);

        horizontalLayout->addWidget(buttonBox);


        verticalLayout_3->addLayout(horizontalLayout);


        retranslateUi(UserConfirmDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), UserConfirmDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), UserConfirmDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(UserConfirmDialog);
    } // setupUi

    void retranslateUi(QDialog *UserConfirmDialog)
    {
        UserConfirmDialog->setWindowTitle(QApplication::translate("UserConfirmDialog", "Dialog", 0));
        label_message->setText(QApplication::translate("UserConfirmDialog", "TextLabel", 0));
        label_7->setText(QApplication::translate("UserConfirmDialog", "Please input password:", 0));
        btn_blockpage->setText(QApplication::translate("UserConfirmDialog", "Block Page", 0));
    } // retranslateUi

};

namespace Ui {
    class UserConfirmDialog: public Ui_UserConfirmDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_USERCONFIRMDIALOG_H
