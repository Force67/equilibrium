// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <network/tcp_client.h>
#include <QThread>

class NetClient final : public network::TCPClient, public QThread {
 public:
  NetClient();
  ~NetClient();

  bool Start();
  void Stop();

 private:
  void run() override;

  int _idleSetting = 0;
  bool _runState = false;
  network::Context _ctx;

  static Qt::HANDLE s_Handle;
};

NetClient* GNetClient();
bool IsOnNetThread();