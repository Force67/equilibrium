// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
// This syncs the IDA database.

#pragma once

#include "net/NetClient.h"
#include "net/protocol/MsgList_generated.h"

namespace noda::sync
{
  // prototypes for message dispatch
  using apply_t = bool(void *, const protocol::MessageRoot &);
  using react_t = bool(net::NetClient &, va_list);

  apply_t *GetNetApplicant(uint32_t idx);
  react_t *GetReactor_IDB(uint32_t idx);
} // namespace noda::sync