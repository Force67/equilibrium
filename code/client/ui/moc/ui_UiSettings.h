/********************************************************************************
** Form generated from reading UI file 'UiSettings.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UISETTINGS_H
#define UI_UISETTINGS_H

#include <QtCore/QLocale>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_UiSettings
{
public:
    QAction *openFileAct;
    QAction *showGeneralOptAct;
    QAction *exitAct;
    QAction *aboutAppAct;
    QAction *updateAct;
    QAction *clearRecentAct;
    QAction *freezeRecentAct;
    QAction *aboutQtAct;
    QAction *aboutFBXAct;
    QAction *showServerSettingsAct;
    QWidget *centralWidget;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QTabWidget *tabBar;
    QWidget *tabSync;
    QGroupBox *groupBox;
    QLabel *labelIP;
    QLabel *labelPort;
    QLabel *label;
    QLineEdit *editIp;
    QLineEdit *editPort;
    QLineEdit *editPassword;
    QLabel *labelUsername;
    QLineEdit *editUsername;
    QWidget *tabStyle;
    QRadioButton *radioButton;
    QStatusBar *statusBar;

    void setupUi(QDialog *UiSettings)
    {
        if (UiSettings->objectName().isEmpty())
            UiSettings->setObjectName(QStringLiteral("UiSettings"));
        UiSettings->resize(714, 439);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(UiSettings->sizePolicy().hasHeightForWidth());
        UiSettings->setSizePolicy(sizePolicy);
        UiSettings->setMinimumSize(QSize(4, 0));
        UiSettings->setAutoFillBackground(false);
        openFileAct = new QAction(UiSettings);
        openFileAct->setObjectName(QStringLiteral("openFileAct"));
        showGeneralOptAct = new QAction(UiSettings);
        showGeneralOptAct->setObjectName(QStringLiteral("showGeneralOptAct"));
        exitAct = new QAction(UiSettings);
        exitAct->setObjectName(QStringLiteral("exitAct"));
        aboutAppAct = new QAction(UiSettings);
        aboutAppAct->setObjectName(QStringLiteral("aboutAppAct"));
        updateAct = new QAction(UiSettings);
        updateAct->setObjectName(QStringLiteral("updateAct"));
        clearRecentAct = new QAction(UiSettings);
        clearRecentAct->setObjectName(QStringLiteral("clearRecentAct"));
        freezeRecentAct = new QAction(UiSettings);
        freezeRecentAct->setObjectName(QStringLiteral("freezeRecentAct"));
        freezeRecentAct->setCheckable(true);
        aboutQtAct = new QAction(UiSettings);
        aboutQtAct->setObjectName(QStringLiteral("aboutQtAct"));
        aboutFBXAct = new QAction(UiSettings);
        aboutFBXAct->setObjectName(QStringLiteral("aboutFBXAct"));
        showServerSettingsAct = new QAction(UiSettings);
        showServerSettingsAct->setObjectName(QStringLiteral("showServerSettingsAct"));
        centralWidget = new QWidget(UiSettings);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        centralWidget->setGeometry(QRect(0, 0, 1061, 531));
        sizePolicy.setHeightForWidth(centralWidget->sizePolicy().hasHeightForWidth());
        centralWidget->setSizePolicy(sizePolicy);
        centralWidget->setLocale(QLocale(QLocale::English, QLocale::Germany));
        verticalLayoutWidget = new QWidget(centralWidget);
        verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(0, 0, 1061, 531));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetMaximumSize);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        tabBar = new QTabWidget(verticalLayoutWidget);
        tabBar->setObjectName(QStringLiteral("tabBar"));
        tabSync = new QWidget();
        tabSync->setObjectName(QStringLiteral("tabSync"));
        groupBox = new QGroupBox(tabSync);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(0, 10, 391, 251));
        groupBox->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
        labelIP = new QLabel(groupBox);
        labelIP->setObjectName(QStringLiteral("labelIP"));
        labelIP->setGeometry(QRect(10, 30, 201, 20));
        labelPort = new QLabel(groupBox);
        labelPort->setObjectName(QStringLiteral("labelPort"));
        labelPort->setGeometry(QRect(140, 30, 121, 20));
        label = new QLabel(groupBox);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(10, 90, 201, 20));
        editIp = new QLineEdit(groupBox);
        editIp->setObjectName(QStringLiteral("editIp"));
        editIp->setGeometry(QRect(10, 50, 113, 25));
        editPort = new QLineEdit(groupBox);
        editPort->setObjectName(QStringLiteral("editPort"));
        editPort->setGeometry(QRect(140, 50, 121, 25));
        editPassword = new QLineEdit(groupBox);
        editPassword->setObjectName(QStringLiteral("editPassword"));
        editPassword->setGeometry(QRect(10, 110, 251, 25));
        labelUsername = new QLabel(groupBox);
        labelUsername->setObjectName(QStringLiteral("labelUsername"));
        labelUsername->setGeometry(QRect(10, 150, 81, 20));
        editUsername = new QLineEdit(groupBox);
        editUsername->setObjectName(QStringLiteral("editUsername"));
        editUsername->setGeometry(QRect(10, 170, 251, 25));
        tabBar->addTab(tabSync, QString());
        tabStyle = new QWidget();
        tabStyle->setObjectName(QStringLiteral("tabStyle"));
        radioButton = new QRadioButton(tabStyle);
        radioButton->setObjectName(QStringLiteral("radioButton"));
        radioButton->setGeometry(QRect(10, 10, 181, 24));
        tabBar->addTab(tabStyle, QString());

        verticalLayout->addWidget(tabBar);

        statusBar = new QStatusBar(UiSettings);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        statusBar->setGeometry(QRect(0, 0, 3, 24));

        retranslateUi(UiSettings);

        tabBar->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(UiSettings);
    } // setupUi

    void retranslateUi(QDialog *UiSettings)
    {
        UiSettings->setWindowTitle(QApplication::translate("UiSettings", "NODA Settings", Q_NULLPTR));
        openFileAct->setText(QApplication::translate("UiSettings", "Open File", Q_NULLPTR));
        showGeneralOptAct->setText(QApplication::translate("UiSettings", "Tool Settings", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
        showGeneralOptAct->setToolTip(QApplication::translate("UiSettings", "Configure CPU", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        exitAct->setText(QApplication::translate("UiSettings", "Exit", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
        exitAct->setToolTip(QApplication::translate("UiSettings", "Exit FormatX", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        exitAct->setStatusTip(QApplication::translate("UiSettings", "Exit the application", Q_NULLPTR));
#endif // QT_NO_STATUSTIP
        aboutAppAct->setText(QApplication::translate("UiSettings", "About RCon2", Q_NULLPTR));
        updateAct->setText(QApplication::translate("UiSettings", "Check for Updates", Q_NULLPTR));
        clearRecentAct->setText(QApplication::translate("UiSettings", "List Clear", Q_NULLPTR));
        freezeRecentAct->setText(QApplication::translate("UiSettings", "List Freeze", Q_NULLPTR));
        aboutQtAct->setText(QApplication::translate("UiSettings", "About Qt", Q_NULLPTR));
        aboutFBXAct->setText(QApplication::translate("UiSettings", "About Autodesk\302\256 FBX\302\256", Q_NULLPTR));
        showServerSettingsAct->setText(QApplication::translate("UiSettings", "Server Settings", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
        tabBar->setToolTip(QApplication::translate("UiSettings", "x", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        groupBox->setTitle(QApplication::translate("UiSettings", "Synchronium Server", Q_NULLPTR));
        labelIP->setText(QApplication::translate("UiSettings", "Address", Q_NULLPTR));
        labelPort->setText(QApplication::translate("UiSettings", "Port", Q_NULLPTR));
        label->setText(QApplication::translate("UiSettings", "Password", Q_NULLPTR));
        labelUsername->setText(QApplication::translate("UiSettings", "Username", Q_NULLPTR));
        tabBar->setTabText(tabBar->indexOf(tabSync), QApplication::translate("UiSettings", "Sync", Q_NULLPTR));
        radioButton->setText(QApplication::translate("UiSettings", "Dark mode", Q_NULLPTR));
        tabBar->setTabText(tabBar->indexOf(tabStyle), QApplication::translate("UiSettings", "Style", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class UiSettings: public Ui_UiSettings {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_UISETTINGS_H
