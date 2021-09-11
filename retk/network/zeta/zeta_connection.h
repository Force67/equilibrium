// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

// The entity that handles messages for a Zeta client or server
// On the client side, the Connection handles raw reads & processing.
#pragma once

#include <sockpp/inet_address.h>
#include <base/peer_base.h>
#include <network/zeta/zeta_protocol.h>

namespace network {

class ZetaPacketWriter;

class ZetaConnection final : public PeerBase {
 public:
  explicit ZetaConnection(sockpp::inet_address, ZetaPacketWriter* writer);
  ~ZetaConnection();

  void WriteDatagram(const uint8_t* ptr, size_t length);

  void CloseConnection();

  void ProcessCommand(FrameType, const uint8_t* data, size_t length);

  bool connected() const { return connected_; }

private:
	template<typename T>
	void DoSubmitDatagram(const T& value) {
		writer_->WritePacket(*this, &value, sizeof(T));
	}

 private:
  bool connected_ = false;
  sockpp::inet_address address_;
  // unowned
  ZetaPacketWriter* writer_;
};
}