// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "base/arch.h"
#include "base/logging.h"
#include "base/win/minwin.h"

namespace {
typedef enum IORING_VERSION {
  IORING_VERSION_INVALID,
  IORING_VERSION_1,
  IORING_VERSION_2,
  IORING_VERSION_3
};

typedef enum IORING_FEATURE_FLAGS {
  IORING_FEATURE_FLAGS_NONE,
  IORING_FEATURE_UM_EMULATION,
  IORING_FEATURE_SET_COMPLETION_EVENT
};

typedef struct IORING_CAPABILITIES {
  IORING_VERSION MaxVersion;
  u32 MaxSubmissionQueueSize;
  u32 MaxCompletionQueueSize;
  IORING_FEATURE_FLAGS FeatureFlags;
} IORING_CAPABILITIES;

typedef enum IORING_CREATE_REQUIRED_FLAGS { IORING_CREATE_REQUIRED_FLAGS_NONE };
typedef enum IORING_CREATE_ADVISORY_FLAGS { IORING_CREATE_ADVISORY_FLAGS_NONE };

typedef struct IORING_CREATE_FLAGS {
  IORING_CREATE_REQUIRED_FLAGS Required;
  IORING_CREATE_ADVISORY_FLAGS Advisory;
} IORING_CREATE_FLAGS;

extern "C" __declspec(dllimport) HRESULT
    QueryIoRingCapabilities(IORING_CAPABILITIES* capabilities);

extern "C" __declspec(dllimport) HRESULT CreateIoRing(IORING_VERSION ioringVersion,
                                                      IORING_CREATE_FLAGS flags,
                                                      u32 submissionQueueSize,
                                                      u32 completionQueueSize,
                                                      void* h);

extern "C" __declspec(dllimport) HRESULT CloseIoRing(void* ioRing);

typedef struct IORING_CQE {
  UINT_PTR UserData;
  HRESULT ResultCode;
  ULONG_PTR Information;
} IORING_CQE;

extern "C" __declspec(dllimport) HRESULT
    PopIoRingCompletion(_In_ void* ioRing, IORING_CQE* cqe);
}  // namespace

#include "filesystem/io_uring.h"

namespace base {
bool IOUring::Create() {
  IORING_CAPABILITIES caps{};
  if (FAILED(::QueryIoRingCapabilities(&caps))) {
    LOG_ERROR("Failed to query IO ring capabilities.");
    return false;
  }

  // Log the capabilities for debugging.
  LOG_TRACE(
      "MaxVersion: {}, MaxSubmissionQueueSize: {}, MaxCompletionQueueSize: {}, "
      "FeatureFlags: {}",
      caps.MaxVersion, caps.MaxSubmissionQueueSize, caps.MaxCompletionQueueSize,
      caps.FeatureFlags);

  if (caps.MaxVersion < IORING_VERSION_3) {
    LOG_ERROR("API outdated");
    return false;
  }

  IORING_CREATE_FLAGS flags{};
  if (FAILED(
          ::CreateIoRing(IORING_VERSION_3, flags, 0x10000, 0x20000, ring_handle_))) {
    LOG_ERROR("Failed to create IO ring.");
    return false;
  }

  return false;
}

void IOUring::Destroy() {
  if (ring_handle_) {
    // Before destroying the IO ring, you might want to ensure there are no pending
    // operations. Depending on your use case, you might also want to check
    // completions or other states.

    if (FAILED(::CloseIoRing(ring_handle_))) {
      LOG_ERROR("Failed to close IO ring.");
    }
    ring_handle_ = nullptr;
  } else {
    LOG_WARNING("Attempted to destroy an uninitialized or already destroyed IO ring.");
  }
}

bool IOUring::SubmitReadFile(HANDLE fileHandle,
                             void* buffer,
                             UINT32 size,
                             UINT64 offset,
                             IOCompletionCallback callback) {
  IORING_HANDLE_REF fileRef = IoRingHandleRefFromHandle(fileHandle);
  IORING_BUFFER_REF bufferRef = IoRingBufferRefFromPointer(buffer);

  IOOperation op;
  op.userData = reinterpret_cast<UINT_PTR>(
      buffer);  // Example: using buffer address as user data for simplicity
  op.callback = callback;

  HRESULT result = BuildIoRingReadFile(ioRingHandle, fileRef, bufferRef, size,
                                       offset, op.userData, IOSQE_FLAGS_NONE);
  if (FAILED(result)) {
    LOG_ERROR("Failed to submit read operation.");
    return false;
  }

  operationsQueue.push(op);
  return true;
}

void IOUring::WaitForCompletions() {
  while (!pending_queue_.empty()) {
    IORING_CQE cqe;
    HRESULT result = ::PopIoRingCompletion(ring_handle_, &cqe);
    if (SUCCEEDED(result)) {
      Operation &op = pending_queue_.front();
      pending_queue_.pop();

      // Check if user data matches (for validation)
      if (op.userData == cqe.UserData && op.callback) {
        op.callback(cqe);
      }
    }
  }
}

}  // namespace base