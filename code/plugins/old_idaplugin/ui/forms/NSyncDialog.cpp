// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "NSyncDialog.h"
#include <QStandardItemModel>
#include "Pch.h"

namespace noda {
NSyncDialog::NSyncDialog(QWidget* parent) {
  setupUi(this);
  // treeView->setHeader(new QT::QHeaderView())

  auto* treeModel = new QStandardItemModel();
  auto* rootNode = treeModel->invisibleRootItem();

  auto* t1 = new QStandardItem("America");
  auto* t2 = new QStandardItem("CALI");

  t1->appendRow(t2);

  rootNode->appendRow(t1);
  treeView->setModel(treeModel);
  treeView->expandAll();
}

}  // namespace noda