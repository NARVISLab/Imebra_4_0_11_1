#include <imebra/imebra.h>
#include <gtest/gtest.h>

namespace imebra
{

namespace tests
{

TEST(stringHandlerTest, DSTest)
{
    {
        DataSet testDataSet;
        testDataSet.setDouble(TagId(0x0028, 0x1051), 0.000001, tagVR_t::DS);
        std::string decimalString = testDataSet.getString(TagId(0x0028, 0x1051), 0);
        ASSERT_TRUE(decimalString == "1e-006" || decimalString == "1e-06" || decimalString == "1e-6");
        double decimalValue = testDataSet.getDouble(TagId(0x0028, 0x1051), 0);
        ASSERT_FLOAT_EQ(0.000001, decimalValue);
        ASSERT_EQ(tagVR_t::DS, testDataSet.getDataType(TagId(0x0028, 0x1051)));
        ASSERT_THROW(testDataSet.getDate(TagId(0x0028, 0x1051), 0), DataHandlerConversionError);
        ASSERT_THROW(testDataSet.getAge(TagId(0x0028, 0x1051), 0), DataHandlerConversionError);
        EXPECT_THROW(testDataSet.getUnicodeString(TagId(0x0028, 0x1051), 1), MissingItemError);
        EXPECT_THROW(testDataSet.getString(TagId(0x0028, 0x1051), 1), MissingItemError);
        EXPECT_THROW(testDataSet.getSignedLong(TagId(0x0028, 0x1051), 1), MissingItemError);
        EXPECT_THROW(testDataSet.getUnsignedLong(TagId(0x0028, 0x1051), 1), MissingItemError);
        EXPECT_THROW(testDataSet.getDouble(TagId(0x0028, 0x1051), 1), MissingItemError);
    }

    {
        DataSet testDataSet;
        testDataSet.setString(TagId(0x0028, 0x1051), "-2e-8", tagVR_t::DS);
        ASSERT_FLOAT_EQ(-0.00000002, testDataSet.getDouble(TagId(0x0028, 0x1051), 0));
        ASSERT_THROW(testDataSet.getSignedLong(TagId(0x0028, 0x1051), 1), MissingItemError);
        ASSERT_EQ(tagVR_t::DS, testDataSet.getDataType(TagId(0x0028, 0x1051)));
    }

    {
        DataSet testDataSet;
        testDataSet.setString(TagId(0x0028, 0x1051), "20", tagVR_t::DS);
        ASSERT_FLOAT_EQ(20, testDataSet.getDouble(TagId(0x0028, 0x1051), 0));
        ASSERT_EQ(20, testDataSet.getSignedLong(TagId(0x0028, 0x1051), 0));
        ASSERT_EQ(20, testDataSet.getUnsignedLong(TagId(0x0028, 0x1051), 0));
        ASSERT_EQ(tagVR_t::DS, testDataSet.getDataType(TagId(0x0028, 0x1051)));
    }

    {
        DataSet testDataSet;
        testDataSet.setSignedLong(TagId(0x0028, 0x1051), 40, tagVR_t::DS);
        ASSERT_EQ("40", testDataSet.getString(TagId(0x0028, 0x1051), 0));
        ASSERT_EQ(40, testDataSet.getSignedLong(TagId(0x0028, 0x1051), 0));
        ASSERT_EQ(40, testDataSet.getUnsignedLong(TagId(0x0028, 0x1051), 0));
        ASSERT_EQ(tagVR_t::DS, testDataSet.getDataType(TagId(0x0028, 0x1051)));
    }

    {
        DataSet testDataSet;
        testDataSet.setUnsignedLong(TagId(0x0028, 0x1051), 50, tagVR_t::DS);
        ASSERT_EQ("50", testDataSet.getString(TagId(0x0028, 0x1051), 0));
        ASSERT_EQ(50, testDataSet.getSignedLong(TagId(0x0028, 0x1051), 0));
        ASSERT_EQ(50, testDataSet.getUnsignedLong(TagId(0x0028, 0x1051), 0));
        ASSERT_EQ(tagVR_t::DS, testDataSet.getDataType(TagId(0x0028, 0x1051)));
    }

    {
        DataSet testDataSet;
        testDataSet.setString(TagId(0x0028, 0x1051), "Hello", tagVR_t::DS);
        ASSERT_THROW(testDataSet.getDouble(TagId(0x0028, 0x1051), 0), DataHandlerConversionError);
    }
}


TEST(stringHandlerTest, ISTest)
{
    {
        DataSet testDataSet;
        testDataSet.setDouble(TagId(0x0028, 0x1051), 12.004, tagVR_t::IS);
        ASSERT_EQ("12", testDataSet.getString(TagId(0x0028, 0x1051), 0));
        ASSERT_EQ(12, testDataSet.getSignedLong(TagId(0x0028, 0x1051), 0));
        ASSERT_FLOAT_EQ(12.0, testDataSet.getDouble(TagId(0x0028, 0x1051), 0));
        ASSERT_EQ(tagVR_t::IS, testDataSet.getDataType(TagId(0x0028, 0x1051)));
        ASSERT_THROW(testDataSet.getDate(TagId(0x0028, 0x1051), 0), DataHandlerConversionError);
        ASSERT_THROW(testDataSet.getAge(TagId(0x0028, 0x1051), 0), DataHandlerConversionError);
    }

    {
        DataSet testDataSet;
        testDataSet.setSignedLong(TagId(0x0028, 0x1051), 20, tagVR_t::IS);
        ASSERT_EQ("20", testDataSet.getString(TagId(0x0028, 0x1051), 0));
        ASSERT_EQ(20, testDataSet.getSignedLong(TagId(0x0028, 0x1051), 0));
        ASSERT_FLOAT_EQ(20.0, testDataSet.getDouble(TagId(0x0028, 0x1051), 0));
        ASSERT_EQ(tagVR_t::IS, testDataSet.getDataType(TagId(0x0028, 0x1051)));
    }

    {
        DataSet testDataSet;
        testDataSet.setString(TagId(0x0028, 0x1051), "123456789012", tagVR_t::IS);
        ASSERT_THROW(testDataSet.setString(TagId(0x0028, 0x1051), "1234567890123", tagVR_t::IS), DataHandlerInvalidDataError);
    }

}


TEST(stringHandlerTest, AETest)
{
    DataSet testDataSet;

    testDataSet.setString(TagId(0x0010, 0x0010), "0123456789012345", tagVR_t::AE);
    ASSERT_EQ("0123456789012345", testDataSet.getString(TagId(0x0010, 0x0010), 0));
    ASSERT_THROW(testDataSet.getDate(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getAge(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);

    testDataSet.setString(TagId(0x0010, 0x0010), "012345678901234", tagVR_t::AE);
    ASSERT_EQ("012345678901234", testDataSet.getString(TagId(0x0010, 0x0010), 0));

    ASSERT_EQ(tagVR_t::AE, testDataSet.getDataType(TagId(0x0010, 0x0010)));

    ASSERT_THROW(testDataSet.setString(TagId(0x0010, 0x0010), "01234567890123456", tagVR_t::AE), DataHandlerInvalidDataError);
}


TEST(stringHandlerTest, zeroPad)
{
    DataSet testDataSet;

    testDataSet.setString(TagId(0x0010, 0x0010), "0\000", tagVR_t::AE);
    ASSERT_EQ("0", testDataSet.getString(TagId(0x0010, 0x0010), 0));
    ASSERT_EQ(1, testDataSet.getString(TagId(0x0010, 0x0010), 0).length());
}


TEST(stringHandlerTest, URTest)
{
    DataSet testDataSet;

    std::string longString((size_t)40000, 'a');
    testDataSet.setString(TagId(0x0010, 0x0010), longString, tagVR_t::UR);
    ASSERT_EQ(longString, testDataSet.getString(TagId(0x0010, 0x0010), 0));

    ASSERT_THROW(testDataSet.getDate(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getAge(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getUnsignedLong(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getSignedLong(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getDouble(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);

    ASSERT_EQ(tagVR_t::UR, testDataSet.getDataType(TagId(0x0010, 0x0010)));

    {
        std::unique_ptr<WritingDataHandler> dataHandler(testDataSet.getWritingDataHandler(TagId(0x0010, 0x0010), 0, tagVR_t::LT));
        dataHandler->setString(0, "test");
        ASSERT_THROW(dataHandler->setString(1, "test");, DataHandlerInvalidDataError);
    }
}


TEST(stringHandlerTest, LOTest)
{
    DataSet testDataSet;

    testDataSet.setString(TagId(0x0010, 0x0010), "0123456789012345", tagVR_t::LO);
    ASSERT_EQ("0123456789012345", testDataSet.getString(TagId(0x0010, 0x0010), 0));
    ASSERT_THROW(testDataSet.getDate(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getAge(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);

    {
        std::string longString((size_t)63, 'a');
        testDataSet.setString(TagId(0x0010, 0x0010), longString, tagVR_t::LO);
        ASSERT_EQ(longString, testDataSet.getString(TagId(0x0010, 0x0010), 0));
        ASSERT_EQ(tagVR_t::LO, testDataSet.getDataType(TagId(0x0010, 0x0010)));
    }

    {
        std::string longString((size_t)64, 'a');
        testDataSet.setString(TagId(0x0010, 0x0010), longString, tagVR_t::LO);
        ASSERT_EQ(longString, testDataSet.getString(TagId(0x0010, 0x0010), 0));
        ASSERT_EQ(tagVR_t::LO, testDataSet.getDataType(TagId(0x0010, 0x0010)));
    }

    {
        std::string longString((size_t)65, 'a');
        ASSERT_THROW(testDataSet.setString(TagId(0x0010, 0x0010), longString, tagVR_t::LO), DataHandlerInvalidDataError);
    }
}


TEST(stringHandlerTest, LTTest)
{
    DataSet testDataSet;

    testDataSet.setString(TagId(0x0010, 0x0010), "0123456789012345\\0123", tagVR_t::LT);
    ASSERT_EQ("0123456789012345\\0123", testDataSet.getString(TagId(0x0010, 0x0010), 0));
    ASSERT_THROW(testDataSet.getDate(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getAge(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);

    {
        std::unique_ptr<WritingDataHandler> dataHandler(testDataSet.getWritingDataHandler(TagId(0x0010, 0x0010), 0, tagVR_t::LT));
        dataHandler->setString(0, "test\\test1");
        ASSERT_THROW(dataHandler->setString(1, "test");, DataHandlerInvalidDataError);
    }

    {
        std::string longString((size_t)63, 'a');
        testDataSet.setString(TagId(0x0010, 0x0010), longString, tagVR_t::LT);
        ASSERT_EQ(longString, testDataSet.getString(TagId(0x0010, 0x0010), 0));
        ASSERT_EQ(tagVR_t::LT, testDataSet.getDataType(TagId(0x0010, 0x0010)));
    }

    {
        std::string longString((size_t)10240, 'a');
        testDataSet.setString(TagId(0x0010, 0x0010), longString, tagVR_t::LT);
        ASSERT_EQ(longString, testDataSet.getString(TagId(0x0010, 0x0010), 0));
        ASSERT_EQ(tagVR_t::LT, testDataSet.getDataType(TagId(0x0010, 0x0010)));
    }

    {
        std::string longString((size_t)10241, 'a');
        ASSERT_THROW(testDataSet.setString(TagId(0x0010, 0x0010), longString, tagVR_t::LT), DataHandlerInvalidDataError);
    }
}


TEST(stringHandlerTest, SHTest)
{
    DataSet testDataSet;

    testDataSet.setString(TagId(0x0010, 0x0010), "0123456789012345", tagVR_t::SH);
    ASSERT_EQ("0123456789012345", testDataSet.getString(TagId(0x0010, 0x0010), 0));
    ASSERT_THROW(testDataSet.getDate(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getAge(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);

    {
        std::unique_ptr<WritingDataHandler> shHandler(testDataSet.getWritingDataHandler(TagId(0x0010, 0x0010), 0, tagVR_t::SH));
        shHandler->setString(0, "item 0");
        shHandler->setString(1, "item 1");
        shHandler->setString(2, "item 2");
        ASSERT_EQ(tagVR_t::SH, shHandler->getDataType());
    }
    ASSERT_EQ("item 0", testDataSet.getString(TagId(0x0010, 0x0010), 0));
    ASSERT_EQ("item 1", testDataSet.getString(TagId(0x0010, 0x0010), 1));
    ASSERT_EQ("item 2", testDataSet.getString(TagId(0x0010, 0x0010), 2));
    std::unique_ptr<ReadingDataHandlerNumeric> rawHandler(testDataSet.getReadingDataHandlerRaw(TagId(0x0010, 0x0010), 0));
    size_t dataSize;
    const char* data = rawHandler->data(&dataSize);
    std::string fullString(data, dataSize);
    ASSERT_EQ("item 0\\item 1\\item 2", fullString);

    ASSERT_THROW(testDataSet.setString(TagId(0x0010, 0x0010), "01234567890123456", tagVR_t::LT), DataHandlerInvalidDataError);
}


TEST(stringHandlerTest, STTest)
{
    DataSet testDataSet;

    testDataSet.setString(TagId(0x0010, 0x0010), "0123456789012345", tagVR_t::ST);
    ASSERT_EQ("0123456789012345", testDataSet.getString(TagId(0x0010, 0x0010), 0));
    ASSERT_THROW(testDataSet.getDate(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getAge(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);

    {
        std::unique_ptr<WritingDataHandler> dataHandler(testDataSet.getWritingDataHandler(TagId(0x0010, 0x0010), 0, tagVR_t::ST));
        dataHandler->setString(0, "test");
        ASSERT_THROW(dataHandler->setString(1, "test");, DataHandlerInvalidDataError);
    }

    {
        std::string longString((size_t)1024, 'a');
        testDataSet.setString(TagId(0x0010, 0x0010), longString, tagVR_t::ST);
        ASSERT_EQ(longString, testDataSet.getString(TagId(0x0010, 0x0010), 0));
        ASSERT_EQ(tagVR_t::ST, testDataSet.getDataType(TagId(0x0010, 0x0010)));

    }

    {
        std::string longString((size_t)1025, 'a');
        ASSERT_THROW(testDataSet.setString(TagId(0x0010, 0x0010), longString, tagVR_t::ST), DataHandlerInvalidDataError);
    }
}


TEST(stringHandlerTest, UITest)
{
    DataSet testDataSet;

    testDataSet.setString(TagId(0x0010, 0x0010), "0123456789012345", tagVR_t::UI);
    ASSERT_EQ("0123456789012345", testDataSet.getString(TagId(0x0010, 0x0010), 0));
    ASSERT_THROW(testDataSet.getDate(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getAge(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);

    {
        std::unique_ptr<WritingDataHandler> dataHandler(testDataSet.getWritingDataHandler(TagId(0x0010, 0x0010), 0, tagVR_t::UI));
        dataHandler->setString(0, "test");
        ASSERT_THROW(dataHandler->setString(1, "test");, DataHandlerInvalidDataError);
    }

    {
        std::string longString((size_t)64, 'a');
        testDataSet.setString(TagId(0x0010, 0x0010), longString, tagVR_t::UI);
        ASSERT_EQ(longString, testDataSet.getString(TagId(0x0010, 0x0010), 0));
        ASSERT_EQ(tagVR_t::UI, testDataSet.getDataType(TagId(0x0010, 0x0010)));
    }

    {
        std::string longString((size_t)65, 'a');
        ASSERT_THROW(testDataSet.setString(TagId(0x0010, 0x0010), longString, tagVR_t::UI), DataHandlerInvalidDataError);
    }
}


TEST(stringHandlerTest, UCTest)
{
    DataSet testDataSet;

    testDataSet.setUnicodeString(TagId(0x0010, 0x0010), L"0123456789012345", tagVR_t::UC);
    ASSERT_EQ("0123456789012345", testDataSet.getString(TagId(0x0010, 0x0010), 0));
    ASSERT_EQ(L"0123456789012345", testDataSet.getUnicodeString(TagId(0x0010, 0x0010), 0));
    ASSERT_THROW(testDataSet.getDate(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getAge(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);

    {
        {
            std::unique_ptr<WritingDataHandler> dataHandler(testDataSet.getWritingDataHandler(TagId(0x0010, 0x0010), 0, tagVR_t::UC));
            dataHandler->setUnicodeString(0, L"test0");
            dataHandler->setUnicodeString(1, L"test1");
        }
        ASSERT_EQ(L"test0", testDataSet.getUnicodeString(TagId(0x0010, 0x0010), 0));
        ASSERT_EQ(L"test1", testDataSet.getUnicodeString(TagId(0x0010, 0x0010), 1));
    }

    {
        std::string longString((size_t)100000, 'a');
        testDataSet.setString(TagId(0x0010, 0x0010), longString, tagVR_t::UC);
        ASSERT_EQ(longString, testDataSet.getString(TagId(0x0010, 0x0010), 0));
        ASSERT_EQ(tagVR_t::UC, testDataSet.getDataType(TagId(0x0010, 0x0010)));
    }

}


TEST(stringHandlerTest, UTTest)
{
    DataSet testDataSet;

    testDataSet.setString(TagId(0x0010, 0x0010), "0123456789012345", tagVR_t::UT);
    ASSERT_EQ("0123456789012345", testDataSet.getString(TagId(0x0010, 0x0010), 0));
    ASSERT_THROW(testDataSet.getDate(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getAge(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);

    {
        std::unique_ptr<WritingDataHandler> dataHandler(testDataSet.getWritingDataHandler(TagId(0x0010, 0x0010), 0, tagVR_t::UT));
        dataHandler->setString(0, "test");
        ASSERT_THROW(dataHandler->setString(1, "test");, DataHandlerInvalidDataError);
    }

    {
        std::string longString((size_t)100000, 'a');
        testDataSet.setString(TagId(0x0010, 0x0010), longString, tagVR_t::UT);
        ASSERT_EQ(longString, testDataSet.getString(TagId(0x0010, 0x0010), 0));
        ASSERT_EQ(tagVR_t::UT, testDataSet.getDataType(TagId(0x0010, 0x0010)));
    }

}


TEST(stringHandlerTest, PNTest)
{
    DataSet testDataSet;

    testDataSet.setString(TagId(0x0010, 0x0010), "PatientName", tagVR_t::PN);
    ASSERT_EQ("PatientName", testDataSet.getString(TagId(0x0010, 0x0010), 0));
    ASSERT_THROW(testDataSet.getDate(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getAge(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);

    {
        std::unique_ptr<WritingDataHandler> pnHandler(testDataSet.getWritingDataHandler(TagId(0x0010, 0x0010), 0, tagVR_t::PN));
        pnHandler->setString(0, "Patient 0");
        pnHandler->setString(1, "Patient 1");
        pnHandler->setString(2, "Patient 2");
    }
    ASSERT_EQ("Patient 0", testDataSet.getString(TagId(0x0010, 0x0010), 0));
    ASSERT_EQ("Patient 1", testDataSet.getString(TagId(0x0010, 0x0010), 1));
    ASSERT_EQ("Patient 2", testDataSet.getString(TagId(0x0010, 0x0010), 2));
    ASSERT_THROW(testDataSet.getUnsignedLong(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getSignedLong(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getDouble(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);

    std::unique_ptr<ReadingDataHandlerNumeric> rawHandler(testDataSet.getReadingDataHandlerRaw(TagId(0x0010, 0x0010), 0));
    size_t dataSize;
    const char* data = rawHandler->data(&dataSize);
    std::string fullString(data, dataSize);
    ASSERT_EQ("Patient 0=Patient 1=Patient 2 ", fullString);

    {
        std::string longString((size_t)65, 'a');
        ASSERT_THROW(testDataSet.setString(TagId(0x0010, 0x0010), longString, tagVR_t::PN), DataHandlerInvalidDataError);
    }
}


TEST(stringHandlerTest, CSTest)
{
    DataSet testDataSet;

    testDataSet.setString(TagId(0x0010, 0x0010), "0123456789012345", tagVR_t::CS);
    ASSERT_EQ("0123456789012345", testDataSet.getString(TagId(0x0010, 0x0010), 0));
    ASSERT_THROW(testDataSet.getDate(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getAge(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);

    testDataSet.setString(TagId(0x0010, 0x0010), "012345678901234", tagVR_t::CS);
    ASSERT_EQ("012345678901234", testDataSet.getString(TagId(0x0010, 0x0010), 0));

    ASSERT_THROW(testDataSet.setString(TagId(0x0010, 0x0010), "01234567890123456", tagVR_t::CS), DataHandlerInvalidDataError);

}


TEST(stringHandlerTest, ASTest)
{
    imebra::DataSet dataSet;

    dataSet.setAge(TagId(0x0010, 0x1010), Age(3, imebra::ageUnit_t::days));
    std::unique_ptr<imebra::Age> age(dataSet.getAge(TagId(imebra::tagId_t::PatientAge_0010_1010), 0));
    EXPECT_EQ(3, age->age);
    EXPECT_EQ(imebra::ageUnit_t::days, age->units);
    EXPECT_EQ("003D", dataSet.getString(TagId(imebra::tagId_t::PatientAge_0010_1010), 0));
    EXPECT_FLOAT_EQ(0.008219178, age->years());

    ASSERT_THROW(dataSet.setDouble(TagId(imebra::tagId_t::PatientAge_0010_1010), .01), imebra::DataHandlerConversionError);

    ASSERT_THROW(dataSet.setUnsignedLong(TagId(imebra::tagId_t::PatientAge_0010_1010), 1), imebra::DataHandlerConversionError);

    ASSERT_THROW(dataSet.setSignedLong(TagId(imebra::tagId_t::PatientAge_0010_1010), 1), imebra::DataHandlerConversionError);

    ASSERT_THROW(dataSet.setDate(TagId(imebra::tagId_t::PatientAge_0010_1010), imebra::Date(2000, 1, 1, 0, 0, 0, 0, 0, 0)), imebra::DataHandlerConversionError);

    dataSet.setString(TagId(imebra::tagId_t::PatientAge_0010_1010), "005M");
    age.reset(dataSet.getAge(TagId(imebra::tagId_t::PatientAge_0010_1010), 0));
    EXPECT_EQ(5, age->age);
    EXPECT_EQ(imebra::ageUnit_t::months, age->units);
    EXPECT_DOUBLE_EQ(0.41666666666666669, age->years());

    dataSet.setString(TagId(imebra::tagId_t::PatientAge_0010_1010), "018W");
    age.reset(dataSet.getAge(TagId(imebra::tagId_t::PatientAge_0010_1010), 0));
    EXPECT_EQ(18, age->age);
    EXPECT_EQ(imebra::ageUnit_t::weeks, age->units);
    EXPECT_DOUBLE_EQ(0.34520548039782323, age->years());

    dataSet.setString(TagId(imebra::tagId_t::PatientAge_0010_1010), "090Y");
    age.reset(dataSet.getAge(TagId(imebra::tagId_t::PatientAge_0010_1010), 0));
    EXPECT_EQ(90, age->age);
    EXPECT_EQ(imebra::ageUnit_t::years, age->units);
    EXPECT_DOUBLE_EQ(90, age->years());
}





} // namespace tests

} // namespace imebra
