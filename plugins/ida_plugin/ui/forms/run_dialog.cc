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
    ResetSelection();
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
  _index = item.row() + 1;

  QDialog::close();
}

tools::Toolbox::ActionCode RunDialog::SelectedIndex() const {
  using tb_t = tools::Toolbox::ActionCode;

  switch (_index) {
    case 1:
      return tb_t::kSignature;
    default:
      return tb_t::kNone;
  }
}

void RunDialog::ResetSelection() {
  // pre select the 1st item
  optionList->setItemSelected(optionList->item(0), true);
}
}  // namespace forms