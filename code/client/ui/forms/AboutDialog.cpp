// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "AboutDialog.h"

namespace noda::ui {
  AboutDialog::AboutDialog(QWidget *pParent) :
      QDialog(pParent)
  {
	setupUi(this);
  }
} // namespace noda::ui