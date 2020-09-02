// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#include "UiStatusBar.h"

void UiStatusBar::SetColor(const char* name) {
  std::string col = "QWidget { background-color: " + std::string(name) + "; }";
  setStyleSheet(col.c_str());
}

