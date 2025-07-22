#include "gtest/gtest.h"
#include "red_black_tree_2.h"

// Helper function to validate the Red-Black Tree properties recursively
template <typename Node, typename Nil>
int validate_rbtree_properties(Node* node, Nil nil_node) {
  if (node == nil_node) {
    return 1;  // Base case: NIL nodes are black, height is 1
  }

  // Property 2: Root is black (checked in tests)
  // Property 3: All leaves (NIL) are black (guaranteed by sentinel)

  // Property 4: If a node is red, then both its children are black.
  if (node->color == base::RedBlackTree2<int>::RED) {
    EXPECT_EQ(node->left->color, base::RedBlackTree2<int>::BLACK);
    EXPECT_EQ(node->right->color, base::RedBlackTree2<int>::BLACK);
  }

  // Property 5: Every path from a given node to any of its descendant NIL nodes
  // contains the same number of black nodes.
  int left_black_height = validate_rbtree_properties(node->left, nil_node);
  int right_black_height = validate_rbtree_properties(node->right, nil_node);
  EXPECT_EQ(left_black_height, right_black_height);

  // Return black height of the current subtree
  return left_black_height + (node->color == base::RedBlackTree2<int>::BLACK ? 1 : 0);
}

// Main test fixture for our Red-Black Tree
class RedBlackTreeTest : public ::testing::Test {
 protected:
  base::RedBlackTree2<int> tree;
};

TEST_F(RedBlackTreeTest, IsEmptyInitially) {
  ASSERT_TRUE(tree.empty());
  ASSERT_EQ(tree.root(), tree.nil());
}

TEST_F(RedBlackTreeTest, InsertSingleElement) {
  ASSERT_TRUE(tree.Insert(10));
  ASSERT_FALSE(tree.empty());
  ASSERT_EQ(tree.root()->value, 10);
  ASSERT_EQ(tree.root()->color, base::RedBlackTree2<int>::BLACK);  // Root must be black
  ASSERT_TRUE(tree.Search(10));
  ASSERT_FALSE(tree.Search(99));
}

TEST_F(RedBlackTreeTest, InsertDoesNotAllowDuplicates) {
  tree.Insert(10);
  ASSERT_FALSE(tree.Insert(10));  // Second insert should fail
  tree.Insert(20);
  ASSERT_FALSE(tree.Insert(20));
}

TEST_F(RedBlackTreeTest, SimpleInsertAndErase) {
  tree.Insert(10);
  tree.Insert(20);
  ASSERT_TRUE(tree.Search(10));
  ASSERT_TRUE(tree.Search(20));

  ASSERT_TRUE(tree.Erase(10));
  ASSERT_FALSE(tree.Search(10));
  ASSERT_TRUE(tree.Search(20));
  ASSERT_EQ(tree.root()->value, 20);
  ASSERT_EQ(tree.root()->color, base::RedBlackTree2<int>::BLACK);

  ASSERT_TRUE(tree.Erase(20));
  ASSERT_FALSE(tree.Search(20));
  ASSERT_TRUE(tree.empty());
}

TEST_F(RedBlackTreeTest, EraseNonExistentElement) {
  tree.Insert(10);
  ASSERT_FALSE(tree.Erase(99));
  ASSERT_TRUE(tree.Search(10));
}

TEST_F(RedBlackTreeTest, ClearFunction) {
  tree.Insert(10);
  tree.Insert(5);
  tree.Insert(15);
  ASSERT_FALSE(tree.empty());
  tree.Clear();
  ASSERT_TRUE(tree.empty());
  ASSERT_EQ(tree.root(), tree.nil());
  // Can still insert after clearing
  ASSERT_TRUE(tree.Insert(100));
  ASSERT_TRUE(tree.Search(100));
}

// Replace the old test with this new, correct version.
TEST_F(RedBlackTreeTest, InsertionTriggeringCase1Fix) {
  /*
   * This test creates a scenario that specifically triggers Case 1 of the
   * insertion fix-up algorithm (when the new node's uncle is RED).
   *
   * Sequence:
   * 1. Insert 20 -> Becomes root 20(B)
   * 2. Insert 10 -> Becomes left child 10(R)
   * 3. Insert 30 -> Becomes right child 30(R)
   *    Tree is now: 20(B) / \ 10(R) 30(R)
   *
   * 4. Insert 5 -> Inserted as left child of 10(R). Now we have a problem:
   *    - New node k=5 is RED.
   *    - Parent of k (10) is RED. -> Fixup loop begins.
   *    - Grandparent is 20(B).
   *    - Uncle (grandparent's other child) is 30(R). -> UNCLE IS RED. This is Case 1.
   *
   * The fix-up should:
   * - Recolor parent (10) to BLACK.
   * - Recolor uncle (30) to BLACK.
   * - Recolor grandparent (20) to RED.
   * - Move k up to the grandparent (20).
   * - The loop terminates as 20's parent is nil (BLACK).
   * - The final step of FixInsert colors the root (20) back to BLACK.
   */
  tree.Insert(20);
  tree.Insert(10);
  tree.Insert(30);
  tree.Insert(5);  // This insertion triggers the Case 1 fix-up.

  // Final state assertions:
  // Root must be 20 and must be BLACK.
  EXPECT_EQ(tree.root()->value, 20);
  EXPECT_EQ(tree.root()->color, base::RedBlackTree2<int>::BLACK);

  // Parent (10) and Uncle (30) must have been recolored to BLACK.
  EXPECT_EQ(tree.root()->left->value, 10);
  EXPECT_EQ(tree.root()->left->color, base::RedBlackTree2<int>::BLACK);
  EXPECT_EQ(tree.root()->right->value, 30);
  EXPECT_EQ(tree.root()->right->color, base::RedBlackTree2<int>::BLACK);

  // The new node (5) remains RED.
  EXPECT_EQ(tree.root()->left->left->value, 5);
  EXPECT_EQ(tree.root()->left->left->color, base::RedBlackTree2<int>::RED);

  // And finally, validate the entire tree's properties.
  validate_rbtree_properties(tree.root(), tree.nil());
}

