// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "NetClient.h"
#include "utils/Opt.h"

using namespace network::constants;

namespace {
  // Server Port Property
  utils::Opt<int> NetPort{ kServerPort, "Nd_NetPort" };

  // Server Ip/Address property
  utils::Opt<QString> NetIp{ kServerIp, "Nd_NetIp" };

  // How long the net thread should yield
  utils::Opt<int> NetIdle{ kNetworkerThreadIdle, "Nd_NetThreadIdle" };
} // namespace

static NetClient *s_NetClient{ nullptr };

NetClient *GNetClient()
{
  return s_NetClient;
}

NetClient::NetClient()
{
  s_NetClient = this;
}

NetClient::~NetClient()
{
  Stop();
  s_NetClient = nullptr;
}

bool NetClient::Start()
{
  const QString ip = NetIp;

  _runState = TCPClient::Connect(
      ip.toUtf8().data(), NetPort);

  if(_runState) {
	_idleSetting = NetIdle;

	QThread::start(QThread::HighPriority);
  }

  return _runState;
}

void NetClient::run()
{
  while(_runState) {
	// when we get disconnected, the tcpclient
	// updates the runstate to false
	_runState = TCPClient::Update();

	QThread::msleep(_idleSetting);
  }
}

void NetClient::Stop()
{
  TCPClient::Disconnect();

  _runState = false;

  // ensure thread is killed properly
  QThread::quit();
  QThread::wait();
}