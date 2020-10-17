// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <netlib/NetLib.h>
#include "moc_protocol/Message_generated.h"

#include "Server.h"

#ifdef _WIN32
#undef GetMessage
#endif

namespace noda {

  Server::Server(int argc, char **) :
      NetServerBase(netlib::constants::kServerPort)
  {
  }

  Server::Server() :
      NetServerBase(netlib::constants::kServerPort)
  {
  }

  Server::Status Server::Initialize(bool enabledStorage)
  {
	if(enabledStorage) {
	  MountProjects(_workspace);
	}

	return Status::Success;
  }

  Server::Status Server::InitializeNetThread()
  {
	return Status::Success;
  }

  bool Server::OnConnection(ENetPeer *peer)
  {
	return false;
  }

  bool Server::OnDisconnection(ENetPeer *)
  {
	return false;
  }

  void Server::OnConsume(ENetPeer *source, const uint8_t *data, const size_t len)
  {
	flatbuffers::Verifier verifier(data, len);
	if(!protocol::VerifyMessageBuffer(verifier))
	  return;

	const auto *message = protocol::GetMessage(static_cast<const void *>(data));
	if (message->msg_type() == protocol::MsgType_HandshakeRequest) {
	  auto *packet = message->msg_as_HandshakeRequest();

	  if (packet->protocolVersion() < netlib::constants::kClientVersion) {
		  // kick...
	  }



	}

	if (message->msg_type() == protocol::MsgType_LocalProjectInfo) {
		// workspace append..

	}

  }

  void Server::Tick()
  {
  }

  bool Server::IsListening() const
  {
	return _isListening;
  }
} // namespace noda