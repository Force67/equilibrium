// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <QThread>
#include "server/Server.h"

namespace noda {

  class LocalServer final : public QThread,
                            public noda::Server {
  public:
	LocalServer();
	~LocalServer();

	bool Start();
	void Stop();

	bool Active()
	{
	  return _run;
	}

  private:
	void run() override;
	bool _run = false;
  };
} // namespace noda