// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <QTableWidget>

#include "idb_storage/address_book_data.h"

namespace forms {

class AddressBookViewProvider;

class AddressBookView final : public QAbstractTableModel {
  Q_OBJECT;

 public:
  AddressBookView(AddressBookData&);
  ~AddressBookView();

  void Show(bool toggle);
  void HandleNotification(int code, va_list args);
 private:
  int rowCount(const QModelIndex& parent) const override;
  int columnCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role) const override;

  void Cleanup();
 private:
  TWidget* wrapper_;
  QTableView view_;
  AddressBookData& data_;
};
}  // namespace forms