// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <thread>

#include <TCPServer.h>

// stupid windows
#undef GetMessage
#undef GetMessageW

#include "flatbuffers/flatbuffers.h"

#include "protocol/generated/Message_generated.h"
#include "protocol/generated/Handshake_generated.h"
#include "protocol/generated/IdaSync_generated.h"

#include "utility/Thread.h"

using namespace std::chrono_literals;

class TestServer2 final : public network::TCPServerConsumer {
public:
  explicit TestServer2() :
      _server(*this)
  {
	_run = _server.Host(network::constants::kServerPort);
  }

  ~TestServer2()
  {
  }

  void ConsumeMessage() override
  {
  }

  bool ShouldRun() const
  {
	return _run;
  }

  void Tick()
  {
	_server.Tick();

	std::puts("Tick3!");
  }

private:
  bool _run = true;
  network::TCPServer _server;
};

int main()
{
  network::ScopedSocket sockInit;

  std::puts("Initializing test_server");
  TestServer2 server;

  while(server.ShouldRun()) {
	server.Tick();
	std::this_thread::sleep_for(1ms);
  }

  std::puts("Exiting test_server");
  return 0;
}