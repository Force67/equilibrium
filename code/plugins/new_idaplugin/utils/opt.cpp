// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Opt.h"
#include "Pch.h"

#include <QSettings>
#include <QVariant>

namespace utils {

size_t OptRegistry::Load() {
  size_t total = 0;
  QSettings set;

  for (auto* i = Root(); i;) {
    OptBase* item = i->item;
    if (item && item->name) {
      if (!set.contains(item->name)) {
        set.setValue(item->name, item->data);
      } else {
        item->data = set.value(item->name);
      }

      ++total;
    }

    OptRegistry* j = i->next;
    // i->next = nullptr;
    i = j;
  }

  return total;
}

void OptRegistry::Save() {
  QSettings set;

  for (auto* i = Root(); i;) {
    OptBase* item = i->item;
    if (item && item->name) {
      set.setValue(item->name, item->data);
    }

    OptRegistry* j = i->next;
    i = j;
  }
}
}  // namespace utils