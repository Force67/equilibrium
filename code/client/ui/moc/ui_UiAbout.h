/********************************************************************************
** Form generated from reading UI file 'UiAbout.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UIABOUT_H
#define UI_UIABOUT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_AboutDialog {
  public:
  QGridLayout *gridLayout;
  QVBoxLayout *textLayout;
  QHBoxLayout *horizontalLayout;
  QLabel *labelTitle;
  QLabel *labelIcon;
  QLabel *labelDesc;

  void setupUi(QDialog *AboutDialog) {
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
	labelTitle = new QLabel(AboutDialog);
	labelTitle->setObjectName(QStringLiteral("labelTitle"));
	labelTitle->setTextFormat(Qt::RichText);

	horizontalLayout->addWidget(labelTitle);

	labelIcon = new QLabel(AboutDialog);
	labelIcon->setObjectName(QStringLiteral("labelIcon"));
	labelIcon->setPixmap(QPixmap(QString::fromUtf8(":/logo")));
	labelIcon->setScaledContents(false);
	labelIcon->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);

	horizontalLayout->addWidget(labelIcon);

	textLayout->addLayout(horizontalLayout);

	gridLayout->addLayout(textLayout, 0, 0, 1, 1);

	labelDesc = new QLabel(AboutDialog);
	labelDesc->setObjectName(QStringLiteral("labelDesc"));

	gridLayout->addWidget(labelDesc, 1, 0, 1, 1);

	retranslateUi(AboutDialog);

	QMetaObject::connectSlotsByName(AboutDialog);
  } // setupUi

  void retranslateUi(QDialog *AboutDialog) {
	AboutDialog->setWindowTitle(QApplication::translate("AboutDialog", "About NODA", Q_NULLPTR));
#ifndef QT_NO_STATUSTIP
	labelTitle->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
	labelTitle->setWhatsThis(QString());
#endif // QT_NO_WHATSTHIS
	labelTitle->setText(QApplication::translate("AboutDialog", "<html><head/><body><p align=\"center\"><span style=\" font-size:9pt; font-weight:600;\">Noda - Supercharge your IDA</span></p></body></html>", Q_NULLPTR));
	labelIcon->setText(QString());
	labelDesc->setText(QApplication::translate("AboutDialog", "<html><head/><body><p align=\"center\"><span style=\" font-size:11pt; font-weight:600; vertical-align:super;\">Developed by NOMAD Group&lt;nomad-group.net&gt;</span></p><p>Uses flatbuffers by google</p><p><br/></p><p><br/></p></body></html>", Q_NULLPTR));
  } // retranslateUi
};

namespace Ui {
  class AboutDialog : public Ui_AboutDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_UIABOUT_H
