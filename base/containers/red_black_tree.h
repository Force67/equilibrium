// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the head_ of this distribution.
//
// A red–black tree is a kind of self-balancing binary search tree. Each node stores
// an extra bit representing "color" ("red" or "black"), used to ensure that the tree
// remains balanced during insertions and deletions. Red–black tree. Type. Tree.
#pragma once

#include <base/arch.h>
#include <base/check.h>
#include <base/memory/move.h>

namespace base {

// template <typename T>
//  do we really want to instantiate the whole tree?
class RedBlackTree {
 public:
  RedBlackTree()
      : head_(nullptr), size_(0) {}

  BASE_NOCOPYMOVE(RedBlackTree);

  struct Node {
    Node() = default;
    Node(const Node&) = delete;

    Node* right{nullptr};
    Node* left{nullptr};
    Node* parent{nullptr};
    bool is_black{false};  // false if red, true if black
    bool is_nil{false};
    int data{};
  };

  enum class State { kRight, kLeft, kUnused };
  struct _Tree_id {
    Node* parent;  // the leaf node under which a new node should be inserted
    State state;
  };

  Node* find(int k) { return SearchTree(head_, k); }

  void insert(int value) {
    Node* n = new Node();
    n->data = value;
    insert(n);
  }

 private:
  void insert(Node* new_node);

  static Node* minimum(Node* node) {
    while (node->left->is_nil)
      node = node->left;
    return node;
  }

  static Node* maximum(Node* node) {
    while (node->right->is_nil)
      node = node->right;
    return node;
  }

  Node* Extract(Node* where);
  Node* Emplace(_Tree_id loc, Node* new_node);

  void LeftRotate(Node* x);

  void RightRotate(Node* x);

  Node* getRoot() { return this->head_; }

  void deleteNode(int data) { deleteNodeHelper(this->head_, data); }

  Node* SearchTree(Node* node, int key) {
    if (node == nullptr || key == node->data) {
      return node;
    }

    if (key < node->data) {
      return SearchTree(node->left, key);
    }
    return SearchTree(node->right, key);
  }

  // For balancing the tree after deletion
  void deleteFix(Node* x);

  void rbTransplant(Node* u, Node* v);

  void deleteNodeHelper(Node* node, int key);

  // For balancing the tree after insertion
  void insertFix(Node* k);

