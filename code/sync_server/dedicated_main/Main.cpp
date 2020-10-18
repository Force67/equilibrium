// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <cstdio>
#include <Server.h>

#include <netlib/NetLib.h>

int main(int argc, char **argv)
{
  netlib::ScopedNetContext context;

  std::puts("Initializing NODA");

  noda::Server server(4523);
  auto result = server.Initialize(true);
  if(result != noda::Server::Status::Success) {
	std::printf("Failed to initialize the server instance (status: %d)\n", static_cast<int>(result));
  }

  while(server.IsListening()) {
	server.Tick();
  }

  return 0;
}