// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the head_ of this distribution.
//
// A red-black tree is a kind of self-balancing binary search tree. Each node
// stores an extra bit representing "color" ("red" or "black"), used to ensure
// that the tree remains balanced during insertions and deletions.
#pragma once

#include <base/memory/move.h>

namespace base {

// Default comparator using '<' and '==' operators.
template <typename T>
struct RBComparator {
  static bool less_than(const T& lhs, const T& rhs) { return lhs < rhs; }
  static bool equals(const T& lhs, const T& rhs) { return lhs == rhs; }
};

template <typename T, typename TComparator = RBComparator<T>>
class RedBlackTree2 {
 public:
  using Comparator = TComparator;

  enum NodeColor { RED, BLACK };

  struct Node {
    T value;
    NodeColor color;
    Node *left, *right, *parent;

    // This constructor is for regular, data-holding nodes. Forwarding keeps
    // move-only T usable.
    template <typename U>
    explicit Node(U&& val)
        : value(base::forward<U>(val)),
          color(RED),
          left(nullptr),
          right(nullptr),
          parent(nullptr) {}
  };

 private:
  Node* root_;
  Node* nil_;  // Sentinel node for all leaves

  // We must manage the memory for the sentinel node manually to avoid
  // requiring T to have a default constructor.
  char* nil_memory_;

