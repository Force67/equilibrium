#pragma once
// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <QString>
#include <network/NetBuffer.h>

namespace sync_utils {

  // Copies a QString onto the NetBuffer and returns the reference
  network::FbsStringRef CreateFbStringRef(network::FbsBuffer &buffer, const QString &other);

  const QString &GetDefaultUserName();

  // returns a unique identifier based on the machine the user is using
  const QString &GetUserGuid();
} // namespace sync_utils