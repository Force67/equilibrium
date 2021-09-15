// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "run_dialog.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QModelIndexList>

namespace forms {
RunDialog::RunDialog(QWidget* parent) : QDialog(parent) {
  setupUi(this);
  ResetSelection();

  // disallow multiple selection
  optionList->setSelectionMode(
      QAbstractItemView::SelectionMode::SingleSelection);

  connect(btOk, &QPushButton::clicked, this, &RunDialog::OnClickOK);
  connect(btQuit, &QPushButton::clicked, [&]() {
    // Not resetting the selection is a feature now.
    //ResetSelection();
    QDialog::close();
  });
}

void RunDialog::OnClickOK() {
  auto selected = optionList->selectionModel()->selectedIndexes();
  if (selected.count() > 1) {
    LOG_TRACE("RunDialog::OnClickOK Multiple items were selected");
    return;
  }

  const auto& item = selected.at(0);
  current_index_ = static_cast<Index>(item.row() + 1);

  QDialog::close();
}

RunDialog::Index RunDialog::SelectedIndex() const {
  return current_index_;
}

void RunDialog::ResetSelection() {
  // pre select the 1st item
  optionList->setItemSelected(optionList->item(0), true);
}
}  // namespace forms