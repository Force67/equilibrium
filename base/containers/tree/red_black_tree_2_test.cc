#include <gtest/gtest.h>
#include "red_black_tree_2.h"

namespace {
// Test fixture for RedBlackTree
class RedBlackTree2Test : public ::testing::Test {
 protected:
  base::RedBlackTree2<int> tree;

  int CheckBlackHeight(base::RedBlackTree2<int>::Node* node) {
    if (node == nullptr) {
      return 1;  // Null nodes are black by definition
    }

    int leftHeight = CheckBlackHeight(node->left);
    int rightHeight = CheckBlackHeight(node->right);

    if (leftHeight == 0 || rightHeight == 0 || leftHeight != rightHeight) {
      return 0;  // Unbalanced or heights are different
    }

    return leftHeight +
           (node->color == base::RedBlackTree2<int>::NodeColor::BLACK ? 1 : 0);
  }

  bool CheckTreeBalance(base::RedBlackTree2<int>::Node* root) {
    return CheckBlackHeight(root) != 0;
  }

    // Check if a node is red

  bool IsRed(const base::RedBlackTree2<int>::Node* node) const {
    return node != nullptr &&
           node->color == base::RedBlackTree2<int>::NodeColor::RED;
  }
    // Recursively verify Red-Black properties and compute black height
  bool VerifyProperties(const base::RedBlackTree2<int>::Node* node,
                        int& blackHeight,
                        int currentBlackHeight = 0) const {
    if (node == nullptr) {
      // Base case: Reached a leaf node (null). Set black height for this path.
      blackHeight = currentBlackHeight;
      return true;
    }

    // Increment black height if the current node is black
    if (!IsRed(node)) {
      currentBlackHeight++;
    }

    int leftBlackHeight = 0, rightBlackHeight = 0;

    // Check left subtree
    if (!VerifyProperties(node->left, leftBlackHeight, currentBlackHeight)) {
      return false;
    }

    // Check right subtree
    if (!VerifyProperties(node->right, rightBlackHeight, currentBlackHeight)) {
      return false;
    }

    // Check for red violation (two consecutive red nodes)
    if (IsRed(node) && (IsRed(node->left) || IsRed(node->right))) {
      return false;
    }

    // Check for consistent black height in subtrees
    if (leftBlackHeight != rightBlackHeight) {
      return false;
    }

    // Propagate the black height up for the parent to check
    blackHeight = leftBlackHeight;
    return true;
  }

  bool CheckPropertiesWrapper() const {
    if (tree.empty()) {
      return true;  // An empty tree is a valid Red-Black Tree
    }

    // The root must be black (property 2)
    if (IsRed(tree.root())) {
      return false;
    }

    int blackHeight = 0;
    return VerifyProperties(tree.root(), blackHeight);
  }
};

TEST_F(RedBlackTree2Test, InsertTest) {
  tree.Insert(10);
  tree.Insert(20);
  tree.Insert(5);
  // Check if elements are inserted
  EXPECT_TRUE(tree.Search(10));
  EXPECT_TRUE(tree.Search(20));
  EXPECT_TRUE(tree.Search(5));
  // Check if the tree maintains Red-Black properties
  EXPECT_TRUE(CheckPropertiesWrapper());
}

TEST_F(RedBlackTree2Test, StructureTest) {
  // Add elements to the tree
  tree.Insert(30);
  tree.Insert(15);
  tree.Insert(60);

  // Check if the tree maintains Red-Black properties
  EXPECT_TRUE(CheckPropertiesWrapper());
  // 1. Check if the root is black
  EXPECT_FALSE(IsRed(tree.root()));
  // 2. Check if the tree is balanced in terms of black height
  EXPECT_TRUE(CheckTreeBalance(tree.root()));
}

TEST_F(RedBlackTree2Test, BalancingTest) {
  // Insert elements in a way that requires tree balancing
  tree.Insert(5);
  tree.Insert(3);
  tree.Insert(4);  // This should trigger some rotations
  EXPECT_TRUE(CheckTreeBalance(tree.root()));
}

// Test for searching non-existent elements
TEST_F(RedBlackTree2Test, SearchNonExistent) {
  tree.Insert(10);
  tree.Insert(20);
  tree.Insert(30);
  EXPECT_FALSE(tree.Search(40));
}

TEST_F(RedBlackTree2Test, EraseTest) {
  // Insert some elements
  tree.Insert(10);
  tree.Insert(20);
  tree.Insert(30);
  tree.Insert(40);
  tree.Insert(50);

  // Remove an element and check if it's correctly deleted
  tree.Erase(30);
  EXPECT_FALSE(tree.Search(30));

  // Check if the tree maintains Red-Black properties after deletion
  EXPECT_TRUE(CheckPropertiesWrapper());
}

TEST_F(RedBlackTree2Test, EraseRootTest) {
  // Insert elements
  tree.Insert(25);
  tree.Insert(15);
  tree.Insert(30);

  // Remove the root
  tree.Erase(25);
  EXPECT_FALSE(tree.Search(25));

  // Check if the tree maintains Red-Black properties after deleting root
  EXPECT_TRUE(CheckPropertiesWrapper());
}

TEST_F(RedBlackTree2Test, EraseRedNodeTest) {
  // Insert elements
  tree.Insert(40);
  tree.Insert(30);
  tree.Insert(50);
  tree.Insert(35);

  // Remove a red node
  tree.Erase(35);
  EXPECT_FALSE(tree.Search(35));

  // Check if the tree maintains Red-Black properties after deleting a red node
  EXPECT_TRUE(CheckPropertiesWrapper());
}

TEST_F(RedBlackTree2Test, EraseBlackNodeTest) {
  // Insert elements
  tree.Insert(50);
  tree.Insert(30);
  tree.Insert(70);
  tree.Insert(20);
  tree.Insert(40);

  // Remove a black node
  tree.Erase(20);
  EXPECT_FALSE(tree.Search(20));

  // Check if the tree maintains Red-Black properties after deleting a black
  // node
  EXPECT_TRUE(CheckPropertiesWrapper());
}

TEST_F(RedBlackTree2Test, EraseNonexistentTest) {
  // Insert elements
  tree.Insert(10);
  tree.Insert(20);
  tree.Insert(30);

  // Try to remove a non-existent element
  tree.Erase(100);
  EXPECT_TRUE(CheckPropertiesWrapper());
}
}  // namespace