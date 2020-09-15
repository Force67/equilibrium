// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
// This syncs IDA Processor events.
#pragma once

namespace noda::sync
{
  class SyncController;

  class ProcessorSync
  {
  public:
	ProcessorSync(SyncController &);

  private:
	SyncController &_controller;
  };
}