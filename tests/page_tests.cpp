#include <gtest/gtest.h>
#include "page.hpp"

// NOTE: FINISH ME :)

TEST(PageCreationTests, CorrectConstruction)
{
    PageID pID;
    pID.pageNumber = 0;
    pID.tableID = 0;

    Page p(pID);

    EXPECT_EQ(p.getTableID(), pID.tableID);
    EXPECT_EQ(p.getPageNumber(), pID.pageNumber);
}