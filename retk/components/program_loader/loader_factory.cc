// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <base/logging.h>
#include <program_loader/loader_factory.h>

#include "inbuilt/loader_pe.h"

namespace program_loader {

LoaderFactory::LoaderFactory() {
  // register inbuilt loaders.
  known_loaders_.push_back(LoaderPE::descriptor());
}

void LoaderFactory::FindApplicableCanidates(
    const FileClassificationInfo& intel,
    std::vector<const ProgramLoadDescriptor*>& out) {
  for (const ProgramLoadDescriptor* it : known_loaders_) {
    // TODO: We allow the user to choose architecture, but recommend one.
    if (it->format == intel.format_type)
      out.push_back(it);
  }
}

std::unique_ptr<ProgramLoader> LoaderFactory::CreateLoader(
    const ProgramLoadDescriptor& desc,
    const FileClassificationInfo& info) {
  const bool is_inbuilt = desc.flags & ProgramLoadDescriptor::Flags::kInternal;

  if (!is_inbuilt) {
    // destroy is optional
    if (!desc.CreateLoader) {
      LOG_WARNING("Unable to create loader {} as desc.CreateLoader is not set",
                  desc.name);
      return nullptr;
    }
  }

  switch (info.format_type) {
    case Format::kELF:
    case Format::kXEX:
    case Format::kPE: {
      if (is_inbuilt)
        return std::make_unique<LoaderPE>();

      return nullptr;
    }
    case Format::kMachO:
    case Format::kSELF:
      break;
    default:
      return nullptr;
  }

  return nullptr;
}

void LoaderFactory::CollectCustomLoaders() {}

}  // namespace program_loader