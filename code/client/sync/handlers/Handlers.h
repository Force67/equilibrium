#pragma once
#include "Dispatcher.h"

#include "net/NetClient.h"
#include "net/protocol/MsgList_generated.h"

namespace noda::sync_NameAddr
{
  bool Apply(void *, const protocol::MessageRoot &);
  bool React(net::NetClient &, va_list);
} // namespace noda::sync_NameAddr