// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
// This syncs the IDA database.

#include "Dispatcher.h"
#include "Handlers.h"

namespace noda::sync
{
	// this can react to processor and idb
  static apply_t *g_applicantRegistry[]{
	&sync_NameAddr::Apply,
  };

  static react_t *g_idbReact[]{
	&sync_NameAddr::React,
  };

  apply_t *GetNetApplicant(uint idx)
  {
	if(idx > (sizeof(g_applicantRegistry) / sizeof(void *)))
	  __debugbreak();

	return g_applicantRegistry[idx];
  }

  react_t *GetReactor_IDB(uint idx)
  {
	if(idx > (sizeof(g_idbReact) / sizeof(void *)))
	  __debugbreak();

	return g_idbReact[idx];
  }
} // namespace noda::sync