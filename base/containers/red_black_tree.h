// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// A red–black tree is a kind of self-balancing binary search tree. Each node stores
// an extra bit representing "color" ("red" or "black"), used to ensure that the tree
// remains balanced during insertions and deletions. Red–black tree. Type. Tree.
#pragma once

#include <base/arch.h>

namespace base {

//template <typename T>
// do we really want to instantiate the whole tree?
class RedBlackTree {
 public:
  struct Node {
    Node() = default;
    Node(const Node&) = delete;

    Node* right{nullptr};
    Node* left{nullptr};
    Node* parent{nullptr};
    bool is_black{false};
    int data{};
  };

  RedBlackTree() {
    TNULL = new Node();
    TNULL->is_black = false;
    TNULL->left = nullptr;
    TNULL->right = nullptr;
    root = TNULL;
  }

  void preorder() { preOrderHelper(this->root); }

  void inorder() { inOrderHelper(this->root); }

  void postorder() { postOrderHelper(this->root); }

  Node* searchTree(int k) { return searchTreeHelper(this->root, k); }

  Node* minimum(Node* node) {
    while (node->left != TNULL) {
      node = node->left;
    }
    return node;
  }

  Node* maximum(Node* node) {
    while (node->right != TNULL) {
      node = node->right;
    }
    return node;
  }

  Node* successor(Node* x) {
    if (x->right != TNULL) {
      return minimum(x->right);
    }

    Node* y = x->parent;
    while (y != TNULL && x == y->right) {
      x = y;
      y = y->parent;
    }
    return y;
  }

  Node* predecessor(Node* x) {
    if (x->left != TNULL) {
      return maximum(x->left);
    }

    Node* y = x->parent;
    while (y != TNULL && x == y->left) {
      x = y;
      y = y->parent;
    }

    return y;
  }

