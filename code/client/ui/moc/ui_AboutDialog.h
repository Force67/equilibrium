/********************************************************************************
** Form generated from reading UI file 'AboutDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUTDIALOG_H
#define UI_ABOUTDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AboutDialog
{
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
	QLabel *label;

	void setupUi(QDialog *AboutDialog)
	{
		if(AboutDialog->objectName().isEmpty())
			AboutDialog->setObjectName(QStringLiteral("AboutDialog"));
		AboutDialog->resize(546, 314);
		QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		sizePolicy.setHorizontalStretch(0);
		sizePolicy.setVerticalStretch(0);
		sizePolicy.setHeightForWidth(AboutDialog->sizePolicy().hasHeightForWidth());
		AboutDialog->setSizePolicy(sizePolicy);
		QIcon icon;
		icon.addFile(QStringLiteral(":/logo"), QSize(), QIcon::Normal, QIcon::Off);
		AboutDialog->setWindowIcon(icon);
		verticalLayout_3 = new QVBoxLayout(AboutDialog);
		verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
		verticalLayout_3->setContentsMargins(0, 0, 0, 0);
		headerFrame = new QWidget(AboutDialog);
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
		label = new QLabel(AboutDialog);
		label->setObjectName(QStringLiteral("label"));

		verticalLayout->addWidget(label);

		verticalLayout_3->addLayout(verticalLayout);

		retranslateUi(AboutDialog);

		QMetaObject::connectSlotsByName(AboutDialog);
	} // setupUi

	void retranslateUi(QDialog *AboutDialog)
	{
		AboutDialog->setWindowTitle(QApplication::translate("AboutDialog", "About NODA", Q_NULLPTR));
		labelLogo->setText(QString());
		labelTitle->setText(QApplication::translate("AboutDialog", "<html><head/><body><p><span style=\" font-weight:600;\">NODA - Supercharge your ida. </span><span style=\" font-family:'apple color emoji','segoe ui emoji','noto color emoji','android emoji','emojisymbols','emojione mozilla','twemoji mozilla','segoe ui symbol'; font-size:xx-small; color:#e8e7e3;\">\342\232\241</span></p></body></html>", Q_NULLPTR));
		labelFeatures->setText(QApplication::translate("AboutDialog", "<html><head/><body><p><span style=\" font-size:9pt;\">NODA uses the following libraries/components:</span></p><p>- <a href=\"https://github.com/google/flatbuffers\"><span style=\" text-decoration: underline; color:#0000ff;\">Google Flatbuffers</span></a> for packet serilization</p><p>- A custom fork of <a href=\"https://github.com/lsalzman/enet\"><span style=\" text-decoration: underline; color:#0000ff;\">ENet</span></a> for networking</p><p>- The <a href=\"https://www.hex-rays.com/\"><span style=\" text-decoration: underline; color:#0000ff;\">IDA Pro</span></a> SDK</p><p>- Icons by <a href=\"https://fontawesome.com/license\"><span style=\" text-decoration: underline; color:#0000ff;\">Font Awesome</span></a>, color customized<br/></p></body></html>", Q_NULLPTR));
		label->setText(QApplication::translate("AboutDialog", "<html><head/><body><p align=\"center\"><span style=\" font-weight:600;\">Copyright (C) NOMAD Group &lt;</span><a href=\"https://www.nomad-group.net/\"><span style=\" font-weight:600; text-decoration: underline; color:#0000ff;\">nomad-group.ne</span></a><span style=\" font-weight:600;\">t&gt;</span></p></body></html>", Q_NULLPTR));
	} // retranslateUi
};

namespace Ui
{
	class AboutDialog : public Ui_AboutDialog
	{
	};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUTDIALOG_H
