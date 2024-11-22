// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/win/minwin.h>
#include <base/containers/vector.h>
#include <base/filesystem/common_directory_providers.h>

namespace {
extern "C" __declspec(dllimport) DWORD GetModuleFileNameW(HMODULE hModule,
                                                          wchar_t* lpFilename,
                                                          DWORD nSize);
}

namespace base {

base::Optional<base::Path> FetchKnownPath(const KnownPath path_id) {
  // Start with a buffer of size MAX_PATH.
  base::Vector<wchar_t> system_buffer(MAX_PATH, base::VectorReservePolicy::kForData);

  DWORD buffer_size = static_cast<DWORD>(system_buffer.size());

  base::Path result;
  switch (path_id) {
    case base::KnownPath::CurrentExecutable: {
      DWORD return_length =
          ::GetModuleFileNameW(nullptr, system_buffer.data(), buffer_size);

      // If the function fails because the buffer is too small
      while (return_length == buffer_size &&
             ::GetLastError() == 122 /* ERROR_INSUFFICIENT_BUFFER*/) {
        // Double the buffer size
        buffer_size *= 2;
        system_buffer.resize(buffer_size);

        return_length = ::GetModuleFileNameW(nullptr, system_buffer.data(), buffer_size);
      }

      if (return_length == 0) {
        return {};
      }
      return base::Path(base::StringRefW(system_buffer.data(), system_buffer.size()));
    }
  }
  return result;
}
}  // namespace base