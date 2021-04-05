// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <kernwin.hpp>

#include "address_book_view.h"
#include "ui/plugin_ui.h"

namespace forms {

// TODO: clean this shit up
constexpr char kTitleName[] = "Address Book";

constexpr auto kColumCount = 3u;
constexpr int kWidths[kColumCount] = {(8 | CHCOL_HEX), (4 | CHCOL_DEC), 190};

static const char* const kHeader[kColumCount] = {"Address", "Name", "Pattern"};

struct {
  const char* address;
  const char* name;
  const char* pattern;
} g_x[] = {{"1400D5Ad2", "FUNC_Veryimportant", "0F B7 79 ? E8"}, 
    {"1400D5A00", "sub_1400D5A00", "88 4C 24 ? 55"}

};

AddressBookView::AddressBookView()
    : chooser_multi_t(CH_QFTYP_DEFAULT,
                      kColumCount,
                      kWidths,
                      kHeader,
                      kTitleName) {}

const void* AddressBookView::get_obj_id(size_t* len) const {
  *len = sizeof(kTitleName);
  return static_cast<const void*>(kTitleName);
}

bool idaapi AddressBookView::init() {
  return true;
}

size_t idaapi AddressBookView::get_count() const {
  return 2;
}

#include <ida.hpp>
#include <idp.hpp>
#include <loader.hpp>
#include <kernwin.hpp>


void idaapi AddressBookView::get_row(qstrvec_t* cols,
                                     int* icon_,
                                     chooser_item_attrs_t* attrs,
                                     size_t n) const {
  auto& colums = *cols;
  colums[0].sprnt(g_x[n].address);
  colums[1].sprnt(g_x[n].name);
  colums[2].sprnt(g_x[n].pattern);

  //find_widget();
  //see: Sample Qt subwindow sample from ida
  /*
  bool idaapi plugin_ctx_t::run(size_t)
{
  TWidget *g_widget = find_widget("Sample Qt subwindow");
  if ( g_widget == nullptr )
  {
    widget = create_empty_widget("Sample Qt subwindow");
    display_widget(widget, WOPN_DP_TAB|WOPN_RESTORE);
  }
  else
  {
    close_widget(g_widget, WCLS_SAVE);
    widget = nullptr; // make lint happy
  }
  return true;
}
  */
}

void idaapi AddressBookView::closed() {}

}  // namespace forms