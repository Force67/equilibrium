
#include "id_set.h"
#include <gtest/gtest.h>

TEST(IdSetTest, GenerateSequentialIds) {
  base::IdSet id_set;
  EXPECT_EQ(id_set.GenerateId(), 0);
  EXPECT_EQ(id_set.GenerateId(), 1);
  EXPECT_EQ(id_set.GenerateId(), 2);
}

TEST(IdSetTest, ReuseReleasedIds) {
  base::IdSet id_set;
  auto id1 = id_set.GenerateId();
  auto id2 = id_set.GenerateId();
  id_set.ReleaseId(id1);
  EXPECT_EQ(id_set.GenerateId(),
            id1);  // Sollte die freigegebene ID wiederverwenden
  EXPECT_NE(
      id_set.GenerateId(),
      id2);  // Sollte nicht die ID wiederverwenden, die nicht freigegeben wurde
}

TEST(IdSetTest, ReleaseLastGeneratedId) {
  base::IdSet id_set;
  auto id1 = id_set.GenerateId();
  id_set.ReleaseId(id1);
  EXPECT_EQ(id_set.GenerateId(),
            id1);  // Sollte die freigegebene ID wiederverwenden
}

TEST(IdSetTest, GenerateLargeNumberOfIds) {
  base::IdSet id_set;
  for (int i = 0; i < 1000; ++i) {
    EXPECT_EQ(id_set.GenerateId(), i);
  }
}