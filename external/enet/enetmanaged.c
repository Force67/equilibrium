// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
// Support for cross language interop

#ifdef ENET_BUILD_MANAGED

#include <string.h>
#define ENET_BUILDING_LIB 1
#include "enet/enet.h"

#include <stdint.h>

// == PEER == 

ENET_API enet_uint32 enet_peer_get_id(const ENetPeer *peer) {
  return peer->incomingPeerID;
}

ENET_API enet_uint32 enet_peer_get_mtu(const ENetPeer *peer) {
  return peer->mtu;
}

ENET_API ENetPeerState enet_peer_get_state(const ENetPeer *peer) {
  return peer->state;
}

ENET_API enet_uint32 enet_peer_get_rtt(const ENetPeer *peer) {
  return peer->roundTripTime;
}

ENET_API enet_uint32 enet_peer_get_last_rtt(const ENetPeer *peer) {
  return peer->lastRoundTripTime;
}

ENET_API enet_uint32 enet_peer_get_lastsendtime(const ENetPeer *peer) {
  return peer->lastSendTime;
}

ENET_API enet_uint32 enet_peer_get_lastreceivetime(const ENetPeer *peer) {
  return peer->lastReceiveTime;
}

ENET_API float enet_peer_get_packets_throttle(const ENetPeer *peer) {
  return peer->packetThrottle / (float)ENET_PEER_PACKET_THROTTLE_SCALE * 100.0f;
}

ENET_API void *enet_peer_get_data(const ENetPeer *peer) {
  return (void *)peer->data;
}

ENET_API void enet_peer_set_data(ENetPeer *peer, const void *data) {
  peer->data = (uint32_t *)data;
}

// == PACKET == 

ENET_API void enet_packet_dispose(ENetPacket *packet) {
  if (packet->referenceCount == 0)
    enet_packet_destroy(packet);
}

ENET_API void *enet_packet_get_data(const ENetPacket *packet) {
  return (void *)packet->data;
}

ENET_API int enet_packet_get_length(const ENetPacket *packet) {
  return packet->dataLength;
}

ENET_API void *enet_packet_get_user_data(const ENetPacket *packet) {
  return packet->userData;
}

ENET_API void enet_packet_set_user_data(ENetPacket *packet, void *userData) {
  packet->userData = userData;
}

ENET_API int enet_packet_check_references(const ENetPacket *packet) {
  return (int)packet->referenceCount;
}

// == HOST == 

ENET_API enet_uint32 enet_host_get_peers_count(const ENetHost *host) {
  return host->connectedPeers;
}

ENET_API enet_uint32 enet_host_get_packets_sent(const ENetHost *host) {
  return host->totalSentPackets;
}

// == ADDRESS == 

ENET_API int enet_address_get_ip(const ENetAddress *address, char *ip,
                                 size_t ipLength) {
  return enet_address_get_host_ip(address, ip, ipLength);
}

#endif