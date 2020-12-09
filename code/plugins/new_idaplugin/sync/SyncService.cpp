// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "SyncService.h"

SyncService::SyncService(SyncSession &session) :
    _session(session)
{
  BindTaskHandlers();
}

SyncService::~SyncService()
{
}

void SyncService::BindTaskHandlers()
{
}

bool SyncService::ProcessMessage(const protocol::MessageRoot *root)
{
  return true;
}