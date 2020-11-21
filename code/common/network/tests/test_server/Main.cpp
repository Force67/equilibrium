// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <thread>

#include <fmt/format.h>
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

  void ConsumeMessage(network::TCPPeer &source, const uint8_t *ptr, size_t size) override
  {
	fmt::print("Consume(): {}: {}\n", static_cast<const void *>(ptr), size);
  }

  void OnConnection(const network::TCPPeer &peer) override
  {
	fmt::print("OnConnection(): -> {}", peer.id);
  }

  void OnDisconnection(const network::TCPPeer &peer) override
  {
	fmt::print("OnDisconnection()");
  }

  bool ShouldRun() const
  {
	return _run;
  }

  void Tick()
  {
	_server.Tick();
  }

private:
  bool _run = true;
  network::TCPServer _server;
};

int main()
{
  network::ScopedSocket sockInit;

  fmt::print("Initializing test_server");
  TestServer2 server;

  while(server.ShouldRun()) {
	server.Tick();
	std::this_thread::sleep_for(1ms);
  }

  fmt::print("Exiting test_server");
  return 0;
}