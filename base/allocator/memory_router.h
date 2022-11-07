// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace base {

class NewMemoryRouter {
 public:
  // v1
  inline void* Allocate(size_t size) {
    void* block = router_.Allocate(size /*void* block*/);
    tracker_.TrackOperation(block, pointer_diff(size));
    BASE_PROFILE_ALLOCATION(block, size);
    return block;
  }


};
}  // namespace base