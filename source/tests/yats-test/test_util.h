#pragma once

#include <cstdint>

struct constructor_counter
{
    constructor_counter() = default;
    constructor_counter(const constructor_counter& rhs)
    {
        copied = rhs.copied + 1;
        moved = rhs.moved;
    }

    constructor_counter(constructor_counter&& rhs)
    {
        copied = rhs.copied;
        moved = rhs.moved + 1;
    }

    uint8_t copied = 0;
    uint8_t moved = 0;
};
