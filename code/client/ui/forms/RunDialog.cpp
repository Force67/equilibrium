// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "RunDialog.h"

namespace noda::ui {
  RunDialog::RunDialog(QWidget *parent) :
      QDialog(parent)
  {
	setupUi(this);
  }
}