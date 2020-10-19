// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <memory>

namespace noda {

  class Server {
  public:
	enum class Status {
	  Success,
	  NetError,
	  FsError,
	};

	inline bool StatusOk(Status s) const
	{
	  return s == Status::Success;
	}

	Server(uint16_t port);
	~Server();

	Status Initialize(bool enableStorage);

	bool IsListening() const;

	void ProcessNet();
	void ProcessData();

  private:
	class Impl;
	std::unique_ptr<Impl> _impl;
  };
} // namespace noda