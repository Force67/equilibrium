#pragma once
// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <vector>
#include "utils/ida_plus.h"

// Base class for IDB storage nodes,
// it automatically registers to IdbStore
// on construction
class IdbNode : public ial::NetNode {
  friend class IdbStore;
 public:
  IdbNode(const char *name);
  ~IdbNode();

  const char* GetName() const { return name_; }

  // primitives for parent nodes.
  virtual void Save() = 0;
  virtual void Load() = 0;

 private:
  const char* name_;
};

class IdbStore final : public ial::UiEventHandler {
  friend class IdbNode;

 public:
  IdbStore();
  ~IdbStore();

  void DumpNodes();

 private:
  void RegisterNode(IdbNode*);
  void HandleEvent(ui_notification_t code, va_list args) override;

 private:
  std::vector<IdbNode*> nodes_;
};