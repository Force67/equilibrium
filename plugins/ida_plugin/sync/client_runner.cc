// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "client_runner.h"
#include "utils/Opt.h"

#include <sync/engine/sync_client.h>

namespace {
utils::Opt<int> NetPort{network::kDefaultServerPort, "Nd_NetPort"};
utils::Opt<QString> NetIp{network::kDefaultServerIp, "Nd_NetIp"};
utils::Opt<int> NetIdle{1, "Nd_NetThreadIdle"};
}

static Qt::HANDLE s_Tid{nullptr};

ClientRunner::ClientRunner(sync::SyncClient& client) : client_(client) {
}

ClientRunner::~ClientRunner() {
  if (running_)
    Stop();
}

bool ClientRunner::Start() {
  const QString ip = NetIp;

  running_ = client_.Connect(ip.toUtf8().data(), NetPort);

  if (running_) {
    idletime_ = NetIdle;

    QThread::start(QThread::HighPriority);
  }

  return running_;
}

void ClientRunner::Stop() {
  client_.Disconnect();

  running_ = false;

  // ensure thread is killed properly
  QThread::quit();
  QThread::wait();

  LOG_TRACE("ClientRunner::Stop");
}

void ClientRunner::run() {
  s_Tid = QThread::currentThreadId();

  while (running_) {
    running_ = client_.Process();
    QThread::msleep(idletime_);
  }

  LOG_TRACE("ClientRunner thread quit!");
  s_Tid = nullptr;
}

bool ClientRunner::OnNetThread() {
  return s_Tid == QThread::currentThreadId();
}