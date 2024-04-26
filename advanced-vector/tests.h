#pragma once

#include <cassert>
#include <iostream>
#include <numeric>

#include "vector.h"

void BasicFunctionalityTests() {
    Vector<int> v;

    // Size, Capacity, Empty
    assert(v.Size() == 0);
    assert(v.Capacity() == 0);
    assert(v.Empty());

    // Reserve
    v.Reserve(10);
    assert(v.Capacity() == 10);
    assert(v.Size() == 0);
    assert(v.Empty());
    v.Reserve(5);
    assert(v.Capacity() == 10);
    assert(v.Size() == 0);
    assert(v.Empty());

    // PushBack
    v.PushBack(42);
    assert(v.Size() == 1);
    assert(!v.Empty());
    v.PushBack(56);
    assert(v.Size() == 2);
    assert(!v.Empty());

    // By index
    assert(v[0] == 42);
    assert(v[1] == 56);

    // By At
    assert(v.At(0) == 42);
    assert(v.At(1) == 56);

    // At out of range
    bool out_of_range = false;
    try {
        v.At(2);
    } catch (...) {
        out_of_range = true;
    }
    assert(out_of_range);

    // begin() and front()
    assert(*v.begin() == 42);
    assert(v.front() == 42);
    v.front() += 8;
    assert(v.front() == 50);
    v.front() -= 8;

    // end() and back()
    assert(*(v.end() - 1) == 56);
    assert(v.back() == 56);
    v.back() += 4;
    assert(v.back() == 60);
    v.back() -= 4;
    assert(*v.rbegin() == 56);
    assert(*(v.rend() - 1) == 42);

    // Resize
    v.Resize(3);
    assert(v.Size() == 3);
    assert(v.Capacity() == 10);
    assert(!v.Empty());
    assert(v[0] == 42);
    assert(v[1] == 56);
    assert(v[2] == 0);
    v.Resize(5);
    assert(v.Size() == 5);
    assert(v.Capacity() == 10);
    assert(!v.Empty());
    assert(v[0] == 42);
    assert(v[1] == 56);
    assert(v[2] == 0);
    assert(v[3] == 0);
    assert(v[4] == 0);

    // Erase, Clear
    v.Erase(v.begin());
    assert(v.Size() == 4);
    v.Erase(v.end()-1);
    assert(v.Size() == 3);
    v.Erase(v.begin());
    assert(v.Size() == 2);
    v.Erase(v.end()-1);
    assert(v.Size() == 1);
    assert(!v.Empty());
    v.Clear();
    assert(v.Empty());

    // rbegin()
    v.PushBack(1);
    v.PushBack(2);
    v.PushBack(3);
    auto rbegin = v.rbegin();
    assert(*rbegin == 3);
    ++rbegin;
    assert(*rbegin == 2);
    ++rbegin;
    assert(*rbegin == 1);
    assert(*rbegin == *(v.rend() - 1));
    assert(rbegin == v.rend() - 1);

    // rend()
    auto rend = v.rend();
    --rend;
    assert(*rend == 1);
    --rend;
    assert(*rend == 2);
    --rend;
    assert(*rend == 3);
    assert(rend == v.rbegin());

    // EmplaceBack
    v.Resize(0);
    assert(v.Empty());
    v.EmplaceBack(17);
    assert(v.Size() == 1);
    assert(v[0] == 17);
    v.EmplaceBack(13);
    assert(v.Size() == 2);
    assert(v[1] == 13);

    // Copy Constructor
    Vector<int> v_copy = v;
    assert(v_copy.Size() == v.Size());
    assert(v_copy[0] == v[0]);
    assert(v_copy[1] == v[1]);

    // Move Constructor
    Vector<int> v_moved = std::move(v);
    assert(v_moved.Size() == 2);
    assert(v_moved[0] == 17);
    assert(v_moved[1] == 13);

    // Copy Assignment Operator
    Vector<int> v_copy_assigned;
    v_copy_assigned = v_copy;
    assert(v_copy_assigned.Size() == v_copy.Size());
    assert(v_copy_assigned[0] == v_copy[0]);
    assert(v_copy_assigned[1] == v_copy[1]);

    // Move Assignment Operator
    Vector<int> v_moved_assigned;
    v_moved_assigned = std::move(v_moved);
    assert(v_moved_assigned.Size() == 2);
    assert(v_moved_assigned[0] == 17);
    assert(v_moved_assigned[1] == 13);

    // iterator functionality
    int sum = 0;
    for (auto it = v_copy_assigned.begin(); it != v_copy_assigned.end(); ++it) {
        sum += *it;
    }
    assert(sum == 30);

    // const_iterator functionality
    sum = 0;
    const Vector<int>& const_v = v_copy_assigned;
    for (auto it = const_v.cbegin(); it != const_v.cend(); ++it) {
        sum += *it;
    }
    assert(sum == 30);

    int accumulate_sum = std::accumulate(v_copy_assigned.cbegin(), v_copy_assigned.cend(), 0);
    assert(accumulate_sum == 30);

    // Reverse iterator functionality
    int reverse_sum = 0;
    for (auto rit = v_copy_assigned.rbegin(); rit != v_copy_assigned.rend(); ++rit) {
        reverse_sum += *rit;
    }
    assert(reverse_sum == 30);

    // Const reverse iterator functionality
    int const_reverse_sum = 0;
    for (auto crit = const_v.rbegin(); crit != const_v.rend(); ++crit) {
        const_reverse_sum += *crit;
    }
    assert(const_reverse_sum == 30);

    std::cerr << "Vector: basic functionality tests passed successfully!" << std::endl;
}