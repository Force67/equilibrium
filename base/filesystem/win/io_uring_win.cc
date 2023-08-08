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

// enum used as discriminator for references to resources that
// support preregistration in an IORING
typedef enum IORING_REF_KIND {
  IORING_REF_RAW,
  IORING_REF_REGISTERED,
} IORING_REF_KIND;

typedef struct IORING_HANDLE_REF {
#ifdef __cplusplus
  explicit IORING_HANDLE_REF(HANDLE h)
      : Kind(IORING_REF_KIND::IORING_REF_RAW), Handle(h) {}

  explicit IORING_HANDLE_REF(u32 index)
      : Kind(IORING_REF_KIND::IORING_REF_REGISTERED), Handle(index) {}
#endif

  IORING_REF_KIND Kind;
  union HandleUnion {
#ifdef __cplusplus
    HandleUnion(HANDLE h) : Handle(h) {}

    HandleUnion(u32 index) : Index(index) {}
#endif
    // Handle to the file object if Kind == IORING_REF_RAW
    HANDLE Handle;

    // Index of registered file handle if Kind == IORING_REF_REGISTERED
    u32 Index;
  } Handle;
} IORING_HANDLE_REF;

typedef struct IORING_REGISTERED_BUFFER {
  // Index of pre-registered buffer
  u32 BufferIndex;

  // Offset into the pre-registered buffer
  u32 Offset;
} IORING_REGISTERED_BUFFER;

typedef struct IORING_BUFFER_REF {
#ifdef __cplusplus
  explicit IORING_BUFFER_REF(void* address)
      : Kind(IORING_REF_KIND::IORING_REF_RAW), Buffer(address) {}

  explicit IORING_BUFFER_REF(IORING_REGISTERED_BUFFER registeredBuffer)
      : Kind(IORING_REF_KIND::IORING_REF_REGISTERED), Buffer(registeredBuffer) {}

  IORING_BUFFER_REF(u32 index, u32 offset)
      : IORING_BUFFER_REF(IORING_REGISTERED_BUFFER{index, offset}) {}
#endif

  IORING_REF_KIND Kind;
  union BufferUnion {
#ifdef __cplusplus
    BufferUnion(void* address) : Address(address) {}

    BufferUnion(IORING_REGISTERED_BUFFER indexAndOffset)
        : IndexAndOffset(indexAndOffset) {}
#endif
    // Address of the buffer if Kind == IORING_REF_RAW
    void* Address;

    // Registered buffer details if Kind == IORING_REF_REGISTERED
    IORING_REGISTERED_BUFFER IndexAndOffset;
  } Buffer;
} IORING_BUFFER_REF;

extern "C" __declspec(dllimport) HRESULT
    PopIoRingCompletion(_In_ void* ioRing, IORING_CQE* cqe);

typedef enum IORING_SQE_FLAGS {
  IOSQE_FLAGS_NONE = 0,
} IORING_SQE_FLAGS;

extern "C" __declspec(dllimport) HRESULT
    BuildIoRingReadFile(void* ioRing,
                        IORING_HANDLE_REF fileRef,
                        IORING_BUFFER_REF dataRef,
                        u32 numberOfBytesToRead,
                        u64 fileOffset,
                        UINT_PTR userData,
                        IORING_SQE_FLAGS flags);

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
    LOG_WARNING(
        "Attempted to destroy an uninitialized or already destroyed IO ring.");
  }
}

bool IOUring::SubmitReadFile(HANDLE fileHandle,
                             void* buffer,
                             UINT32 size,
                             UINT64 offset,
                             CompletionCallback callback) {
  IORING_HANDLE_REF fileRef = IoRingHandleRefFromHandle(fileHandle);
  IORING_BUFFER_REF bufferRef = IoRingBufferRefFromPointer(buffer);
  Operation op{buffer, base::move(callback)};
  HRESULT result = ::BuildIoRingReadFile(
      ring_handle_, fileRef, bufferRef, size, offset, op.extra_data, IOSQE_FLAGS_NONE);
  if (FAILED(result)) {
    LOG_ERROR("Failed to submit read operation.");
    return false;
  }
  pending_queue_.push_back(op);
  return true;
}

void IOUring::WaitForCompletions() {
  while (!pending_queue_.empty()) {
    IORING_CQE cqe;
    HRESULT result = ::PopIoRingCompletion(ring_handle_, &cqe);
    if (SUCCEEDED(result)) {
      Operation& op = pending_queue_.front();
      pending_queue_.erase();

      // Check if user data matches (for validation)
      if (op.extra_data == cqe.UserData && op.callback) {
        op.callback(cqe);
      }
    }
  }
}

}  // namespace base