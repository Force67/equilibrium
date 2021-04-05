// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace forms {

// while i originally intended to implement that in pure qt,
// but its much better to use IDA's view, since it's already registered
// in the IDA queues.. etc
class AddressBookView final : public chooser_multi_t {
 public:
  AddressBookView();

 private:
  // impl for : chooser_base_t
  const void* get_obj_id(size_t* len) const override;

  bool idaapi init() override;

  size_t idaapi get_count() const override;

  void idaapi get_row(qstrvec_t* cols,
                      int* icon_,
                      chooser_item_attrs_t* attrs,
                      size_t n) const override;

  void idaapi closed() override;
};
}  // namespace forms
