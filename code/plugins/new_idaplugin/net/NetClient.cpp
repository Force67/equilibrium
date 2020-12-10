// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "NetClient.h"
#include "utils/Opt.h"

using namespace network::constants;

utils::Opt<int> NetPort{ kServerPort, "Nd_NetPort" };
utils::Opt<int> NetIdle{ kNetworkerThreadIdle, "Nd_NetThreadIdle" };
utils::Opt<QString> NetIp{ kServerIp, "Nd_NetIp" };

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

  const bool result = TCPClient::Connect(
      ip.toUtf8().data(), NetPort);

  if(result) {
	_runState = true;
	_idleSetting = NetIdle;

	QThread::start(QThread::HighPriority);
  }

  return result;
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