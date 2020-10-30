/********************************************************************************
** Form generated from reading UI file 'AboutDialog2.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUTDIALOG2_H
#define UI_ABOUTDIALOG2_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AboutDialog
{
public:
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QLabel *nameLabel;
    QLabel *iconLabel;
    QTabWidget *tabWidget;
    QWidget *aboutTab;
    QVBoxLayout *verticalLayout_3;
    QSpacerItem *verticalSpacer_3;
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout_6;
    QLabel *labelFeatures;
    QWidget *widget;
    QVBoxLayout *verticalLayout_7;
    QLabel *labelLogo;
    QSpacerItem *verticalSpacer;
    QWidget *contribTab;
    QVBoxLayout *verticalLayout;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout_5;
    QLabel *contributors;
    QLabel *seeContributions;
    QWidget *debugTab;
    QVBoxLayout *verticalLayout_4;
    QLabel *label;
    QPlainTextEdit *debugInfo;
    QPushButton *copyToClipboard;
    QLabel *labelCopyright;
    QDialogButtonBox *btBox;

    void setupUi(QDialog *AboutDialog)
    {
        if (AboutDialog->objectName().isEmpty())
            AboutDialog->setObjectName(QStringLiteral("AboutDialog"));
        AboutDialog->resize(592, 511);
        verticalLayout_2 = new QVBoxLayout(AboutDialog);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(-1, 15, -1, 20);
        nameLabel = new QLabel(AboutDialog);
        nameLabel->setObjectName(QStringLiteral("nameLabel"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(nameLabel->sizePolicy().hasHeightForWidth());
        nameLabel->setSizePolicy(sizePolicy);
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        nameLabel->setFont(font);
        nameLabel->setText(QStringLiteral("<html><head/><body><p><span style=\" font-size:20pt;\">NODA ${buildTag}</span></p></body></html>"));
        nameLabel->setMargin(0);
        nameLabel->setIndent(11);

        horizontalLayout->addWidget(nameLabel, 0, Qt::AlignHCenter);

        iconLabel = new QLabel(AboutDialog);
        iconLabel->setObjectName(QStringLiteral("iconLabel"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(iconLabel->sizePolicy().hasHeightForWidth());
        iconLabel->setSizePolicy(sizePolicy1);
        iconLabel->setMinimumSize(QSize(48, 48));
        iconLabel->setMaximumSize(QSize(48, 48));

        horizontalLayout->addWidget(iconLabel, 0, Qt::AlignVCenter);


        verticalLayout_2->addLayout(horizontalLayout);

        tabWidget = new QTabWidget(AboutDialog);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        aboutTab = new QWidget();
        aboutTab->setObjectName(QStringLiteral("aboutTab"));
        verticalLayout_3 = new QVBoxLayout(aboutTab);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        verticalSpacer_3 = new QSpacerItem(20, 5, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout_3->addItem(verticalSpacer_3);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        verticalLayout_6 = new QVBoxLayout();
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        labelFeatures = new QLabel(aboutTab);
        labelFeatures->setObjectName(QStringLiteral("labelFeatures"));
        QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(labelFeatures->sizePolicy().hasHeightForWidth());
        labelFeatures->setSizePolicy(sizePolicy2);
        labelFeatures->setWordWrap(true);

        verticalLayout_6->addWidget(labelFeatures);


        horizontalLayout_2->addLayout(verticalLayout_6);

        widget = new QWidget(aboutTab);
        widget->setObjectName(QStringLiteral("widget"));
        verticalLayout_7 = new QVBoxLayout(widget);
        verticalLayout_7->setObjectName(QStringLiteral("verticalLayout_7"));
        labelLogo = new QLabel(widget);
        labelLogo->setObjectName(QStringLiteral("labelLogo"));
        labelLogo->setMaximumSize(QSize(80, 80));
        labelLogo->setPixmap(QPixmap(QString::fromUtf8(":/logo")));
        labelLogo->setScaledContents(true);
        labelLogo->setAlignment(Qt::AlignRight|Qt::AlignTop|Qt::AlignTrailing);

        verticalLayout_7->addWidget(labelLogo);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_7->addItem(verticalSpacer);


        horizontalLayout_2->addWidget(widget);


        verticalLayout_3->addLayout(horizontalLayout_2);

        tabWidget->addTab(aboutTab, QString());
        contribTab = new QWidget();
        contribTab->setObjectName(QStringLiteral("contribTab"));
        verticalLayout = new QVBoxLayout(contribTab);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        scrollArea = new QScrollArea(contribTab);
        scrollArea->setObjectName(QStringLiteral("scrollArea"));
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 532, 234));
        verticalLayout_5 = new QVBoxLayout(scrollAreaWidgetContents);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        contributors = new QLabel(scrollAreaWidgetContents);
        contributors->setObjectName(QStringLiteral("contributors"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(contributors->sizePolicy().hasHeightForWidth());
        contributors->setSizePolicy(sizePolicy3);
        contributors->setCursor(QCursor(Qt::IBeamCursor));
        contributors->setText(QStringLiteral(""));
        contributors->setTextFormat(Qt::AutoText);
        contributors->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        contributors->setWordWrap(true);
        contributors->setMargin(5);
        contributors->setTextInteractionFlags(Qt::TextBrowserInteraction);

        verticalLayout_5->addWidget(contributors);

        scrollArea->setWidget(scrollAreaWidgetContents);

        verticalLayout->addWidget(scrollArea);

        seeContributions = new QLabel(contribTab);
        seeContributions->setObjectName(QStringLiteral("seeContributions"));
        seeContributions->setAlignment(Qt::AlignCenter);
        seeContributions->setOpenExternalLinks(true);

        verticalLayout->addWidget(seeContributions);

        tabWidget->addTab(contribTab, QString());
        debugTab = new QWidget();
        debugTab->setObjectName(QStringLiteral("debugTab"));
        verticalLayout_4 = new QVBoxLayout(debugTab);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        label = new QLabel(debugTab);
        label->setObjectName(QStringLiteral("label"));
        QSizePolicy sizePolicy4(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy4);

        verticalLayout_4->addWidget(label);

        debugInfo = new QPlainTextEdit(debugTab);
        debugInfo->setObjectName(QStringLiteral("debugInfo"));
        debugInfo->setReadOnly(true);
        debugInfo->setPlainText(QStringLiteral(""));
        debugInfo->setTextInteractionFlags(Qt::TextBrowserInteraction);

        verticalLayout_4->addWidget(debugInfo);

        copyToClipboard = new QPushButton(debugTab);
        copyToClipboard->setObjectName(QStringLiteral("copyToClipboard"));

        verticalLayout_4->addWidget(copyToClipboard);

        tabWidget->addTab(debugTab, QString());

        verticalLayout_2->addWidget(tabWidget);

        labelCopyright = new QLabel(AboutDialog);
        labelCopyright->setObjectName(QStringLiteral("labelCopyright"));

        verticalLayout_2->addWidget(labelCopyright);

        btBox = new QDialogButtonBox(AboutDialog);
        btBox->setObjectName(QStringLiteral("btBox"));
        btBox->setStandardButtons(QDialogButtonBox::Close);

        verticalLayout_2->addWidget(btBox);

        verticalLayout_2->setStretch(1, 1);
        QWidget::setTabOrder(tabWidget, scrollArea);
        QWidget::setTabOrder(scrollArea, debugInfo);
        QWidget::setTabOrder(debugInfo, copyToClipboard);

        retranslateUi(AboutDialog);

        tabWidget->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(AboutDialog);
    } // setupUi

    void retranslateUi(QDialog *AboutDialog)
    {
        AboutDialog->setWindowTitle(QApplication::translate("AboutDialog", "About KeePassXC", Q_NULLPTR));
        labelFeatures->setText(QApplication::translate("AboutDialog", "<html><head/><body><p><span style=\" font-size:9pt;\">NODA uses the following libraries/components:</span></p><p>- <a href=\"https://github.com/google/flatbuffers\"><span style=\" text-decoration: underline; color:#0000ff;\">Google Flatbuffers</span></a> for packet serilization</p><p>- A custom fork of <a href=\"https://github.com/lsalzman/enet\"><span style=\" text-decoration: underline; color:#0000ff;\">ENet</span></a> for networking</p><p>- The <a href=\"https://www.hex-rays.com/\"><span style=\" text-decoration: underline; color:#0000ff;\">IDA Pro</span></a> SDK</p><p>- Icons by <a href=\"https://fontawesome.com/license\"><span style=\" text-decoration: underline; color:#0000ff;\">Font Awesome</span></a>, color customized<br/></p></body></html>", Q_NULLPTR));
        labelLogo->setText(QString());
        tabWidget->setTabText(tabWidget->indexOf(aboutTab), QApplication::translate("AboutDialog", "About", Q_NULLPTR));
        seeContributions->setText(QApplication::translate("AboutDialog", "<a href=\"https://github.com/keepassxreboot/keepassxc/graphs/contributors\">See Contributions on GitHub</a>", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(contribTab), QApplication::translate("AboutDialog", "Contributors", Q_NULLPTR));
        label->setText(QApplication::translate("AboutDialog", "Include the following information whenever you report a bug:", Q_NULLPTR));
        copyToClipboard->setText(QApplication::translate("AboutDialog", "Copy to clipboard", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(debugTab), QApplication::translate("AboutDialog", "Debug Info", Q_NULLPTR));
        labelCopyright->setText(QApplication::translate("AboutDialog", "<html><head/><body><p align=\"center\"><span style=\" font-weight:600;\">Copyright (C) NOMAD Group &lt;</span><a href=\"https://www.nomad-group.net/\"><span style=\" font-weight:600; text-decoration: underline; color:#0000ff;\">nomad-group.ne</span></a><span style=\" font-weight:600;\">t&gt;</span></p></body></html>", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class AboutDialog: public Ui_AboutDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUTDIALOG2_H