  void leftRotate(Node* x) {
    Node* y = x->right;
    x->right = y->left;
    if (y->left != TNULL) {
      y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nullptr) {
      this->root = y;
    } else if (x == x->parent->left) {
      x->parent->left = y;
    } else {
      x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
  }

  void rightRotate(Node* x) {
    Node* y = x->left;
    x->left = y->right;
    if (y->right != TNULL) {
      y->right->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nullptr) {
      this->root = y;
    } else if (x == x->parent->right) {
      x->parent->right = y;
    } else {
      x->parent->left = y;
    }
    y->right = x;
    x->parent = y;
  }

  void insert(int key) {
    Node* node = new Node;
    node->right = nullptr;
    node->left = nullptr;
    node->parent = nullptr;
    node->is_black = true;
    node->data = key;

    Node* y = nullptr;
    Node* x = this->root;

    while (x != TNULL) {
      y = x;
      if (node->data < x->data) {
        x = x->left;
      } else {
        x = x->right;
      }
    }

    node->parent = y;
    if (y == nullptr) {
      root = node;
    } else if (node->data < y->data) {
      y->left = node;
    } else {
      y->right = node;
    }

    if (node->parent == nullptr) {
      node->is_black = false;
      return;
    }

    if (node->parent->parent == nullptr) {
      return;
    }

    insertFix(node);
  }

  Node* getRoot() { return this->root; }

  void deleteNode(int data) { deleteNodeHelper(this->root, data); }

 private:
  // Preorder
  void preOrderHelper(Node* node) {
    if (node != TNULL) {
      preOrderHelper(node->left);
      preOrderHelper(node->right);
    }
  }

  // Inorder
  void inOrderHelper(Node* node) {
    if (node != TNULL) {
      inOrderHelper(node->left);
      inOrderHelper(node->right);
    }
  }

  // Post order
  void postOrderHelper(Node* node) {
    if (node != TNULL) {
      postOrderHelper(node->left);
      postOrderHelper(node->right);
    }
  }

  Node* searchTreeHelper(Node* node, int key) {
    if (node == TNULL || key == node->data) {
      return node;
    }

    if (key < node->data) {
      return searchTreeHelper(node->left, key);
    }
    return searchTreeHelper(node->right, key);
  }

  // For balancing the tree after deletion
  void deleteFix(Node* x) {
    Node* s;
    while (x != root && x->is_black == 0) {
      if (x == x->parent->left) {
        s = x->parent->right;
        if (s->is_black) {
          s->is_black = false;
          x->parent->is_black = true;
          leftRotate(x->parent);
          s = x->parent->right;
        }

        if (!s->left->is_black && !s->right->is_black) {
          s->is_black = true;
          x = x->parent;
        } else {
          if (!s->right->is_black) {
            s->left->is_black = false;
            s->is_black = true;
            rightRotate(s);
            s = x->parent->right;
          }

          s->is_black = x->parent->is_black;
          x->parent->is_black = false;
          s->right->is_black = false;
          leftRotate(x->parent);
          x = root;
        }
      } else {
        s = x->parent->left;
        if (s->is_black) {
          s->is_black = false;
          x->parent->is_black = true;
          rightRotate(x->parent);
          s = x->parent->left;
        }

        if (!s->right->is_black && !s->right->is_black) {
          s->is_black = true;
          x = x->parent;
        } else {
          if (!s->left->is_black) {
            s->right->is_black = false;
            s->is_black = true;
            leftRotate(s);
            s = x->parent->left;
          }

          s->is_black = x->parent->is_black;
          x->parent->is_black = false;
          s->left->is_black = false;
          rightRotate(x->parent);
          x = root;
        }
      }
    }
    x->is_black = false;
  }

  void rbTransplant(Node* u, Node* v) {
    if (u->parent == nullptr) {
      root = v;
    } else if (u == u->parent->left) {
      u->parent->left = v;
    } else {
      u->parent->right = v;
    }
    v->parent = u->parent;
  }

  void deleteNodeHelper(Node* node, int key) {
    Node* z = TNULL;
    Node *x, *y;
    while (node != TNULL) {
      if (node->data == key) {
        z = node;
      }

      if (node->data <= key) {
        node = node->right;
      } else {
        node = node->left;
      }
    }

    if (z == TNULL) {
      return;
    }

    y = z;
    bool y_original_color = y->is_black;
    if (z->left == TNULL) {
      x = z->right;
      rbTransplant(z, z->right);
    } else if (z->right == TNULL) {
      x = z->left;
      rbTransplant(z, z->left);
    } else {
      y = minimum(z->right);
      y_original_color = y->is_black;
      x = y->right;
      if (y->parent == z) {
        x->parent = y;
      } else {
        rbTransplant(y, y->right);
        y->right = z->right;
        y->right->parent = y;
      }

      rbTransplant(z, y);
      y->left = z->left;
      y->left->parent = y;
      y->is_black = z->is_black;
    }
    delete z;
    if (y_original_color == 0) {
      deleteFix(x);
    }
  }

  // For balancing the tree after insertion
  void insertFix(Node* k) {
    Node* u;
    while (k->parent->is_black) {
      if (k->parent == k->parent->parent->right) {
        u = k->parent->parent->left;
        if (u->is_black) {
          u->is_black = false;
          k->parent->is_black = false;
          k->parent->parent->is_black = true;
          k = k->parent->parent;
        } else {
          if (k == k->parent->left) {
            k = k->parent;
            rightRotate(k);
          }
          k->parent->is_black = false;
          k->parent->parent->is_black = true;
          leftRotate(k->parent->parent);
        }
      } else {
        u = k->parent->parent->right;

        if (u->is_black) {
          u->is_black = false;
          k->parent->is_black = false;
          k->parent->parent->is_black = true;
          k = k->parent->parent;
        } else {
          if (k == k->parent->right) {
            k = k->parent;
            leftRotate(k);
          }
          k->parent->is_black = false;
          k->parent->parent->is_black = true;
          rightRotate(k->parent->parent);
        }
      }
      if (k == root) {
        break;
      }
    }
    root->is_black = false;
  }

 private:
  Node* root;
  Node* TNULL;
};

}  // namespace base