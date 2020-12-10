// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <QThread>
#include <network/TCPClient.h>

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
};

NetClient *GNetClient();