#pragma once

#include "net/protocol/IdaSync_generated.h"

namespace noda::sync
{
  using namespace protocol::sync;

  class SyncController;

  namespace NameAddr
  {
	bool Apply(SyncController &, const NameEa &);
	bool React(SyncController &, va_list);
  } // namespace NameAddr

} // namespace noda::sync