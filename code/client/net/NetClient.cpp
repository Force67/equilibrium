// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <qsettings.h>
#include "NetClient.h"

namespace noda::net
{
  FbsStringRef MakeFbStringRef(FbsBuilder &msg, const QString &other)
  {
	const char *str = const_cast<const char *>(other.toUtf8().data());
	size_t sz = static_cast<size_t>(other.size());
	return msg.CreateString(str, sz);
  }

  NetClient::NetClient(NetDelegate &handler) :
      _delegate(handler)
  {
	assert(enet_initialize() == 0);
	_host = enet_host_create(
	    nullptr,
	    1, // max 1 peer
	    2, // 2 channels
	    0, // no rate limiting
	    0);
  }

  NetClient::~NetClient()
  {
	if(!_updateNet)
	  QThread::terminate();

	if(_host)
	  enet_host_destroy(_host);
	enet_deinitialize();
  }

  bool NetClient::ConnectServer()
  {
	QSettings settings;
	uint NdPort = settings.value("Nd_SyncPort", constants::kServerPort).toUInt();
	auto NdAddress = settings.value("Nd_SyncIp", constants::kServerIp).toString();

	_address.port = static_cast<uint16_t>(NdPort);
	if(enet_address_set_host(&_address,
	                         NdAddress.toUtf8().data()) < 0)
	  return false;

	_serverPeer = enet_host_connect(_host, &_address, 2, 0);
	if(!_serverPeer) {
	  return false;
	}

	_updateNet = true;
	QThread::start();
	return true;
  }

  bool NetClient::SendReliable(uint8_t *ptr, size_t size)
  {
	auto *packet = enet_packet_create(
	    static_cast<const void *>(ptr),
	    size,
	    ENET_PACKET_FLAG_RELIABLE);

	if(packet) {
	  return enet_peer_send(_serverPeer, 1, packet) == 0;
	}

	return false;
  }

  bool NetClient::SendFbsPacketReliable(
      net::FbsBuilder &fbb,
      protocol::MsgType type,
      const net::FbsOffset<void> packetRef)
  {
	fbb.Finish(protocol::CreateMessageRoot(
	    fbb, type, packetRef));

	return SendReliable(
	    fbb.GetBufferPointer(),
	    fbb.GetSize());
  }

  void NetClient::Disconnect()
  {
	_updateNet = false;
	enet_peer_disconnect(_serverPeer,
	                     protocol::DisconnectReason::DisconnectReason_Quit);

	while(enet_host_service(_host, &_netEvent, constants::kTimeout) > 0) {
	  switch(_netEvent.type) {
	  case ENET_EVENT_TYPE_RECEIVE:
		// while the client dies, we drop every packet
		enet_packet_destroy(_netEvent.packet);
		break;
	  case ENET_EVENT_TYPE_DISCONNECT:
		_serverPeer = nullptr;
		return;
	  }
	}

	// kill it by force
	enet_peer_reset(_serverPeer);
	_serverPeer = nullptr;
  }

  void NetClient::run()
  {
	while(_updateNet) {
	  if(enet_host_service(_host, &_netEvent, 0) > 0) {
		switch(_netEvent.type) {
		case ENET_EVENT_TYPE_CONNECT: {
		  _delegate.OnConnectRequest();
		  break;
		}
		case ENET_EVENT_TYPE_DISCONNECT: {
		  _delegate.OnDisconnect(_netEvent.data);
		  break;
		}
		case ENET_EVENT_TYPE_RECEIVE: {
		  _totalDataRecieved += _netEvent.packet->dataLength;

		  flatbuffers::Verifier verifier(
		      _netEvent.packet->data,
		      _netEvent.packet->dataLength);

		  // validate the packet is not corrupted
		  if(protocol::VerifyMessageRootBuffer(verifier)) {
			_delegate.ProcessPacket(_netEvent.packet->data, _netEvent.packet->dataLength);
		  }

		  enet_packet_destroy(_netEvent.packet);
		  break;
		}
		default:
		  break;
		}
	  }

	  QThread::msleep(constants::kNetworkerThreadIdle);
	}
  }
} // namespace noda::net