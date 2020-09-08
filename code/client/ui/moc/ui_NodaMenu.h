/********************************************************************************
** Form generated from reading UI file 'NodaMenu.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NODAMENU_H
#define UI_NODAMENU_H

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

class Ui_MainMenu {
  public:
  QHBoxLayout *horizontalLayout;
  QHBoxLayout *horizontalLayout_2;
  QListWidget *list;
  QVBoxLayout *verticalLayout;
  QPushButton *buttonOkay;
  QPushButton *buttonCancel;
  QSpacerItem *verticalSpacer;

  void setupUi(QDialog *MainMenu) {
	if (MainMenu->objectName().isEmpty())
	  MainMenu->setObjectName(QStringLiteral("MainMenu"));
	MainMenu->resize(306, 263);
	horizontalLayout = new QHBoxLayout(MainMenu);
	horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
	horizontalLayout_2 = new QHBoxLayout();
	horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
	list = new QListWidget(MainMenu);
	new QListWidgetItem(list);
	new QListWidgetItem(list);
	list->setObjectName(QStringLiteral("list"));

	horizontalLayout_2->addWidget(list);

	verticalLayout = new QVBoxLayout();
	verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
	buttonOkay = new QPushButton(MainMenu);
	buttonOkay->setObjectName(QStringLiteral("buttonOkay"));

	verticalLayout->addWidget(buttonOkay);

	buttonCancel = new QPushButton(MainMenu);
	buttonCancel->setObjectName(QStringLiteral("buttonCancel"));

	verticalLayout->addWidget(buttonCancel);

	verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

	verticalLayout->addItem(verticalSpacer);

	horizontalLayout_2->addLayout(verticalLayout);

	horizontalLayout->addLayout(horizontalLayout_2);

	retranslateUi(MainMenu);

	QMetaObject::connectSlotsByName(MainMenu);
  } // setupUi

  void retranslateUi(QDialog *MainMenu) {
	MainMenu->setWindowTitle(QApplication::translate("MainMenu", "Noda", Q_NULLPTR));

	const bool __sortingEnabled = list->isSortingEnabled();
	list->setSortingEnabled(false);
	QListWidgetItem *___qlistwidgetitem = list->item(0);
	___qlistwidgetitem->setText(QApplication::translate("MainMenu", "Connect", Q_NULLPTR));
	QListWidgetItem *___qlistwidgetitem1 = list->item(1);
	___qlistwidgetitem1->setText(QApplication::translate("MainMenu", "Request Updates", Q_NULLPTR));
	list->setSortingEnabled(__sortingEnabled);

	buttonOkay->setText(QApplication::translate("MainMenu", "OK", Q_NULLPTR));
	buttonCancel->setText(QApplication::translate("MainMenu", "Cancel", Q_NULLPTR));
  } // retranslateUi
};

namespace Ui {
  class MainMenu : public Ui_MainMenu {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NODAMENU_H
