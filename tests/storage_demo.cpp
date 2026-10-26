#include "database.cpp"
#include <iostream>
#include <sstream>
#include <windows.h>
std::string directoryString = "./storage";
std::string tableName = "students";
int retVal = 0;
RID aliceRID;
RID bobRID;

// Windows, gets path to executable
std::filesystem::path getExecutablePath()
{
    std::wstring buffer(32768, L'\0');

    DWORD length = GetModuleFileNameW(
        nullptr,
        buffer.data(),
        static_cast<DWORD>(buffer.size())
    );

    if(length == 0 || length >= buffer.size())
        throw std::runtime_error("Failed to retrieve executable path");

    buffer.resize(length);

    return std::filesystem::path(buffer);
}

bool compareRID(RID& r1, RID& r2)
{
    if(r1.pageID.tableID != r2.pageID.tableID || r1.pageID.pageNumber != r2.pageID.pageNumber || r1.tupleSlot != r2.tupleSlot) {retVal = -1; return false;}
    return true;
}

bool compareTupleIndex(Tuple& t1, Tuple& t2, std::size_t tupleIndex)
{
    FieldVariant expectedFieldValue = t1.getTupleValueIndex(tupleIndex);
    FieldVariant readFieldValue = t2.getTupleValueIndex(tupleIndex);

    if(expectedFieldValue != readFieldValue)
    {
        std::cout << "Field [" << tupleIndex << "] value mismatch\n";
        retVal = -1;
        return false;
    }

    return true;
}

bool step_one(Database& database);

bool step_two(Database& database);

bool step_three(Database& database);

bool step_four(Database& database);

bool step_five(Database& database);

bool step_six(Database& database);

bool step_seven(Database& database);

int main(int argc, char* argv[])
{
    
    if(argc == 1)
    {
        std::filesystem::path directoryPath = directoryString;
        std::filesystem::remove_all(directoryString);
        Database database(directoryPath);

        std::cout << "\t---Step 1 Test---";
        std::cout << "\t" << ((step_one(database)) ? "[Pass" : "[Fail") << "]\n";

        std::cout << "\t---Step 2 Test---";
        std::cout << "\t" << ((step_two(database)) ? "[Pass" : "[Fail") << "]\n";

        std::cout << "\t---Step 3 Test---";
        std::cout << "\t" << ((step_three(database)) ? "[Pass" : "[Fail") << "]\n";

        std::cout << "\t---Step 4 Test---";
        std::cout << "\t" << ((step_four(database)) ? "[Pass" : "[Fail") << "]\n";

        std::cout << "\t---Step 5 Test---";
        std::cout << "\t" << ((step_five(database)) ? "[Pass" : "[Fail") << "]\n";

        std::cout << "\t---Step 6 Test---";
        std::cout << "\t" << ((step_six(database)) ? "[Pass" : "[Fail") << "]\n";
        database.flushAllPages();

        std::cout << "Starting new process for Step 7 Test...\n";
        std::filesystem::path exePath = getExecutablePath();

        std::ostringstream command;

        command << "\"" << exePath.string() << "\" "
                << aliceRID.pageID.tableID << " "
                << aliceRID.pageID.pageNumber << " "
                << aliceRID.tupleSlot << " "
                << bobRID.pageID.tableID << " "
                << bobRID.pageID.pageNumber << " "
                << bobRID.tupleSlot;

        int retVal = std::system(command.str().c_str());
        return retVal;
    }
    else if(argc == 7) // [] A's{ TableID, PageNumber, SlotNumber} B's {TableID, PageNumber, SlotNumber}  .... these 6 args passed in as integers
    {
        std::filesystem::path directoryPath = directoryString;
        Database database(directoryPath);
        aliceRID = { { std::stoll(argv[1]), std::stoll(argv[2])}, static_cast<std::int16_t>(std::stoll(argv[3])) };
        bobRID = { {std::stoll(argv[4]), std::stoll(argv[5])}, static_cast<std::int16_t>(std::stoll(argv[6])) };

        std::cout << "\t---Step 7 Test---";
        std::cout << "\t" << ((step_seven(database)) ? "[Pass" : "[Fail") << "]\n";

        return retVal;
    }

    return 0;
}

