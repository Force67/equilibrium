// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
// This syncs the IDA database.
#pragma once

namespace noda::sync
{
  class SyncController;

  class IdbSync
  {
  public:
	IdbSync(SyncController &);

  private:
	SyncController &_controller;
  };
}
