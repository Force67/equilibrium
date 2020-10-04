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
    QLabel *label_2;
    QLabel *label_5;
    QSpacerItem *verticalSpacer_3;
    QLabel *label_3;
    QSpacerItem *verticalSpacer_2;
    QLabel *label_8;
    QLabel *maintainers;
    QLabel *label_7;
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
    QDialogButtonBox *buttonBox;

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

        horizontalLayout->addWidget(nameLabel);

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
        label_2 = new QLabel(aboutTab);
        label_2->setObjectName(QStringLiteral("label_2"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy2);
        label_2->setText(QStringLiteral("Website: <a href=\"https://keepassxc.org/\" style=\"text-decoration: underline\">https://keepassxc.org</a>"));
        label_2->setOpenExternalLinks(true);

        verticalLayout_3->addWidget(label_2);

        label_5 = new QLabel(aboutTab);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setOpenExternalLinks(true);

        verticalLayout_3->addWidget(label_5);

        verticalSpacer_3 = new QSpacerItem(20, 5, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout_3->addItem(verticalSpacer_3);

        label_3 = new QLabel(aboutTab);
        label_3->setObjectName(QStringLiteral("label_3"));
        QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(label_3->sizePolicy().hasHeightForWidth());
        label_3->setSizePolicy(sizePolicy3);
        label_3->setWordWrap(true);

        verticalLayout_3->addWidget(label_3);

        verticalSpacer_2 = new QSpacerItem(0, 5, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout_3->addItem(verticalSpacer_2);

        label_8 = new QLabel(aboutTab);
        label_8->setObjectName(QStringLiteral("label_8"));

        verticalLayout_3->addWidget(label_8);

        maintainers = new QLabel(aboutTab);
        maintainers->setObjectName(QStringLiteral("maintainers"));
        sizePolicy2.setHeightForWidth(maintainers->sizePolicy().hasHeightForWidth());
        maintainers->setSizePolicy(sizePolicy2);
        maintainers->setText(QStringLiteral(""));
        maintainers->setOpenExternalLinks(true);
        maintainers->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);

        verticalLayout_3->addWidget(maintainers);

        label_7 = new QLabel(aboutTab);
        label_7->setObjectName(QStringLiteral("label_7"));
        sizePolicy2.setHeightForWidth(label_7->sizePolicy().hasHeightForWidth());
        label_7->setSizePolicy(sizePolicy2);
        label_7->setWordWrap(true);
        label_7->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard|Qt::LinksAccessibleByMouse);

        verticalLayout_3->addWidget(label_7);

        verticalSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer);

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
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 532, 262));
        verticalLayout_5 = new QVBoxLayout(scrollAreaWidgetContents);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        contributors = new QLabel(scrollAreaWidgetContents);
        contributors->setObjectName(QStringLiteral("contributors"));
        QSizePolicy sizePolicy4(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(contributors->sizePolicy().hasHeightForWidth());
        contributors->setSizePolicy(sizePolicy4);
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
        sizePolicy2.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy2);

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

        buttonBox = new QDialogButtonBox(AboutDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setStandardButtons(QDialogButtonBox::Close);

        verticalLayout_2->addWidget(buttonBox);

        verticalLayout_2->setStretch(1, 1);
        QWidget::setTabOrder(tabWidget, scrollArea);
        QWidget::setTabOrder(scrollArea, debugInfo);
        QWidget::setTabOrder(debugInfo, copyToClipboard);

        retranslateUi(AboutDialog);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(AboutDialog);
    } // setupUi

    void retranslateUi(QDialog *AboutDialog)
    {
        AboutDialog->setWindowTitle(QApplication::translate("AboutDialog", "About KeePassXC", Q_NULLPTR));
        label_5->setText(QApplication::translate("AboutDialog", "Report bugs at: <a href=\"https://github.com/keepassxreboot/keepassxc/issues\" style=\"text-decoration: underline;\">https://github.com</a>", Q_NULLPTR));
        label_3->setText(QApplication::translate("AboutDialog", "KeePassXC is distributed under the terms of the GNU General Public License (GPL) version 2 or (at your option) version 3.", Q_NULLPTR));
        label_8->setText(QApplication::translate("AboutDialog", "Project Maintainers:", Q_NULLPTR));
        label_7->setText(QApplication::translate("AboutDialog", "Special thanks from the KeePassXC team go to debfx for creating the original KeePassX.", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(aboutTab), QApplication::translate("AboutDialog", "About", Q_NULLPTR));
        seeContributions->setText(QApplication::translate("AboutDialog", "<a href=\"https://github.com/keepassxreboot/keepassxc/graphs/contributors\">See Contributions on GitHub</a>", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(contribTab), QApplication::translate("AboutDialog", "Contributors", Q_NULLPTR));
        label->setText(QApplication::translate("AboutDialog", "Include the following information whenever you report a bug:", Q_NULLPTR));
        copyToClipboard->setText(QApplication::translate("AboutDialog", "Copy to clipboard", Q_NULLPTR));
        tabWidget->setTabText(tabWidget->indexOf(debugTab), QApplication::translate("AboutDialog", "Debug Info", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class AboutDialog: public Ui_AboutDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUTDIALOG2_H
