#include "Array.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

struct Tracked {
    static int alive;

    int value;

    explicit Tracked(int value) : value(value) { ++alive; }
    Tracked(const Tracked& other) : value(other.value) { ++alive; }
    Tracked(Tracked&& other) noexcept : value(other.value) { ++alive; }
    Tracked& operator=(const Tracked&) = default;
    Tracked& operator=(Tracked&&) noexcept = default;
    ~Tracked() { --alive; }
};

int Tracked::alive = 0;

TEST(ArrayTest, InsertsAndRemoves) {
    Array<int> array(1);

    EXPECT_EQ(array.insert(2), 0);
    EXPECT_EQ(array.insert(0, 1), 0);
    EXPECT_EQ(array.insert(2, 3), 2);
    EXPECT_EQ(array.size(), 3);

    array.remove(2);
    EXPECT_EQ(array[0], 1);
    EXPECT_EQ(array[1], 2);

    array.remove(1);
    array.remove(0);
    EXPECT_EQ(array.size(), 0);
}

TEST(ArrayTest, ReallocatesAndPreservesValues) {
    Array<int> array(2);

    for (int i = 0; i < 100; ++i) {
        array.insert(i);
    }

    ASSERT_EQ(array.size(), 100);
    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(array[i], i);
    }
}

TEST(ArrayTest, NonTrivialMiddleInsertion) {
    Array<std::string> array;
    array.insert("one");
    array.insert("three");
    array.insert(1, "two");

    EXPECT_EQ(array[0], "one");
    EXPECT_EQ(array[1], "two");
    EXPECT_EQ(array[2], "three");
}

TEST(ArrayTest, CopyAndMoveConstructorsAndAssignments) {
    Array<std::string> original;
    original.insert("one");
    original.insert("two");

    Array<std::string> copy(original);
    copy[0] = "changed";
    EXPECT_EQ(original[0], "one");

    Array<std::string> moved(std::move(copy));
    EXPECT_EQ(moved[0], "changed");

    Array<std::string> copyOfMoved(copy);
    EXPECT_EQ(copyOfMoved.size(), 0);

    Array<std::string> assigned;
    assigned = original;
    EXPECT_EQ(assigned[1], "two");

    Array<std::string> moveAssigned;
    moveAssigned = std::move(assigned);
    EXPECT_EQ(moveAssigned[1], "two");
}

TEST(ArrayTest, DestroysAllStoredObjects) {
    Tracked::alive = 0;
    {
        Array<Tracked> array(2);
        for (int i = 0; i < 12; ++i) {
            array.insert(Tracked(i));
        }
        EXPECT_EQ(Tracked::alive, 12);
        array.remove(0);
        EXPECT_EQ(Tracked::alive, 11);
    }
    EXPECT_EQ(Tracked::alive, 0);
}

TEST(ArrayTest, IteratorsAndConstIterator) {
    Array<int> array;
    array.insert(1);
    array.insert(2);
    array.insert(3);

    std::vector<int> forward;
    for (auto iterator = array.iterator(); iterator.hasNext(); iterator++) {
        forward.push_back(*iterator);
    }
    EXPECT_EQ(forward, (std::vector<int>{1, 2, 3}));

    std::vector<int> backward;
    for (auto iterator = array.reverseIterator(); iterator.hasNext(); iterator.next()) {
        backward.push_back(iterator.get());
    }
    EXPECT_EQ(backward, (std::vector<int>{3, 2, 1}));

    const Array<int>& constArray = array;
    auto constIterator = constArray.iterator();
    EXPECT_EQ(constArray[0], constIterator.get());
    EXPECT_EQ(*constIterator, 1);

    auto iterator = array.iterator();
    iterator.set(10);
    EXPECT_EQ(array[0], 10);
    *iterator = 20;
    EXPECT_EQ(array[0], 20);

    Array<int> empty;
    EXPECT_FALSE(empty.iterator().hasNext());
    EXPECT_FALSE(empty.reverseIterator().hasNext());
}

TEST(ArrayTest, AliasingInsertIsSafe) {
    Array<int> array(1);
    array.insert(42);
    array.insert(array[0]);

    EXPECT_EQ(array[0], 42);
    EXPECT_EQ(array[1], 42);

    Array<int> middle;
    middle.insert(1);
    middle.insert(2);
    middle.insert(0, middle[1]);
    EXPECT_EQ(middle[0], 2);
    EXPECT_EQ(middle[1], 1);
    EXPECT_EQ(middle[2], 2);
}

TEST(ArrayTest, RangeForWorksForMutableConstAndEmptyArrays) {
    Array<int> array;
    array.insert(1);
    array.insert(2);
    array.insert(3);

    for (auto& value : array) {
        value *= 2;
    }
    EXPECT_EQ(array[0], 2);
    EXPECT_EQ(array[1], 4);
    EXPECT_EQ(array[2], 6);

    const Array<int>& constArray = array;
    int sum = 0;
    for (const int& value : constArray) {
        sum += value;
    }
    EXPECT_EQ(sum, 12);

    Array<int> empty;
    int count = 0;
    for (int value : empty) {
        count += value;
    }
    EXPECT_EQ(count, 0);
}
