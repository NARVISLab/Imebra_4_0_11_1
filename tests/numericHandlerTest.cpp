#include <imebra/imebra.h>
#include <string.h>
#include <memory>

#include <gtest/gtest.h>

namespace imebra
{

namespace tests
{

tagVR_t integerTags[] = {tagVR_t::OB, tagVR_t::OL, tagVR_t::SB, tagVR_t::UN, tagVR_t::OW, tagVR_t::AT, tagVR_t::SL, tagVR_t::SS, tagVR_t::UL, tagVR_t::US};
tagVR_t floatTags[] = {tagVR_t::FL, tagVR_t::OF, tagVR_t::FD, tagVR_t::OD};
tagVR_t allTags[] = {
    tagVR_t::OB,
    tagVR_t::OL,
    tagVR_t::SB,
    tagVR_t::UN,
    tagVR_t::OW,
    tagVR_t::AT,
    tagVR_t::SL,
    tagVR_t::SS,
    tagVR_t::UL,
    tagVR_t::US,
    tagVR_t::FL,
    tagVR_t::OF,
    tagVR_t::FD,
    tagVR_t::OD};

TEST(numericHandlerTest, testDouble)
{
    for(size_t scanVR(0); scanVR != sizeof(floatTags) / sizeof(tagVR_t); ++scanVR)
    {
        DataSet testDataSet;

        {
            std::unique_ptr<WritingDataHandlerNumeric> handler(testDataSet.getWritingDataHandlerNumeric(TagId(10, 10), 0, floatTags[scanVR]));
            handler->setDouble(0, 5.6);
            handler->setDouble(1, 3.6);
            handler->setDouble(3, 2.6);
            handler->setSignedLong(4, 10);
            handler->setUnsignedLong(5, 20);
            handler->setString(6, "123.7");
            handler->setUnicodeString(7, L"124.9");
            ASSERT_THROW(handler->setDate(8, Date(2000, 1, 1, 0, 0, 0, 0, 0, 0)), DataHandlerConversionError);
            ASSERT_THROW(handler->setAge(8, Age(3, ageUnit_t::days)), DataHandlerConversionError);
            ASSERT_THROW(handler->setString(8, "test"), DataHandlerConversionError);

            ASSERT_EQ(floatTags[scanVR], handler->getDataType());

            ASSERT_TRUE(handler->isSigned());
            ASSERT_TRUE(handler->isFloat());
        }

        std::unique_ptr<ReadingDataHandlerNumeric> readingHandler(testDataSet.getReadingDataHandlerNumeric(TagId(10, 10), 0));
        ASSERT_TRUE(readingHandler->isFloat());

        ASSERT_FLOAT_EQ(5.6, testDataSet.getDouble(TagId(10, 10), 0));
        ASSERT_FLOAT_EQ(3.6, testDataSet.getDouble(TagId(10, 10), 1));
        ASSERT_FLOAT_EQ(2.6, testDataSet.getDouble(TagId(10, 10), 3));
        ASSERT_FLOAT_EQ(10, testDataSet.getDouble(TagId(10, 10), 4));
        ASSERT_FLOAT_EQ(20, testDataSet.getDouble(TagId(10, 10), 5));
        ASSERT_FLOAT_EQ(123.7, testDataSet.getDouble(TagId(10, 10), 6));
        ASSERT_FLOAT_EQ(124.9, testDataSet.getDouble(TagId(10, 10), 7));
        ASSERT_THROW(testDataSet.getDouble(TagId(10, 10), 8), MissingItemError);

        ASSERT_THROW(testDataSet.getDate(TagId(10, 10), 0), DataHandlerConversionError);
        ASSERT_THROW(testDataSet.getAge(TagId(10, 10), 0), DataHandlerConversionError);

        ASSERT_EQ(5, testDataSet.getSignedLong(TagId(10, 10), 0));
        ASSERT_EQ(3, testDataSet.getSignedLong(TagId(10, 10), 1));
        ASSERT_EQ(2, testDataSet.getSignedLong(TagId(10, 10), 3));
        ASSERT_EQ(10, testDataSet.getSignedLong(TagId(10, 10), 4));
        ASSERT_EQ(20, testDataSet.getSignedLong(TagId(10, 10), 5));
        ASSERT_EQ(123, testDataSet.getSignedLong(TagId(10, 10), 6));
        ASSERT_EQ(124, testDataSet.getSignedLong(TagId(10, 10), 7));
        ASSERT_THROW(testDataSet.getSignedLong(TagId(10, 10), 8), MissingItemError);

        ASSERT_EQ(5, testDataSet.getUnsignedLong(TagId(10, 10), 0));
        ASSERT_EQ(3, testDataSet.getUnsignedLong(TagId(10, 10), 1));
        ASSERT_EQ(2, testDataSet.getUnsignedLong(TagId(10, 10), 3));
        ASSERT_EQ(10, testDataSet.getUnsignedLong(TagId(10, 10), 4));
        ASSERT_EQ(20, testDataSet.getUnsignedLong(TagId(10, 10), 5));
        ASSERT_EQ(123, testDataSet.getUnsignedLong(TagId(10, 10), 6));
        ASSERT_EQ(124, testDataSet.getUnsignedLong(TagId(10, 10), 7));
        ASSERT_THROW(testDataSet.getUnsignedLong(TagId(10, 10), 8), MissingItemError);

        ASSERT_FLOAT_EQ(5.6, std::stod(testDataSet.getString(TagId(10, 10), 0).c_str()));
        ASSERT_FLOAT_EQ(3.6, std::stod(testDataSet.getString(TagId(10, 10), 1).c_str()));
        ASSERT_FLOAT_EQ(2.6, std::stod(testDataSet.getString(TagId(10, 10), 3).c_str()));
        ASSERT_FLOAT_EQ(10, std::stod(testDataSet.getString(TagId(10, 10), 4).c_str()));
        ASSERT_FLOAT_EQ(20, std::stod(testDataSet.getString(TagId(10, 10), 5).c_str()));
        ASSERT_FLOAT_EQ(123.7, std::stod(testDataSet.getString(TagId(10, 10), 6).c_str()));
        ASSERT_FLOAT_EQ(124.9, std::stod(testDataSet.getString(TagId(10, 10), 7).c_str()));
        ASSERT_THROW(testDataSet.getString(TagId(10, 10), 8), MissingItemError);

        ASSERT_FLOAT_EQ(5.6, std::stod(testDataSet.getUnicodeString(TagId(10, 10), 0).c_str()));
        ASSERT_FLOAT_EQ(3.6, std::stod(testDataSet.getUnicodeString(TagId(10, 10), 1).c_str()));
        ASSERT_FLOAT_EQ(2.6, std::stod(testDataSet.getUnicodeString(TagId(10, 10), 3).c_str()));
        ASSERT_FLOAT_EQ(10, std::stod(testDataSet.getUnicodeString(TagId(10, 10), 4).c_str()));
        ASSERT_FLOAT_EQ(20, std::stod(testDataSet.getUnicodeString(TagId(10, 10), 5).c_str()));
        ASSERT_FLOAT_EQ(123.7, std::stod(testDataSet.getUnicodeString(TagId(10, 10), 6).c_str()));
        ASSERT_FLOAT_EQ(124.9, std::stod(testDataSet.getUnicodeString(TagId(10, 10), 7).c_str()));
        ASSERT_THROW(testDataSet.getString(TagId(10, 10), 8), MissingItemError);
    }
}


TEST(numericHandlerTest, testInteger)
{
    for(size_t scanVR(0); scanVR != sizeof(integerTags) / sizeof(tagVR_t); ++scanVR)
    {
        DataSet testDataSet;

        {
            std::unique_ptr<WritingDataHandlerNumeric> handler(testDataSet.getWritingDataHandlerNumeric(TagId(10, 10), 0, integerTags[scanVR]));
            handler->setDouble(0, 5.6);
            handler->setDouble(1, 3.6);
            handler->setDouble(3, 2.6);
            handler->setSignedLong(4, 10);
            handler->setUnsignedLong(5, 20);
            handler->setString(6, "123.7");
            handler->setUnicodeString(7, L"124.9");
            ASSERT_THROW(handler->setDate(8, Date(2000, 1, 1, 0, 0, 0, 0, 0, 0)), DataHandlerConversionError);
            ASSERT_THROW(handler->setAge(8, Age(3, ageUnit_t::days)), DataHandlerConversionError);
            ASSERT_THROW(handler->setString(8, "test"), DataHandlerConversionError);

            ASSERT_EQ(integerTags[scanVR], handler->getDataType());

            ASSERT_EQ(integerTags[scanVR] == tagVR_t::OL ||
                      integerTags[scanVR] == tagVR_t::SB ||
                      integerTags[scanVR] == tagVR_t::SL ||
                      integerTags[scanVR] == tagVR_t::SS,
                      handler->isSigned());

            ASSERT_FALSE(handler->isFloat());
        }

        std::unique_ptr<ReadingDataHandlerNumeric> readingHandler(testDataSet.getReadingDataHandlerNumeric(TagId(10, 10), 0));
        ASSERT_FALSE(readingHandler->isFloat());

        ASSERT_FLOAT_EQ(5, testDataSet.getDouble(TagId(10, 10), 0));
        ASSERT_FLOAT_EQ(3, testDataSet.getDouble(TagId(10, 10), 1));
        ASSERT_FLOAT_EQ(2, testDataSet.getDouble(TagId(10, 10), 3));
        ASSERT_FLOAT_EQ(10, testDataSet.getDouble(TagId(10, 10), 4));
        ASSERT_FLOAT_EQ(20, testDataSet.getDouble(TagId(10, 10), 5));
        ASSERT_FLOAT_EQ(123, testDataSet.getDouble(TagId(10, 10), 6));
        ASSERT_FLOAT_EQ(124, testDataSet.getDouble(TagId(10, 10), 7));
        ASSERT_THROW(testDataSet.getDouble(TagId(10, 10), 8), MissingItemError);

        ASSERT_THROW(testDataSet.getDate(TagId(10, 10), 0), DataHandlerConversionError);
        ASSERT_THROW(testDataSet.getAge(TagId(10, 10), 0), DataHandlerConversionError);

        ASSERT_EQ(5, testDataSet.getSignedLong(TagId(10, 10), 0));
        ASSERT_EQ(3, testDataSet.getSignedLong(TagId(10, 10), 1));
        ASSERT_EQ(2, testDataSet.getSignedLong(TagId(10, 10), 3));
        ASSERT_EQ(10, testDataSet.getSignedLong(TagId(10, 10), 4));
        ASSERT_EQ(20, testDataSet.getSignedLong(TagId(10, 10), 5));
        ASSERT_EQ(123, testDataSet.getSignedLong(TagId(10, 10), 6));
        ASSERT_EQ(124, testDataSet.getSignedLong(TagId(10, 10), 7));
        ASSERT_THROW(testDataSet.getSignedLong(TagId(10, 10), 8), MissingItemError);

        ASSERT_EQ(5, testDataSet.getUnsignedLong(TagId(10, 10), 0));
        ASSERT_EQ(3, testDataSet.getUnsignedLong(TagId(10, 10), 1));
        ASSERT_EQ(2, testDataSet.getUnsignedLong(TagId(10, 10), 3));
        ASSERT_EQ(10, testDataSet.getUnsignedLong(TagId(10, 10), 4));
        ASSERT_EQ(20, testDataSet.getUnsignedLong(TagId(10, 10), 5));
        ASSERT_EQ(123, testDataSet.getUnsignedLong(TagId(10, 10), 6));
        ASSERT_EQ(124, testDataSet.getUnsignedLong(TagId(10, 10), 7));
        ASSERT_THROW(testDataSet.getUnsignedLong(TagId(10, 10), 8), MissingItemError);

        ASSERT_EQ(5, std::stol(testDataSet.getString(TagId(10, 10), 0).c_str()));
        ASSERT_EQ(3, std::stol(testDataSet.getString(TagId(10, 10), 1).c_str()));
        ASSERT_EQ(2, std::stol(testDataSet.getString(TagId(10, 10), 3).c_str()));
        ASSERT_EQ(10, std::stol(testDataSet.getString(TagId(10, 10), 4).c_str()));
        ASSERT_EQ(20, std::stol(testDataSet.getString(TagId(10, 10), 5).c_str()));
        ASSERT_EQ(123, std::stol(testDataSet.getString(TagId(10, 10), 6).c_str()));
        ASSERT_EQ(124, std::stol(testDataSet.getString(TagId(10, 10), 7).c_str()));
        ASSERT_THROW(testDataSet.getString(TagId(10, 10), 8), MissingItemError);

        ASSERT_EQ(5, std::stol(testDataSet.getUnicodeString(TagId(10, 10), 0).c_str()));
        ASSERT_EQ(3, std::stol(testDataSet.getUnicodeString(TagId(10, 10), 1).c_str()));
        ASSERT_EQ(2, std::stol(testDataSet.getUnicodeString(TagId(10, 10), 3).c_str()));
        ASSERT_EQ(10, std::stol(testDataSet.getUnicodeString(TagId(10, 10), 4).c_str()));
        ASSERT_EQ(20, std::stol(testDataSet.getUnicodeString(TagId(10, 10), 5).c_str()));
        ASSERT_EQ(123, std::stol(testDataSet.getUnicodeString(TagId(10, 10), 6).c_str()));
        ASSERT_EQ(124, std::stol(testDataSet.getUnicodeString(TagId(10, 10), 7).c_str()));
        ASSERT_THROW(testDataSet.getString(TagId(10, 10), 8), MissingItemError);
    }
}


TEST(numericHandlerTest, testCopyFrom)
{
    for(size_t destVR(0); destVR != sizeof(allTags) / sizeof(tagVR_t); ++destVR)
    {

        for(size_t sourceVR(0); sourceVR != sizeof(allTags) / sizeof(tagVR_t); ++sourceVR)
        {
            DataSet testDataSet;

            {
                std::unique_ptr<WritingDataHandlerNumeric> handler(testDataSet.getWritingDataHandlerNumeric(TagId(10, 11), 0, allTags[sourceVR]));
                for(size_t fillData(0); fillData != 10; ++fillData)
                {
                    handler->setSignedLong(fillData, fillData);
                }
            }

            {
                std::unique_ptr<WritingDataHandlerNumeric> handler(testDataSet.getWritingDataHandlerNumeric(TagId(10, 10), 0, allTags[destVR]));
                std::unique_ptr<ReadingDataHandlerNumeric> source(testDataSet.getReadingDataHandlerNumeric(TagId(10, 11), 0));

                handler->copyFrom(*source);
            }

            std::unique_ptr<ReadingDataHandlerNumeric> source(testDataSet.getReadingDataHandlerNumeric(TagId(10, 11), 0));
            std::unique_ptr<ReadingDataHandlerNumeric> dest(testDataSet.getReadingDataHandlerNumeric(TagId(10, 10), 0));

            ASSERT_EQ(10, dest->getSize());
            ASSERT_EQ(10, source->getSize());

            for(size_t checkData(0); checkData != 10; ++checkData)
            {
                ASSERT_FLOAT_EQ((double)checkData, dest->getDouble(checkData));
                ASSERT_FLOAT_EQ((double)checkData, source->getDouble(checkData));
            }

        }

    }
}


TEST(numericHandlerTest, testCopyTo)
{
    for(size_t destVR(0); destVR != sizeof(allTags) / sizeof(tagVR_t); ++destVR)
    {

        for(size_t sourceVR(0); sourceVR != sizeof(allTags) / sizeof(tagVR_t); ++sourceVR)
        {
            DataSet testDataSet;

            {
                std::unique_ptr<WritingDataHandlerNumeric> handler(testDataSet.getWritingDataHandlerNumeric(TagId(10, 11), 0, allTags[sourceVR]));
                for(size_t fillData(0); fillData != 10; ++fillData)
                {
                    handler->setSignedLong(fillData, fillData);
                }
            }

            {
                std::unique_ptr<WritingDataHandlerNumeric> handler(testDataSet.getWritingDataHandlerNumeric(TagId(10, 10), 0, allTags[destVR]));
                handler->setSize(12);
                std::unique_ptr<ReadingDataHandlerNumeric> source(testDataSet.getReadingDataHandlerNumeric(TagId(10, 11), 0));

                source->copyTo(*handler);
            }

            std::unique_ptr<ReadingDataHandlerNumeric> source(testDataSet.getReadingDataHandlerNumeric(TagId(10, 11), 0));
            std::unique_ptr<ReadingDataHandlerNumeric> dest(testDataSet.getReadingDataHandlerNumeric(TagId(10, 10), 0));

            ASSERT_EQ(12, dest->getSize());
            ASSERT_EQ(10, source->getSize());

            for(size_t checkData(0); checkData != 10; ++checkData)
            {
                ASSERT_FLOAT_EQ((double)checkData, dest->getDouble(checkData));
                ASSERT_FLOAT_EQ((double)checkData, source->getDouble(checkData));
            }

        }

    }
}




} // namespace tests

} // namespace imebra

