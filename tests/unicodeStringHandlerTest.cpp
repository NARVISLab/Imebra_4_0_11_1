#include <imebra/imebra.h>
#include <gtest/gtest.h>

namespace imebra
{

namespace tests
{

TEST(unicodeStringHandlerTest, unicodeTest)
{

	// Try a cyrillic/arabic patient name
    std::wstring patientName0 = L"??\x0628\x062a\x062b\x062f^\0x400\0x410\0x420";
	std::wstring patientName1 = L"\0x420\x062a\x062b^\0x400\0x410\x0628\x062a";

    ReadWriteMemory streamMemory;
    {
        charsetsList_t charsets;
        charsets.push_back("ISO_IR 6");
        DataSet testDataSet("1.2.840.10008.1.2.1", charsets);

        {
            std::unique_ptr<WritingDataHandler> handler(testDataSet.getWritingDataHandler(TagId(0x10, 0x10), 0));

            handler->setUnicodeString(0, patientName0);
            handler->setUnicodeString(1, patientName1);
        }

        MemoryStreamOutput writeStream(streamMemory);
        StreamWriter writer(writeStream);
        CodecFactory::save(testDataSet, writer, codecType_t::dicom);
	}

	{
        MemoryStreamInput readStream(streamMemory);
        StreamReader reader(readStream);
        std::unique_ptr<DataSet> testDataSet(CodecFactory::load(reader));

        EXPECT_EQ(patientName0, testDataSet->getUnicodeString(TagId(0x0010, 0x0010), 0));
        EXPECT_EQ(patientName1, testDataSet->getUnicodeString(TagId(0x0010, 0x0010), 1));
        EXPECT_THROW(testDataSet->getUnicodeString(TagId(0x0010, 0x0010), 2), MissingItemError);
        EXPECT_THROW(testDataSet->getString(TagId(0x0010, 0x0010), 2), MissingItemError);
        EXPECT_THROW(testDataSet->getSignedLong(TagId(0x0010, 0x0010), 2), MissingItemError);
        EXPECT_THROW(testDataSet->getUnsignedLong(TagId(0x0010, 0x0010), 2), MissingItemError);
        EXPECT_THROW(testDataSet->getDouble(TagId(0x0010, 0x0010), 2), MissingItemError);
    }
}

TEST(unicodeStringHandlerTest, iso2022Test)
{
	// Try a cyrillic /greek/latin patient name
    std::wstring patientName0 = L"???\x0430\x0402\x0403\x00c9^\x00d0\x0410\x00d5";
	std::wstring patientName1 = L"\x0420\x00df\x0062^\x0394\x0410\x00d7\x0072";

    ReadWriteMemory streamMemory;
    {
        charsetsList_t charsets;
        charsets.push_back("ISO 2022 IR 6");
        DataSet testDataSet("1.2.840.10008.1.2.1", charsets);

        {
            std::unique_ptr<WritingDataHandler> handler(testDataSet.getWritingDataHandler(TagId(0x10, 0x10), 0));

            handler->setUnicodeString(0, patientName0);
            handler->setUnicodeString(1, patientName1);
        }

        MemoryStreamOutput writeStream(streamMemory);
        StreamWriter writer(writeStream);
        CodecFactory::save(testDataSet, writer, codecType_t::dicom);
    }

	{
        MemoryStreamInput readStream(streamMemory);
        StreamReader reader(readStream);
        std::unique_ptr<DataSet> testDataSet(CodecFactory::load(reader));

        EXPECT_EQ(patientName0, testDataSet->getUnicodeString(TagId(0x0010, 0x0010), 0));
        EXPECT_EQ(patientName1, testDataSet->getUnicodeString(TagId(0x0010, 0x0010), 1));
    }
}

#if !defined(_WIN32) && !defined(__CYGWIN__)
TEST(unicodeStringHandlerTest, supplementaryUnicodeChars)
{
    // Try supplementary chars
    std::wstring patientName0 = L"\x2070e\x20731\x20779\x20c53^\x00d0\x0410\x00d5";
    std::wstring patientName1 = L"\x0420\x00df\x0062^\x0394\x0410\x00d7\x0072";

    ReadWriteMemory streamMemory;
    {
        charsetsList_t charsets;
        charsets.push_back("ISO_IR 192");
        DataSet testDataSet("1.2.840.10008.1.2.1", charsets);

        {
            std::unique_ptr<WritingDataHandler> handler(testDataSet.getWritingDataHandler(TagId(0x10, 0x10), 0));

            handler->setUnicodeString(0, patientName0);
            handler->setUnicodeString(1, patientName1);
        }

        MemoryStreamOutput writeStream(streamMemory);
        StreamWriter writer(writeStream);
        CodecFactory::save(testDataSet, writer, codecType_t::dicom);
    }

    {
        MemoryStreamInput readStream(streamMemory);
        StreamReader reader(readStream);
        std::unique_ptr<DataSet> testDataSet(CodecFactory::load(reader));

        EXPECT_EQ(patientName0, testDataSet->getUnicodeString(TagId(0x0010, 0x0010), 0));
        EXPECT_EQ(patientName1, testDataSet->getUnicodeString(TagId(0x0010, 0x0010), 1));
    }
}
#endif

TEST(unicodeStringHandlerTest, unicodeNumericConversion)
{

    ReadWriteMemory streamMemory;
    {
        charsetsList_t charsets;
        charsets.push_back("ISO_IR 6");
        DataSet testDataSet("1.2.840.10008.1.2.1", charsets);

        {
            std::unique_ptr<WritingDataHandler> handler(testDataSet.getWritingDataHandler(TagId(0x10, 0x10), 0));

            handler->setDouble(0, 1000);
            handler->setUnsignedLong(1, 2000);
            handler->setSignedLong(2, 3000);
        }

        MemoryStreamOutput writeStream(streamMemory);
        StreamWriter writer(writeStream);
        CodecFactory::save(testDataSet, writer, codecType_t::dicom);
    }

    {
        MemoryStreamInput readStream(streamMemory);
        StreamReader reader(readStream);
        std::unique_ptr<DataSet> testDataSet(CodecFactory::load(reader));

        EXPECT_FLOAT_EQ(std::stod(L"1000"), std::stod(testDataSet->getUnicodeString(TagId(0x0010, 0x0010), 0)));
        EXPECT_EQ(1000, testDataSet->getSignedLong(TagId(0x0010, 0x0010), 0));
        EXPECT_EQ(1000, testDataSet->getUnsignedLong(TagId(0x0010, 0x0010), 0));
        EXPECT_FLOAT_EQ(1000, testDataSet->getDouble(TagId(0x0010, 0x0010), 0));
        EXPECT_EQ(2000, testDataSet->getSignedLong(TagId(0x0010, 0x0010), 1));
        EXPECT_EQ(2000, testDataSet->getUnsignedLong(TagId(0x0010, 0x0010), 1));
        EXPECT_FLOAT_EQ(2000, testDataSet->getDouble(TagId(0x0010, 0x0010), 1));
        EXPECT_EQ(3000, testDataSet->getSignedLong(TagId(0x0010, 0x0010), 2));
        EXPECT_EQ(3000, testDataSet->getUnsignedLong(TagId(0x0010, 0x0010), 2));
        EXPECT_FLOAT_EQ(3000, testDataSet->getDouble(TagId(0x0010, 0x0010), 2));
        EXPECT_THROW(testDataSet->getDate(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);
        EXPECT_THROW(testDataSet->getAge(TagId(0x0010, 0x0010), 0), DataHandlerConversionError);
    }
}



} // namespace tests

} // namespace imebra

