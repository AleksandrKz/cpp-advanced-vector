#pragma once
#include <cassert>
#include <cstdlib>
#include <new>
#include <utility>
#include <type_traits>
#include <memory>
#include <iterator>

template <typename T>
class RawMemory;

template <typename T>
class Vector {
public:

    //iterator///////////////////////////////////
    using iterator = T*;
    using const_iterator = const T*;
    iterator begin() noexcept;
    iterator end() noexcept;
    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;
    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;
    //iterator//////////////////////////end//////
    
    Vector() = default;

    explicit Vector(size_t size);
    
    ~Vector();
    
    Vector(const Vector& other);
    
    Vector(Vector&& other) noexcept;

    Vector& operator=(const Vector& rhs);
    
    Vector& operator=(Vector&& rhs) noexcept;

    void Swap(Vector& other) noexcept;
    
    void Reserve(size_t new_capacity);
    
    size_t Size() const noexcept;

    size_t Capacity() const noexcept;

    const T& operator[](size_t index) const noexcept;

    T& operator[](size_t index) noexcept;
    
    void Resize(size_t new_size);
    
    template <typename Val>
    void PushBack(Val&& value);
    
    template <typename... Args>
    T& EmplaceBack(Args&&... args);
    
    template <typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args);
    
    iterator Insert(const_iterator pos, const T& value);
    
    iterator Insert(const_iterator pos, T&& value);
    
    iterator Erase(const_iterator pos); /*noexcept(std::is_nothrow_move_assignable_v<T>)*/
    
    void PopBack() /* noexcept */;

private:
    void MoveData(T* old_b, size_t count, T* new_b);
    
    RawMemory<T> data_;
    size_t size_ = 0;
};


//iterator///////////////////////////////////
template <typename T>
typename Vector<T>::iterator Vector<T>::begin() noexcept {
    return const_cast<iterator>(cbegin());
}