 private:
  Node* head_;     // head_
  mem_size size_;  // _MySize
};

void RedBlackTree::RightRotate(Node* where_node) {
  Node* left_node = where_node->left;
  where_node->left = left_node->right;

  if (!left_node->right->is_nil)
    left_node->right->parent = where_node;

  left_node->parent = where_node->parent;

  if (where_node == head_->parent)
    head_->parent = left_node;
  else if (where_node == where_node->parent->right) {
    where_node->parent->right = left_node;
  } else {
    where_node->parent->left = left_node;
  }

  left_node->right = where_node;
  where_node->parent = where_node;
}

void RedBlackTree::LeftRotate(Node* where_node) {
  Node* right_node = where_node->right;
  where_node->right = right_node->left;

  if (!right_node->left->is_nil)
    right_node->left->parent = where_node;

  right_node->parent = where_node->parent;

  if (where_node == head_->parent)
    head_->parent = right_node;
  else if (where_node == where_node->parent->left)
    where_node->parent->left = right_node;
  else
    where_node->parent->right = right_node;

  right_node->left = where_node;
  where_node->parent = right_node;
}

RedBlackTree::Node* RedBlackTree::Extract(Node* where_node) {
  Node* erased_node = where_node;  // node to erase
  
  // save successor iterator for return
  {
    if (where_node->right->is_nil) {  // climb looking for right subtree
      Node* _Pnode;
      while (!(_Pnode = where_node->parent)->is_nil &&
             where_node == _Pnode->right) {
        where_node = _Pnode;  // ==> parent while right subtree
      }

      where_node = _Pnode;  // ==> parent (head if end())
    } else {
      where_node = minimum(where_node->right);  // ==> smallest of right subtree
    }
  }

  Node* fix_node;        // the node to recolor as needed
  Node* _Fixnodeparent;  // parent of _Fixnode (which may be nil)
  Node* p_node = erased_node;

  if (p_node->left->is_nil) {
    fix_node = p_node->right;  // stitch up right subtree
  } else if (p_node->right->is_nil) {
    fix_node = p_node->left;  // stitch up left subtree
  } else {  // two subtrees, must lift successor node to replace erased
    p_node = where_node;      // _Pnode is successor node
    fix_node = p_node->right;  // _Fixnode is only subtree
  }

  if (p_node == erased_node) {  // at most one subtree, relink it
    _Fixnodeparent = erased_node->parent;
    if (!fix_node->is_nil) {
      fix_node->parent = _Fixnodeparent;  // link up
    }

    if (head_->parent == erased_node) {
      head_->parent = fix_node;  // link down from root
    } else if (_Fixnodeparent->left == erased_node) {
      _Fixnodeparent->left = fix_node;  // link down to left
    } else {
      _Fixnodeparent->right = fix_node;  // link down to right
    }

    if (head_->left == erased_node) {
      head_->left = fix_node->is_nil
                        ? _Fixnodeparent      // smallest is parent of erased node
                        : minimum(fix_node);  // smallest in relinked subtree
    }

    if (head_->right == erased_node) {
      head_->right = fix_node->is_nil
                         ? _Fixnodeparent      // largest is parent of erased node
                         : maximum(fix_node);  // largest in relinked subtree
    }
  } else {  // erased has two subtrees, _Pnode is successor to erased
    erased_node->left->parent = p_node;  // link left up
    p_node->left = erased_node->left;    // link successor down

    if (p_node == erased_node->right) {
      _Fixnodeparent = p_node;  // successor is next to erased
    } else {                    // successor further down, link in place of erased
      _Fixnodeparent = p_node->parent;  // parent is successor's
      if (!fix_node->is_nil) {
        fix_node->parent = _Fixnodeparent;  // link fix up
      }

      _Fixnodeparent->left = fix_node;      // link fix down
      p_node->right = erased_node->right;   // link next down
      erased_node->right->parent = p_node;  // right up
    }

    if (head_->parent == erased_node) {
      head_->parent = p_node;  // link down from root
    } else if (erased_node->parent->left == erased_node) {
      erased_node->parent->left = p_node;  // link down to left
    } else {
      erased_node->parent->right = p_node;  // link down to right
    }

    p_node->parent = erased_node->parent;            // link successor up

    p_node->is_black = erased_node->is_black;
    erased_node->is_black = p_node->is_black;
  }

  if (erased_node->is_black) {  // erasing black link, must recolor/rebalance tree
    for (; fix_node != head_->parent && fix_node->is_black;
         _Fixnodeparent = fix_node->parent) {
      if (fix_node == _Fixnodeparent->left) {  // fixup left subtree
        p_node = _Fixnodeparent->right;
        if (!p_node->is_black) {  // rotate red up from right subtree
          p_node->is_black = true;
          _Fixnodeparent->is_black = false;
          LeftRotate(_Fixnodeparent);
          p_node = _Fixnodeparent->right;
        }

        if (p_node->is_nil) {
          fix_node = _Fixnodeparent;  // shouldn't happen
        } else if (p_node->left->is_black &&
                   p_node->right
                       ->is_black) {  // redden right subtree with black children
          p_node->is_black = false;
          fix_node = _Fixnodeparent;
        } else {                          // must rearrange right subtree
          if (p_node->right->is_black) {  // rotate red up from left sub-subtree
            p_node->left->is_black = true;
            p_node->is_black = false;
            RightRotate(p_node);
            p_node = _Fixnodeparent->right;
          }

          p_node->is_black = _Fixnodeparent->is_black;
          _Fixnodeparent->is_black = true;
          p_node->right->is_black = true;
          LeftRotate(_Fixnodeparent);
          break;  // tree now recolored/rebalanced
        }
      } else {  // fixup right subtree
        p_node = _Fixnodeparent->left;
        if (!p_node->is_black) {  // rotate red up from left subtree
          p_node->is_black = true;
          _Fixnodeparent->is_black = false;
          RightRotate(_Fixnodeparent);
          p_node = _Fixnodeparent->left;
        }

        if (p_node->is_nil) {
          fix_node = _Fixnodeparent;  // shouldn't happen
        } else if (p_node->right->is_black &&
                   p_node->left
                       ->is_black) {  // redden left subtree with black children
          p_node->is_black = false;
          fix_node = _Fixnodeparent;
        } else {                         // must rearrange left subtree
          if (p_node->left->is_black) {  // rotate red up from right sub-subtree
            p_node->right->is_black = true;
            p_node->is_black = false;
            LeftRotate(p_node);
            p_node = _Fixnodeparent->left;
          }

          p_node->is_black = _Fixnodeparent->is_black;
          _Fixnodeparent->is_black = true;
          p_node->left->is_black = true;
          RightRotate(_Fixnodeparent);
          break;  // tree now recolored/rebalanced
        }
      }
    }

    fix_node->is_black = true;  // stopping node is black
  }

  if (0 < size_) {
    --size_;
  }

  return erased_node;
}

inline RedBlackTree::Node* RedBlackTree::Emplace(_Tree_id loc,
                                                 Node* new_node) {
  ++size_;

  const auto head = head_;
  new_node->parent = loc.parent;
  if (loc.parent == head) {  // first node in tree, just set head values
    head->left = new_node;
    head->parent = new_node;
    head->right = new_node;
    new_node->is_black = true;  // the root is black
    return new_node;
  }

  DCHECK(loc.state != State::kUnused);

  if (loc.state == State::kRight) {  // add to right of _Loc.parent
    DCHECK(loc.parent->right->is_nil);
    loc.parent->right = new_node;
    if (loc.parent == head->right) {  // remember rightmost node
      head->right = new_node;
    }
  } else {  // add to left of _Loc.parent
    DCHECK(loc.parent->left->is_nil);
    loc.parent->left = new_node;
    if (loc.parent == head->left) {  // remember leftmost node
      head->left = new_node;
    }
  }

  for (Node* current_node = new_node; current_node->parent->is_black == false;) {
    if (current_node->parent ==
        current_node->parent->parent->left) {  // fixup red-red in left subtree
      const auto parent_sibling = current_node->parent->parent->right;
      if (!parent_sibling
               ->is_black) {  // parent's sibling has two red children, blacken both
        current_node->parent->is_black = true;
        parent_sibling->is_black = true;
        current_node->parent->parent->is_black = false;
        current_node = current_node->parent->parent;
      } else {  // parent's sibling has red and black children
        if (current_node ==
            current_node->parent->right) {  // rotate right child to left
          current_node = current_node->parent;
          LeftRotate(current_node);
        }

        current_node->parent->is_black = true;  // propagate red up
        current_node->parent->parent->is_black = false;
        RightRotate(current_node->parent->parent);
      }
    } else {  // fixup red-red in right subtree
      const auto parent_sibling = current_node->parent->parent->left;
      if (!parent_sibling
               ->is_black) {  // parent's sibling has two red children, blacken both
        current_node->parent->is_black = true;
        parent_sibling->is_black = true;
        current_node->parent->parent->is_black = false;
        current_node = current_node->parent->parent;
      } else {  // parent's sibling has red and black children
        if (current_node ==
            current_node->parent->left) {  // rotate left child to right
          current_node = current_node->parent;
          RightRotate(current_node);
        }

        current_node->parent->is_black = true;  // propagate red up
        current_node->parent->parent->is_black = false;
        LeftRotate(current_node->parent->parent);
      }
    }
  }

  head->parent->is_black = true;  // root is always black
  return new_node;
}

// https://gist.github.com/aagontuk/38b4070911391dd2806f
void RedBlackTree::insert(Node* new_node) {
  ++size_;
  Node* y = nullptr;
  Node* x = this->head_;

  while (x != nullptr) {
    y = x;
    if (new_node->data < x->data) {
      x = x->left;
    } else {
      x = x->right;
    }
  }

  new_node->parent = y;
  if (y == nullptr) {
    head_ = new_node;
  } else if (new_node->data < y->data) {
    y->left = new_node;
  } else {
    y->right = new_node;
  }

  if (new_node->parent == nullptr) {
    new_node->is_black = false;
    return;
  }

  if (new_node->parent->parent == nullptr) {
    return;
  }

  insertFix(new_node);
}

void RedBlackTree::insertFix(Node* k) {
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
          RightRotate(k);
        }
        k->parent->is_black = false;
        k->parent->parent->is_black = true;
        LeftRotate(k->parent->parent);
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
          LeftRotate(k);
        }
        k->parent->is_black = false;
        k->parent->parent->is_black = true;
        RightRotate(k->parent->parent);
      }
    }
    if (k == head_) {
      break;
    }
  }
  head_->is_black = false;
}

