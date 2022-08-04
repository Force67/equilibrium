// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <base/logging.h>
#include <program_loader/loader_factory.h>

#include "inbuilt/loader_pe.h"
#include "inbuilt/loader_elf.h"
#include "inbuilt/loader_mach-o.h"

namespace program_loader {

LoaderFactory::LoaderFactory() {
  // register inbuilt loaders.
  //known_loaders_.resize(3);
  known_loaders_.push_back(LoaderPE::descriptor());
  known_loaders_.push_back(LoaderELF::descriptor());
  known_loaders_.push_back(LoaderMachO::descriptor());
}

void LoaderFactory::FindApplicableCanidates(
    const FileClassificationInfo& intel,
    base::Vector<const ProgramLoadDescriptor*>& out) {
  for (const ProgramLoadDescriptor* it : known_loaders_) {
    // TODO: We allow the user to choose architecture, but recommend one.
    if (it->format == intel.format_type)
      out.push_back(it);
  }
}

// TODO: owningptr, class 
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

    return std::unique_ptr<ProgramLoader>(desc.CreateLoader());
  }

  switch (info.format_type) {
    case Format::kELF:
      return std::make_unique<LoaderELF>();
    case Format::kXEX:
    case Format::kPE:
      return std::make_unique<LoaderPE>();
    case Format::kMachO:
      return std::make_unique<LoaderMachO>();
    case Format::kSELF:
      break;
    default:
      return nullptr;
  }

  return nullptr;
}

void LoaderFactory::CollectCustomLoaders() {}

}  // namespace program_loader