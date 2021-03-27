// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "sock_util.h"
#include <sockpp/socket.h>

#if defined(OS_POSIX)

namespace network::util {

// TODO: these should base_LOG!
bool SetTCPKeepAlive(sockpp::socket& sock, bool toggle, int delay) {
  int on = toggle ? 1 : 0;

  if (!sock.set_option(SOL_SOCKET, SO_KEEPALIVE, on)) {
    return false;
  }

  if (!toggle)
    return true;

#if defined(OS_LINUX)
  if (!sock.set_option(SOL_TCP, TCP_KEEPIDLE, delay)) {
    return false;
  }

  if (!sock.set_option(SOL_TCP, TCP_KEEPINTVL, delay)) {
    return false;
  }

#elif defined(OS_APPLE)

  if (!sock.set_option(IPPROTO_TCP, TCP_KEEPALIVE, delay)) {
    return false;
  }
#endif
  return true;
}
}  // namespace network::util

#endif