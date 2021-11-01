// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>
#include "grid_layout.h"

namespace ui {
namespace {
TEST(GridLayout, LayoutNoChildren) {
  GridLayout g;
  g.columns(10.f, 20.f, 30.f).rows(20.f, 30.f, 40.f).gap(10.f);

  EXPECT_EQ(g.child_count(), 0);
  EXPECT_EQ(g.column_count(), 3);
  EXPECT_EQ(g.row_count(), 3);

  g.Build();
}

TEST(GridLayout, LayoutWithChildren) {
  GridLayout g;
  // TODO: repair this in the future...
  for (size_t i = 0; i < 6; i++) {
    g.children_.push_back(new Element());
  }

  // just some distinct values
  g.columns(10.f, 20.f, 30.f).rows(60.f, 70.f, 80.f).gap(10.f);

  EXPECT_EQ(g.child_count(), 6);
  EXPECT_EQ(g.column_count(), 3);
  EXPECT_EQ(g.row_count(), 3);

  g.Build();

  // first row bounds
  EXPECT_EQ(g.children_[0]->bounds, SkPoint::Make(10.f, 60.f));
  EXPECT_EQ(g.children_[1]->bounds, SkPoint::Make(20.f, 60.f));
  EXPECT_EQ(g.children_[2]->bounds, SkPoint::Make(30.f, 60.f));

  // second row bounds
  EXPECT_EQ(g.children_[3]->bounds, SkPoint::Make(10.f, 70.f));
  EXPECT_EQ(g.children_[4]->bounds, SkPoint::Make(20.f, 70.f));
  EXPECT_EQ(g.children_[5]->bounds, SkPoint::Make(30.f, 70.f));

  // 3 colums x 3 rows would give 9; leave out 3rd row deliberatly

  // remember; gap = row_gap; column_gap
  EXPECT_EQ(g.children_[0]->screen_pos, SkPoint::Make(0.f, 0.f));
  EXPECT_EQ(g.children_[1]->screen_pos, SkPoint::Make(10.f + 10.f, 0.f));
  EXPECT_EQ(g.children_[2]->screen_pos, SkPoint::Make(10.f + 20.f + 10.f + 10.f, 0.f));
  
  EXPECT_EQ(g.children_[3]->screen_pos, SkPoint::Make(0.f, 60.f + 10.f));
  EXPECT_EQ(g.children_[4]->screen_pos, SkPoint::Make(10.f + 10.f, 60.f + 10.f));
  EXPECT_EQ(g.children_[5]->screen_pos, SkPoint::Make(10.f + 20.f + 10.f + 10.f, 60.f + 10.f));
}
}  // namespace
}  // namespace ui