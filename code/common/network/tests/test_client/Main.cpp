// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <thread>

#include <TCPClient.h>

// stupid windows
#undef GetMessage
#undef GetMessageW

#include "flatbuffers/flatbuffers.h"

#include "protocol/generated/Message_generated.h"
#include "protocol/generated/Handshake_generated.h"
#include "protocol/generated/IdaSync_generated.h"

#include "utility/Thread.h"

using namespace std::chrono_literals;

class TestClient2 final : public network::TCPClientConsumer {
public:
  explicit TestClient2() :
      _client(*this)
  {
	_run = _client.Connect(nullptr, network::constants::kServerPort);

	if(!_run) {
	  std::printf("Oh no! connect() failed with '%s'\n", _client.LastError().c_str());
	}
  }

  ~TestClient2()
  {
	if(_run)
	  _client.Disconnect();
  }

  void ConsumeMessage() override
  {
  }

  void OnDisconnect(int reason) override
  {
  }

  bool ShouldRun() const
  {
	return _run;
  }

  void Tick()
  {
	_client.Tick();

	std::puts("Tick3!");
  }

private:
  bool _run;
  network::TCPClient _client;
};

int main()
{
  network::ScopedSocket sockInit;

  std::puts("Initializing test_client");
  TestClient2 client;

  while(client.ShouldRun()) {
	client.Tick();
	std::this_thread::sleep_for(1ms);
  }

  std::puts("Exiting test_client");
  return 0;
}