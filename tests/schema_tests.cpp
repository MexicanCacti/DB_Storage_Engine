#include <gtest/gtest.h>
#include "schema.hpp"

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
    while(currentWidth < MAX_TUPLE_WIDTH)
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
  
}

TEST(EncodeTests, EncodeTupleList)
{
 
}

TEST(DecodeTests, DecodeTupleList)
{

}

TEST(RoundTripTests, EncodeDecodeTupleList)
{

}
/*
TEST(ConstructionTests, EncodeTuple)
{

}
*/