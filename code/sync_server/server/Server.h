// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace noda {

class Server {
public:
  enum class Status {
	  Success,
	  NetError,
  };

  inline bool StatusOk(Status s) {
	return s == Status::Success;
  }

// ctor with command line options
  explicit Server(int argc, char**);

// default ctor
  Server();

  Status Initialize();


  void Tick();

  bool IsListening() const;

private:
  Status InitializeNetThread();

  bool _isListening = false;
};
}