#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <new>

#include "raw_memory.h"


/*
эффективный класс Vector с поддержкой методов:
Resize, PopBack, EmplaceBack, Insert, Emplace, Erase
*/
template <typename T>
class Vector {
public:

    using iterator = T*;
    using const_iterator = const T*;

    Vector() = default;

    // конструктор, создающий вектор заданного размера
    explicit Vector(size_t size)
        : data_(size)
        , size_(size) 
    {
        std::uninitialized_value_construct_n(data_.GetAddress(), size);
    }

    // копирующий конструктор
    Vector(const Vector& other)
        : data_(other.size_)
        , size_(other.size_)
    {
        std::uninitialized_copy_n(other.data_.GetAddress(), size_, data_.GetAddress());
    }

    // перемещающий конструктор
    Vector(Vector&& other) noexcept {
        Swap(other);
    }

    // копирующее присваивание
    Vector& operator=(const Vector& rhs) {
        if (this != &rhs) {
            if (rhs.size_ > data_.Capacity()) { // copy-and-swap только при нехватке места в векторе-приёмнике
                Vector rhs_copy(rhs);
                Swap(rhs_copy);

            } else if (rhs.size_ < size_) { // удалить лишние элементы из вектора-приёмника, присваивать вектор меньшего размера
                std::copy(rhs.data_.GetAddress(), rhs.data_.GetAddress() + rhs.size_, data_.GetAddress());
                std::destroy_n(data_.GetAddress() + rhs.size_, size_ - rhs.size_);

            } else { // скопировать новые элементы
                std::copy(rhs.data_.GetAddress(), rhs.data_.GetAddress() + size_, data_.GetAddress());
                std::uninitialized_copy_n(rhs.data_.GetAddress() + size_, rhs.size_ - size_, data_.GetAddress() + size_);
            }
    
            size_ = rhs.size_;
        }
        return *this;
    }

    // перемещающее присваивание
    Vector& operator=(Vector&& rhs) noexcept {
        if (this != &rhs) {
            Swap(rhs);
        }
        return *this;
    }

    // деструктор уничтожает вектор, высвобождает память
    ~Vector() {
        std::destroy_n(data_.GetAddress(), size_);
    }

    // обмен содержимого вектора с другим вектором
    void Swap(Vector& other) noexcept {
        data_.Swap(other.data_);
    	std::swap(size_, other.size_);
    }

    // текущий размер
    size_t Size() const noexcept {
        return size_;
    }

    // вместимость
    size_t Capacity() const noexcept {
        return data_.Capacity();
    }

    // резервирование памяти под элементы вектора
    void Reserve(size_t new_capacity) {
        if (new_capacity <= data_.Capacity()) { // есть место - не выделяем память
            return;
        }
        RawMemory<T> new_data(new_capacity); // создаём новый блок памяти и релоцируем элементы из старого блока
        ReallocateData(data_.GetAddress(), size_, new_data.GetAddress());
        data_.Swap(new_data);
        // При выходе из метода старая память будет возвращена в кучу
    }

    // изменяет количество элементов в векторе
    void Resize(size_t new_size) {
        if (new_size == size_) { // новый размер равен текущему размеру - ничего не делаем
            return; 
        }

        if (new_size < size_) { // удалить лишние элементы вектора, вызвав их деструкторы, а затем изменить размер вектора
            std::destroy_n(data_.GetAddress() + new_size, size_ - new_size);

        } else if (new_size > size_) { // вызвать метод Reserve при добавлении
            Reserve(new_size);
            std::uninitialized_value_construct_n(data_.GetAddress() + size_, new_size - size_);
        }
        size_ = new_size;
    }

    // добавляет новое значение в конец вектора, реализация на основе EmplaceBack
    void PushBack(const T& value) {
        EmplaceBack(std::forward<const T&>(value));   
    }

    void PushBack(T&& value) {
        EmplaceBack(std::forward<T&&>(std::move(value)));
    }
 
    // разрушает последний элемент вектора и уменьшает размер вектора на единицу
    void PopBack() noexcept {
        assert(size_ > 0);
        std::destroy_at(data_.GetAddress() + size_ - 1);
        --size_;
    }

