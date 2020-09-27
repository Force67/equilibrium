/********************************************************************************
** Form generated from reading UI file 'Settings.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGS_H
#define UI_SETTINGS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SettingsDialog
{
public:
    QVBoxLayout *settings_dialog_layout;
    QTabWidget *tabBar;
    QWidget *tabSync;
    QVBoxLayout *networkTab_layout;
    QHBoxLayout *layoutSync;
    QGroupBox *gbSyncConfig;
    QVBoxLayout *gb_network_status_layout;
    QGroupBox *gbNetAddress;
    QVBoxLayout *gb_netStatusBox_layout;
    QHBoxLayout *horizontalLayout_5;
    QLabel *labelIP;
    QLabel *labelPort;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *editIP;
    QLineEdit *editPort;
    QGroupBox *gbEditSyncConfig;
    QVBoxLayout *gb_edit_dns_layout;
    QHBoxLayout *horizontalLayout_4;
    QLabel *labelUser;
    QLabel *labelPass;
    QHBoxLayout *horizontalLayout_3;
    QLineEdit *editUser;
    QLineEdit *editPass;
    QSpacerItem *networkTabSpacerLeft;
    QGroupBox *gbNetConfig;
    QVBoxLayout *gb_psn_config_layout;
    QGroupBox *gbNetClient;
    QVBoxLayout *gb_psnStatusBox_layout;
    QLabel *labelTimeout;
    QLineEdit *editTimeout;
    QSpacerItem *networkTabSpacerRight;
    QWidget *guiTab;
    QVBoxLayout *guiTab_layout;
    QHBoxLayout *guiTabLayout;
    QVBoxLayout *guiTabLayoutLeft;
    QGroupBox *gbStyleSheet;
    QVBoxLayout *gb_stylesheets_layout;
    QComboBox *comboStyles;
    QPushButton *pbApplySheet;
    QGroupBox *gb_colors;
    QVBoxLayout *gb_colors_layout;
    QCheckBox *cbCustomColors;
    QSpacerItem *guiTabSpacerLeft;
    QVBoxLayout *guiTabLayoutRight;
    QGroupBox *gbUiOptions;
    QVBoxLayout *gb_gui_options_layout;
    QCheckBox *cbShowWelcome;
    QCheckBox *cbShowAutoconnect;
    QSpacerItem *guiTabSpacerRight;
    QGroupBox *gb_updates;
    QVBoxLayout *layout_gb_updates;
    QComboBox *combo_updates;
    QGroupBox *gbDiscord;
    QVBoxLayout *gb_discord_layout;
    QCheckBox *cbUseDiscordRPC;
    QWidget *tabDebug;
    QVBoxLayout *debugTab_layout;
    QHBoxLayout *debugTabLayout;
    QGroupBox *gb_debug_gpu;
    QVBoxLayout *gb_debug_gpu_layout;
    QCheckBox *debugOutput;
    QGroupBox *dbDebugMain;
    QVBoxLayout *gb_debug_core_layout;
    QCheckBox *cbXPSDebug;
    QSpacerItem *verticalSpacer;
    QSpacerItem *debugTabSpacer;

    void setupUi(QDialog *SettingsDialog)
    {
        if (SettingsDialog->objectName().isEmpty())
            SettingsDialog->setObjectName(QStringLiteral("SettingsDialog"));
        SettingsDialog->resize(635, 483);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(SettingsDialog->sizePolicy().hasHeightForWidth());
        SettingsDialog->setSizePolicy(sizePolicy);
        QIcon icon;
        icon.addFile(QStringLiteral(":/logo"), QSize(), QIcon::Normal, QIcon::Off);
        SettingsDialog->setWindowIcon(icon);
        settings_dialog_layout = new QVBoxLayout(SettingsDialog);
        settings_dialog_layout->setObjectName(QStringLiteral("settings_dialog_layout"));
        tabBar = new QTabWidget(SettingsDialog);
        tabBar->setObjectName(QStringLiteral("tabBar"));
        tabBar->setEnabled(true);
        sizePolicy.setHeightForWidth(tabBar->sizePolicy().hasHeightForWidth());
        tabBar->setSizePolicy(sizePolicy);
        tabSync = new QWidget();
        tabSync->setObjectName(QStringLiteral("tabSync"));
        networkTab_layout = new QVBoxLayout(tabSync);
        networkTab_layout->setObjectName(QStringLiteral("networkTab_layout"));
        layoutSync = new QHBoxLayout();
        layoutSync->setObjectName(QStringLiteral("layoutSync"));
        gbSyncConfig = new QGroupBox(tabSync);
        gbSyncConfig->setObjectName(QStringLiteral("gbSyncConfig"));
        gb_network_status_layout = new QVBoxLayout(gbSyncConfig);
        gb_network_status_layout->setObjectName(QStringLiteral("gb_network_status_layout"));
        gbNetAddress = new QGroupBox(gbSyncConfig);
        gbNetAddress->setObjectName(QStringLiteral("gbNetAddress"));
        gb_netStatusBox_layout = new QVBoxLayout(gbNetAddress);
        gb_netStatusBox_layout->setObjectName(QStringLiteral("gb_netStatusBox_layout"));
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        labelIP = new QLabel(gbNetAddress);
        labelIP->setObjectName(QStringLiteral("labelIP"));

        horizontalLayout_5->addWidget(labelIP);

        labelPort = new QLabel(gbNetAddress);
        labelPort->setObjectName(QStringLiteral("labelPort"));

        horizontalLayout_5->addWidget(labelPort);


        gb_netStatusBox_layout->addLayout(horizontalLayout_5);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        editIP = new QLineEdit(gbNetAddress);
        editIP->setObjectName(QStringLiteral("editIP"));

        horizontalLayout_2->addWidget(editIP);

        editPort = new QLineEdit(gbNetAddress);
        editPort->setObjectName(QStringLiteral("editPort"));

        horizontalLayout_2->addWidget(editPort);


        gb_netStatusBox_layout->addLayout(horizontalLayout_2);


        gb_network_status_layout->addWidget(gbNetAddress);

        gbEditSyncConfig = new QGroupBox(gbSyncConfig);
        gbEditSyncConfig->setObjectName(QStringLiteral("gbEditSyncConfig"));
        gb_edit_dns_layout = new QVBoxLayout(gbEditSyncConfig);
        gb_edit_dns_layout->setObjectName(QStringLiteral("gb_edit_dns_layout"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        labelUser = new QLabel(gbEditSyncConfig);
        labelUser->setObjectName(QStringLiteral("labelUser"));

        horizontalLayout_4->addWidget(labelUser);

        labelPass = new QLabel(gbEditSyncConfig);
        labelPass->setObjectName(QStringLiteral("labelPass"));

        horizontalLayout_4->addWidget(labelPass);


        gb_edit_dns_layout->addLayout(horizontalLayout_4);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        editUser = new QLineEdit(gbEditSyncConfig);
        editUser->setObjectName(QStringLiteral("editUser"));

        horizontalLayout_3->addWidget(editUser);

        editPass = new QLineEdit(gbEditSyncConfig);
        editPass->setObjectName(QStringLiteral("editPass"));

        horizontalLayout_3->addWidget(editPass);


        gb_edit_dns_layout->addLayout(horizontalLayout_3);


        gb_network_status_layout->addWidget(gbEditSyncConfig);

        networkTabSpacerLeft = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gb_network_status_layout->addItem(networkTabSpacerLeft);


        layoutSync->addWidget(gbSyncConfig);

        gbNetConfig = new QGroupBox(tabSync);
        gbNetConfig->setObjectName(QStringLiteral("gbNetConfig"));
        gb_psn_config_layout = new QVBoxLayout(gbNetConfig);
        gb_psn_config_layout->setObjectName(QStringLiteral("gb_psn_config_layout"));
        gbNetClient = new QGroupBox(gbNetConfig);
        gbNetClient->setObjectName(QStringLiteral("gbNetClient"));
        gb_psnStatusBox_layout = new QVBoxLayout(gbNetClient);
        gb_psnStatusBox_layout->setObjectName(QStringLiteral("gb_psnStatusBox_layout"));
        labelTimeout = new QLabel(gbNetClient);
        labelTimeout->setObjectName(QStringLiteral("labelTimeout"));

        gb_psnStatusBox_layout->addWidget(labelTimeout);

        editTimeout = new QLineEdit(gbNetClient);
        editTimeout->setObjectName(QStringLiteral("editTimeout"));

        gb_psnStatusBox_layout->addWidget(editTimeout);


        gb_psn_config_layout->addWidget(gbNetClient);

        networkTabSpacerRight = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gb_psn_config_layout->addItem(networkTabSpacerRight);


        layoutSync->addWidget(gbNetConfig);


        networkTab_layout->addLayout(layoutSync);

        QIcon icon1;
        icon1.addFile(QStringLiteral(":/sync"), QSize(), QIcon::Normal, QIcon::Off);
        tabBar->addTab(tabSync, icon1, QString());
        guiTab = new QWidget();
        guiTab->setObjectName(QStringLiteral("guiTab"));
        guiTab_layout = new QVBoxLayout(guiTab);
        guiTab_layout->setObjectName(QStringLiteral("guiTab_layout"));
        guiTabLayout = new QHBoxLayout();
        guiTabLayout->setObjectName(QStringLiteral("guiTabLayout"));
        guiTabLayoutLeft = new QVBoxLayout();
        guiTabLayoutLeft->setObjectName(QStringLiteral("guiTabLayoutLeft"));
        gbStyleSheet = new QGroupBox(guiTab);
        gbStyleSheet->setObjectName(QStringLiteral("gbStyleSheet"));
        gb_stylesheets_layout = new QVBoxLayout(gbStyleSheet);
        gb_stylesheets_layout->setObjectName(QStringLiteral("gb_stylesheets_layout"));
        comboStyles = new QComboBox(gbStyleSheet);
        comboStyles->setObjectName(QStringLiteral("comboStyles"));

        gb_stylesheets_layout->addWidget(comboStyles);

        pbApplySheet = new QPushButton(gbStyleSheet);
        pbApplySheet->setObjectName(QStringLiteral("pbApplySheet"));

        gb_stylesheets_layout->addWidget(pbApplySheet);


        guiTabLayoutLeft->addWidget(gbStyleSheet);

        gb_colors = new QGroupBox(guiTab);
        gb_colors->setObjectName(QStringLiteral("gb_colors"));
        gb_colors_layout = new QVBoxLayout(gb_colors);
        gb_colors_layout->setObjectName(QStringLiteral("gb_colors_layout"));
        gb_colors_layout->setContentsMargins(9, 9, 9, 9);
        cbCustomColors = new QCheckBox(gb_colors);
        cbCustomColors->setObjectName(QStringLiteral("cbCustomColors"));

        gb_colors_layout->addWidget(cbCustomColors);

        guiTabSpacerLeft = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

        gb_colors_layout->addItem(guiTabSpacerLeft);


        guiTabLayoutLeft->addWidget(gb_colors);


        guiTabLayout->addLayout(guiTabLayoutLeft);

        guiTabLayoutRight = new QVBoxLayout();
        guiTabLayoutRight->setObjectName(QStringLiteral("guiTabLayoutRight"));
        gbUiOptions = new QGroupBox(guiTab);
        gbUiOptions->setObjectName(QStringLiteral("gbUiOptions"));
        gb_gui_options_layout = new QVBoxLayout(gbUiOptions);
        gb_gui_options_layout->setObjectName(QStringLiteral("gb_gui_options_layout"));
        cbShowWelcome = new QCheckBox(gbUiOptions);
        cbShowWelcome->setObjectName(QStringLiteral("cbShowWelcome"));

        gb_gui_options_layout->addWidget(cbShowWelcome);

        cbShowAutoconnect = new QCheckBox(gbUiOptions);
        cbShowAutoconnect->setObjectName(QStringLiteral("cbShowAutoconnect"));

        gb_gui_options_layout->addWidget(cbShowAutoconnect);

        guiTabSpacerRight = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

        gb_gui_options_layout->addItem(guiTabSpacerRight);


        guiTabLayoutRight->addWidget(gbUiOptions);

        gb_updates = new QGroupBox(guiTab);
        gb_updates->setObjectName(QStringLiteral("gb_updates"));
        layout_gb_updates = new QVBoxLayout(gb_updates);
        layout_gb_updates->setObjectName(QStringLiteral("layout_gb_updates"));
        combo_updates = new QComboBox(gb_updates);
        combo_updates->setObjectName(QStringLiteral("combo_updates"));

        layout_gb_updates->addWidget(combo_updates);


        guiTabLayoutRight->addWidget(gb_updates);

        gbDiscord = new QGroupBox(guiTab);
        gbDiscord->setObjectName(QStringLiteral("gbDiscord"));
        gb_discord_layout = new QVBoxLayout(gbDiscord);
        gb_discord_layout->setObjectName(QStringLiteral("gb_discord_layout"));
        cbUseDiscordRPC = new QCheckBox(gbDiscord);
        cbUseDiscordRPC->setObjectName(QStringLiteral("cbUseDiscordRPC"));

        gb_discord_layout->addWidget(cbUseDiscordRPC);


        guiTabLayoutRight->addWidget(gbDiscord);


        guiTabLayout->addLayout(guiTabLayoutRight);

        guiTabLayout->setStretch(0, 1);
        guiTabLayout->setStretch(1, 1);

        guiTab_layout->addLayout(guiTabLayout);

        QIcon icon2;
        icon2.addFile(QStringLiteral(":/uikit"), QSize(), QIcon::Normal, QIcon::Off);
        tabBar->addTab(guiTab, icon2, QString());
        tabDebug = new QWidget();
        tabDebug->setObjectName(QStringLiteral("tabDebug"));
        debugTab_layout = new QVBoxLayout(tabDebug);
        debugTab_layout->setObjectName(QStringLiteral("debugTab_layout"));
        debugTabLayout = new QHBoxLayout();
        debugTabLayout->setObjectName(QStringLiteral("debugTabLayout"));
        gb_debug_gpu = new QGroupBox(tabDebug);
        gb_debug_gpu->setObjectName(QStringLiteral("gb_debug_gpu"));
        gb_debug_gpu->setEnabled(true);
        gb_debug_gpu->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        gb_debug_gpu_layout = new QVBoxLayout(gb_debug_gpu);
        gb_debug_gpu_layout->setObjectName(QStringLiteral("gb_debug_gpu_layout"));
        debugOutput = new QCheckBox(gb_debug_gpu);
        debugOutput->setObjectName(QStringLiteral("debugOutput"));

        gb_debug_gpu_layout->addWidget(debugOutput, 0, Qt::AlignTop);


        debugTabLayout->addWidget(gb_debug_gpu);

        dbDebugMain = new QGroupBox(tabDebug);
        dbDebugMain->setObjectName(QStringLiteral("dbDebugMain"));
        gb_debug_core_layout = new QVBoxLayout(dbDebugMain);
        gb_debug_core_layout->setObjectName(QStringLiteral("gb_debug_core_layout"));
        cbXPSDebug = new QCheckBox(dbDebugMain);
        cbXPSDebug->setObjectName(QStringLiteral("cbXPSDebug"));

        gb_debug_core_layout->addWidget(cbXPSDebug);

        verticalSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gb_debug_core_layout->addItem(verticalSpacer);


        debugTabLayout->addWidget(dbDebugMain);

        debugTabLayout->setStretch(0, 1);
        debugTabLayout->setStretch(1, 1);

        debugTab_layout->addLayout(debugTabLayout);

        debugTabSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        debugTab_layout->addItem(debugTabSpacer);

        QIcon icon3;
        icon3.addFile(QStringLiteral(":/bug"), QSize(), QIcon::Normal, QIcon::Off);
        tabBar->addTab(tabDebug, icon3, QString());

        settings_dialog_layout->addWidget(tabBar);


        retranslateUi(SettingsDialog);

        tabBar->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(SettingsDialog);
    } // setupUi

    void retranslateUi(QDialog *SettingsDialog)
    {
        SettingsDialog->setWindowTitle(QApplication::translate("SettingsDialog", "NODA Settings", Q_NULLPTR));
        gbSyncConfig->setTitle(QApplication::translate("SettingsDialog", "Synchronium Server Configuration", Q_NULLPTR));
        gbNetAddress->setTitle(QApplication::translate("SettingsDialog", "Network Address", Q_NULLPTR));
        labelIP->setText(QApplication::translate("SettingsDialog", "IP Address", Q_NULLPTR));
        labelPort->setText(QApplication::translate("SettingsDialog", "Port", Q_NULLPTR));
        gbEditSyncConfig->setTitle(QApplication::translate("SettingsDialog", "User Configuration", Q_NULLPTR));
        labelUser->setText(QApplication::translate("SettingsDialog", "Username", Q_NULLPTR));
        labelPass->setText(QApplication::translate("SettingsDialog", "Password", Q_NULLPTR));
        gbNetConfig->setTitle(QApplication::translate("SettingsDialog", "Network Configuration", Q_NULLPTR));
        gbNetClient->setTitle(QApplication::translate("SettingsDialog", "Net Client", Q_NULLPTR));
        labelTimeout->setText(QApplication::translate("SettingsDialog", "Timeout", Q_NULLPTR));
        tabBar->setTabText(tabBar->indexOf(tabSync), QApplication::translate("SettingsDialog", "Sync", Q_NULLPTR));
        gbStyleSheet->setTitle(QApplication::translate("SettingsDialog", "UI Stylesheets", Q_NULLPTR));
        pbApplySheet->setText(QApplication::translate("SettingsDialog", "Apply", Q_NULLPTR));
        gb_colors->setTitle(QApplication::translate("SettingsDialog", "UI Colors", Q_NULLPTR));
        cbCustomColors->setText(QApplication::translate("SettingsDialog", "Use custom UI Colors", Q_NULLPTR));
        gbUiOptions->setTitle(QApplication::translate("SettingsDialog", "UI Options", Q_NULLPTR));
        cbShowWelcome->setText(QApplication::translate("SettingsDialog", "Show Welcome Screen", Q_NULLPTR));
        cbShowAutoconnect->setText(QApplication::translate("SettingsDialog", "Show Auto Connect Promt", Q_NULLPTR));
        gb_updates->setTitle(QApplication::translate("SettingsDialog", "Check for updates on startup", Q_NULLPTR));
        gbDiscord->setTitle(QApplication::translate("SettingsDialog", "Discord", Q_NULLPTR));
        cbUseDiscordRPC->setText(QApplication::translate("SettingsDialog", "Use Discord Rich Presence", Q_NULLPTR));
        tabBar->setTabText(tabBar->indexOf(guiTab), QApplication::translate("SettingsDialog", "Ui", Q_NULLPTR));
        gb_debug_gpu->setTitle(QApplication::translate("SettingsDialog", "Plugin", Q_NULLPTR));
        debugOutput->setText(QApplication::translate("SettingsDialog", "Debug Output", Q_NULLPTR));
        dbDebugMain->setTitle(QApplication::translate("SettingsDialog", "Main", Q_NULLPTR));
        cbXPSDebug->setText(QApplication::translate("SettingsDialog", "XPS Debug", Q_NULLPTR));
        tabBar->setTabText(tabBar->indexOf(tabDebug), QApplication::translate("SettingsDialog", "Debug", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class SettingsDialog: public Ui_SettingsDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGS_H
