// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <sockpp/tcp_connector.h>
#include <network/util/chrono_helper.h>

namespace network {

struct PeerBase {
  using Id = uint32_t;
  using Adress = sockpp::inet_address;
  using Socket = sockpp::stream_socket;

  // reserved connection id's that are guaranteed to never
  // be randomly assigned.
  static constexpr uint32_t kInvalidConnectId = UINT_MAX;
  static constexpr uint32_t kAllConnectId = UINT_MAX - 1;

  // when the peer will be removed
  static constexpr auto const kPeerTimeout = 2s;

  explicit PeerBase(Socket, Id, Adress);

  // lifetime management
  void Kill();
  void Touch();
  bool IsDead();

  Id id;
  Adress address;
  Socket sock;

  // if the socket can die.
  // TODO: flags?
  bool immortal;

  // when was the peer last touched
  util::MilliSeconds last_seen{};
};
}  // namespace network