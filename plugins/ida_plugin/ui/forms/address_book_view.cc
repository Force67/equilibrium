// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <QHBoxLayout>
#include "address_book_view.h"

namespace forms {

constexpr char kViewTitle[] = "Address Book";

struct {
  const char* address;
  const char* name;
  const char* pattern;
} g_fakeData[] = {{"1400D5AD2", "FUNC_Veryimportant", "0F B7 79 ? E8"},
                  {"1400D5A00", "sub_1400D5A00", "88 4C 24 ? 55"}

};

AddressBookView::AddressBookView(AddressBookData& data)
    : data_(data),
      wrapper_(create_empty_widget(kViewTitle)) {
  auto* parent = reinterpret_cast<QWidget*>(wrapper_);

  auto* layout = new QHBoxLayout(parent);
  layout->addWidget(&view_);

  QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(view_.sizePolicy().hasHeightForWidth());

  view_.setModel(this);
  view_.setColumnWidth(0, view_.width() / 3);
  view_.setColumnWidth(1, view_.width() / 2);
  view_.setColumnWidth(2, view_.width() / 1);
}

AddressBookView::~AddressBookView() {
  __debugbreak();
  if (wrapper_) {
    Show(false);
  }
}

void AddressBookView::HandleNotification(int code, va_list args) {
  if (code == ui_widget_invisible) {
    if (wrapper_ == va_arg(args, TWidget*)) {


    }
  }
}

void AddressBookView::Cleanup() {

  view_.close();
}

constexpr int kxxx = WOPN_DP_TAB;

void AddressBookView::Show(bool toggle) {
  toggle ? display_widget(wrapper_, WOPN_DP_TAB)
         : close_widget(wrapper_, WOPN_DP_TAB);
}

int AddressBookView::rowCount(const QModelIndex& parent) const {
  return 2;
}

int AddressBookView::columnCount(const QModelIndex& parent) const {
  return 3;
}

QVariant AddressBookView::headerData(int section,
                                     Qt::Orientation orientation,
                                     int role) const {
  if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
    return QVariant();

  switch (section) {
    case 0:
      return QObject::tr("Address");
    case 1:
      return QObject::tr("Name");
    case 2:
      return QObject::tr("Pattern");
    default:
      return QVariant();
  }
}

QVariant AddressBookView::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || role != Qt::DisplayRole) {
    return QVariant();
  }

  auto& node = g_fakeData[index.row()];

  switch (index.column()) {
    case 0:
      return node.address;
    case 1:
      return node.name;
    case 2:
      return node.pattern;
    default:
      return QVariant();
  }
}

}  // namespace forms