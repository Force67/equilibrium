// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "SyncService.h"
#include "SyncHandler.h"

SyncService::SyncService(SyncSession &session) :
    _session(session)
{
  BindStaticHandlers();

  hook_to_notification_point(hook_type_t::HT_IDB, IdbEvent, this);
  hook_to_notification_point(hook_type_t::HT_IDP, IdpEvent, this);
}

SyncService::~SyncService()
{
  unhook_from_notification_point(hook_type_t::HT_IDB, IdbEvent, this);
  unhook_from_notification_point(hook_type_t::HT_IDP, IdpEvent, this);
}

ssize_t SyncService::IdbEvent(void *ptr, int code, va_list args)
{
  reinterpret_cast<SyncService *>(ptr)->
	  HandleIDAEvent(hook_type_t::HT_IDB, code, args);

  return 0;
}

ssize_t SyncService::IdpEvent(void *ptr, int code, va_list args)
{
  reinterpret_cast<SyncService *>(ptr)->
	  HandleIDAEvent(hook_type_t::HT_IDP, code, args);

  return 0;
}

const SyncStats& SyncService::Stats() const {
  return _stats;
}

void SyncService::BindStaticHandlers()
{
  size_t count = 0;

  for(auto *i = sync::StaticHandler::ROOT(); i;) {
	if(auto *it = i->item) {
	  _idaEvents[std::make_pair(it->hookType, it->hookEvent)] = it;
	  _netEvents[it->msgType] = it;
	}

	auto *j = i->next;
	i->next = nullptr;
	i = j;

	++count;
  }

  LOG_TRACE("BindStaticHandlers() -> count: {}", count);
}

/*
	Executes the queued net messages on the IDA thread in batches.
*/
int SyncService::execute()
{
  return 0;
}

void SyncService::HandleIDAEvent(hook_type_t type, int code, va_list args)
{
  const auto it = _idaEvents.find(std::make_pair(type, code));

  if (it == _idaEvents.end()) {
	//LOG_TRACE("Unhandeled IDB event! {}", code);
	return;
  }

  const auto &handler = it->second->delegates;
  const bool result = handler.react(*this, args);

  if(!result)
	LOG_ERROR("Failed to execute handler for {}", code);
  else {
	LOG_TRACE("Executed handler for {}", code);
	_stats.idaCount++;
  }
}

bool SyncService::ProcessNetMessage(const protocol::MessageRoot *root)
{



  return true;
}