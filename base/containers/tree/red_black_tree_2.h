// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the head_ of this distribution.
//
// A red–black tree is a kind of self-balancing binary search tree. Each node
// stores an extra bit representing "color" ("red" or "black"), used to ensure
// that the tree remains balanced during insertions and deletions. Red–black
// tree. Type. Tree.
#pragma once

namespace base {
template <typename T>
class RedBlackTree2 {
 public:
  enum NodeColor { RED, BLACK };
  struct Node {
    T value;
    NodeColor color;
    Node *left, *right, *parent;

    Node(T value)
        : value(value),
          color(RED),
          left(nullptr),
          right(nullptr),
          parent(nullptr) {}
  };

 private:
  Node* root_;

  void RotateLeft(Node* x) {
    Node* y = x->right;
    x->right = y->left;
    if (y->left != nullptr) {
      y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nullptr) {
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
    if (y->right != nullptr) {
      y->right->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nullptr) {
      this->root_ = y;
    } else if (x == x->parent->right) {
      x->parent->right = y;
    } else {
      x->parent->left = y;
    }
    y->right = x;
    x->parent = y;
  }

  void FixInsert(Node* k) {
    while (k != root_ && k->parent->color == RED) {
      if (k->parent == k->parent->parent->left) {
        Node* u = k->parent->parent->right;  // Uncle node
        if (u != nullptr && u->color == RED) {
          // Case 1: Uncle is red
          u->color = BLACK;
          k->parent->color = BLACK;
          k->parent->parent->color = RED;
          k = k->parent->parent;
        } else {
          // Cases 2 and 3: Uncle is black
          if (k == k->parent->right) {
            k = k->parent;
            RotateLeft(k);
          }
          k->parent->color = BLACK;
          k->parent->parent->color = RED;
          RotateRight(k->parent->parent);
        }
      } else {
        Node* u = k->parent->parent->left;  // Uncle node
        if (u != nullptr && u->color == RED) {
          // Case 1: Uncle is red
          u->color = BLACK;
          k->parent->color = BLACK;
          k->parent->parent->color = RED;
          k = k->parent->parent;
        } else {
          // Cases 2 and 3: Uncle is black
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

  void FixDelete(Node* x) {
    while (x != root_ && x->color == BLACK) {
      if (x == x->parent->left) {
        Node* w = x->parent->right;
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
      } else {
        // Similar code for the right child
      }
    }
    x->color = BLACK;
  }


  void Transplant(Node* u, Node* v) {
    if (u->parent == nullptr) {
      root_ = v;
    } else if (u == u->parent->left) {
      u->parent->left = v;
    } else {
      u->parent->right = v;
    }
    if (v != nullptr) {
      v->parent = u->parent;
    }
  }

  Node* Minimum(Node* node) const {
    while (node->left != nullptr) {
      node = node->left;
    }
    return node;
  }

  Node* SearchTree(Node* node, const T& value) const {
    if (node == nullptr || value == node->value) {
      return node;
    }

    if (value < node->value) {
      return SearchTree(node->left, value);
    } else {
      return SearchTree(node->right, value);
    }
  }

  void DeleteTree(Node* node) {
    if (node != nullptr) {
      DeleteTree(node->left);
      DeleteTree(node->right);
      delete node;
    }
  }

 public:
  RedBlackTree2() : root_(nullptr) {}
  ~RedBlackTree2() { DeleteTree(root_); }

  Node* root() const { return root_; }

  bool empty() const { return root_ == nullptr; }

  void Insert(const T& value) {
    Node* node = new Node(value);
    node->parent = nullptr;
    node->value = value;
    node->left = nullptr;
    node->right = nullptr;
    node->color = RED;

    Node* y = nullptr;
    Node* x = this->root_;

    while (x != nullptr) {
      y = x;
      if (node->value < x->value) {
        x = x->left;
      } else {
        x = x->right;
      }
    }

    node->parent = y;
    if (y == nullptr) {
      root_ = node;
    } else if (node->value < y->value) {
      y->left = node;
    } else {
      y->right = node;
    }

    if (node->parent == nullptr) {
      node->color = BLACK;
      return;
    }

    if (node->parent->parent == nullptr) {
      return;
    }

    FixInsert(node);
  }

  bool Erase(const T& value) {
    Node* nodeToDelete = SearchTree(root_, value);
    if (!nodeToDelete)
      return false;  // Node not found

    Node* x;
    Node* y = nodeToDelete;  // Temporary pointer y
    NodeColor originalColor = y->color;

    // If the node to be deleted has fewer than two children:
    if (!nodeToDelete->left) {
      x = nodeToDelete->right;
      Transplant(nodeToDelete, nodeToDelete->right);
    } else if (!nodeToDelete->right) {
      x = nodeToDelete->left;
      Transplant(nodeToDelete, nodeToDelete->left);
    } else {
      // Node has two children, find its in-order successor
      y = Minimum(nodeToDelete->right);
      originalColor = y->color;
      x = y->right;

      if (y->parent == nodeToDelete) {
        if (x)
          x->parent = y;
      } else {
        Transplant(y, y->right);
        y->right = nodeToDelete->right;
        y->right->parent = y;
      }

      Transplant(nodeToDelete, y);
      y->left = nodeToDelete->left;
      y->left->parent = y;
      y->color = nodeToDelete->color;
    }

    delete nodeToDelete;

    if (originalColor == BLACK) {
      // Fix up the tree
      FixDelete(x);
    }
    return true;
  }

  bool Search(const T& value) const {
    Node* result = SearchTree(root_, value);
    return result != nullptr;
  }
};
}  // namespace base