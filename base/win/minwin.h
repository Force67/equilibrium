// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// This header helps us to get rid of all instances of <Windows.h>
#pragma once

// Needed for function prototypes.
#include <concurrencysal.h>
#include <sal.h>
#include <specstrings.h>

#ifdef __cplusplus
extern "C" {
#endif

// typedef and define the most commonly used Windows integer types.

typedef unsigned long DWORD;
typedef long LONG;
typedef __int64 LONGLONG;
typedef unsigned __int64 ULONGLONG;

#define VOID void
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
typedef int INT;
typedef unsigned int UINT;
typedef unsigned int* PUINT;
typedef unsigned __int64 UINT64;
typedef void* LPVOID;
typedef void* PVOID;
typedef void* HANDLE;
typedef int BOOL;
typedef unsigned char BYTE;
typedef BYTE BOOLEAN;
typedef DWORD ULONG;
typedef unsigned short WORD;
typedef WORD UWORD;
typedef WORD ATOM;

#if defined(_WIN64)
typedef __int64 INT_PTR, *PINT_PTR;
typedef unsigned __int64 UINT_PTR, *PUINT_PTR;

typedef __int64 LONG_PTR, *PLONG_PTR;
typedef unsigned __int64 ULONG_PTR, *PULONG_PTR;
#else
typedef __w64 int INT_PTR, *PINT_PTR;
typedef __w64 unsigned int UINT_PTR, *PUINT_PTR;

typedef __w64 long LONG_PTR, *PLONG_PTR;
typedef __w64 unsigned long ULONG_PTR, *PULONG_PTR;
#endif

typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;
#define LRESULT LONG_PTR
typedef _Return_type_success_(return >= 0) long HRESULT;

typedef ULONG_PTR SIZE_T, *PSIZE_T;
typedef LONG_PTR SSIZE_T, *PSSIZE_T;

typedef DWORD ACCESS_MASK;
typedef ACCESS_MASK REGSAM;

typedef LONG NTSTATUS;

// As defined in guiddef.h.
#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#define REFGUID const GUID&
#endif

// Forward declare Windows compatible handles.

#define CHROME_WINDOWS_HANDLE_TYPE(name) \
  struct name##__;                       \
  typedef struct name##__* name;
#include "base/win/win_handle_types_list.inl"
#undef CHROME_WINDOWS_HANDLE_TYPE

typedef LPVOID HINTERNET;
typedef HICON HCURSOR;
typedef HINSTANCE HMODULE;
typedef PVOID LSA_HANDLE;
typedef PVOID HDEVINFO;

// Forward declare some Windows struct/typedef sets.

typedef struct _OVERLAPPED OVERLAPPED;
typedef struct tagMSG MSG, *PMSG, *NPMSG, *LPMSG;
typedef struct tagTOUCHINPUT TOUCHINPUT;
typedef struct tagPOINTER_INFO POINTER_INFO;

typedef struct _RTL_SRWLOCK RTL_SRWLOCK;
typedef RTL_SRWLOCK SRWLOCK, *PSRWLOCK;

typedef struct _GUID GUID;
typedef GUID CLSID;
typedef GUID IID;

typedef struct tagLOGFONTW LOGFONTW, *PLOGFONTW, *NPLOGFONTW, *LPLOGFONTW;
typedef LOGFONTW LOGFONT;

typedef struct _FILETIME FILETIME;

typedef struct tagMENUITEMINFOW MENUITEMINFOW, MENUITEMINFO;

typedef struct tagNMHDR NMHDR;

typedef struct _SP_DEVINFO_DATA SP_DEVINFO_DATA;

typedef PVOID PSID;

typedef HANDLE HLOCAL;

typedef /* [wire_marshal] */ WORD CLIPFORMAT;
typedef struct tagDVTARGETDEVICE DVTARGETDEVICE;

typedef struct tagFORMATETC FORMATETC;

// Use WIN32_FIND_DATAW when you just need a forward declaration. Use
// CHROME_WIN32_FIND_DATA when you need a concrete declaration to reserve
// space.
typedef struct _WIN32_FIND_DATAW WIN32_FIND_DATAW;
typedef WIN32_FIND_DATAW WIN32_FIND_DATA;

// Declare Chrome versions of some Windows structures. These are needed for
// when we need a concrete type but don't want to pull in Windows.h. We can't
// declare the Windows types so we declare our types and cast to the Windows
// types in a few places. The sizes must match the Windows types so we verify
// that with static asserts in win_includes_unittest.cc.
// ChromeToWindowsType functions are provided for pointer conversions.

struct CHROME_SRWLOCK {
  PVOID Ptr;
};

struct CHROME_CONDITION_VARIABLE {
  PVOID Ptr;
};

struct CHROME_LUID {
  DWORD LowPart;
  LONG HighPart;
};

// _WIN32_FIND_DATAW is 592 bytes and the largest built-in type in it is a
// DWORD. The buffer declaration guarantees the correct size and alignment.
struct CHROME_WIN32_FIND_DATA {
  DWORD buffer[592 / sizeof(DWORD)];
};

struct CHROME_FORMATETC {
  CLIPFORMAT cfFormat;
  /* [unique] */ DVTARGETDEVICE* ptd;
  DWORD dwAspect;
  LONG lindex;
  DWORD tymed;
};

struct CHROME_POINT {
  LONG x;
  LONG y;
};

struct CHROME_MSG {
  HWND hwnd;
  UINT message;
  WPARAM wParam;
  LPARAM lParam;
  DWORD time;
  CHROME_POINT pt;
};

// Define some commonly used Windows constants. Note that the layout of these
// macros - including internal spacing - must be 100% consistent with windows.h.

// clang-format off

#ifndef INVALID_HANDLE_VALUE
// Work around there being two slightly different definitions in the SDK.
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
#endif
#define TLS_OUT_OF_INDEXES ((DWORD)0xFFFFFFFF)
#define HTNOWHERE 0
#define MAX_PATH 260
#define CS_GLOBALCLASS 0x4000

#define ERROR_SUCCESS 0L
#define ERROR_FILE_NOT_FOUND 2L
#define ERROR_ACCESS_DENIED 5L
#define ERROR_INVALID_HANDLE 6L
#define ERROR_SHARING_VIOLATION 32L
#define ERROR_LOCK_VIOLATION 33L
#define ERROR_MORE_DATA 234L
#define REG_BINARY ( 3ul )
#define REG_NONE ( 0ul )

#define STATUS_PENDING ((DWORD   )0x00000103L)
#define STILL_ACTIVE STATUS_PENDING
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr) (((HRESULT)(hr)) < 0)

