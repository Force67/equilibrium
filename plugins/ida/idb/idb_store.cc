// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "idb_store.h"

static IdbStore* idb_store{nullptr};

IdbNode::IdbNode(const char* name) : name_(name) {
  if (!idb_store)
    __debugbreak();

  idb_store->RegisterNode(this);
}

IdbNode::~IdbNode() {
  // noop for now
}

IdbStore::IdbStore() {
  idb_store = this;
}

IdbStore::~IdbStore() {
  idb_store = nullptr;
}

void IdbStore::RegisterNode(IdbNode* node) {
  nodes_.push_back(node);
}

void IdbStore::DumpNodes() {
  LOG_INFO("IdbStore::DumpNodes() -> {} nodes registered", nodes_.size());

  // TODO: save a "last saved" timestamp
  // for every node
  for (auto* n : nodes_) {
    LOG_INFO("Node {}", n->name_);
  }
}

void IdbStore::HandleEvent(ui_notification_t code, va_list args) {
  // without question the UI events are still the most stable way
  // of achieving reliable saving data reliably
  if (code == ui_notification_t::ui_saving) {
    for (IdbNode* n : nodes_)
      n->Save();
  }

  if (code == ui_notification_t::ui_database_inited) {
    for (IdbNode* n : nodes_) {
      // create the net node if it does not already exist.
      n->Reset(n->name_);
      n->Load();
    }
  }
}
