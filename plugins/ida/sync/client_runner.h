// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <QThread>

namespace sync {
class SyncClient;
}

class ClientRunner final : public QThread {
 public:
  ClientRunner(sync::SyncClient&);
  ~ClientRunner();

  bool Start();
  void Stop();

  static bool OnNetThread();
 private:
  void run() override;

 private:
  sync::SyncClient& client_;
  bool running_ = false;
  int idletime_ = 0;
};