#define HKEY_CLASSES_ROOT (( HKEY ) (ULONG_PTR)((LONG)0x80000000) )
#define HKEY_LOCAL_MACHINE (( HKEY ) (ULONG_PTR)((LONG)0x80000002) )
#define HKEY_CURRENT_USER (( HKEY ) (ULONG_PTR)((LONG)0x80000001) )
#define KEY_QUERY_VALUE (0x0001)
#define KEY_SET_VALUE (0x0002)
#define KEY_CREATE_SUB_KEY (0x0004)
#define KEY_ENUMERATE_SUB_KEYS (0x0008)
#define KEY_NOTIFY (0x0010)
#define KEY_CREATE_LINK (0x0020)
#define KEY_WOW64_32KEY (0x0200)
#define KEY_WOW64_64KEY (0x0100)
#define KEY_WOW64_RES (0x0300)

#define PROCESS_QUERY_INFORMATION (0x0400)
#define READ_CONTROL (0x00020000L)
#define SYNCHRONIZE (0x00100000L)

#define STANDARD_RIGHTS_READ (READ_CONTROL)
#define STANDARD_RIGHTS_WRITE (READ_CONTROL)
#define STANDARD_RIGHTS_ALL (0x001F0000L)

#define KEY_READ                ((STANDARD_RIGHTS_READ       |\
                                  KEY_QUERY_VALUE            |\
                                  KEY_ENUMERATE_SUB_KEYS     |\
                                  KEY_NOTIFY)                 \
                                  &                           \
                                 (~SYNCHRONIZE))


