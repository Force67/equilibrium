// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "sock_util.h"
#include <sockpp/socket.h>

#if defined(OS_WIN)

#if !defined(SIO_KEEPALIVE_VALS)
#define SIO_KEEPALIVE_VALS _WSAIOW(IOC_VENDOR, 4)
struct tcp_keepalive {
  u_long onoff;
  u_long keepalivetime;
  u_long keepaliveinterval;
};
#endif

namespace network::util {

// TODO: these should base_LOG!
bool SetTCPKeepAlive(sockpp::socket& sock, bool toggle, int delay) {
  unsigned scaledDelay = delay * 1000;
  struct tcp_keepalive keepalive_vals = {
      toggle ? 1u : 0u,
      scaledDelay,
      scaledDelay,
  };

  DWORD bytes_returned = 0xABAB;
  int rv = WSAIoctl(sock.handle(), SIO_KEEPALIVE_VALS, &keepalive_vals,
                    sizeof(keepalive_vals), nullptr, 0, &bytes_returned,
                    nullptr, nullptr);

  return rv == 0;
}
}  // namespace network::util

#endif