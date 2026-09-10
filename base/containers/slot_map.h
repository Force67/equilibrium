// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Associative container with persistent unique keys. Insertion returns a key
// for later access or removal. All operations are O(1). Fits collections
// needing stable, safe references without clear ownership.
#pragma once

namespace base {
class SlotMap {};

}  // namespace base