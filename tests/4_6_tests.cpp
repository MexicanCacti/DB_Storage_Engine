#include <iostream>
#include "data_page.hpp"
#include "page.hpp"
#include "tuple.hpp"
#include "schema.hpp"
#include <exception>
#include <stdexcept>
#include "basic_types.hpp"
#include <variant>

/*
    This test file just tests the demonstrated minimum sequence written out in 4.6
    The google_tests for each class cover the other functions
*/

Schema testSchema({
        {"id", FieldType::INT64},
        {"name", FieldType::STRING32},
        {"age", FieldType::INT64}
});

std::vector<FieldVariant> testValues{
        std::int64_t{0},
        std::string{"Alice"},
        std::int64_t{35}
};

/*
    Checks
        Tuples can be inserted
        Error returned if page full & page not changed
        written tuples read correctly
        return slots & bytes correct
*/
void insertTuplesTillFull(Page& page);

void updateTuple(Page& page);

void deleteTuple(Page& page);

int main()
{
    try{
        Page page({1,1}); // Page with PID: {1,1}
        // Check empty alloc

        for(std::byte& b : page.getPageBytes())
        {
            if(b != std::byte{0}) throw;
        }

        std::cout << "\nInserting tuples...\n";
        insertTuplesTillFull(page);

        std::cout << "\nUpdating first tuple...\n";
        updateTuple(page);

        std::cout << "\nDeleting first tuple...\n";
        deleteTuple(page);

        std::cout << "\nAttempting to copy page byte buffer...\n";
        std::vector<std::byte> pageByteCopy = page.getPageBytesCopy();
        Page copiedPage({2,1}, pageByteCopy);

        std::cout << "Comparing Page Headers...\n";
        DataPage originalDataPage(page);
        DataPage copiedDataPage(copiedPage);

        originalDataPage.Open(testSchema.getTupleWidth());
        copiedDataPage.Open(testSchema.getTupleWidth());

        PageHeader p1 = originalDataPage.getPageHeader();
        PageHeader p2 = copiedDataPage.getPageHeader();
        if(!PageHeader::similarFormats(p1, p2)) throw std::runtime_error("Copied & original differ in headers");
        
        
        std::cout << "\nAttempting deleted tuple read on page with copied bytes...\n";
        try
        {
            std::vector<std::byte> readTupleBytes = copiedDataPage.readTuple(0);
        }
        catch(std::exception& e)
        {
            std::cout << "Can't read deleted tuple on new page: " << e.what() << "\n";
        }

        std::cout << "\nAttempting first tuple update on page with copied bytes...\n";
        try
        {
             updateTuple(copiedPage);
        }
        catch(std::exception& e)
        {
            std::cout << "Can't update deleted tuple on new page: " << e.what() << "\n";
        }

        std::cout << "\nAttempting first tuple delete on page with copied bytes...\n";
        try
        {
             deleteTuple(copiedPage);
        }
        catch(std::exception& e)
        {
            std::cout << "Can't delete deleted tuple on new page: " << e.what() << "\n";
        }

    } catch(std::exception& e)
    {
        std::cerr << "Failed Test: " << e.what() << "\n";
    }

}

void insertTuplesTillFull(Page& page)
{
    std::int64_t id = 0;

    DataPage dataPage(page);
    dataPage.Initialize(testSchema.getTupleWidth());
    std::cout << "Tuple Width: " << dataPage.getTupleWidth() << '\n';
    std::cout << "Slot Capacity: " << dataPage.getSlotCapacity() << '\n';
    std::cout << "Allocated Slots: " << dataPage.getAllocatedSlotCount() << '\n';
    std::cout << "Has Free Slot: " << dataPage.hasFreeSlot() << '\n';

    while(dataPage.hasFreeSlot())
    {
        testValues[0] = id++;
        Tuple testTuple(testValues);
        try{
            RID tupleID = dataPage.insertTuple(testTuple.encode());
            std::cout << "---Inserted [" << id - 1 << "]. RID: " << tupleID.tupleSlot << "---\n";

            std::vector<std::byte> tupleBytesWritten = dataPage.readTuple(tupleID.tupleSlot);
            Tuple writtenTuple(Tuple::decode(tupleBytesWritten, testSchema.getFieldList()));
            if(tupleBytesWritten.size() != dataPage.getTupleWidth()) throw std::runtime_error("written tuple size does not match!");

            std::int64_t id_written = std::get<std::int64_t>(writtenTuple.getTupleValueIndex(0));
            std::string string_written = std::get<std::string>(writtenTuple.getTupleValueIndex(1));
            std::int64_t age_written = std::get<std::int64_t>(writtenTuple.getTupleValueIndex(2));

            if(id_written != std::get<std::int64_t>(testValues[0]) || string_written != std::get<std::string>(testValues[1]) || age_written != std::get<std::int64_t>(testValues[2])) throw std::runtime_error("tuple written not correct"); 

        }
        catch(std::exception& e)
        {
            std::cerr << "Failed Test: " << e.what() << "\n";
        }
    }

    try{
        testValues[0] = id++;
        Tuple badTuple(testValues);
        RID tupleID = dataPage.insertTuple(badTuple.encode());
        std::cout << "---Inserted tuple into full list!---\n";
    }
    catch(std::exception& e)
    {
        std::cerr << "Can't insert into full page: " << e.what() << "\n";
    }
    
}

