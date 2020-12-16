// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <memory>

namespace sync_server {

enum class LogLevel {
  kTrace,    // < Debug Only
  kInfo,     // < Regular
  kWarning,  // < Minor Error
  kError     // < Fatal Error
};

using logcallback_t = void (*)(LogLevel, const char*);

class ServerImpl;

class Server {
 public:
  explicit Server(int16_t port = 0);
  ~Server();

  enum class ResultStatus {
    kSuccess,
    kErrorNotInitalized,
    kErrorNetwork,
    kErrorStorage,
  };

  inline bool StatusOk(ResultStatus s) const {
    return s == ResultStatus::kSuccess;
  }

  ResultStatus Initialize(bool useStorage);

  // updates the main server thread
  void Update();

  bool IsListening() const;
  int16_t GetPort() const;

  // Sets a log callback
  static void SetLogCallback(logcallback_t);

 private:
  std::unique_ptr<ServerImpl> _impl;
};
}  // namespace sync_server