#pragma once
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

namespace sockpp {
class socket;
}

namespace network::util {

bool SetTCPKeepAlive(sockpp::socket& sock, bool toggle, int delay);
}