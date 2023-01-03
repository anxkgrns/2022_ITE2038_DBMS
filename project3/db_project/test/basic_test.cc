#include <gtest/gtest.h>
#include <cmath>
#include <vector>

int Factorial(int n) {
  if (n == 0) {
    return 1;
  }

  int result = 1;
  for (int i = 2; i <= n; i++) {
    result *= i;
  }
  return result;
}

bool IsPrime(int n) {
  if (n < 2) {
    return false;
  }

  for (int i = 2; i <= sqrt(n); i++) {
    if (n % i == 0) {
      return false;
    }
  }
  return true;
}

// Tests factorial of 0
TEST(FactorialTest, HandlesZeroInput) {
  // Tests if the value of Factorial(0) is EQ(equal) to 1
  EXPECT_EQ(Factorial(0), 1);
}

// Tests factorial of positive numbers
TEST(FactorialTest, HandlesPositiveInput) {
  EXPECT_EQ(Factorial(1), 1);
  EXPECT_EQ(Factorial(3), 6);
  EXPECT_EQ(Factorial(8), 40320);
}

// Tests if 1 is a prime or not
TEST(PrimeTest, HandlesOneInput) {
  // 1 is not a prime
  EXPECT_FALSE(IsPrime(1));
}

// Tests if a given positive number is a prime or not
TEST(PrimeTest, HandlesPositiveInput) {
  EXPECT_TRUE(IsPrime(2));
  EXPECT_TRUE(IsPrime(3));
  EXPECT_FALSE(IsPrime(4));
  EXPECT_TRUE(IsPrime(5));
}

// Assertion
TEST(VectorTest, HasSameContent) {
  std::vector<int> x = {1, 2, 3, 4, 5};
  std::vector<int> y = {1, 2, 3, 4, 5};
  std::vector<int> z = {};

  // You can also provide custom failure message.
  // If the two vectors have different length,
  // the given stream(e.g. string) is printed as an error.
  ASSERT_EQ(x.size(), y.size()) << "Vectors x and y are of unequal length";

  // Tests if the contents of the vectors are the same.
  for (int i = 0; i < x.size(); ++i) {
    EXPECT_EQ(x[i], y[i]) << "Vectors x and y differ at index" << i;
  }

  // Failing case
  // ASSERT_EQ(x.size(), z.size()) << "Vectors x and y are of unequal length";
}

class IntVector {
 public:
  void Push(int x) { v.push_back(x); }

  void Pop(void) { v.pop_back(); }

  int Front(void) { return v.front(); }

  int Back(void) { return v.back(); }

  int At(int idx) { return v.at(idx); }

  size_t size() const { return v.size(); }

 private:
  std::vector<int> v;
};

class IntVectorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    v1_.Push(1);
    v1_.Push(2);
    v2_.Push(1);
    v2_.Push(2);
  }

  void TearDown() override {}

  IntVector v1_;
  IntVector v2_;
};

TEST_F(IntVectorTest, HasSameContent) {
  int v1_size = v1_.size();
  int v2_size = v2_.size();

  EXPECT_EQ(v1_size, v2_size);

  for (int i = 0; i < v1_size; ++i) {
    EXPECT_EQ(v1_.At(i), v2_.At(i)) << "Vectors v1 and v2 differ at index" << i;
  }
}

TEST_F(IntVectorTest, IsNotEmptyInitialily) {
  EXPECT_NE(v1_.size(), 0);
  EXPECT_NE(v2_.size(), 0);
}