/*
    Creates a fresh table named students
    Checks stored schema is correct
    Checks no tuples are in the table
*/
bool step_one(Database& database)
{
    Schema schema({
    {"id", FieldType::INT64},
    {"name", FieldType::STRING32}
    });

    database.createTable(tableName, schema);

    TableHeader tableHeader = database.queryTableInfo(tableName);
    Schema storedSchema = tableHeader.getSchema();

    std::cout << "\n\tComparing stored vs expected Schema info\n";

    if(storedSchema.getTupleWidth() != schema.getTupleWidth())
    {
        std::cout << "Stored Schema tuple width [" << storedSchema.getTupleWidth() << "] Expected: [" << schema.getTupleWidth() << "]\n";
        retVal = -1;
        return false;
    }

    if( storedSchema.getFieldList().size() !=  schema.getFieldList().size())
    {
        std::cout << "Stored Schema Field Count [" << storedSchema.getFieldList().size() << "] Expect: [" << schema.getFieldList().size() << "]\n";
        retVal = -1;
        return false;
    }
    std::cout << "\n\tComparing Field List\n";
    for(std::size_t index = 0; index < storedSchema.getFieldList().size(); ++index)
    {
        Field storedSchemaField = storedSchema.getField(index);
        Field schemaField = schema.getField(index);
        
        std::string storedSchemaName = storedSchemaField.name;
        std::string schemaName = schemaField.name;

        FieldType storedSchemaType = storedSchemaField.type;
        FieldType schemaType = schemaField.type;

        if(storedSchemaName != schemaName || storedSchemaType != schemaType)
        {
            std::cout << "Stored Name[" << storedSchemaName << "]: " << " Expected: [" << schemaName << "]\n";
            std::cout << "Stored Type[" << static_cast<int>(storedSchemaType) << "]: " << " Expected: [" << static_cast<int>(schemaType) << "]\n";
            retVal = -1;
            return false;
        }

    }

    std::cout << "\n\tRunning table scan\n";
    std::vector<std::pair<Tuple, RID>> scanResults = database.scan(tableName);
    if(scanResults.size() != 0)
    {
        std::cout << "Scan results size: " << scanResults.size() << ", Expected 0\n";
        retVal = -1;

        return false;
    }

    return true;
}

/*
    Insert (1, "Alice") and save the returned RID. Read that RID. 
    The read returns exactly (1, "Alice"). 
*/
bool step_two(Database& database)
{
    std::vector<FieldVariant> tupleValues{
    std::int64_t{1},
    std::string{"Alice"}
    };

    Tuple tuple(tupleValues);

    RID tupleRID = database.insertTuple(tableName, tuple);
    aliceRID = tupleRID;
    TableHeader tableHeader = database.queryTableInfo(tableName);
    std::int64_t tableID = tableHeader.getTableID();
    RID expectedRID{ {tableID, 1}, 0};

    std::cout << "\n\tComparing Returned vs Expected RIDs\n";
    if(!compareRID(tupleRID, expectedRID))
    {
        std::cout << "Returned - { " << " { " << tupleRID.pageID.tableID << ", " << tupleRID.pageID.pageNumber << "}, " << tupleRID.tupleSlot << "} ";
        std::cout << "Expected - { " << " { " << expectedRID.pageID.tableID << ", " << expectedRID.pageID.pageNumber << "}, " << expectedRID.tupleSlot << "}\n";
        retVal = -1;
        retVal = -1;
        return false;
    }

    

    std::cout << "\n\tReading with returned RID\n";

    Tuple readTuple = database.readTuple(tableName, tupleRID);

    std::cout << "\n\tComparing Read vs Expected Tuple contents\n";
    if(tuple.getTupleValues().size() != readTuple.getTupleValues().size())
    {
        std::cout << "Read TupleValuesList Size [" << readTuple.getTupleValues().size() << "], Expected [" << tuple.getTupleValues().size() << "]\n";
        retVal = -1;
        retVal = -1;
        return false;
    }

    for(std::size_t index = 0; index < tuple.getTupleValues().size(); ++index)
    {
        FieldVariant expectedFieldValue = tuple.getTupleValueIndex(index);
        FieldVariant readFieldValue = readTuple.getTupleValueIndex(index);

        if(expectedFieldValue != readFieldValue)
        {
            std::cout << "Field [" << index << "] value mismatch\n";
            retVal = -1;
            return false;
        }
    }

    return true;
}

