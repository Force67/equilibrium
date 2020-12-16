// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "SyncService.h"
#include "Pch.h"
#include "SyncHandler.h"
#include "SyncSession.h"

SyncService::SyncService(SyncSession& session) : _session(session) {
  BindStaticHandlers();

  hook_to_notification_point(hook_type_t::HT_IDB, IdbEvent, this);
  hook_to_notification_point(hook_type_t::HT_IDP, IdpEvent, this);

  connect(
      &session, &SyncSession::TransportStateChange, this,
      [&](SyncSession::TransportState newState) {
        if (newState == SyncSession::TransportState::ACTIVE) {
          SendSessionInfo();
          _active = true;
        }
        if (newState == SyncSession::TransportState::DISABLED) {
          _active = false;
        }
      },
      Qt::QueuedConnection);
}

SyncService::~SyncService() {
  unhook_from_notification_point(hook_type_t::HT_IDB, IdbEvent, this);
  unhook_from_notification_point(hook_type_t::HT_IDP, IdpEvent, this);
}

ssize_t SyncService::IdbEvent(void* ptr, int code, va_list args) {
  reinterpret_cast<SyncService*>(ptr)->HandleIDAEvent(hook_type_t::HT_IDB, code,
                                                      args);

  return 0;
}

ssize_t SyncService::IdpEvent(void* ptr, int code, va_list args) {
  reinterpret_cast<SyncService*>(ptr)->HandleIDAEvent(hook_type_t::HT_IDP, code,
                                                      args);

  return 0;
}

const SyncStats& SyncService::Stats() const {
  return _stats;
}

void SyncService::BindStaticHandlers() {
  size_t count = 0;

  for (auto* i = sync::StaticHandler::ROOT(); i;) {
    if (auto* it = i->item) {
      _idaEvents[std::make_pair(it->hookType, it->hookEvent)] = it;
      _netEvents[it->msgType] = it;
    }

    auto* j = i->next;
    i->next = nullptr;
    i = j;

    ++count;
  }

  LOG_TRACE("BindStaticHandlers() -> count: {}", count);
}

void SyncService::SendSessionInfo() {
  assert(!IsOnNetThread());

  uchar md5[16]{};
  bool result = retrieve_input_file_md5(md5);
  assert(result != false);

  char md5Str[32 + 1]{};

  // convert bytes to str
  constexpr char lookup[] = "0123456789abcdef";
  for (int i = 0; i < 16; i++) {
    md5Str[i * 2] = lookup[(md5[i]) >> 4 & 0xF];
    md5Str[i * 2 + 1] = lookup[(md5[i]) & 0xF];
  }

  char fileName[128]{};
  get_root_filename(fileName, sizeof(fileName) - 1);

  LOG_TRACE("SendSessionInfo() -> md5: {} fileName: {}", md5Str, fileName);

  network::FbsBuffer buffer;
  auto request = protocol::CreateLocalProjectInfoDirect(
      buffer, md5Str, fileName, _session.Store().LocalVersion());

  GNetClient()->SendPacket(protocol::MsgType_LocalProjectInfo, buffer,
                           request.Union());
}

/*
        Executes the queued net messages on the IDA thread in batches.
*/
int SyncService::execute() {
  return 0;
}

void SyncService::HandleIDAEvent(hook_type_t type, int code, va_list args) {
  if (!_active)
    return;

  const auto it = _idaEvents.find(std::make_pair(type, code));

  if (it == _idaEvents.end()) {
    // LOG_TRACE("Unhandeled IDB event! {}", code);
    return;
  }

  const auto& handler = it->second->delegates;
  const bool result = handler.react(*this, args);

  if (!result)
    LOG_ERROR("Failed to execute handler for {}", code);
  else {
    LOG_TRACE("Executed handler for {}", code);
    _stats.idaCount++;
  }
}

bool SyncService::ProcessNetMessage(const protocol::MessageRoot* root) {
  // root->s

  return true;
}