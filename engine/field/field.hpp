#pragma once

// This header file defines the valid field types of a column

#include <cstddef> // byte type
#include <cstdint> // int64_t type
#include <string>
#include <variant>
#include <vector>
#include <cstring> // Byte array
#include <bitset>
#include <stdexcept>
#include <algorithm>

// INT64 & STRING32... Add as needed
using TupleValue = std::variant<std::int64_t, std::string>;
using SupportedIntType = std::variant<std::int64_t, std::int32_t, std::int16_t>;

// Guarenteed enum classes are 1 byte in size
enum class FieldType : int8_t{
    INT64 = 0,
    STRING32 = 1
};

enum class IntType : int8_t{
    INT64 = 0,
    INT32 = 1,
    INT16 = 2
};

struct Field {
    std::string name;
    FieldType type;
};


void appendBytes(std::vector<std::byte>& dest, std::vector<std::byte>& src);

void copyBytes(std::vector<std::byte>& dest, std::vector<std::byte>& src);

std::size_t getIntTypeSize(IntType intType);

std::size_t getFieldTypeSize(FieldType fieldType);

bool isValidString32(const std::string& str);

std::vector<std::byte> encodeFieldType(const FieldType& fieldType);

std::vector<std::byte> encodeIntType(const SupportedIntType& intValue);

std::vector<std::byte> encodeTupleValue(const TupleValue& tupleValue);

std::vector<std::byte> encodeSchemaField(const std::vector<Field>& schemaField);

SupportedIntType decodeIntType(const std::vector<std::byte>& bytes, IntType intType);

TupleValue decodeBytes(const std::vector<std::byte>& bytes, FieldType fieldType);

std::vector<Field> decodeSchemaField(const std::vector<std::byte>& bytes);

FieldType decodeFieldType(std::byte& byte);