/*
    Insert (2, "Bob") and save the returned RID. Scan the table. 
    The scan returns exactly the two inserted tuples, with no duplicates. 
*/

bool step_three(Database& database)
{
    std::vector<std::vector<FieldVariant>> tupleValues
    {
        {
            std::int64_t{1},
            std::string{"Alice"}
        },
        {
            std::int64_t{2},
            std::string{"Bob"}
        }

    };


    Tuple tuple(tupleValues[1]);

    RID tupleRID = database.insertTuple(tableName, tuple);
    bobRID = tupleRID;
    TableHeader tableHeader = database.queryTableInfo(tableName);
    std::int64_t tableID = tableHeader.getTableID();

    std::vector<RID> expectedRIDList
    {
        RID{ {tableID, 1}, 0},
        RID{ {tableID, 1}, 1}
    };


    std::cout << "\n\tRunning table scan\n";
    std::vector<std::pair<Tuple, RID>> scanResults = database.scan(tableName);
    if(scanResults.size() != tupleValues.size())
    {
        std::cout << "Scan results size: " << scanResults.size() << ", Expected 0\n";
        retVal = -1;
        return false;
    }

    for(std::size_t index = 0 ; index < scanResults.size(); ++index)
    {
        Tuple resultTuple = scanResults[index].first;
        RID tupleRID = scanResults[index].second;

        Tuple expectedTuple = Tuple(tupleValues[index]);
        RID expectedRID = expectedRIDList[index];

        if(!compareRID(tupleRID, expectedRID))
        {
            std::cout << "RID mismatch\n";
            std::cout << "Returned - { " << " { " << tupleRID.pageID.tableID << ", " << tupleRID.pageID.pageNumber << "}, " << tupleRID.tupleSlot << "} ";
            std::cout << "Expected - { " << " { " << expectedRID.pageID.tableID << ", " << expectedRID.pageID.pageNumber << "}, " << expectedRID.tupleSlot << "}\n";
            retVal = -1;
            return false;
        }
        for(std::size_t tupleIndex = 0; tupleIndex < expectedTuple.getTupleValues().size(); ++tupleIndex)
        {
            if(!compareTupleIndex(resultTuple, expectedTuple, tupleIndex))
            {
                std::cout << "Field [" << tupleIndex << "] value mismatch\n";
                retVal = -1;
                return false;
            }
        }
    }

    return true;
}

/*
    Update Alice's tuple, using its saved RID, to (1, "Alicia"). Read it again.
    The same RID returns (1, "Alicia"). 
*/
bool step_four(Database& database)
{
    std::vector<FieldVariant> tupleValues{
        std::int64_t{1},
        std::string{"Alicia"}
    };

    Tuple updatedTuple(tupleValues);

    std::cout << "\n\tUpdating Alice's Tuple\n";
    RID updatedTupleRID = database.updateTuple(tableName, aliceRID, updatedTuple);

    if(!compareRID(updatedTupleRID, aliceRID))
    {
        std::cout << "RID mismatch\n";
        std::cout << "Returned - { " << " { " << updatedTupleRID.pageID.tableID << ", " << updatedTupleRID.pageID.pageNumber << "}, " << updatedTupleRID.tupleSlot << "} ";
        std::cout << "Expected - { " << " { " << aliceRID.pageID.tableID << ", " << aliceRID.pageID.pageNumber << "}, " << aliceRID.tupleSlot << "}\n";
        retVal = -1;
        return false;;
    }

    Tuple readUpdateTuple = database.readTuple(tableName, aliceRID);

    std::cout <<"\n\tComparing Read Updated Tuple vs Expected Tuple\n";
    for(std::size_t tupleIndex = 0; tupleIndex < updatedTuple.getTupleValues().size(); ++tupleIndex)
    {
        if(!compareTupleIndex(updatedTuple, readUpdateTuple, tupleIndex))
        {
            std::cout << "Field [" << tupleIndex << "] value mismatch\n";
            retVal = -1;
            return false;
        }
    }

    return true;
}

