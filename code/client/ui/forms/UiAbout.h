// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#pragma once

#include "ui/moc/ui_UiAbout.h"

class UiAbout final : public QDialog, public Ui::AboutDialog {
  Q_OBJECT;

  public:
  UiAbout(QWidget *parent);
  ~UiAbout() = default;
};