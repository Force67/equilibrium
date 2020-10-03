/********************************************************************************
** Form generated from reading UI file 'NSyncDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NSYNCDIALOG_H
#define UI_NSYNCDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_NSyncDialog {
public:
  QVBoxLayout *verticalLayout_3;
  QHBoxLayout *horizontalLayout;
  QTreeView *treeView;
  QLabel *workspaceLabel;

  void setupUi(QDialog *NSyncDialog)
  {
	if(NSyncDialog->objectName().isEmpty())
	  NSyncDialog->setObjectName(QStringLiteral("NSyncDialog"));
	NSyncDialog->resize(1429, 921);
	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(NSyncDialog->sizePolicy().hasHeightForWidth());
	NSyncDialog->setSizePolicy(sizePolicy);
	QIcon icon;
	icon.addFile(QStringLiteral(":/logo"), QSize(), QIcon::Normal, QIcon::Off);
	NSyncDialog->setWindowIcon(icon);
	verticalLayout_3 = new QVBoxLayout(NSyncDialog);
	verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
	verticalLayout_3->setContentsMargins(0, 0, 0, 0);
	horizontalLayout = new QHBoxLayout();
	horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
	treeView = new QTreeView(NSyncDialog);
	treeView->setObjectName(QStringLiteral("treeView"));

	horizontalLayout->addWidget(treeView);

	workspaceLabel = new QLabel(NSyncDialog);
	workspaceLabel->setObjectName(QStringLiteral("workspaceLabel"));

	horizontalLayout->addWidget(workspaceLabel, 0, Qt::AlignTop);

	verticalLayout_3->addLayout(horizontalLayout);

	retranslateUi(NSyncDialog);

	QMetaObject::connectSlotsByName(NSyncDialog);
  } // setupUi

  void retranslateUi(QDialog *NSyncDialog)
  {
	NSyncDialog->setWindowTitle(QApplication::translate("NSyncDialog", "NSync - Manage Projects", Q_NULLPTR));
	workspaceLabel->setText(QApplication::translate("NSyncDialog", "TextLabel", Q_NULLPTR));
  } // retranslateUi
};

namespace Ui {
  class NSyncDialog : public Ui_NSyncDialog {
  };
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NSYNCDIALOG_H