/*
    Delete Bob's tuple using its saved RID. Attempt to read it.
    The read returns the documented not-found/deleted error.
*/
bool step_five(Database& database)
{
    std::cout << "\n\tDeleting Bob's Tuple\n";
    database.deleteTuple(tableName, bobRID);


    Tuple readDeleted = database.readTuple(tableName, bobRID);
    if(!readDeleted.getTupleValues().size() == 0)
    {
        std::cout << "Deleted Tuple read successfully\n";
        retVal = -1;
        return false;
    }
    
    return true;
}

/*
    Scan the table. 
    The scan returns exactly one tuple: (1, "Alicia").
*/

bool step_six(Database& database)
{
    std::vector<std::vector<FieldVariant>> tupleValues
    {
        {
            std::int64_t{1},
            std::string{"Alicia"}
        }

    };

    TableHeader tableHeader = database.queryTableInfo(tableName);
    std::int64_t tableID = tableHeader.getTableID();

    std::vector<RID> expectedRIDList
    {
        RID{ {tableID, 1}, 0}
    };

    std::cout << "\n\tRunning table scan\n";
    std::vector<std::pair<Tuple, RID>> scanResults = database.scan(tableName);
    if(scanResults.size() != tupleValues.size())
    {
        std::cout << "Scan results size: " << scanResults.size() << ", Expected 1\n";
        retVal = -1;
        return false;
    }

    for(std::size_t index = 0 ; index < scanResults.size(); ++index)
    {
        Tuple resultTuple = scanResults[index].first;
        RID tupleRID = scanResults[index].second;

        Tuple expectedTuple = Tuple(tupleValues[index]);
        RID expectedRID = expectedRIDList[index];

        if(!compareRID(tupleRID, expectedRID))
        {
            std::cout << "RID mismatch\n";
            std::cout << "Returned - { " << " { " << tupleRID.pageID.tableID << ", " << tupleRID.pageID.pageNumber << "}, " << tupleRID.tupleSlot << "} ";
            std::cout << "Expected - { " << " { " << expectedRID.pageID.tableID << ", " << expectedRID.pageID.pageNumber << "}, " << expectedRID.tupleSlot << "}\n";
            retVal = -1;
            return false;
        }
        for(std::size_t tupleIndex = 0; tupleIndex < expectedTuple.getTupleValues().size(); ++tupleIndex)
        {
            if(!compareTupleIndex(resultTuple, expectedTuple, tupleIndex))
            {
                std::cout << "Field [" << tupleIndex << "] value mismatch\n";
                retVal = -1;
                return false;
            }
        }
    }

    return true;
}

