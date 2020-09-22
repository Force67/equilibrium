// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
// This syncs the IDA database.

#pragma once

#include "net/NetClient.h"
#include "net/protocol/Message_generated.h"

namespace noda::sync
{
  // (Not typesafe) prototypes for message dispatch
  using apply_t = bool(void *, const void *);
  using react_t = bool(void *, va_list);

  apply_t *GetNetApplicant(uint32_t index);
  react_t *GetReactor_IDB(uint32_t index, bool idp);
} // namespace noda::sync