// Copyright (C) 2023 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include "linked_list.h"
#include <gtest/gtest.h>

namespace base {
class MyNodeType : public LinkNode<MyNodeType> {
 public:
  explicit MyNodeType(int val) : value(val) {}
  int value;
};

TEST(LinkedListTest, Construct) {
  LinkedList<MyNodeType> list;
  EXPECT_TRUE(list.empty());
}

TEST(LinkedListTest, AppendNodes) {
  LinkedList<MyNodeType> list;
  MyNodeType n1(10), n2(20), n3(30);

  list.Append(&n1);
  list.Append(&n2);
  list.Append(&n3);

  EXPECT_EQ(list.head()->value()->value, 10);
  EXPECT_EQ(list.tail()->value()->value, 30);
  EXPECT_FALSE(list.empty());
}

TEST(LinkedListTest, InsertBeforeAndAfter) {
  LinkedList<MyNodeType> list;
  MyNodeType n1(10), n2(20), n3(30);

  list.Append(&n1);
  n3.InsertBefore(&n1);
  n2.InsertAfter(&n1);

  EXPECT_EQ(list.head()->value()->value, 30);
  EXPECT_EQ(n1.next()->value()->value, 20);
  EXPECT_EQ(list.tail()->value()->value, 10);
}

TEST(LinkedListTest, RemoveFromList) {
  LinkedList<MyNodeType> list;
  MyNodeType n1(10), n2(20), n3(30);

  list.Append(&n1);
  list.Append(&n2);
  list.Append(&n3);
  
  n2.RemoveFromList();
  
  EXPECT_EQ(list.head()->value()->value, 10);
  EXPECT_EQ(list.tail()->value()->value, 30);
}

TEST(LinkedListTest, AccessHeadTailEnd) {
  LinkedList<MyNodeType> list;
  MyNodeType n1(10);
  list.Append(&n1);

  EXPECT_EQ(list.head()->value()->value, 10);
  EXPECT_EQ(list.tail()->value()->value, 10);
  EXPECT_EQ(list.head()->next(), list.end());
}
} // namespace base