TEST_F(RedBlackTreeTest, InsertionTriggeringRotations) {
  // This sequence triggers left and right rotations
  tree.Insert(10);
  tree.Insert(20);
  tree.Insert(30);  // Triggers left rotation on 10, then recoloring

  ASSERT_EQ(tree.root()->value, 20);
  ASSERT_EQ(tree.root()->color, base::RedBlackTree2<int>::BLACK);
  ASSERT_EQ(tree.root()->left->value, 10);
  ASSERT_EQ(tree.root()->left->color, base::RedBlackTree2<int>::RED);
  ASSERT_EQ(tree.root()->right->value, 30);
  ASSERT_EQ(tree.root()->right->color, base::RedBlackTree2<int>::RED);
  validate_rbtree_properties(tree.root(), tree.nil());

  tree.Clear();
  tree.Insert(30);
  tree.Insert(20);
  tree.Insert(10);  // Triggers right rotation
  ASSERT_EQ(tree.root()->value, 20);
  validate_rbtree_properties(tree.root(), tree.nil());
}

TEST_F(RedBlackTreeTest, ComplexInsertionAndDeletionStressTest) {
  // Insert 1 to 10
  for (int i = 1; i <= 10; ++i) {
    ASSERT_TRUE(tree.Insert(i));
  }

  ASSERT_EQ(tree.root()->color, base::RedBlackTree2<int>::BLACK);
  validate_rbtree_properties(tree.root(), tree.nil());

  // Delete in a specific order to trigger various deletion cases
  // Delete 1 (leaf)
  ASSERT_TRUE(tree.Erase(1));
  validate_rbtree_properties(tree.root(), tree.nil());

  // Delete 3 (leaf)
  ASSERT_TRUE(tree.Erase(3));
  validate_rbtree_properties(tree.root(), tree.nil());

  // Delete 5 (leaf)
  ASSERT_TRUE(tree.Erase(5));
  validate_rbtree_properties(tree.root(), tree.nil());

  // Delete 7 (node with one child)
  ASSERT_TRUE(tree.Erase(7));
  validate_rbtree_properties(tree.root(), tree.nil());

  // Delete 9 (leaf)
  ASSERT_TRUE(tree.Erase(9));
  validate_rbtree_properties(tree.root(), tree.nil());

  // Delete root (4)
  ASSERT_TRUE(tree.Erase(4));
  ASSERT_EQ(tree.root()->color, base::RedBlackTree2<int>::BLACK);
  validate_rbtree_properties(tree.root(), tree.nil());

  // Check remaining values
  ASSERT_TRUE(tree.Search(2));
  ASSERT_TRUE(tree.Search(6));
  ASSERT_TRUE(tree.Search(8));
  ASSERT_TRUE(tree.Search(10));
  ASSERT_FALSE(tree.Search(1));
  ASSERT_FALSE(tree.Search(3));
}

TEST_F(RedBlackTreeTest, DeletionOfRootNode) {
  tree.Insert(10);
  tree.Insert(5);
  tree.Insert(15);
  tree.Insert(12);

  ASSERT_EQ(tree.root()->value, 10);
  ASSERT_TRUE(tree.Erase(10));        // Delete the root
  ASSERT_EQ(tree.root()->value, 12);  // Successor becomes new root
  ASSERT_FALSE(tree.Search(10));
  validate_rbtree_properties(tree.root(), tree.nil());
}

// A fuzz test to catch edge cases
TEST_F(RedBlackTreeTest, RandomOperationsFuzzTest) {
  const int num_operations = 5000;
  const int value_range = 1000;

  // Use a simple array as a shadow structure to verify correctness
  bool values_present[value_range] = {false};

  for (int i = 0; i < num_operations; ++i) {
    int val = rand() % value_range;

    // 60% chance to insert, 40% chance to erase
    if (rand() % 10 < 6) {
      bool inserted = tree.Insert(val);
      if (values_present[val]) {
        ASSERT_FALSE(inserted);
      } else {
        ASSERT_TRUE(inserted);
        values_present[val] = true;
      }
    } else {
      bool erased = tree.Erase(val);
      if (values_present[val]) {
        ASSERT_TRUE(erased);
        values_present[val] = false;
      } else {
        ASSERT_FALSE(erased);
      }
    }
  }

  // Final validation
  ASSERT_EQ(tree.root()->color, base::RedBlackTree2<int>::BLACK);
  validate_rbtree_properties(tree.root(), tree.nil());

  for (int i = 0; i < value_range; ++i) {
    ASSERT_EQ(values_present[i], tree.Search(i));
  }
}