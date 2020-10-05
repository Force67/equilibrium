// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ui/moc/ui_ProgressDialog.h"

namespace noda {
  class ProgressDialog final : public QDialog, public Ui::ProgressDialog {
	Q_OBJECT;

  public:
	explicit ProgressDialog(QWidget *parent,  const QString& title, const QString& desc);
	~ProgressDialog() = default;

	void SetProgress(int value, const QString &status);
  };
} // namespace noda