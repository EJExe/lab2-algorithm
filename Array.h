#ifndef LAB2_ARRAY_H
#define LAB2_ARRAY_H
#include <cassert>
#include <cstdlib>
#include <new>
#include <cmath>
#include <utility>

template <typename T>
class Array final {
    static constexpr int DEFAULT_CAPACITY = 8;
    static constexpr double GROWTH_FACTOR = 1.6;
    T* data_ = nullptr;
    int size_ = 0;
    int capacity_ = 0;

    static T* allocate(int n) {
        void* p = std::malloc(sizeof(T) * static_cast<std::size_t>(n));
        if (!p) throw std::bad_alloc();
        return static_cast<T*>(p);
    }

    static void destroy(T* p, int n) {
        for (int i = 0; i < n; ++i) p[i].~T();
        std::free(p);
    }

    int nextCapacity() const {
        int n = static_cast<int>(std::ceil(capacity_ * GROWTH_FACTOR));
        return n > capacity_ ? n : capacity_ + 1;
    }
    void reallocate(int n) {
        T* fresh = allocate(n);
        int copied = 0;
        try {
            for (; copied < size_; ++copied) {
                new (fresh + copied) T(std::move_if_noexcept(data_[copied]));
            }
        } catch (...) {
            destroy(fresh, copied);
            throw;
        }
        destroy(data_, size_);
        data_ = fresh;
        capacity_ = n;
    }
    void checkIndex([[maybe_unused]] int index) const {
        assert(index >= 0 && index < size_);
    }

public:
    class ConstIterator {
    protected:
        const Array* array_;
        int index_;
        int step_;
        ConstIterator(const Array* a, int i, int step)
            : array_(a), index_(i), step_(step) {}
        friend class Array;
    public:
        const T& get() const { return (*array_)[index_]; }
        void next() { index_ += step_; }
        const T& operator*() const { return get(); }
        ConstIterator& operator++() { next(); return *this; }
        ConstIterator operator++(int) { ConstIterator copy = *this; next(); return copy; }
        bool operator!=(const ConstIterator& other) const { return index_ != other.index_; }
        bool operator==(const ConstIterator& other) const { return index_ == other.index_; }
        bool hasNext() const { return step_ > 0 ? index_ < array_->size_ : index_ >= 0; }
    };
    class Iterator : public ConstIterator {
        Array* mutable_array_;
        Iterator(Array* a, int i, int step) : ConstIterator(a, i, step), mutable_array_(a) {}
        friend class Array;
    public:
        using ConstIterator::operator*;
        void set(const T& value) { (*mutable_array_)[this->index_] = value; }
        T& operator*() { return (*mutable_array_)[this->index_]; }
        Iterator& operator++() { this->next(); return *this; }
        Iterator operator++(int) { Iterator copy = *this; this->next(); return copy; }
        bool operator!=(const Iterator& other) const { return this->index_ != other.index_; }
        bool operator==(const Iterator& other) const { return this->index_ == other.index_; }
    };

    Array() : Array(DEFAULT_CAPACITY) {}
    explicit Array(int capacity) {
        assert(capacity > 0);
        capacity_ = capacity;
        data_ = allocate(capacity_);
    }

    ~Array() { destroy(data_, size_); }

    Array(const Array& other)
        : Array(other.capacity_ > 0 ? other.capacity_ : DEFAULT_CAPACITY) {
        for (int i = 0; i < other.size_; ++i) {
            new (data_ + i) T(other.data_[i]);
            ++size_;
        }
    }

    Array(Array&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    Array& operator=(const Array& other) {
        if (this != &other) {
            Array copy(other);
            swap(copy);
        }
        return *this;
    }

    Array& operator=(Array&& other) noexcept {
        if (this != &other) {
            destroy(data_, size_);
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    void swap(Array& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
    int insert(const T& value) {
        return insert(size_, value);
    }
    int insert(int index, const T& value) {
        assert(index >= 0 && index <= size_);
        T saved(value);
        if (size_ == capacity_) {
            reallocate(nextCapacity());
        }
        if (index == size_) {
            new (data_ + size_) T(std::move(saved));
        } else {
            new (data_ + size_) T(std::move_if_noexcept(data_[size_ - 1]));
            for (int i = size_ - 1; i > index; --i) {
                data_[i] = std::move_if_noexcept(data_[i - 1]);
            }
            data_[index] = std::move(saved);
        }
        ++size_;
        return index;
    }
    void remove(int index) {
        checkIndex(index);
        for (int i = index; i + 1 < size_; ++i) {
            data_[i] = std::move_if_noexcept(data_[i + 1]);
        }
        data_[size_ - 1].~T();
        --size_;
    }
    const T& operator[](int index) const {
        checkIndex(index);
        return data_[index];
    }
    T& operator[](int index) {
        checkIndex(index);
        return data_[index];
    }
    int size() const {
        return size_;
    }
    Iterator iterator() { return Iterator(this, 0, 1); }
    ConstIterator iterator() const { return ConstIterator(this, 0, 1); }
    Iterator reverseIterator() { return Iterator(this, size_ - 1, -1); }
    ConstIterator reverseIterator() const { return ConstIterator(this, size_ - 1, -1); }
    Iterator begin() { return iterator(); }
    ConstIterator begin() const { return iterator(); }
    ConstIterator cbegin() const { return iterator(); }
    Iterator end() { return Iterator(this, size_, 1); }
    ConstIterator end() const { return ConstIterator(this, size_, 1); }
    ConstIterator cend() const { return ConstIterator(this, size_, 1); }
};
#endif // !LAB2_ARRAY_H
