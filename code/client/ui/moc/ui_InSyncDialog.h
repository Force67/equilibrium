/********************************************************************************
** Form generated from reading UI file 'InSyncDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INSYNCDIALOG_H
#define UI_INSYNCDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListView>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_InSyncDialog
{
public:
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout;
    QListView *listView;
    QGroupBox *groupBox;

    void setupUi(QDialog *InSyncDialog)
    {
        if (InSyncDialog->objectName().isEmpty())
            InSyncDialog->setObjectName(QStringLiteral("InSyncDialog"));
        InSyncDialog->resize(1429, 921);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(InSyncDialog->sizePolicy().hasHeightForWidth());
        InSyncDialog->setSizePolicy(sizePolicy);
        QIcon icon;
        icon.addFile(QStringLiteral(":/logo"), QSize(), QIcon::Normal, QIcon::Off);
        InSyncDialog->setWindowIcon(icon);
        verticalLayout_3 = new QVBoxLayout(InSyncDialog);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        listView = new QListView(InSyncDialog);
        listView->setObjectName(QStringLiteral("listView"));

        horizontalLayout->addWidget(listView);

        groupBox = new QGroupBox(InSyncDialog);
        groupBox->setObjectName(QStringLiteral("groupBox"));

        horizontalLayout->addWidget(groupBox);


        verticalLayout_3->addLayout(horizontalLayout);


        retranslateUi(InSyncDialog);

        QMetaObject::connectSlotsByName(InSyncDialog);
    } // setupUi

    void retranslateUi(QDialog *InSyncDialog)
    {
        InSyncDialog->setWindowTitle(QApplication::translate("InSyncDialog", "INSync", Q_NULLPTR));
        groupBox->setTitle(QApplication::translate("InSyncDialog", "GroupBox", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class InSyncDialog: public Ui_InSyncDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INSYNCDIALOG_H
