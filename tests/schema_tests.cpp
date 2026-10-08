#include <gtest/gtest.h>
#include "schema.hpp"
#include "tuple.hpp"

TEST(ConstructionTests, EmptyArgs)
{
    ASSERT_THROW(
      Schema s(std::vector<Field>{}); , std::invalid_argument
    );
}

TEST(ConstructionTests, DuplicateNames)
{
    std::vector<Field> sameTypeSameNames = std::vector<Field>{
        {"Name", FieldType::STRING32},
        {"Name", FieldType::STRING32}
    };

    std::vector<Field> differentTypeSameNames = std::vector<Field>{
        {"Name", FieldType::STRING32},
        {"Name", FieldType::INT64}
    };
    
    ASSERT_THROW(
      Schema s(sameTypeSameNames), std::invalid_argument
    );
    
    ASSERT_THROW(
      Schema s(differentTypeSameNames), std::invalid_argument
    );
}

TEST(ConstructionTests, TooLargeTupleWidth)
{
    // NOTE: If MAX_TUPLE_WIDTH changes, must change this as well!
    std::vector<Field> TooLargeTupleWidth;

    size_t currentWidth = 0;
    size_t typeSize = getFieldTypeSize(FieldType::STRING32);
    size_t nameNumber = 0;
    while(currentWidth <= MAX_TUPLE_WIDTH)
    {
        std::string name = "Name " + nameNumber++;
        TooLargeTupleWidth.push_back({name, FieldType::STRING32});
        currentWidth += typeSize;
    }

    ASSERT_THROW(
        Schema s(TooLargeTupleWidth), std::invalid_argument
    );
}

TEST(ConstructionTests, CorrectConstruction)
{
    std::vector<Field> SchemaField { 
      {"Int64Field", FieldType::INT64}, 
      {"String32Field", FieldType::STRING32} 
    };
  
    ASSERT_NO_THROW(Schema s(SchemaField));
}

TEST(ConstructionTests, ExpectedConstruction)
{
    std::vector<Field> SchemaField { 
        {"Int64Field", FieldType::INT64}, 
        {"String32Field", FieldType::STRING32} 
    };
  
    Schema s(SchemaField);

    std::vector<Field> saveFieldList = s.getFieldList();

    ASSERT_EQ(saveFieldList.size(), SchemaField.size());

    for(std::size_t currentField = 0 ; currentField < saveFieldList.size(); ++currentField)
    {
        EXPECT_EQ(saveFieldList[currentField].name, SchemaField[currentField].name);
        EXPECT_EQ(saveFieldList[currentField].type, SchemaField[currentField].type);
    }
}

TEST(FunctionTests, ValidateTupleList)
{
    std::vector<Field> SchemaField { 
        {"Int64Field", FieldType::INT64}, 
        {"String32Field", FieldType::STRING32} 
    };
    std::vector<TupleValue> TupleField {
      {2},
      {"blah"}
    };
    
    std::vector<TupleValue> TupleField2 {
      {"blah"},
      {2}
    };

    Schema s(SchemaField);
    Tuple t(TupleField);
    Tuple t2(TupleField2);

    ASSERT_TRUE(s.isValidTuple(t.getTupleValues()));
    ASSERT_FALSE(s.isValidTuple(t2.getTupleValues()));
}

TEST(EncodeTests, EncodeTupleList)
{
    std::vector<Field> SchemaField { 
    {"Int64Field", FieldType::INT64}, 
    {"String32Field", FieldType::STRING32} 
    };
    std::vector<TupleValue> TupleField {
      {2},
      {"blah"}
    };

    Schema s(SchemaField);
    Tuple t(TupleField);

    std::vector<std::byte> encodedList;
    for(TupleValue& val : TupleField)
    {
        std::vector<std::byte> encodedTuple = encodeTupleValue(val);
        for(std::byte& b : encodedTuple)
        {
            encodedList.push_back(b);
        }
    }

    std::vector<std::byte> schemaEncode = s.encode(t.getTupleValues());
    
    for(std::size_t i = 0 ; i < encodedList.size(); ++i)
    {
        ASSERT_EQ(schemaEncode[i], encodedList[i]);
    }
}

TEST(DecodeTests, DecodeTupleList)
{

}

TEST(RoundTripTests, EncodeDecodeTupleList)
{
    std::vector<Field> SchemaField { 
    {"Int64Field", FieldType::INT64}, 
    {"String32Field", FieldType::STRING32} 
    };
    std::vector<TupleValue> TupleField {
      {2},
      {"blah"}
    };

    Schema s(SchemaField);
    Tuple t(TupleField);

    std::vector<std::byte> encodedList;
    for(TupleValue& val : TupleField)
    {
        std::vector<std::byte> encodedTuple = encodeTupleValue(val);
        for(std::byte& b : encodedTuple)
        {
            encodedList.push_back(b);
        }
    }

    std::vector<std::byte> schemaEncode = s.encode(t.getTupleValues());
    
    for(std::size_t i = 0 ; i < encodedList.size(); ++i)
    {
        ASSERT_EQ(schemaEncode[i], encodedList[i]);
    }

    std::vector<TupleValue> decodedList;
    size_t b = 0;
    for(size_t i = 0 ; i < SchemaField.size(); ++i)
    {
        if(SchemaField[i].type == FieldType::INT64)
        {
            std::vector<std::byte> subEncode(encodedList.begin() + b, encodedList.begin() + b + 8);
            decodedList.push_back(decodeBytes(subEncode, FieldType::INT64));
            b += 8;
        }
        else if (SchemaField[i].type == FieldType::STRING32)
        {
            std::vector<std::byte> subEncode(encodedList.begin() + b, encodedList.begin() + b + 32);
            decodedList.push_back(decodeBytes(subEncode, FieldType::STRING32));
            b += 32;
        }
    }

    std::vector<TupleValue> schemaDecode = s.decode(schemaEncode);

    for(size_t i = 0 ; i < SchemaField.size(); ++i)
    {
        if(SchemaField[i].type == FieldType::INT64)
        {
            ASSERT_EQ(std::get<std::int64_t>(schemaDecode[i]),std::get<std::int64_t>(decodedList[i]));
        }
        else if (SchemaField[i].type == FieldType::STRING32)
        {
            ASSERT_EQ(std::get<std::string>(schemaDecode[i]), std::get<std::string>(decodedList[i]));
        }
    }

}