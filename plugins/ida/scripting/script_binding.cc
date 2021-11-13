// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Exposes RETK plugin bindings to IDC and IDA-python.

#include "script_binding.h"

static BindingBase* rootBinding{nullptr};

BindingBase::BindingBase(const char* const name,
    const char* const args,
    idc_func_t* funcptr) {
  desc_ = {name, funcptr, args, nullptr, 0, EXTFUN_BASE};
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

  //add_idc_class(nullptr, nullptr);

  rootBinding = nullptr;
}

void BindingBase::UnBindAll() {
  for (BindingBase* i = rootBinding; i;) {
    del_idc_func(i->desc_.name);
    i = i->next_;
  }
}