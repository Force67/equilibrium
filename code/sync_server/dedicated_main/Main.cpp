// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <cstdio>
#include <thread>
#include <Server.h>

#include <netlib/NetLib.h>

void InitializeNetThread(noda::Server &server)
{
  std::thread([&] {
	while(true) {
	  server.ProcessNet();
	}
  }).detach();

  std::thread([&] {
	while(true) {
	  server.ProcessData();
	}
  }).detach();
}

int main(int argc, char **argv)
{
  netlib::ScopedNetContext context;

  std::puts("Initializing NODA");

  noda::Server server(4523);
  auto result = server.Initialize(true);
  if(result != noda::Server::Status::Success) {
	std::printf("Failed to initialize the server instance (status: %d)\n", static_cast<int>(result));
  }

  InitializeNetThread(server);

  while(true) {
	if(getchar())
	  break;
  }

  return 0;
}