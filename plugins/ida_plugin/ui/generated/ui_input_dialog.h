/********************************************************************************
** Form generated from reading UI file 'input_dialog.ui'
**
** Created by: Qt User Interface Compiler version 5.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INPUT_DIALOG_H
#define UI_INPUT_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_InputDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QHBoxLayout *textBoxLayout;
    QLineEdit *lineEdit;
    QHBoxLayout *buttonLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *buttonOK;
    QPushButton *buttonCancel;
    QSpacerItem *horizontalSpacer_2;

    void setupUi(QDialog *InputDialog)
    {
        if (InputDialog->objectName().isEmpty())
            InputDialog->setObjectName(QStringLiteral("InputDialog"));
        InputDialog->resize(524, 126);
        QIcon icon;
        icon.addFile(QStringLiteral(":/logo"), QSize(), QIcon::Normal, QIcon::Off);
        InputDialog->setWindowIcon(icon);
        verticalLayout = new QVBoxLayout(InputDialog);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        label = new QLabel(InputDialog);
        label->setObjectName(QStringLiteral("label"));
        label->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(label);

        textBoxLayout = new QHBoxLayout();
        textBoxLayout->setObjectName(QStringLiteral("textBoxLayout"));
        lineEdit = new QLineEdit(InputDialog);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));

        textBoxLayout->addWidget(lineEdit);


        verticalLayout->addLayout(textBoxLayout);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setObjectName(QStringLiteral("buttonLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        buttonLayout->addItem(horizontalSpacer);

        buttonOK = new QPushButton(InputDialog);
        buttonOK->setObjectName(QStringLiteral("buttonOK"));

        buttonLayout->addWidget(buttonOK);

        buttonCancel = new QPushButton(InputDialog);
        buttonCancel->setObjectName(QStringLiteral("buttonCancel"));

        buttonLayout->addWidget(buttonCancel);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        buttonLayout->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(buttonLayout);


        retranslateUi(InputDialog);

        QMetaObject::connectSlotsByName(InputDialog);
    } // setupUi

    void retranslateUi(QDialog *InputDialog)
    {
        InputDialog->setWindowTitle(QApplication::translate("InputDialog", "Input Dialog", Q_NULLPTR));
        label->setText(QApplication::translate("InputDialog", "Confirm Input", Q_NULLPTR));
        buttonOK->setText(QApplication::translate("InputDialog", "OK", Q_NULLPTR));
        buttonCancel->setText(QApplication::translate("InputDialog", "Cancel", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class InputDialog: public Ui_InputDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INPUT_DIALOG_H
