// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#pragma once

#include "IdaInc.h"
#include <qobject.h>

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