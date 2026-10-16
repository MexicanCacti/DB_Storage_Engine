#pragma once
#include <vector>
#include <cstddef>
#include <limits>

#include "basic_types.hpp"
#include "../helpers/helpers.hpp"

// 32 Byte for field name, 1 Byte for type
const std::size_t fieldEncodeLength = 33;

std::vector<std::byte> encodeFieldVariant(const FieldVariant& fieldVariant);

std::vector<std::byte> encodeIntVariant(const IntVariant& intVariant);

std::byte encodeFieldType(const FieldType fieldType);

std::byte encodeIntType(const IntType intType);

std::vector<std::byte> encodeField(const Field& field);

std::vector<std::byte> encodeFieldList(const std::vector<Field>& fieldList);


FieldVariant decodeFieldVariant(const std::vector<std::byte>& bytes, FieldType fieldType);

IntVariant decodeIntVariant(const std::vector<std::byte>& bytes, IntType intType);

FieldType decodeFieldType(const std::byte& byte);

IntType decodeIntType(const std::byte& byte);

Field decodeField(const std::vector<std::byte>& bytes);

std::vector<Field> decodeFieldList(const std::vector<std::byte>& bytes);