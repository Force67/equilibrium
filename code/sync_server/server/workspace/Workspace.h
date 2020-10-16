// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <string>
#include <memory>

namespace noda {

struct Project {
enum class Status {

};  

Status status;
std::string name;
};

struct Workspace {
  
};

std::unique_ptr<Workspace> CreateWorkspace();
}