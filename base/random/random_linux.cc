// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "random.h"

#include <fcntl.h>
#include <unistd.h>

namespace base {

  uint64_t SourceTrueRandomSeed() {
		 int fd = ::open("/dev/urandom", O_RDONLY);
  if (fd == -1) {
    // Handle error
    return 0;
  }

  uint64_t seed;
  ssize_t result = ::read(fd, &seed, sizeof(seed));
  ::close(fd);

  if (result != sizeof(seed)) {
    // Handle error
    return 0;
  }

  return seed;
}
}