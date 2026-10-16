#include "./table_header.hpp"

TableHeader::TableHeader(std::string& tableName, std::uint16_t tableID, const Schema& tableSchema)
    : tableID(tableID), tableSchema(tableSchema), nextPageNumber(nextPage) 
{
    if(!isValidString32(tableName)) throw std::invalid_argument("tableName not string32!");
    this.tableName = tableName;
}

const std::vector<std::byte> TableHeader::encodeTableHeader()
{
    std::vector<std::byte> tableHeaderBytes(4096, std::byte{0});
    std::vector<Field> fieldList = tableSchema.getFieldList();
    
    std::vector<std::byte> tableNameBytes = encodeTupleValue(tableName);
    std::vector<std::byte> tableIDBytes = encodeIntType(tableID, IntType::INT16);
    std::vector<std::byte> schemaInfoBytes = encodeSchemaField(fieldList);

    std::size_t offset = 0;
    copyBytes(tableHeaderBytes, tableNameBytes, offset);
    copyBytes(tableHeaderBytes, tableIDBytes, offset);
    copyBytes(tableHeaderBytes, schemaInfoBytes, offset);
}


static TableHeader TableHeader::decodeTableHeader(std::vector<std::byte>& tableHeaderBytes)
{
    // NOTE: implement a helper function for reading bytes based on offset & # of bytes read!
    std::size_t offset = 0;
    std::string tableName = decodeBytes( /*Subvector of tableHeaderBytes containing 32 starting from beginning*/, FieldType::STRING32);
    std::uint16_t tableID = decodeIntType( /* */, IntType::INT16);
    std::vector<Field> schemaField = decodeSchemaField(/**/)

    // For schema field we just pass in the rest:
    // First 2 bytes are the # of schema fields
    // Use that to determine bytes to read
    // std::size_t schemaFieldByteSize = 34 * # of fields 


}