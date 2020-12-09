// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "UiStorage.h"

namespace {
  // unique identifier for IDB storage
  constexpr char kSessionNodeId[] = "$ noda_ui_storage";

  // increment this whenever a breaking change happens
  constexpr int kUiStorageVersion = 1;

  enum NodeIndex : nodeidx_t {
	StorageVersion,
	Uptime,
	IdbUsageFlags
  };
} // namespace

UiStorage::UiStorage()
{
  hook_to_notification_point(hook_type_t::HT_UI, Event, this);
}

UiStorage::~UiStorage()
{
  unhook_from_notification_point(hook_type_t::HT_UI, Event, this);
}

ssize_t UiStorage::Event(void *, int, va_list)
{
  return 0;
}