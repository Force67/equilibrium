#pragma once
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"

#include <QObject>
#include <map>

class SyncSession;

namespace protocol {
struct MessageRoot;
}

namespace sync {
struct StaticHandler;
}

class SyncService : QObject, exec_request_t {
  Q_OBJECT;

 public:
  SyncService(SyncSession&);
  ~SyncService();

 private:

  void SendSessionInfo();

};

/*

network::FbsStringRef CreateFbStringRef(network::FbsBuffer& buffer,
                                        const QString& other) {
  const char* str = const_cast<const char*>(other.toUtf8().data());
  size_t sz = static_cast<size_t>(other.size());
  return buffer.CreateString(str, sz);
}

*/