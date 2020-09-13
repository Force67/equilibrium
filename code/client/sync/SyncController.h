// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "IdaInc.h"
#include <qobject.h>

namespace noda
{
  class SyncClient;

  // syncdispatcher?
  class SyncController final : public QObject
  {
	Q_OBJECT;

  public:
	SyncController(SyncClient &);
	~SyncController();

  private:
	static ssize_t idaapi OnIdaEvent(void *, int, va_list);

	SyncClient &_client;
  };
}