void updateTuple(Page& page)
{
    std::vector<FieldVariant> updateValues{
            std::int64_t{-1},
            std::string{"Bob"},
            std::int64_t{35}
    };
    Tuple updateTuple(updateValues);

    DataPage dataPage(page);
    dataPage.Open(testSchema.getTupleWidth());
    auto preUpdateTupleWidth = dataPage.getTupleWidth();
    auto preUpdateSlotCapacity = dataPage.getSlotCapacity();
    auto preAllocatedSlots = dataPage.getAllocatedSlotCount();
    auto preHasFreeSlot = dataPage.hasFreeSlot();


    RID updateTupleRID = dataPage.updateTuple(0, updateTuple.encode());

    std::cout << "\nComparing update values...\n";
    if(preUpdateTupleWidth != dataPage.getTupleWidth() 
        || preUpdateSlotCapacity != dataPage.getSlotCapacity()
        || preAllocatedSlots != dataPage.getAllocatedSlotCount()
        || preHasFreeSlot != dataPage.hasFreeSlot()
    ) throw std::runtime_error("Update incorrectly updated page state");

    std::vector<std::byte> tupleBytesWritten = dataPage.readTuple(updateTupleRID.tupleSlot);
    Tuple writtenTuple(Tuple::decode(tupleBytesWritten, testSchema.getFieldList()));
    if(tupleBytesWritten.size() != dataPage.getTupleWidth()) throw std::runtime_error("written tuple size does not match!");

    std::int64_t id_written = std::get<std::int64_t>(writtenTuple.getTupleValueIndex(0));
    std::string string_written = std::get<std::string>(writtenTuple.getTupleValueIndex(1));
    std::int64_t age_written = std::get<std::int64_t>(writtenTuple.getTupleValueIndex(2));

    if(id_written != std::get<std::int64_t>(updateValues[0]) || string_written != std::get<std::string>(updateValues[1]) || age_written != std::get<std::int64_t>(updateValues[2])) throw std::runtime_error("tuple written not correct"); 

}

void deleteTuple(Page& page)
{
    std::vector<FieldVariant> deleteTestValues{
        std::int64_t{2},
        std::string{"Charlie"},
        std::int64_t{35}
    };
    DataPage dataPage(page);
    dataPage.Open(testSchema.getTupleWidth());
    auto preUpdateTupleWidth = dataPage.getTupleWidth();
    auto preUpdateSlotCapacity = dataPage.getSlotCapacity();
    auto preAllocatedSlots = dataPage.getAllocatedSlotCount();
    auto preHasFreeSlot = dataPage.hasFreeSlot(); 

    dataPage.deleteTuple(0);
    if(preUpdateTupleWidth != dataPage.getTupleWidth() 
        || preUpdateSlotCapacity != dataPage.getSlotCapacity()
        || preAllocatedSlots != dataPage.getAllocatedSlotCount()
        || preHasFreeSlot != dataPage.hasFreeSlot()
    ) throw std::runtime_error("Delete incorrectly updated page state");


    try{
        std::vector<std::byte> tupleBytesRead = dataPage.readTuple(0);
    }
    catch(std::exception& e)
    {
        std::cout << "Read correctly fails: " << e.what() << "\n";
    }    

    try{
        dataPage.insertTuple(Tuple(deleteTestValues).encode());
    }
    catch(std::exception& e)
    {
        std::cout << "Insert correctly fails: " << e.what() << "\n";
    }

}