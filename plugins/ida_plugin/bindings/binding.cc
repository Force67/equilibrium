// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "binding.h"

static BindingBase* rootBinding{nullptr};

BindingBase::BindingBase(const char* const name,
    const char* const args,
    idc_func_t* funcptr) {
  desc_ = {name, funcptr, args, nullptr, 0, 0};
  next_ = rootBinding;
  rootBinding = this;
}

void BindingBase::BindAll() {
  for (BindingBase* i = rootBinding; i;) {
    add_idc_func(i->desc_);
    auto *j = i->next_;
    // need to reset this so the rerun for unbind works as expected
    i->next_ = nullptr;
    i = j;
  }
  rootBinding = nullptr;
}

void BindingBase::UnBindAll() {
  for (BindingBase* i = rootBinding; i;) {
    del_idc_func(i->desc_.name);
    i = i->next_;
  }
}