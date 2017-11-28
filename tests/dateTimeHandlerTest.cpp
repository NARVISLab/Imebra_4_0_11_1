#include <imebra/imebra.h>
#include <gtest/gtest.h>

namespace imebra
{

namespace tests
{

TEST(dateTimeHandlerTest, dateTest)
{
    DataSet testDataSet;

    testDataSet.setDate(TagId(0x0008, 0x0012), Date(2004, 11, 5, 9, 20, 30, 5000, 1, 2));
    std::unique_ptr<Date> checkDate(testDataSet.getDate(TagId(0x0008, 0x0012), 0));
    std::string checkString = testDataSet.getString(TagId(0x0008, 0x0012), 0);
    EXPECT_EQ("20041105", checkString);
    EXPECT_EQ(2004, checkDate->year);
    EXPECT_EQ(11, checkDate->month);
    EXPECT_EQ(5, checkDate->day);
    EXPECT_EQ(0, checkDate->hour);
    EXPECT_EQ(0, checkDate->minutes);
    EXPECT_EQ(0, checkDate->seconds);
    EXPECT_EQ(0, checkDate->nanoseconds);
    EXPECT_EQ(0, checkDate->offsetHours);
    EXPECT_EQ(0, checkDate->offsetMinutes);
    EXPECT_EQ(tagVR_t::DA, testDataSet.getDataType(TagId(0x0008, 0x0012)));


    testDataSet.setString(TagId(0x0008, 0x0012), "20120910");
    std::unique_ptr<Date> checkDate1(testDataSet.getDate(TagId(0x0008, 0x0012), 0));
    EXPECT_EQ(2012, checkDate1->year);
    EXPECT_EQ(9, checkDate1->month);
    EXPECT_EQ(10, checkDate1->day);
    EXPECT_EQ(0, checkDate1->hour);
    EXPECT_EQ(0, checkDate1->minutes);
    EXPECT_EQ(0, checkDate1->seconds);
    EXPECT_EQ(0, checkDate1->nanoseconds);
    EXPECT_EQ(0, checkDate1->offsetHours);
    EXPECT_EQ(0, checkDate1->offsetMinutes);
}

TEST(dateTimeHandlerTest, timeTest)
{
    DataSet testDataSet;

    {
        testDataSet.setDate(TagId(0x0008, 0x0013), Date(2004, 11, 5, 9, 20, 30, 5000, 1, 2));
        std::unique_ptr<Date> checkDate(testDataSet.getDate(TagId(0x0008, 0x0013), 0));
        std::string checkString = testDataSet.getString(TagId(0x0008, 0x0013), 0);
        EXPECT_EQ("092030.005000", checkString);
        EXPECT_EQ(0, checkDate->year);
        EXPECT_EQ(0, checkDate->month);
        EXPECT_EQ(0, checkDate->day);
        EXPECT_EQ(9, checkDate->hour);
        EXPECT_EQ(20, checkDate->minutes);
        EXPECT_EQ(30, checkDate->seconds);
        EXPECT_EQ(5000, checkDate->nanoseconds);
        EXPECT_EQ(0, checkDate->offsetHours);
        EXPECT_EQ(0, checkDate->offsetMinutes);
        EXPECT_EQ(tagVR_t::TM, testDataSet.getDataType(TagId(0x0008, 0x0013)));
    }

    {
        testDataSet.setString(TagId(0x0008, 0x0013), "101502");
        std::unique_ptr<Date> checkDate(testDataSet.getDate(TagId(0x0008, 0x0013), 0));
        EXPECT_EQ(0, checkDate->year);
        EXPECT_EQ(0, checkDate->month);
        EXPECT_EQ(0, checkDate->day);
        EXPECT_EQ(10, checkDate->hour);
        EXPECT_EQ(15, checkDate->minutes);
        EXPECT_EQ(2, checkDate->seconds);
        EXPECT_EQ(0, checkDate->nanoseconds);
        EXPECT_EQ(0, checkDate->offsetHours);
        EXPECT_EQ(0, checkDate->offsetMinutes);
    }

    {
        testDataSet.setString(TagId(0x0008, 0x0013), "1015");
        std::unique_ptr<Date> checkDate(testDataSet.getDate(TagId(0x0008, 0x0013), 0));
        EXPECT_EQ(0, checkDate->year);
        EXPECT_EQ(0, checkDate->month);
        EXPECT_EQ(0, checkDate->day);
        EXPECT_EQ(10, checkDate->hour);
        EXPECT_EQ(15, checkDate->minutes);
        EXPECT_EQ(0, checkDate->seconds);
        EXPECT_EQ(0, checkDate->nanoseconds);
        EXPECT_EQ(0, checkDate->offsetHours);
        EXPECT_EQ(0, checkDate->offsetMinutes);
    }
}

TEST(dateTimeHandlerTest, dateTimeTest)
{
    DataSet testDataSet;

    Date testDate(2004, 11, 5, 9, 20, 40, 5000, 1, 2);
    testDataSet.setDate(TagId(0x0008, 0x002A), testDate);

    std::unique_ptr<Date> checkDate(testDataSet.getDate(TagId(0x0008, 0x002A), 0));

    EXPECT_EQ(2004, checkDate->year);
    EXPECT_EQ(11, checkDate->month);
    EXPECT_EQ(5, checkDate->day);
    EXPECT_EQ(9, checkDate->hour);
    EXPECT_EQ(20, checkDate->minutes);
    EXPECT_EQ(40, checkDate->seconds);
    EXPECT_EQ(5000, checkDate->nanoseconds);
    EXPECT_EQ(1, checkDate->offsetHours);
    EXPECT_EQ(2, checkDate->offsetMinutes);

    EXPECT_EQ("20041105092040.005000+0102", testDataSet.getString(TagId(0x0008, 0x002A), 0));
}


TEST(dateTimeHandlerTest, incompleteDateTimeTest)
{
    DataSet testDataSet;

    testDataSet.setString(TagId(0x0008, 0x002A), "19990120");
    std::unique_ptr<Date> checkDate(testDataSet.getDate(TagId(0x0008, 0x002A), 0));

    EXPECT_EQ(1999, checkDate->year);
    EXPECT_EQ(1, checkDate->month);
    EXPECT_EQ(20, checkDate->day);
    EXPECT_EQ(0, checkDate->hour);
    EXPECT_EQ(0, checkDate->minutes);
    EXPECT_EQ(0, checkDate->seconds);
    EXPECT_EQ(0, checkDate->nanoseconds);
    EXPECT_EQ(0, checkDate->offsetHours);
    EXPECT_EQ(0, checkDate->offsetMinutes);

    testDataSet.setString(TagId(0x0008, 0x002A), "1999012012");
    checkDate.reset(testDataSet.getDate(TagId(0x0008, 0x002A), 0));

    EXPECT_EQ(1999, checkDate->year);
    EXPECT_EQ(1, checkDate->month);
    EXPECT_EQ(20, checkDate->day);
    EXPECT_EQ(12, checkDate->hour);
    EXPECT_EQ(0, checkDate->minutes);
    EXPECT_EQ(0, checkDate->seconds);
    EXPECT_EQ(0, checkDate->nanoseconds);
    EXPECT_EQ(0, checkDate->offsetHours);
    EXPECT_EQ(0, checkDate->offsetMinutes);
}

} // namespace tests

} // namespace imebra

