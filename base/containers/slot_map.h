// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// A Slot Map is a high-performance associative container with persistent unique
// keys to access stored values. Upon insertion, a key is returned that can be
// used to later access or remove the values. Insertion, removal, and access are
// all guaranteed to take O(1) time (best, worst, and average case) Great for
// storing collections of objects that need stable, safe references but have no
// clear ownership.
#pragma once

namespace base {
class SlotMap {};

}  // namespace base