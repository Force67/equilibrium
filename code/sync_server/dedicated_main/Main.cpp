// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <cstdio>
#include <Server.h>

int main(int argc, char **argv)
{
  std::puts("Initializing NODA");

  noda::Server server(argc, argv);
  auto result = server.Initialize();
  if (result != noda::Server::Status::Success) {
	std::printf("Failed to initialize the server instance (status: %d)\n", 
		static_cast<int>(result));
  }

  while (server.IsListening()) {
	server.Tick();
  }

  return 0;
}