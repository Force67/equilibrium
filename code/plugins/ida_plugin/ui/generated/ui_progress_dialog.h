/********************************************************************************
** Form generated from reading UI file 'progress_dialog.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROGRESS_DIALOG_H
#define UI_PROGRESS_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ProgressDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *labelDesc;
    QProgressBar *barProgress;
    QLabel *labelStatus;

    void setupUi(QDialog *ProgressDialog)
    {
        if (ProgressDialog->objectName().isEmpty())
            ProgressDialog->setObjectName(QStringLiteral("ProgressDialog"));
        ProgressDialog->resize(431, 109);
        QIcon icon;
        icon.addFile(QStringLiteral(":/logo"), QSize(), QIcon::Normal, QIcon::Off);
        ProgressDialog->setWindowIcon(icon);
        verticalLayout = new QVBoxLayout(ProgressDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        labelDesc = new QLabel(ProgressDialog);
        labelDesc->setObjectName(QStringLiteral("labelDesc"));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        labelDesc->setFont(font);

        verticalLayout->addWidget(labelDesc);

        barProgress = new QProgressBar(ProgressDialog);
        barProgress->setObjectName(QStringLiteral("barProgress"));
        barProgress->setValue(0);
        barProgress->setTextVisible(false);

        verticalLayout->addWidget(barProgress);

        labelStatus = new QLabel(ProgressDialog);
        labelStatus->setObjectName(QStringLiteral("labelStatus"));
        labelStatus->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(labelStatus);


        retranslateUi(ProgressDialog);

        QMetaObject::connectSlotsByName(ProgressDialog);
    } // setupUi

    void retranslateUi(QDialog *ProgressDialog)
    {
        ProgressDialog->setWindowTitle(QString());
        labelDesc->setText(QString());
        labelStatus->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class ProgressDialog: public Ui_ProgressDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROGRESS_DIALOG_H
