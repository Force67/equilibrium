// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "RunDialog.h"
#include "utils/Logger.h"

#include <QModelIndex>
#include <QModelIndexList>
#include <QAbstractItemModel>

namespace forms {
  RunDialog::RunDialog(QWidget *parent) :
      QDialog(parent)
  {
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

  void RunDialog::OnClickOK()
  {
	auto selected = optionList->selectionModel()->selectedIndexes();
	if(selected.count() > 1) {
	  LOG_TRACE("RunDialog::OnClickOK Multiple items were selected");
	  return;
	}

	const auto &item = selected.at(0);
	_index = item.row() + 1;

	QDialog::close();
  }

  features::FeatureIndex RunDialog::SelectedIndex() const
  {
	switch(_index) {
	case 1:
	  return features::FeatureIndex::DumbSig;
	default:
	  return features::FeatureIndex::None;
	}
  }

  void RunDialog::ResetSelection()
  {
	// pre select the 1st item
	optionList->setItemSelected(optionList->item(0), true);
  }
} // namespace forms