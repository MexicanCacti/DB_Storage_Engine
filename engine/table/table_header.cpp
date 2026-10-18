#include "table_header.hpp"

TableHeader::TableHeader(const std::string& tableName, std::int64_t tableID, const Schema& tableSchema)
    : tableID(tableID), tableSchema(tableSchema)
{
    if(!isValidString32(tableName)) throw std::invalid_argument("[TableHeader]: tableName not String32");
    this->tableName = tableName;
}

const std::vector<std::byte> TableHeader::encode()
{
    std::vector<std::byte> tableHeaderBytes(PAGE_BYTE_LENGTH, std::byte{0});
    
    std::vector<std::byte> tableNameBytes = encodeFieldVariant(tableName);
    std::vector<std::byte> tableIDBytes = encodeIntVariant(tableID);
    std::vector<std::byte> schemaInfoBytes = tableSchema.encode();

    std::size_t offset = 0;
    copyBytes(tableHeaderBytes, tableNameBytes, offset);
    copyBytes(tableHeaderBytes, tableIDBytes, offset);
    copyBytes(tableHeaderBytes, schemaInfoBytes, offset);

    return tableHeaderBytes;
}

TableHeader TableHeader::decode(const std::vector<std::byte>& tableHeaderBytes)
{
    if(tableHeaderBytes.size() != PAGE_BYTE_LENGTH) throw std::invalid_argument("[decode]: tableHeaderBytes not size of a page");

    std::size_t offset = 0;
    std::size_t String32Size = getFieldTypeSize(FieldType::STRING32);
    std::vector<std::byte> tableNameBytes(String32Size, std::byte{0});
    copyNBytesSrcOffset(tableNameBytes, tableHeaderBytes, offset, String32Size);
    FieldVariant tableName = decodeFieldVariant(tableNameBytes, FieldType::STRING32);

    std::size_t Int64Size = getIntTypeSize(IntType::INT64);
    std::vector<std::byte> tableIDBytes(Int64Size, std::byte{0});
    copyNBytesSrcOffset(tableIDBytes, tableHeaderBytes, offset, Int64Size);
    IntVariant tableID = decodeIntVariant(tableIDBytes, IntType::INT64);

    std::vector<std::byte> schemaBytes(tableHeaderBytes.size() - offset, std::byte{0});
    copyBytesSrcOffset(schemaBytes, tableHeaderBytes, offset);
    std::vector<Field> tableSchema = Schema::decode(schemaBytes);

    return TableHeader(std::get<std::string>(tableName), std::get<std::int64_t>(tableID), Schema(tableSchema));
}