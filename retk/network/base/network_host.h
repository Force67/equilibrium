// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <memory>
#include <sockpp/tcp6_acceptor.h>
#include <network/base/network_base.h>
#include <network/base/network_encoding.h>

#include <base/detached_queue.h>

namespace network {

class ServerDelegate {
 public:
  virtual ~ServerDelegate() = default;

  virtual void OnConnection(connectid_t){};
  virtual void OnDisconnection(connectid_t) = 0;
  virtual void ProcessData(connectid_t cid,
                           const uint8_t* data,
                           size_t len) = 0;
};

class ClientDelegate {
 public:
  virtual ~ClientDelegate() = default;

  // called when connection to dest succeeds
  virtual void OnConnection(const sockpp::inet_address&){};
  virtual void OnDisconnected(QuitReason) = 0;
  virtual void ProcessData(const uint8_t* ptr, size_t len) = 0;
};

class NetworkHost {
 public:
  explicit NetworkHost(sockpp::socket& ss) : sock_(ss) {}

  // returns the latest os socket error
  // as a string
  std::string LastError() const;
  const auto& Address() const { return address_; }

  static uint32_t GetSeed();
 private:
  sockpp::socket& sock_;

 protected:
  sockpp::inet_address address_{};
  uint8_t chunkbuf_[kChunkSize]{};
};
}  // namespace network