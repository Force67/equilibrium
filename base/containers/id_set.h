// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// IDSet is a container for generating unique integral identifiers, that can be
// released.
#pragma once

#include <base/containers/set.h>

namespace base {

class IdSet {
 public:
  using id_type = i32;

  IdSet() : next_id_(0) {}

  id_type GenerateId() {
#if 0
    // If there are released IDs, reuse them
    if (!released_ids_.empty()) {
      int id = *released_ids_.begin();
      released_ids_.Remove(released_ids_.begin());
      return id;
    }
#endif

    // Otherwise, use the next available ID
    return next_id_++;
  }

  void ReleaseId(id_type id) {
    // If the ID is the last one generated, just decrement the counter
    if (id == next_id_ - 1) {
      --next_id_;
    } else {
      // Otherwise, add it to the set of released IDs
      released_ids_.Insert(id);
    }
  }

 private:
  id_type next_id_;
  base::Set<id_type> released_ids_;
};
}  // namespace base