    // добавляет элемент, возвращает ссылку на добавленный элемент вектора
    template <typename... Args>
    T& EmplaceBack(Args&&... args) {
        if (size_ == Capacity()) { // вектор заполнен - требуется выделить память
            size_t new_size = (size_ == 0) ? 1 : size_ * 2;
            RawMemory<T> new_data(new_size);
            new (new_data + size_) T(std::forward<Args>(args)...);
            try {
                ReallocateData(data_.GetAddress(), size_, new_data.GetAddress());
            } catch (...) {
                std::destroy_at(data_ + size_ - 1);
            }
            data_.Swap(new_data);

        } else { // конструируем в существующей памяти
            new (data_ + size_) T(std::forward<Args>(args)...);
        }

        ++size_;
        return *(data_ + size_ - 1);
    }

    // оператор [] для доступа к элементам вектора
    const T& operator[](size_t index) const noexcept {
        assert(index < size_);
        return const_cast<Vector&>(*this)[index];
    }

    // оператор [] для доступа к элементам вектора
    T& operator[](size_t index) noexcept {
        assert(index < size_);
        return data_[index];
    }

    // возвращает итератор
    iterator begin() noexcept {
        return data_.GetAddress();
    }

    // возвращает итератор, указывающий на конец вектора (на элемент после последнего в векторе)
    iterator end() noexcept {
        return data_.GetAddress() + size_;
    }
    const_iterator begin() const noexcept {
        return const_iterator(data_.GetAddress());
    }

    const_iterator end() const noexcept {
        return const_iterator(data_.GetAddress() + size_);
    }

    const_iterator cbegin() const noexcept {
        return const_iterator(data_.GetAddress());
    }

    const_iterator cend() const noexcept {
        return const_iterator(data_.GetAddress() + size_);
    }

    // возвращает итератор, указывающий на вставленный элемент в новом блоке памяти, реализация на основе Emplace
    iterator Insert(const_iterator pos, const T& value) {
        return Emplace(pos, value);
    }

    iterator Insert(const_iterator pos, T&& value) {
        return Emplace(pos, std::move(value));
    }

    // возвращает итератор, указывающий на вставленный элемент в новом блоке памяти
    template <typename... Args>
    iterator Emplace(const_iterator pos, Args&&... args) {
        assert(pos >= begin() && pos <= end());
        size_t index = static_cast<size_t>(pos - begin());

        if (pos == end()) { // вставка в конец вектора
            EmplaceBack(std::forward<Args>(args)...);
            return begin() + size_ - 1;
        }

        if (size_ < Capacity()) { // вставка в вектор
            T value = T(std::forward<Args>(args)...);
            std::uninitialized_move_n(end() - 1, 1, end());
            try {
                std::move_backward(begin() + index, end() - 1, end());
            } catch (...) {
                std::destroy_at(end() - 1);
            }
            data_[index] = std::move(value);

        } else { // если вектор заполнен
            size_t new_size = (size_ == 0) ? 1 : size_ * 2;
            RawMemory<T> new_data(new_size);
            new (new_data + index) T(std::forward<Args>(args)...);
            try {
                ReallocateData(data_.GetAddress(), index, new_data.GetAddress());
            } catch (...) {
                std::destroy_n(new_data + index, 1);
            }
            try {
                ReallocateData(data_ + index, size_ - index, new_data + index + 1);
            } catch (...) {
                std::destroy_n(new_data.GetAddress(), index + 1);
            }
            data_.Swap(new_data);
        }
        ++size_;
        return begin() + index;
    }

    // удаляет элемент, на который указывает переданный итератор, возвращает итератор, который ссылается на элемент, следующий за удалённым
    iterator Erase(const_iterator pos) {
        assert(pos >= begin() && pos < end());
        size_t index = static_cast<size_t>(pos - begin());
        std::move(begin() + index + 1, end(), begin() + index);
        PopBack();
        return begin() + index;
    }

private:
    RawMemory<T> data_;
    size_t size_ = 0;
    
    /*
    перезаписывает память с учётом строгой гарантии безопасности исключений:
    1. Тип T имеет noexcept-конструктор перемещения.
    2. Тип T не имеет конструктора копирования.
    */
    static void ReallocateData(T* address, size_t count, T* new_address) {
        // constexpr оператор if будет вычислен во время компиляции
        if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
            // Конструируем элементы в new_address, перемещая их из address
            std::uninitialized_move_n(address, count, new_address);
        } else {
            // Копируем элементы в new_address из address
            std::uninitialized_copy_n(address, count, new_address);
        }
        // Разрушаем элементы в address
        std::destroy_n(address, count);
    }

};