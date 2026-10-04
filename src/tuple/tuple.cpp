#include "tuple.hpp"

const TupleValue* Tuple::getTupleValue(std::size_t index) const
{
    if(index < 0 || index >= values.size()) return nullptr;

    return &values[index];
}