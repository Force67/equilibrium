#pragma once
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <network/NetBuffer.h>
#include <QString>

namespace sync_utils {

// Copies a QString onto the NetBuffer and returns the reference
network::FbsStringRef CreateFbStringRef(network::FbsBuffer& buffer,
                                        const QString& other);

const QString& GetDefaultUserName();

// returns a unique identifier based on the machine the user is using
const QString& GetUserGuid();
}  // namespace sync_utils