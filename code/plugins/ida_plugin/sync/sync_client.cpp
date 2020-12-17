// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "utils/Opt.h"
#include "sync/sync_client.h"

namespace {
// Server Port Property
utils::Opt<int> NetPort{network::kDefaultServerPort, "Nd_NetPort"};

// Server Ip/Address property
utils::Opt<QString> NetIp{network::kDefaultServerIp, "Nd_NetIp"};

// How long the net thread should yield
utils::Opt<int> NetIdle{1, "Nd_NetThreadIdle"};
}  // namespace

static NetClient* s_NetClient{nullptr};
static Qt::HANDLE s_netThreadId{nullptr};

bool IsOnNetThread() {
  return s_netThreadId == QThread::currentThreadId();
}

NetClient* GNetClient() {
  if (!s_NetClient)
    __debugbreak();

  return s_NetClient;
}

NetClient::NetClient() {
  s_NetClient = this;
}

NetClient::~NetClient() {
  if (_runState)
    Stop();

  s_NetClient = nullptr;
}

bool NetClient::Start() {
  const QString ip = NetIp;

  _runState = TCPClient::Connect(ip.toUtf8().data(), NetPort);

  if (_runState) {
    _idleSetting = NetIdle;

    QThread::start(QThread::HighPriority);
  }

  return _runState;
}

void NetClient::run() {
  s_netThreadId = QThread::currentThreadId();

  while (_runState) {
    // when we get disconnected, the tcpclient
    // updates the runstate to false
    _runState = TCPClient::Update();

    QThread::msleep(_idleSetting);
  }

  s_netThreadId = nullptr;
}

void NetClient::Stop() {
  TCPClient::Disconnect();

  _runState = false;

  // ensure thread is killed properly
  QThread::quit();
  QThread::wait();
}