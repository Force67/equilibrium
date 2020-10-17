// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <netlib/ServerBase.h>
#include "Workspace.h"

namespace noda {

  class Server final : public netlib::NetServerBase {
  public:
	enum class Status {
	  Success,
	  NetError,
	};

	inline bool StatusOk(Status s)
	{
	  return s == Status::Success;
	}

	// ctor with command line options
	explicit Server(int argc, char **);

	// default ctor
	Server();

	Status Initialize(bool enableStorage);

	void Tick();

	bool IsListening() const;

	virtual bool OnConnection(ENetPeer *);
	virtual bool OnDisconnection(ENetPeer *);
	virtual void OnConsume(ENetPeer *, const uint8_t *data, const size_t len);
  private:
	Status InitializeNetThread();

	bool _isListening = false;
	workspace_t _workspace;
  };
} // namespace noda