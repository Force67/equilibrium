// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
// This syncs the IDA database.

#include "Dispatcher.h"
#include "Handlers.h"

namespace noda::sync
{
  // apply state to client
  static void *g_applicantRegistry[]{
	&NameAddr::Apply,
  };

  // list -> index -> nullptr if not handeled...
  static void *g_idbReact[]{
	&NameAddr::React,
  };

  static void *g_idpReact[]{
	nullptr,
  };

  apply_t *GetNetApplicant(uint index)
  {
	if(index > (sizeof(g_applicantRegistry) / sizeof(void *)))
	  __debugbreak();

	return reinterpret_cast<apply_t *>(g_applicantRegistry[index]);
  }

  react_t *GetReactor_IDB(uint idx, bool idp)
  {
	if(idx > (sizeof(g_idbReact) / sizeof(void *)))
	  __debugbreak();

	return reinterpret_cast<react_t *>(g_idbReact[idx]);
  }
} // namespace noda::sync