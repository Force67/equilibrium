// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "run_dialog.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QModelIndexList>

namespace forms {
RunDialog::RunDialog(QWidget* parent, UiData& data)
    : QDialog(parent), data_(data) {
  setupUi(this);
  // remember last selection
  SetSelectedItem(data.last_run_index);

  // disallow multiple selection
  optionList->setSelectionMode(
      QAbstractItemView::SelectionMode::SingleSelection);

  connect(btOk, &QPushButton::clicked, this, &RunDialog::OnClickOK);
  connect(btQuit, &QPushButton::clicked, this, &QDialog::reject);
}

void RunDialog::OnClickOK() {
  auto selected = optionList->selectionModel()->selectedIndexes();
  const auto& item = selected.at(0);
  data_.last_run_index = item.row() + 1;
  QDialog::accept();
}

void RunDialog::SetSelectedItem(int idx) {
  optionList->setItemSelected(optionList->item(idx), true);
}
}  // namespace forms