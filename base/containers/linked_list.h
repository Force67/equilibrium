// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include "base/export.h"

// Intrusive doubly linked list. The stored type derives from LinkNode<T>, so
// the list needs no per-node heap allocations. Prefer std::list unless erase
// performance or allocation freedom matters: erase is O(1) here versus O(n)
// for std::list<T*>.
//
//   class MyNodeType : public LinkNode<MyNodeType> {};
//   LinkedList<MyNodeType> list;
//   list.Append(node);
//   node->InsertBefore(other);
//   for (LinkNode<MyNodeType>* n = list.head(); n != list.end(); n = n->next())

namespace base {

namespace internal {

// Base class for LinkNode<T> type
class BASE_EXPORT LinkNodeBase {
 public:
  void RemoveFromList();

 protected:
  LinkNodeBase();
  LinkNodeBase(LinkNodeBase* previous, LinkNodeBase* next);
  LinkNodeBase(LinkNodeBase&& rhs);
  LinkNodeBase(const LinkNodeBase&) = delete;
  ~LinkNodeBase() = default;

  LinkNodeBase& operator=(const LinkNodeBase&) = delete;

  // Calling these with |e| as a different LinkNode type as |this| is
  // unsafe. These are protected and only called from LinkNode<T> to
  // ensure safety.
  void InsertBeforeBase(LinkNodeBase* e);
  void InsertAfterBase(LinkNodeBase* e);

  LinkNodeBase* previous_base() const { return previous_; }
  LinkNodeBase* next_base() const { return next_; }

 private:
  LinkNodeBase* previous_ = nullptr;
  LinkNodeBase* next_ = nullptr;
};

}  // namespace internal

template <typename T>
class LinkNode : public internal::LinkNodeBase {
 public:
  LinkNode() = default;
  LinkNode(LinkNode<T>* previous, LinkNode<T>* next)
      : internal::LinkNodeBase(previous, next) {}

  LinkNode(LinkNode<T>&&) = default;

  LinkNode(const LinkNode&) = delete;
  LinkNode& operator=(const LinkNode&) = delete;

  // Insert |this| into the linked list, before |e|. |this| must not
  // already be in a list.
  void InsertBefore(LinkNode<T>* e) { InsertBeforeBase(e); }

  // Insert |this| into the linked list, after |e|. |this| must not
  // already be in a list.
  void InsertAfter(LinkNode<T>* e) { InsertAfterBase(e); }

  LinkNode<T>* previous() const { return static_cast<LinkNode<T>*>(previous_base()); }

  LinkNode<T>* next() const { return static_cast<LinkNode<T>*>(next_base()); }

  // Cast from the node-type to the value type.
  const T* value() const { return static_cast<const T*>(this); }

  T* value() { return static_cast<T*>(this); }
};

template <typename T>
class LinkedList {
 public:
  // The "root" node is self-referential, and forms the basis of a circular
  // list (root_.next() will point back to the start of the list,
  // and root_->previous() wraps around to the end of the list).
  LinkedList() : root_(&root_, &root_) {}
  LinkedList(const LinkedList&) = delete;
  LinkedList& operator=(const LinkedList&) = delete;

  // Appends |e| to the end of the linked list.
  void Append(LinkNode<T>* e) { e->InsertBefore(&root_); }

  LinkNode<T>* head() const { return root_.next(); }

  LinkNode<T>* tail() const { return root_.previous(); }

  const LinkNode<T>* end() const { return &root_; }

  bool empty() const { return head() == end(); }

 private:
  LinkNode<T> root_;
};

}  // namespace base