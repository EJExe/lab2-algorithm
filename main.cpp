#include "Array.h"

#include <iostream>

int main() {
    Array<int> array;

    for (int i = 0; i < 10; ++i) {
        array.insert(i + 1);
    }

    std::cout << "Исходный массив:\n";
    for (auto iterator = array.iterator(); iterator.hasNext(); iterator.next()) {
        std::cout << iterator.get() << '\n';
    }

    for (int i = 0; i < array.size(); ++i) {
        array[i] *= 2;
    }

    std::cout << "\nМассив после умножения на 2:\n";
    for (auto iterator = array.iterator(); iterator.hasNext(); iterator.next()) {
        std::cout << iterator.get() << '\n';
    }

    return 0;
}
