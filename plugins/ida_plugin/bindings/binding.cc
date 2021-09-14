// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "binding.h"

namespace iretk {

static BindingBase* rootBinding{nullptr};

void BindingBase::BindAll() {
  for (BindingBase* i = rootBinding; i;) {
    add_idc_func(i->desc_);
    i = i->next_;
  }
}

void BindingBase::UnBindAll() {
  for (BindingBase* i = rootBinding; i;) {
    del_idc_func(i->desc_.name);
    i = i->next_;
  }
}

}  // namespace iretk