/********************************************************************************
** Form generated from reading UI file 'WelcomeDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WELCOMEDIALOG_H
#define UI_WELCOMEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_WelcomeDialog {
public:
  QVBoxLayout *verticalLayout_3;
  QWidget *headerFrame;
  QVBoxLayout *verticalLayout_5;
  QHBoxLayout *horizontalLayout_2;
  QWidget *widget_2;
  QVBoxLayout *verticalLayout_4;
  QLabel *labelLogo;
  QSpacerItem *verticalSpacer;
  QVBoxLayout *verticalLayout_2;
  QLabel *labelTitle;
  QSpacerItem *horizontalSpacer_3;
  QLabel *labelFeatures;
  QVBoxLayout *verticalLayout;
  QLabel *label_5;
  QHBoxLayout *horizontalLayout;
  QSpacerItem *horizontalSpacer;
  QPushButton *buttonContinue;
  QCheckBox *buttonDontShowAgain;
  QSpacerItem *horizontalSpacer_2;

  void setupUi(QDialog *WelcomeDialog)
  {
	if(WelcomeDialog->objectName().isEmpty())
	  WelcomeDialog->setObjectName(QStringLiteral("WelcomeDialog"));
	WelcomeDialog->resize(546, 425);
	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(WelcomeDialog->sizePolicy().hasHeightForWidth());
	WelcomeDialog->setSizePolicy(sizePolicy);
	QIcon icon;
	icon.addFile(QStringLiteral(":/logo"), QSize(), QIcon::Normal, QIcon::Off);
	WelcomeDialog->setWindowIcon(icon);
	verticalLayout_3 = new QVBoxLayout(WelcomeDialog);
	verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
	verticalLayout_3->setContentsMargins(0, 0, 0, 0);
	headerFrame = new QWidget(WelcomeDialog);
	headerFrame->setObjectName(QStringLiteral("headerFrame"));
	QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Maximum);
	sizePolicy1.setHorizontalStretch(0);
	sizePolicy1.setVerticalStretch(0);
	sizePolicy1.setHeightForWidth(headerFrame->sizePolicy().hasHeightForWidth());
	headerFrame->setSizePolicy(sizePolicy1);
	verticalLayout_5 = new QVBoxLayout(headerFrame);
	verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
	horizontalLayout_2 = new QHBoxLayout();
	horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
	horizontalLayout_2->setSizeConstraint(QLayout::SetDefaultConstraint);
	widget_2 = new QWidget(headerFrame);
	widget_2->setObjectName(QStringLiteral("widget_2"));
	verticalLayout_4 = new QVBoxLayout(widget_2);
	verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
	verticalLayout_4->setContentsMargins(0, 0, 0, 0);
	labelLogo = new QLabel(widget_2);
	labelLogo->setObjectName(QStringLiteral("labelLogo"));
	sizePolicy1.setHeightForWidth(labelLogo->sizePolicy().hasHeightForWidth());
	labelLogo->setSizePolicy(sizePolicy1);
	labelLogo->setMinimumSize(QSize(50, 50));
	labelLogo->setMaximumSize(QSize(50, 50));
	QFont font;
	font.setPointSize(8);
	labelLogo->setFont(font);
	labelLogo->setPixmap(QPixmap(QString::fromUtf8(":/logo")));
	labelLogo->setScaledContents(true);
	labelLogo->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignTop);
	labelLogo->setMargin(0);

	verticalLayout_4->addWidget(labelLogo);

	verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

	verticalLayout_4->addItem(verticalSpacer);

	horizontalLayout_2->addWidget(widget_2);

	verticalLayout_2 = new QVBoxLayout();
	verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
	labelTitle = new QLabel(headerFrame);
	labelTitle->setObjectName(QStringLiteral("labelTitle"));
	QFont font1;
	font1.setFamily(QStringLiteral("Arial"));
	font1.setPointSize(14);
	font1.setBold(false);
	font1.setWeight(50);
	labelTitle->setFont(font1);
	labelTitle->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);

	verticalLayout_2->addWidget(labelTitle);

	horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	verticalLayout_2->addItem(horizontalSpacer_3);

	labelFeatures = new QLabel(headerFrame);
	labelFeatures->setObjectName(QStringLiteral("labelFeatures"));
	QFont font2;
	font2.setFamily(QStringLiteral("Arial"));
	font2.setKerning(true);
	labelFeatures->setFont(font2);
	labelFeatures->setTextFormat(Qt::RichText);
	labelFeatures->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignTop);
	labelFeatures->setWordWrap(false);
	labelFeatures->setOpenExternalLinks(true);

	verticalLayout_2->addWidget(labelFeatures);

	horizontalLayout_2->addLayout(verticalLayout_2);

	verticalLayout_5->addLayout(horizontalLayout_2);

	verticalLayout_3->addWidget(headerFrame);

	verticalLayout = new QVBoxLayout();
	verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
	verticalLayout->setContentsMargins(11, 11, 11, 11);
	label_5 = new QLabel(WelcomeDialog);
	label_5->setObjectName(QStringLiteral("label_5"));
	QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::Maximum);
	sizePolicy2.setHorizontalStretch(0);
	sizePolicy2.setVerticalStretch(0);
	sizePolicy2.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
	label_5->setSizePolicy(sizePolicy2);
	QFont font3;
	font3.setKerning(true);
	label_5->setFont(font3);
	label_5->setTextFormat(Qt::RichText);
	label_5->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
	label_5->setWordWrap(true);
	label_5->setOpenExternalLinks(true);

	verticalLayout->addWidget(label_5);

	horizontalLayout = new QHBoxLayout();
	horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
	horizontalLayout->setContentsMargins(0, 0, 0, 0);
	horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	horizontalLayout->addItem(horizontalSpacer);

	buttonContinue = new QPushButton(WelcomeDialog);
	buttonContinue->setObjectName(QStringLiteral("buttonContinue"));
	buttonContinue->setAutoDefault(true);

	horizontalLayout->addWidget(buttonContinue);

	buttonDontShowAgain = new QCheckBox(WelcomeDialog);
	buttonDontShowAgain->setObjectName(QStringLiteral("buttonDontShowAgain"));

	horizontalLayout->addWidget(buttonDontShowAgain, 0, Qt::AlignLeft);

	horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	horizontalLayout->addItem(horizontalSpacer_2);

	verticalLayout->addLayout(horizontalLayout);

	verticalLayout_3->addLayout(verticalLayout);

	retranslateUi(WelcomeDialog);

	QMetaObject::connectSlotsByName(WelcomeDialog);
  } // setupUi

  void retranslateUi(QDialog *WelcomeDialog)
  {
	WelcomeDialog->setWindowTitle(QApplication::translate("WelcomeDialog", "Welcome to NODA", Q_NULLPTR));
	labelLogo->setText(QString());
	labelTitle->setText(QApplication::translate("WelcomeDialog", "<html><head/><body><p><span style=\" font-weight:600;\">NODA - Supercharge your ida. </span><span style=\" font-family:'apple color emoji','segoe ui emoji','noto color emoji','android emoji','emojisymbols','emojione mozilla','twemoji mozilla','segoe ui symbol'; font-size:xx-small; color:#e8e7e3;\">\342\232\241</span></p></body></html>", Q_NULLPTR));
	labelFeatures->setText(QApplication::translate("WelcomeDialog", "<html><head/><body><p>Noda comes with batteries included. Features include:</p><p>- IDA Sync: Synchronize your database</p><p>- PDBTool: Generate fake PDB's directory from IDA</p><p>- PatternTool: Generate your pattern cache directory from IDA</p><p>- more planned <span style=\" font-family:'apple color emoji','segoe ui emoji','noto color emoji','android emoji','emojisymbols','emojione mozilla','twemoji mozilla','segoe ui symbol'; color:#e8e7e3;\">\342\235\244\357\270\217</span></p><p><br/></p></body></html>", Q_NULLPTR));
	label_5->setText(QApplication::translate("WelcomeDialog", "<html><head/><body><p>Please refer to the <a href=\"https://github.com/NomadGroup/NODA/tree/devel\"><span style=\" text-decoration: underline; color:#0000ff;\">Quickstart</span></a> guide for plugin documentation.</p></body></html>", Q_NULLPTR));
	buttonContinue->setText(QApplication::translate("WelcomeDialog", "Continue", Q_NULLPTR));
	buttonDontShowAgain->setText(QApplication::translate("WelcomeDialog", "Do not show again", Q_NULLPTR));
  } // retranslateUi
};

namespace Ui {
  class WelcomeDialog : public Ui_WelcomeDialog {
  };
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WELCOMEDIALOG_H
