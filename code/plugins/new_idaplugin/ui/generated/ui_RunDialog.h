/********************************************************************************
** Form generated from reading UI file 'RunDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RUNDIALOG_H
#define UI_RUNDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_RunDialog {
public:
  QHBoxLayout *horizontalLayout;
  QHBoxLayout *horizontalLayout_2;
  QListWidget *optionList;
  QVBoxLayout *verticalLayout;
  QPushButton *btOk;
  QPushButton *btQuit;
  QSpacerItem *verticalSpacer;

  void setupUi(QDialog *RunDialog)
  {
	if(RunDialog->objectName().isEmpty())
	  RunDialog->setObjectName(QStringLiteral("RunDialog"));
	RunDialog->resize(306, 263);
	QIcon icon;
	icon.addFile(QStringLiteral(":/logo"), QSize(), QIcon::Normal, QIcon::Off);
	RunDialog->setWindowIcon(icon);
	horizontalLayout = new QHBoxLayout(RunDialog);
	horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
	horizontalLayout_2 = new QHBoxLayout();
	horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
	optionList = new QListWidget(RunDialog);
	new QListWidgetItem(optionList);
	new QListWidgetItem(optionList);
	optionList->setObjectName(QStringLiteral("optionList"));

	horizontalLayout_2->addWidget(optionList);

	verticalLayout = new QVBoxLayout();
	verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
	btOk = new QPushButton(RunDialog);
	btOk->setObjectName(QStringLiteral("btOk"));

	verticalLayout->addWidget(btOk);

	btQuit = new QPushButton(RunDialog);
	btQuit->setObjectName(QStringLiteral("btQuit"));

	verticalLayout->addWidget(btQuit);

	verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

	verticalLayout->addItem(verticalSpacer);

	horizontalLayout_2->addLayout(verticalLayout);

	horizontalLayout->addLayout(horizontalLayout_2);

	retranslateUi(RunDialog);

	QMetaObject::connectSlotsByName(RunDialog);
  } // setupUi

  void retranslateUi(QDialog *RunDialog)
  {
	RunDialog->setWindowTitle(QApplication::translate("RunDialog", "NODA - Run", Q_NULLPTR));

	const bool __sortingEnabled = optionList->isSortingEnabled();
	optionList->setSortingEnabled(false);
	QListWidgetItem *___qlistwidgetitem = optionList->item(0);
	___qlistwidgetitem->setText(QApplication::translate("RunDialog", "Create Dumb Pattern", Q_NULLPTR));
	QListWidgetItem *___qlistwidgetitem1 = optionList->item(1);
	___qlistwidgetitem1->setText(QApplication::translate("RunDialog", "Request Updates", Q_NULLPTR));
	optionList->setSortingEnabled(__sortingEnabled);

	btOk->setText(QApplication::translate("RunDialog", "OK", Q_NULLPTR));
	btQuit->setText(QApplication::translate("RunDialog", "Cancel", Q_NULLPTR));
  } // retranslateUi
};

namespace Ui {
  class RunDialog : public Ui_RunDialog {
  };
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RUNDIALOG_H
