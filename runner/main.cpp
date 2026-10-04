#include <iostream>
#include "tuple.hpp"
#include "schema.hpp"

int main(int argc, char** argv)
{
    std::cout << "Hello!\n";
    Schema schema({{"Hello", FieldType::STRING32}});
    std::cout << "Created Schema :)\n";
    return 0;
}