void RedBlackTree::rbTransplant(Node* u, Node* v) {
  if (u->parent == nullptr) {
    head_ = v;
  } else if (u == u->parent->left) {
    u->parent->left = v;
  } else {
    u->parent->right = v;
  }
  v->parent = u->parent;
}

void RedBlackTree::deleteFix(Node* x) {
  Node* s;
  while (x != head_ && x->is_black == 0) {
    if (x == x->parent->left) {
      s = x->parent->right;
      if (s->is_black) {
        s->is_black = false;
        x->parent->is_black = true;
        LeftRotate(x->parent);
        s = x->parent->right;
      }

      if (!s->left->is_black && !s->right->is_black) {
        s->is_black = true;
        x = x->parent;
      } else {
        if (!s->right->is_black) {
          s->left->is_black = false;
          s->is_black = true;
          RightRotate(s);
          s = x->parent->right;
        }

        s->is_black = x->parent->is_black;
        x->parent->is_black = false;
        s->right->is_black = false;
        LeftRotate(x->parent);
        x = head_;
      }
    } else {
      s = x->parent->left;
      if (s->is_black) {
        s->is_black = false;
        x->parent->is_black = true;
        RightRotate(x->parent);
        s = x->parent->left;
      }

      if (!s->left->is_black && !s->right->is_black) {
        s->is_black = true;
        x = x->parent;
      } else {
        if (!s->left->is_black) {
          s->right->is_black = false;
          s->is_black = true;
          LeftRotate(s);
          s = x->parent->left;
        }

        s->is_black = x->parent->is_black;
        x->parent->is_black = false;
        s->left->is_black = false;
        RightRotate(x->parent);
        x = head_;
      }
    }
  }
  x->is_black = false;
}

void RedBlackTree::deleteNodeHelper(Node* node, int key) {
  Node* z = nullptr;
  Node *x, *y;
  while (node != nullptr) {
    if (node->data == key) {
      z = node;
    }

    if (node->data <= key) {
      node = node->right;
    } else {
      node = node->left;
    }
  }

  if (z == nullptr) {
    return;
  }

  y = z;
  bool y_original_color = y->is_black;
  if (z->left == nullptr) {
    x = z->right;
    rbTransplant(z, z->right);
  } else if (z->right == nullptr) {
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
  if (!y_original_color /*not black*/) {
    deleteFix(x);
  }
}

}  // namespace base