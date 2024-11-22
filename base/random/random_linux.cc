// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "random.h"

#include <fcntl.h>
#include <unistd.h>

namespace base {

u64 SourceTrueRandomSeed() {
  int fd = ::open("/dev/urandom", O_RDONLY);
  if (fd == -1) {
    return 0;
  }

  u64 seed;
  ssize_t result = ::read(fd, &seed, sizeof(seed));
  ::close(fd);

  if (result != sizeof(seed)) {
    return 0;
  }

  return seed;
}
}  // namespace base