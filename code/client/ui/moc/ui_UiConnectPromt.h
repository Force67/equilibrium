/********************************************************************************
** Form generated from reading UI file 'UiConnectPromt.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UICONNECTPROMT_H
#define UI_UICONNECTPROMT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ConnectPromt {
  public:
  QVBoxLayout *verticalLayout;
  QLabel *label;
  QHBoxLayout *horizontalLayout;
  QSpacerItem *horizontalSpacer;
  QPushButton *button_yes;
  QPushButton *button_no;
  QPushButton *button_dontaskagain;

  void setupUi(QDialog *ConnectPromt) {
	if (ConnectPromt->objectName().isEmpty())
	  ConnectPromt->setObjectName(QStringLiteral("ConnectPromt"));
	ConnectPromt->resize(518, 84);
	verticalLayout = new QVBoxLayout(ConnectPromt);
	verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
	label = new QLabel(ConnectPromt);
	label->setObjectName(QStringLiteral("label"));
	label->setAlignment(Qt::AlignCenter);

	verticalLayout->addWidget(label);

	horizontalLayout = new QHBoxLayout();
	horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
	horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	horizontalLayout->addItem(horizontalSpacer);

	button_yes = new QPushButton(ConnectPromt);
	button_yes->setObjectName(QStringLiteral("button_yes"));

	horizontalLayout->addWidget(button_yes);

	button_no = new QPushButton(ConnectPromt);
	button_no->setObjectName(QStringLiteral("button_no"));

	horizontalLayout->addWidget(button_no);

	button_dontaskagain = new QPushButton(ConnectPromt);
	button_dontaskagain->setObjectName(QStringLiteral("button_dontaskagain"));

	horizontalLayout->addWidget(button_dontaskagain);

	verticalLayout->addLayout(horizontalLayout);

	retranslateUi(ConnectPromt);

	button_yes->setDefault(true);

	QMetaObject::connectSlotsByName(ConnectPromt);
  } // setupUi

  void retranslateUi(QDialog *ConnectPromt) {
	ConnectPromt->setWindowTitle(QApplication::translate("ConnectPromt", "NODA: Auto-Connect", Q_NULLPTR));
	label->setText(QApplication::translate("ConnectPromt", "NODA detected a synced IDB. Do you wish to connect to Sync-Server?", Q_NULLPTR));
	button_yes->setText(QApplication::translate("ConnectPromt", "Connect", Q_NULLPTR));
	button_no->setText(QApplication::translate("ConnectPromt", "Cancel", Q_NULLPTR));
	button_dontaskagain->setText(QApplication::translate("ConnectPromt", "Dont ask again", Q_NULLPTR));
  } // retranslateUi
};

namespace Ui {
  class ConnectPromt : public Ui_ConnectPromt {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_UICONNECTPROMT_H
