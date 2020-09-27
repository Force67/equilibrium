/********************************************************************************
** Form generated from reading UI file 'ConnectDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONNECTDIALOG_H
#define UI_CONNECTDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ConnectPromt
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *spacerLeft;
    QPushButton *buttonConnect;
    QPushButton *buttonCancel;
    QCheckBox *buttonNO;
    QSpacerItem *spacerRight;

    void setupUi(QDialog *ConnectPromt)
    {
        if (ConnectPromt->objectName().isEmpty())
            ConnectPromt->setObjectName(QStringLiteral("ConnectPromt"));
        ConnectPromt->resize(518, 84);
        QIcon icon;
        icon.addFile(QStringLiteral(":/logo"), QSize(), QIcon::Normal, QIcon::Off);
        ConnectPromt->setWindowIcon(icon);
        verticalLayout = new QVBoxLayout(ConnectPromt);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label = new QLabel(ConnectPromt);
        label->setObjectName(QStringLiteral("label"));
        label->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        spacerLeft = new QSpacerItem(30, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(spacerLeft);

        buttonConnect = new QPushButton(ConnectPromt);
        buttonConnect->setObjectName(QStringLiteral("buttonConnect"));

        horizontalLayout->addWidget(buttonConnect);

        buttonCancel = new QPushButton(ConnectPromt);
        buttonCancel->setObjectName(QStringLiteral("buttonCancel"));

        horizontalLayout->addWidget(buttonCancel);

        buttonNO = new QCheckBox(ConnectPromt);
        buttonNO->setObjectName(QStringLiteral("buttonNO"));

        horizontalLayout->addWidget(buttonNO);

        spacerRight = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(spacerRight);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(ConnectPromt);

        buttonConnect->setDefault(true);


        QMetaObject::connectSlotsByName(ConnectPromt);
    } // setupUi

    void retranslateUi(QDialog *ConnectPromt)
    {
        ConnectPromt->setWindowTitle(QApplication::translate("ConnectPromt", "NODA: Auto-Connect", Q_NULLPTR));
        label->setText(QApplication::translate("ConnectPromt", "NODA detected a synced IDB. Do you wish to connect to Sync-Server?", Q_NULLPTR));
        buttonConnect->setText(QApplication::translate("ConnectPromt", "Connect", Q_NULLPTR));
        buttonCancel->setText(QApplication::translate("ConnectPromt", "Cancel", Q_NULLPTR));
        buttonNO->setText(QApplication::translate("ConnectPromt", "Don't ask again", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class ConnectPromt: public Ui_ConnectPromt {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONNECTDIALOG_H
