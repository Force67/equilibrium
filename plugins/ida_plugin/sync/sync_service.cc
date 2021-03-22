// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"

#include "sync/sync_service.h"
#include "sync/sync_handler.h"
#include "sync/sync_session.h"

SyncService::SyncService(SyncSession& session) : _session(session) {
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