template <typename T>
typename Vector<T>::iterator Vector<T>::end() noexcept {
    return const_cast<iterator>(cend());
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::begin() const noexcept {
    return data_.GetAddress();
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::end() const noexcept {
    return data_.GetAddress() + size_;
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::cbegin() const noexcept {
    return begin();
}

template <typename T>
typename Vector<T>::const_iterator Vector<T>::cend() const noexcept {
    return end();
}
//iterator//////////////////////////end//////


template <typename T>
Vector<T>::Vector(size_t size)
    : data_(size)
    , size_(size) {
    std::uninitialized_value_construct_n(data_.GetAddress(), size);
}
    
template <typename T>
Vector<T>::~Vector() {
    std::destroy_n(data_.GetAddress(), size_);
}

template <typename T>
Vector<T>::Vector(const Vector& other)
    : data_(other.size_)
    , size_(other.size_)
{
    if (this != &other) {
        std::uninitialized_copy_n(other.data_.GetAddress(), other.size_, data_.GetAddress());
    }
}

template <typename T>
Vector<T>::Vector(Vector&& other) noexcept {
    if (this != &other) {
        Swap(other);
    }
}

template <typename T>
Vector<T>& Vector<T>::operator=(const Vector& rhs) {
    if (this != &rhs) {
        if (rhs.size_ > data_.Capacity()) {
            Vector<T> rhs_copy(rhs);
            Swap(rhs_copy);
        } else {
            std::copy(rhs.data_.GetAddress(), rhs.data_.GetAddress() + std::min(size_, rhs.size_), data_.GetAddress());

            if (size_ < rhs.size_) {
                std::uninitialized_copy_n(rhs.data_.GetAddress() + size_, rhs.size_ - size_, data_.GetAddress() + size_);
            } else {
                std::destroy_n(data_.GetAddress() + rhs.size_, size_ - rhs.size_);
            }

            size_ = rhs.size_;
        }
    }

    return *this;
}

template <typename T>
Vector<T>& Vector<T>::operator=(Vector&& rhs) noexcept{
    if (this != &rhs) {
        Swap(rhs);
    }
    return *this;
}

template <typename T>
void Vector<T>::Swap(Vector& other) noexcept {
    data_.Swap(other.data_);
    std::swap(size_, other.size_);
}

template <typename T>
void Vector<T>::Reserve(size_t new_capacity) {
    if (new_capacity <= data_.Capacity()) {
        return;
    }
    RawMemory<T> new_data(new_capacity);
    MoveData(data_.GetAddress(), size_, new_data.GetAddress());
    std::destroy_n(data_.GetAddress(), size_);

    data_.Swap(new_data);
}

template <typename T>
size_t Vector<T>::Size() const noexcept {
    return size_;
}

template <typename T>
size_t Vector<T>::Capacity() const noexcept {
    return data_.Capacity();
}

template <typename T>
const T& Vector<T>::operator[](size_t index) const noexcept {
    return const_cast<Vector&>(*this)[index];
}

template <typename T>
T& Vector<T>::operator[](size_t index) noexcept {
    assert(index < size_);
    return data_[index];
}

template <typename T>
void Vector<T>::MoveData(T* old_b, size_t count, T* new_b) {
    if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
        std::uninitialized_move_n(old_b, count, new_b);
    }
    else {
        std::uninitialized_copy_n(old_b, count, new_b);
    }
}


template <typename T>
void Vector<T>::Resize(size_t new_size) {
    if (new_size > Capacity()) {
        Reserve(new_size);
    }

    if (new_size > size_) {
        std::uninitialized_value_construct_n(data_.GetAddress() + size_, new_size - size_);
    }
    else {
        std::destroy_n(data_.GetAddress() + new_size, size_ - new_size);
    }

    size_ = new_size;
}
    
template <typename T>
template <typename Val>
void Vector<T>::PushBack(Val&& value) {
    EmplaceBack(std::forward<Val>(value));
}

template <typename T>
template <typename... Args>
T& Vector<T>::EmplaceBack(Args&&... args) {
    iterator it = Emplace(end(), std::forward<Args>(args)...);
    return *it;
}

template <typename T>
template <typename... Args>
typename Vector<T>::iterator Vector<T>::Emplace(const_iterator pos, Args&&... args) {
    assert(pos >= begin() && pos <= end());
    size_t offset = pos - cbegin();

    if (size_ == Capacity()) {
        RawMemory<T> new_data(size_ == 0 ? 1 : (size_ * 2));
        new (new_data + offset) T(std::forward<Args>(args)...);

        try {
            MoveData(data_.GetAddress(), offset, new_data.GetAddress());
        } catch (...) {
            std::destroy_n(new_data.GetAddress() + offset, 1);
            throw;
        }

        try {
            MoveData(data_.GetAddress() + offset, size_ - offset, new_data.GetAddress() + (offset + 1));
        } catch (...) {
            std::destroy_n(new_data.GetAddress(), offset + 1);
            throw;
        }

        std::destroy_n(data_.GetAddress(), size_);

        data_.Swap(new_data);
        } 
        else {
            if (pos == cend()) {
                new (data_ + offset) T(std::forward<Args>(args)...);
            } 
            else {
                T temp_val(std::forward<Args>(args)...);

                new (end()) T(std::move(*(end() - 1)));
                std::move_backward(begin() + offset, end() - 1, end());
                data_[offset] = std::move(temp_val);
            }
        }

        ++size_;

        return begin() + offset;
}

template <typename T>
typename Vector<T>::iterator Vector<T>::Insert(const_iterator pos, const T& value) {
    return Emplace(pos, value);
}

template <typename T>
typename Vector<T>::iterator Vector<T>::Insert(const_iterator pos, T&& value) {
    return Emplace(pos, std::move(value));
}

template <typename T>
typename Vector<T>::iterator Vector<T>::Erase(const_iterator pos) {
    assert(pos >= begin() && pos < end());
    size_t offset = pos - cbegin();

    std::move(begin() + offset + 1, end(), begin() + offset);
    std::destroy_n(end() - 1, 1);
    --size_;

    return begin() + offset;
}

template <typename T>
void Vector<T>::PopBack() /* noexcept */ {
    assert(size_ > 0);

    std::destroy_n(data_.GetAddress() + (size_ - 1), 1);
    --size_;
}



////////////////////////////////////////////RawMemory/////////////////////////////////////////////////
template <typename T>
class RawMemory {
public:
    RawMemory() = default;

    explicit RawMemory(size_t capacity);

    ~RawMemory();
    
    RawMemory(const RawMemory&) = delete;
    
    RawMemory(RawMemory&& other) noexcept;
    
    RawMemory& operator=(const RawMemory& rhs) = delete;
    
    RawMemory& operator=(RawMemory&& rhs) noexcept;

    T* operator+(size_t offset) noexcept;

    const T* operator+(size_t offset) const noexcept;

    const T& operator[](size_t index) const noexcept;

    T& operator[](size_t index) noexcept;

    void Swap(RawMemory& other) noexcept;

    const T* GetAddress() const noexcept;

    T* GetAddress() noexcept;

    size_t Capacity() const;

private:
    // Выделяет сырую память под n элементов и возвращает указатель на неё
    static T* Allocate(size_t n);
    // Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
    static void Deallocate(T* buf) noexcept;

    T* buffer_ = nullptr;
    size_t capacity_ = 0;
};

template <typename T>
RawMemory<T>::RawMemory(size_t capacity)
    : buffer_(Allocate(capacity))
    , capacity_(capacity) {
}

template <typename T>
RawMemory<T>::~RawMemory() {
    Deallocate(buffer_);
}

template <typename T>
RawMemory<T>::RawMemory(RawMemory&& other) noexcept {
    *this = std::move(other);
}

template <typename T>
RawMemory<T>& RawMemory<T>::operator=(RawMemory&& rhs) noexcept {
    if (GetAddress() != rhs.GetAddress()) {
        buffer_ = std::exchange(rhs.buffer_, nullptr);
        capacity_ = std::exchange(rhs.capacity_, 0);
    }

    return *this;
}

template <typename T>
T* RawMemory<T>::operator+(size_t offset) noexcept {
    // Разрешается получать адрес ячейки памяти, следующей за последним элементом массива
    assert(offset <= capacity_);
    return buffer_ + offset;
}

template <typename T>
const T* RawMemory<T>::operator+(size_t offset) const noexcept {
    return const_cast<RawMemory&>(*this) + offset;
}
    
template <typename T>
const T& RawMemory<T>::operator[](size_t index) const noexcept {
    return const_cast<RawMemory&>(*this)[index];
}

template <typename T>
T& RawMemory<T>::operator[](size_t index) noexcept {
    assert(index < capacity_);
    return buffer_[index];
}

template <typename T>
void RawMemory<T>::Swap(RawMemory& other) noexcept {
    std::swap(buffer_, other.buffer_);
    std::swap(capacity_, other.capacity_);
}

template <typename T>
const T* RawMemory<T>::GetAddress() const noexcept {
    return buffer_;
}

template <typename T>
T* RawMemory<T>::GetAddress() noexcept {
    return buffer_;
}

template <typename T>
size_t RawMemory<T>::Capacity() const {
    return capacity_;
}

template <typename T>
T* RawMemory<T>::Allocate(size_t n) {
    return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
}

template <typename T>
void RawMemory<T>::Deallocate(T* buf) noexcept {
    operator delete(buf);
}

