/********************************************************************************
** Form generated from reading UI file 'StatusBar.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STATUSBAR_H
#define UI_STATUSBAR_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_StatusBar {
  public:
  QWidget *dockWidgetContents;

  void setupUi(QDockWidget *StatusBar) {
	if (StatusBar->objectName().isEmpty())
	  StatusBar->setObjectName(QStringLiteral("StatusBar"));
	StatusBar->setWindowModality(Qt::NonModal);
	StatusBar->resize(400, 43);
	StatusBar->setContextMenuPolicy(Qt::NoContextMenu);
	StatusBar->setAutoFillBackground(false);
	StatusBar->setStyleSheet(QLatin1String(
		"QDockWidget\n"
		"{\n"
		"	background-color:rgb(255, 0, 0)\n"
		"}"));
	StatusBar->setFloating(false);
	dockWidgetContents = new QWidget();
	dockWidgetContents->setObjectName(QStringLiteral("dockWidgetContents"));
	StatusBar->setWidget(dockWidgetContents);

	retranslateUi(StatusBar);

	QMetaObject::connectSlotsByName(StatusBar);
  } // setupUi

  void retranslateUi(QDockWidget *StatusBar) {
	StatusBar->setWindowTitle(QApplication::translate("StatusBar", "NODA Sync", Q_NULLPTR));
  } // retranslateUi
};

namespace Ui {
  class StatusBar : public Ui_StatusBar {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STATUSBAR_H
