// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// IDSet is a container for generating unique integral identifiers, that can be
// released.
#pragma once

#include <base/containers/set.h>

namespace base {

template <typename T, T InvalidValue, T MaxValue>
class IdSet {
 public:
  using id_type = T;

  IdSet() : next_id_(0) {}

  id_type GenerateId() {
    // reclaim released ids
    if (!released_ids_.empty()) {
      int id = *released_ids_.begin();
      released_ids_.Remove(id);
      return id;
    }
    // otherwise, use the next id
    if (next_id_ + 1 == MaxValue) {
      return InvalidValue;
    }
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