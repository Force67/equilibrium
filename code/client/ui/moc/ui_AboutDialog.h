/********************************************************************************
** Form generated from reading UI file 'AboutDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUTDIALOG_H
#define UI_ABOUTDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_AboutDialog
{
public:
    QGridLayout *gridLayout;
    QVBoxLayout *textLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *labelIcon;
    QSpacerItem *verticalSpacer;
    QLabel *labelTitle;
    QVBoxLayout *verticalLayout;
    QLabel *labelDesc;
    QSpacerItem *spacerLabelDivide;
    QLabel *labelCopyright;

    void setupUi(QDialog *AboutDialog)
    {
        if (AboutDialog->objectName().isEmpty())
            AboutDialog->setObjectName(QStringLiteral("AboutDialog"));
        AboutDialog->resize(566, 508);
        QIcon icon;
        icon.addFile(QStringLiteral(":/logo"), QSize(), QIcon::Normal, QIcon::Off);
        AboutDialog->setWindowIcon(icon);
        gridLayout = new QGridLayout(AboutDialog);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        textLayout = new QVBoxLayout();
        textLayout->setSpacing(9);
        textLayout->setObjectName(QStringLiteral("textLayout"));
        textLayout->setContentsMargins(9, 9, 9, 9);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        labelIcon = new QLabel(AboutDialog);
        labelIcon->setObjectName(QStringLiteral("labelIcon"));
        labelIcon->setPixmap(QPixmap(QString::fromUtf8(":/logo")));
        labelIcon->setScaledContents(false);
        labelIcon->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);

        horizontalLayout->addWidget(labelIcon);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        horizontalLayout->addItem(verticalSpacer);

        labelTitle = new QLabel(AboutDialog);
        labelTitle->setObjectName(QStringLiteral("labelTitle"));
        labelTitle->setTextFormat(Qt::RichText);

        horizontalLayout->addWidget(labelTitle);


        textLayout->addLayout(horizontalLayout);


        gridLayout->addLayout(textLayout, 0, 0, 1, 1);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        labelDesc = new QLabel(AboutDialog);
        labelDesc->setObjectName(QStringLiteral("labelDesc"));

        verticalLayout->addWidget(labelDesc);

        spacerLabelDivide = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(spacerLabelDivide);

        labelCopyright = new QLabel(AboutDialog);
        labelCopyright->setObjectName(QStringLiteral("labelCopyright"));

        verticalLayout->addWidget(labelCopyright);


        gridLayout->addLayout(verticalLayout, 1, 0, 1, 1);


        retranslateUi(AboutDialog);

        QMetaObject::connectSlotsByName(AboutDialog);
    } // setupUi

    void retranslateUi(QDialog *AboutDialog)
    {
        AboutDialog->setWindowTitle(QApplication::translate("AboutDialog", "About NODA", Q_NULLPTR));
        labelIcon->setText(QString());
#ifndef QT_NO_STATUSTIP
        labelTitle->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        labelTitle->setWhatsThis(QString());
#endif // QT_NO_WHATSTHIS
        labelTitle->setText(QApplication::translate("AboutDialog", "<html><head/><body><p align=\"center\"><span style=\" font-size:9pt; font-weight:600;\">Noda - Supercharge your IDA</span></p></body></html>", Q_NULLPTR));
        labelDesc->setText(QApplication::translate("AboutDialog", "<html><head/><body><p><span style=\" font-size:10pt; font-style:italic;\">Uses the following libraries:</span></p><p><span style=\" font-size:9pt;\">- </span><a href=\"https://github.com/google/flatbuffers\"><span style=\" font-size:9pt; text-decoration: underline; color:#0000ff;\">Google Flatbuffers</span></a></p><p><span style=\" font-size:9pt;\">- A custom fork of </span><a href=\"https://github.com/lsalzman/enet\"><span style=\" font-size:9pt; text-decoration: underline; color:#0000ff;\">ENet</span></a><br/></p></body></html>", Q_NULLPTR));
        labelCopyright->setText(QApplication::translate("AboutDialog", "<html><head/><body><p align=\"center\"><span style=\" font-weight:600;\">Copyright (C) NOMAD Group &lt;</span><a href=\"https://www.nomad-group.net/\"><span style=\" font-weight:600; text-decoration: underline; color:#0000ff;\">nomad-group.ne</span></a><span style=\" font-weight:600;\">t&gt;</span></p></body></html>", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class AboutDialog: public Ui_AboutDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUTDIALOG_H