#define KEY_WRITE               ((STANDARD_RIGHTS_WRITE      |\
                                  KEY_SET_VALUE              |\
                                  KEY_CREATE_SUB_KEY)         \
                                  &                           \
                                 (~SYNCHRONIZE))

#define KEY_ALL_ACCESS          ((STANDARD_RIGHTS_ALL        |\
                                  KEY_QUERY_VALUE            |\
                                  KEY_SET_VALUE              |\
                                  KEY_CREATE_SUB_KEY         |\
                                  KEY_ENUMERATE_SUB_KEYS     |\
                                  KEY_NOTIFY                 |\
                                  KEY_CREATE_LINK)            \
                                  &                           \
                                 (~SYNCHRONIZE))

// The trailing white-spaces after this macro are required, for compatibility
// with the definition in winnt.h.
#define RTL_SRWLOCK_INIT {0}                            // NOLINT
#define SRWLOCK_INIT RTL_SRWLOCK_INIT

// clang-format on

// Define some macros needed when prototyping Windows functions.

#define DECLSPEC_IMPORT __declspec(dllimport)
#define WINBASEAPI DECLSPEC_IMPORT
#define WINUSERAPI DECLSPEC_IMPORT
#define WINAPI __stdcall
#define APIENTRY WINAPI
#define CALLBACK __stdcall

// Needed for LockImpl.
WINBASEAPI _Releases_exclusive_lock_(*SRWLock) VOID WINAPI
    ReleaseSRWLockExclusive(_Inout_ PSRWLOCK SRWLock);
WINBASEAPI BOOLEAN WINAPI TryAcquireSRWLockExclusive(_Inout_ PSRWLOCK SRWLock);
WINBASEAPI
_Acquires_exclusive_lock_(*SRWLock) VOID WINAPI
    AcquireSRWLockExclusive(_Inout_ PSRWLOCK SRWLock);

// Needed to support protobuf's GetMessage macro magic.
WINUSERAPI BOOL WINAPI GetMessageW(LPMSG lpMsg,
                                   HWND hWnd,
                                   UINT wMsgFilterMin,
                                   UINT wMsgFilterMax);

// Needed for thread_local_storage.h
WINBASEAPI LPVOID WINAPI TlsGetValue(DWORD dwTlsIndex);

WINBASEAPI BOOL WINAPI TlsSetValue(DWORD dwTlsIndex, LPVOID lpTlsValue);

// Needed for scoped_handle.h
WINBASEAPI _Check_return_ _Post_equals_last_error_ DWORD WINAPI GetLastError(VOID);

WINBASEAPI VOID WINAPI SetLastError(DWORD dwErrCode);

WINBASEAPI BOOL WINAPI TerminateProcess(HANDLE hProcess, UINT uExitCode);

// Support for a deleter for LocalAlloc memory.
WINBASEAPI HLOCAL WINAPI LocalFree(HLOCAL hMem);

#ifdef __cplusplus
}

// Helper functions for converting between Chrome and Windows native versions of
// type pointers.
// Overloaded functions must be declared outside of the extern "C" block.

inline WIN32_FIND_DATA* ChromeToWindowsType(CHROME_WIN32_FIND_DATA* p) {
  return reinterpret_cast<WIN32_FIND_DATA*>(p);
}

inline const WIN32_FIND_DATA* ChromeToWindowsType(const CHROME_WIN32_FIND_DATA* p) {
  return reinterpret_cast<const WIN32_FIND_DATA*>(p);
}

inline FORMATETC* ChromeToWindowsType(CHROME_FORMATETC* p) {
  return reinterpret_cast<FORMATETC*>(p);
}

inline const FORMATETC* ChromeToWindowsType(const CHROME_FORMATETC* p) {
  return reinterpret_cast<const FORMATETC*>(p);
}

inline MSG* ChromeToWindowsType(CHROME_MSG* p) {
  return reinterpret_cast<MSG*>(p);
}

#endif