  // Rotations
  void RotateLeft(Node* x) {
    Node* y = x->right;
    x->right = y->left;
    if (y->left != nil_) {
      y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nil_) {
      this->root_ = y;
    } else if (x == x->parent->left) {
      x->parent->left = y;
    } else {
      x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
  }

  void RotateRight(Node* x) {
    Node* y = x->left;
    x->left = y->right;
    if (y->right != nil_) {
      y->right->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nil_) {
      this->root_ = y;
    } else if (x == x->parent->right) {
      x->parent->right = y;
    } else {
      x->parent->left = y;
    }
    y->right = x;
    x->parent = y;
  }

  // Insertion Fix-up
  void FixInsert(Node* k) {
    while (k->parent->color == RED) {
      if (k->parent == k->parent->parent->left) {
        Node* u = k->parent->parent->right;  // Uncle
        if (u->color == RED) {
          u->color = BLACK;
          k->parent->color = BLACK;
          k->parent->parent->color = RED;
          k = k->parent->parent;
        } else {
          if (k == k->parent->right) {
            k = k->parent;
            RotateLeft(k);
          }
          k->parent->color = BLACK;
          k->parent->parent->color = RED;
          RotateRight(k->parent->parent);
        }
      } else {
        Node* u = k->parent->parent->left;  // Uncle
        if (u->color == RED) {
          u->color = BLACK;
          k->parent->color = BLACK;
          k->parent->parent->color = RED;
          k = k->parent->parent;
        } else {
          if (k == k->parent->left) {
            k = k->parent;
            RotateRight(k);
          }
          k->parent->color = BLACK;
          k->parent->parent->color = RED;
          RotateLeft(k->parent->parent);
        }
      }
    }
    root_->color = BLACK;
  }

  // Deletion Fix-up
  void FixDelete(Node* x) {
    while (x != root_ && x->color == BLACK) {
      if (x == x->parent->left) {
        Node* w = x->parent->right;  // Sibling
        if (w->color == RED) {
          w->color = BLACK;
          x->parent->color = RED;
          RotateLeft(x->parent);
          w = x->parent->right;
        }
        if (w->left->color == BLACK && w->right->color == BLACK) {
          w->color = RED;
          x = x->parent;
        } else {
          if (w->right->color == BLACK) {
            w->left->color = BLACK;
            w->color = RED;
            RotateRight(w);
            w = x->parent->right;
          }
          w->color = x->parent->color;
          x->parent->color = BLACK;
          w->right->color = BLACK;
          RotateLeft(x->parent);
          x = root_;
        }
      } else {                      // Symmetric case for right child
        Node* w = x->parent->left;  // Sibling
        if (w->color == RED) {
          w->color = BLACK;
          x->parent->color = RED;
          RotateRight(x->parent);
          w = x->parent->left;
        }
        if (w->right->color == BLACK && w->left->color == BLACK) {
          w->color = RED;
          x = x->parent;
        } else {
          if (w->left->color == BLACK) {
            w->right->color = BLACK;
            w->color = RED;
            RotateLeft(w);
            w = x->parent->left;
          }
          w->color = x->parent->color;
          x->parent->color = BLACK;
          w->left->color = BLACK;
          RotateRight(x->parent);
          x = root_;
        }
      }
    }
    x->color = BLACK;
  }

  // Helper to replace subtree rooted at u with subtree rooted at v
  void Transplant(Node* u, Node* v) {
    if (u->parent == nil_) {
      root_ = v;
    } else if (u == u->parent->left) {
      u->parent->left = v;
    } else {
      u->parent->right = v;
    }
    v->parent = u->parent;
  }

  Node* SearchTree(Node* node, const T& value) const {
    if (node == nil_ || Comparator::equals(value, node->value)) {
      return node;
    }
    if (Comparator::less_than(value, node->value)) {
      return SearchTree(node->left, value);
    } else {
      return SearchTree(node->right, value);
    }
  }

  // Recursive deletion for cleanup.
  // This is correct because 'delete node' will automatically invoke the
  // destructor for 'T value' before freeing the node's memory.
  void DeleteTree(Node* node) {
    if (node != nil_) {
      DeleteTree(node->left);
      DeleteTree(node->right);
      delete node;
    }
  }

 public:
  RedBlackTree2() {
    nil_memory_ = new char[sizeof(Node)];
    nil_ = reinterpret_cast<Node*>(nil_memory_);
    nil_->color = BLACK;
    nil_->parent = nil_;
    nil_->left = nil_;
    nil_->right = nil_;
    root_ = nil_;
  }

  ~RedBlackTree2() {
    DeleteTree(root_);
    delete[] nil_memory_;
  }

  Node* root() const { return root_; }
  Node* nil() const { return nil_; }

  Node* Minimum(Node* node) const {
    while (node->left != nil_) {
      node = node->left;
    }
    return node;
  }

  // In-order successor; returns nil() past the maximum. Node pointers are
  // stable across Erase (deletion transplants nodes instead of moving
  // values), which is what makes iteration over this tree possible.
  Node* Successor(Node* node) const {
    if (node->right != nil_) {
      return Minimum(node->right);
    }
    Node* parent = node->parent;
    while (parent != nil_ && node == parent->right) {
      node = parent;
      parent = parent->parent;
    }
    return parent;
  }

  bool empty() const { return root_ == nil_; }

  void Clear() {
    DeleteTree(root_);
    root_ = nil_;
  }

  bool Insert(const T& value) {
    bool inserted = false;
    FindOrInsert(value, &inserted);
    return inserted;
  }

  // Returns the node holding `value`, inserting it first when absent.
  template <typename U>
  Node* FindOrInsert(U&& value, bool* inserted) {
    Node* existing = SearchTree(root_, value);
    if (existing != nil_) {
      *inserted = false;
      return existing;
    }

    Node* node = new Node(base::forward<U>(value));
    node->left = nil_;
    node->right = nil_;

    Node* y = nil_;
    Node* x = this->root_;

    while (x != nil_) {
      y = x;
      if (Comparator::less_than(node->value, x->value)) {
        x = x->left;
      } else {
        x = x->right;
      }
    }

    node->parent = y;
    if (y == nil_) {
      root_ = node;
    } else if (Comparator::less_than(node->value, y->value)) {
      y->left = node;
    } else {
      y->right = node;
    }

    FixInsert(node);
    *inserted = true;
    return node;
  }

  bool Erase(const T& value) {
    Node* z = SearchTree(root_, value);
    if (z == nil_) {
      return false;
    }

    Node* y = z;
    Node* x;
    NodeColor originalColor = y->color;

    if (z->left == nil_) {
      x = z->right;
      Transplant(z, z->right);
    } else if (z->right == nil_) {
      x = z->left;
      Transplant(z, z->left);
    } else {
      y = Minimum(z->right);
      originalColor = y->color;
      x = y->right;
      if (y->parent == z) {
        x->parent = y;
      } else {
        Transplant(y, y->right);
        y->right = z->right;
        y->right->parent = y;
      }
      Transplant(z, y);
      y->left = z->left;
      y->left->parent = y;
      y->color = z->color;
    }

    // THIS IS THE FIX: We simply delete the node. The 'delete' operator
    // handles calling the destructor for us. No manual call is needed.
    delete z;

    if (originalColor == BLACK) {
      FixDelete(x);
    }
    return true;
  }

  Node* Find(const T& value) const { return SearchTree(root_, value); }

  bool Contains(const T& value) const { return SearchTree(root_, value) != nil_; }

  bool Search(const T& value) const { return Contains(value); }
};
}  // namespace base