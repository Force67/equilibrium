// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "data_store.h"

static DataStore* s_Store = nullptr;

NodeTraits::NodeTraits() {
  if (!s_Store)
    __debugbreak();

  s_Store->RegisterNode(this);
}

NodeTraits::~NodeTraits() {
    // noop for now
}

DataStore::DataStore() {
  // the best way is still simply to hook into the ui Event.
  hook_to_notification_point(hook_type_t::HT_UI, Event, this);
  s_Store = this;
}

DataStore::~DataStore() {
  unhook_from_notification_point(hook_type_t::HT_UI, Event, this);
  s_Store = nullptr;
}

void DataStore::RegisterNode(NodeTraits* node) {
  nodes_.push_back(node);
}

void DataStore::DumpNodes() {
  LOG_INFO("DataStore::DumpNodes() -> {} nodes registered", nodes_.size());

  // TODO: save a "last saved" timestamp
  // for every node
  for (auto* n : nodes_) {
    LOG_INFO("Node {}", n->GetName());
  }
}

ssize_t DataStore::Event(void* ptr, int code, va_list args) {
  DataStore* store = reinterpret_cast<DataStore*>(ptr);

  if (code == ui_notification_t::ui_saving) {
    for (auto* n : store->nodes_)
      n->Save();
  }

  if (code == ui_notification_t::ui_database_inited) {
    for (auto* n : store->nodes_)
      n->Load();
  }

  return 0;
}