/*
Successfully checkpoint and close the database, then reopen it in a fresh process. 
Reopen students, read both saved RIDs, and scan. 

The schema persists; Alice's RID returns (1, "Alicia"); 
Bob's RID remains unreadable; the scan returns only Alice's updated tuple. 
*/
bool step_seven(Database& database)
{
    Schema schema({
    {"id", FieldType::INT64},
    {"name", FieldType::STRING32}
    });   

    TableHeader tableHeader = database.queryTableInfo(tableName);
    std::int64_t tableID = tableHeader.getTableID();
    Schema storedSchema = tableHeader.getSchema();
    std::cout << "\n\tComparing stored vs expected Schema info\n";

    if(storedSchema.getTupleWidth() != schema.getTupleWidth())
    {
        std::cout << "Stored Schema tuple width [" << storedSchema.getTupleWidth() << "] Expected: [" << schema.getTupleWidth() << "]\n";
        retVal = -1;
        return false;
    }

    if( storedSchema.getFieldList().size() !=  schema.getFieldList().size())
    {
        std::cout << "Stored Schema Field Count [" << storedSchema.getFieldList().size() << "] Expect: [" << schema.getFieldList().size() << "]\n";
        retVal = -1;
        return false;
    }
    std::cout << "\n\tComparing Field List\n";
    for(std::size_t index = 0; index < storedSchema.getFieldList().size(); ++index)
    {
        Field storedSchemaField = storedSchema.getField(index);
        Field schemaField = schema.getField(index);
        
        std::string storedSchemaName = storedSchemaField.name;
        std::string schemaName = schemaField.name;

        FieldType storedSchemaType = storedSchemaField.type;
        FieldType schemaType = schemaField.type;

        if(storedSchemaName != schemaName || storedSchemaType != schemaType)
        {
            std::cout << "Stored Name[" << storedSchemaName << "]: " << " Expected: [" << schemaName << "]\n";
            std::cout << "Stored Type[" << static_cast<int>(storedSchemaType) << "]: " << " Expected: [" << static_cast<int>(schemaType) << "]\n";
            retVal = -1;
            return false;
        }

    }

    std::vector<std::vector<FieldVariant>> tupleValues
    {
        {
            std::int64_t{1},
            std::string{"Alicia"}
        }
    };

    std::vector<RID> expectedRIDList
    {
        RID{ {tableID, 1}, 0}
    };

    Tuple expectedAliceTuple(tupleValues[0]);

    Tuple aliceTuple = database.readTuple(tableName, aliceRID);

    std::cout <<"\n\tComparing Read Alice Tuple vs Expected Alice Tuple\n";
    if(expectedAliceTuple.getTupleValues().size() != aliceTuple.getTupleValues().size()) return false;
    for(std::size_t tupleIndex = 0; tupleIndex < expectedAliceTuple.getTupleValues().size(); ++tupleIndex)
    {
        if(!compareTupleIndex(aliceTuple, expectedAliceTuple, tupleIndex))
        {
            std::cout << "Field [" << tupleIndex << "] value mismatch\n";
            retVal = -1;
            return false;
        }
    }

    std::cout << "\n\tAttempting deleted Bob Tuple Read\n";
    Tuple readDeleted = database.readTuple(tableName, bobRID);
    if(!readDeleted.getTupleValues().size() == 0)
    {
        std::cout << "Deleted Tuple read successfully\n";
        retVal = -1;
        return false;
    }


    std::cout << "\n\tRunning table scan\n";
    std::vector<std::pair<Tuple, RID>> scanResults = database.scan(tableName);
    if(scanResults.size() != tupleValues.size())
    {
        std::cout << "Scan results size: " << scanResults.size() << ", Expected 1\n";
        retVal = -1;
        return false;
    }

    for(std::size_t index = 0 ; index < scanResults.size(); ++index)
    {
        Tuple resultTuple = scanResults[index].first;
        RID tupleRID = scanResults[index].second;

        Tuple expectedTuple = Tuple(tupleValues[index]);
        RID expectedRID = expectedRIDList[index];

        if(!compareRID(tupleRID, expectedRID))
        {
            std::cout << "RID mismatch\n";
            std::cout << "Returned - { " << " { " << tupleRID.pageID.tableID << ", " << tupleRID.pageID.pageNumber << "}, " << tupleRID.tupleSlot << "} ";
            std::cout << "Expected - { " << " { " << expectedRID.pageID.tableID << ", " << expectedRID.pageID.pageNumber << "}, " << expectedRID.tupleSlot << "}\n";
            retVal = -1;
            return false;
        }
        for(std::size_t tupleIndex = 0; tupleIndex < expectedTuple.getTupleValues().size(); ++tupleIndex)
        {
            if(!compareTupleIndex(resultTuple, expectedTuple, tupleIndex))
            {
                std::cout << "Field [" << tupleIndex << "] value mismatch\n";
                retVal = -1;
                return false;
            }
        }
    }

    return true;
}