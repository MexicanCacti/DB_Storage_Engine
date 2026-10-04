#pragma once

// Contains the values that correspond to its schema. TupleValues order & Schema Field order should correspond 1:1

#include <vector>
#include <variant>
#include <string>
#include "../field/field.hpp"

class Tuple {
    private:
        std::vector<TupleValue> values;
    public:
        Tuple() = default;
        Tuple(const std::vector<TupleValue>& valueList) : values(valueList) {};
        const std::vector<TupleValue>& getTupleValues() const {return values;};
        const TupleValue* getTupleValue(std::size_t index) const;
};