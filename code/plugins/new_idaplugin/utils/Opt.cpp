// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "Opt.h"

#include <QVariant>
#include <QSettings>

namespace utils {

  size_t OptRegistry::Init()
  {
	size_t total = 0;
	QSettings set;

	for(auto *i = Root(); i;) {
	  OptBase *item = i->item;
	  if(item && item->name) {
		if(!set.contains(item->name)) {
		  set.setValue(item->name, item->data);
		}
		else {
		  item->data = set.value(item->name);
		}

		++total;
	  }

	  OptRegistry *j = i->next;
	  i->next = nullptr;
	  i = j;
	}

	return total;
  }
} // namespace utils