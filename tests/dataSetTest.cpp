#include <imebra/imebra.h>

#include "buildImageForTest.h"
#include <list>
#include <string.h>
#include <memory>
#include <gtest/gtest.h>

namespace imebra
{

namespace tests
{

using namespace imebra;

TEST(dataSetTest, testFragmentation)
{
    // Add two images to a dataset, then fragment the first image
    std::unique_ptr<Image> testImage0(buildImageForTest(
        400,
        300,
        imebra::bitDepth_t::depthU8,
        7,
        400,
        300,
        "RGB",
        50));

    std::unique_ptr<Image> testImage1(buildImageForTest(
        400,
        300,
        imebra::bitDepth_t::depthU8,
        7,
        400,
        300,
        "RGB",
        20));

    imebra::DataSet testDataSet("1.2.840.10008.1.2.4.70");
    testDataSet.setImage(0, *testImage0, imageQuality_t::high);
    testDataSet.setImage(1, *testImage1, imageQuality_t::high);

    // Verify the two images
    std::unique_ptr<Image> verifyImage0(testDataSet.getImage(0));
    ASSERT_TRUE(compareImages(*testImage0, *verifyImage0) < 0.000001);
    std::unique_ptr<Image> verifyImage1(testDataSet.getImage(1));
    ASSERT_TRUE(compareImages(*testImage1, *verifyImage1) < 0.000001);

    {
        std::uint32_t offset(0);
        std::unique_ptr<WritingDataHandlerNumeric> offsetHandler(testDataSet.getWritingDataHandlerNumeric(TagId(imebra::tagId_t::PixelData_7FE0_0010), 0, tagVR_t::OB));

        std::list<std::shared_ptr<WritingDataHandler> > handlers;

        // Get the images 1 & 2, fragment them
        //////////////////////////////////////
        for(std::uint32_t scanBuffers = 1; scanBuffers != 3; ++scanBuffers)
        {
            size_t dataSize;
            offsetHandler->setSize(sizeof(std::uint32_t) * scanBuffers);
            std::uint32_t* pOffsetMemory = (std::uint32_t*)offsetHandler->data(&dataSize);
            pOffsetMemory[scanBuffers - 1] = offset;

            std::unique_ptr<ReadingDataHandlerNumeric> wholeHandler(testDataSet.getReadingDataHandlerNumeric(TagId(imebra::tagId_t::PixelData_7FE0_0010), scanBuffers));
            size_t totalSize;
            const char* pWholeMemory = wholeHandler->data(&totalSize);
            size_t fragmentedSize = totalSize / 3;
            if(fragmentedSize & 0x1)
            {
                ++fragmentedSize;
            }
            while(totalSize != 0)
            {
                size_t thisSize = totalSize;
                if(thisSize > fragmentedSize)
                {
                    thisSize = fragmentedSize;
                }
                std::shared_ptr<WritingDataHandlerNumeric> newHandler(testDataSet.getWritingDataHandlerNumeric(TagId(imebra::tagId_t::PixelData_7FE0_0010), handlers.size() + 1, tagVR_t::OB));
                newHandler->setSize(thisSize);
                newHandler->assign(pWholeMemory, thisSize);
                handlers.push_back(newHandler);
                offset += thisSize + 8;
                totalSize -= thisSize;
                pWholeMemory += thisSize;
            }
        }
    }

    std::unique_ptr<Image> compareImage0(testDataSet.getImage(0));
        ASSERT_TRUE(compareImages(*testImage0, *compareImage0) < 0.000001);
    std::unique_ptr<Image> compareImage1(testDataSet.getImage(1));
        ASSERT_TRUE(compareImages(*testImage1, *compareImage1) < 0.000001);
        ASSERT_TRUE(compareImages(*testImage0, *compareImage1) > 30);
}


TEST(dataSetTest, testVOIs)
{
    DataSet testDataSet;

    vois_t vois0 = testDataSet.getVOIs();
    ASSERT_TRUE(vois0.empty());

    {
        std::unique_ptr<WritingDataHandler> centerHandler(testDataSet.getWritingDataHandler(TagId(0x0028, 0x1050), 0));
        centerHandler->setDouble(0, 10.4);
        centerHandler->setDouble(1, 20.4);

        std::unique_ptr<WritingDataHandler> widthHandler(testDataSet.getWritingDataHandler(TagId(0x0028, 0x1051), 0));
        widthHandler->setDouble(0, 12.5);
        widthHandler->setDouble(1, 22.5);

        std::unique_ptr<WritingDataHandler> descriptionHandler(testDataSet.getWritingDataHandler(TagId(0x0028, 0x1055), 0));
        descriptionHandler->setUnicodeString(0, L"Test1");
        descriptionHandler->setUnicodeString(1, L"Test2");
    }

    vois_t vois1 = testDataSet.getVOIs();
    ASSERT_EQ(2, vois1.size());
    ASSERT_FLOAT_EQ(10.4, vois1.at(0).center);
    ASSERT_FLOAT_EQ(12.5, vois1.at(0).width);
    ASSERT_EQ(L"Test1", vois1.at(0).description);

    ASSERT_FLOAT_EQ(20.4, vois1.at(1).center);
    ASSERT_FLOAT_EQ(22.5, vois1.at(1).width);
    ASSERT_EQ(L"Test2", vois1.at(1).description);
}

TEST(dataSetTest, testGetTags)
{
    DataSet testDataSet;

    testDataSet.setString(TagId(tagId_t::PatientName_0010_0010), "Test patient");
    testDataSet.setAge(TagId(tagId_t::PatientAge_0010_1010), Age(3, ageUnit_t::years));

    tagsIds_t tags = testDataSet.getTags();
    bool bPatientName(false);
    bool bPatientAge(false);
    for(tagsIds_t::const_iterator scanTags(tags.begin()), endTags(tags.end()); scanTags != endTags; ++scanTags)
    {
        bPatientName |= (scanTags->getGroupId() == 0x0010 && scanTags->getTagId() == 0x0010);
        bPatientAge |= (scanTags->getGroupId() == 0x0010 && scanTags->getTagId() == 0x1010);
    }
    ASSERT_TRUE(bPatientName);
    ASSERT_TRUE(bPatientAge);
}

TEST(dataSetTest, testCreateTags)
{
    DataSet testDataSet;

    {
        std::unique_ptr<Tag> patientTag(testDataSet.getTagCreate(TagId(tagId_t::PatientName_0010_0010), tagVR_t::PN));
        std::unique_ptr<WritingDataHandler> patientHandler(patientTag->getWritingDataHandler(0));
        patientHandler->setString(0, "test0");
        patientHandler->setString(1, "test1");
    }

    std::unique_ptr<Tag> patientTag(testDataSet.getTag(TagId(tagId_t::PatientName_0010_0010)));
    std::unique_ptr<ReadingDataHandler> patientHandler(patientTag->getReadingDataHandler(0));
    ASSERT_EQ("test0", patientHandler->getString(0));
    ASSERT_EQ("test1", patientHandler->getString(1));
}


TEST(dataSetTest, testSetGetTags)
{
    DataSet testDataSet;

    testDataSet.setAge(TagId(tagId_t::PatientAge_0010_1010), Age(3, ageUnit_t::months));
    testDataSet.setDate(TagId(tagId_t::AcquisitionDateTime_0008_002A), Date(2014, 2, 1, 12, 20, 30, 0, 0, 0));
    testDataSet.setDate(TagId(tagId_t::PatientBirthDate_0010_0030), Date(2000, 1, 2, 13, 30, 40, 0, 0, 0), tagVR_t::DT);
    testDataSet.setString(TagId(tagId_t::PatientName_0010_0010), "Test patient");
    testDataSet.setDouble(TagId(0x20, 0x20), 45.6, tagVR_t::OD);
    testDataSet.setSignedLong(TagId(0x20, 0x21), 50, tagVR_t::SL);
    testDataSet.setUnsignedLong(TagId(0x20, 0x22), 60, tagVR_t::UL);

    std::unique_ptr<Age> age0(testDataSet.getAge(TagId(tagId_t::PatientAge_0010_1010), 0));
    ASSERT_EQ(3, age0->age);
    ASSERT_EQ(ageUnit_t::months, age0->units);
    ASSERT_EQ("003M", testDataSet.getString(TagId(tagId_t::PatientAge_0010_1010), 0));
    ASSERT_THROW(testDataSet.getSignedLong(TagId(tagId_t::PatientAge_0010_1010), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getUnsignedLong(TagId(tagId_t::PatientAge_0010_1010), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getDouble(TagId(tagId_t::PatientAge_0010_1010), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getDouble(TagId(tagId_t::PatientAge_0010_1010), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getDate(TagId(tagId_t::PatientAge_0010_1010), 0), DataHandlerConversionError);

    std::unique_ptr<Date> date0(testDataSet.getDate(TagId(tagId_t::AcquisitionDateTime_0008_002A), 0));
    ASSERT_EQ(2014, date0->year);
    ASSERT_EQ(2, date0->month);
    ASSERT_EQ(1, date0->day);
    ASSERT_THROW(testDataSet.getSignedLong(TagId(tagId_t::AcquisitionDateTime_0008_002A), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getUnsignedLong(TagId(tagId_t::AcquisitionDateTime_0008_002A), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getDouble(TagId(tagId_t::AcquisitionDateTime_0008_002A), 0), DataHandlerConversionError);
    ASSERT_THROW(testDataSet.getAge(TagId(tagId_t::AcquisitionDateTime_0008_002A), 0), DataHandlerConversionError);

    std::unique_ptr<Date> date1(testDataSet.getDate(TagId(tagId_t::PatientBirthDate_0010_0030), 0));
    ASSERT_EQ(2000, date1->year);
    ASSERT_EQ(1, date1->month);
    ASSERT_EQ(2, date1->day);

    ASSERT_EQ("Test patient", testDataSet.getString(TagId(tagId_t::PatientName_0010_0010), 0));
    ASSERT_FLOAT_EQ(45.6, testDataSet.getDouble(TagId(0x20, 0x20), 0));
    ASSERT_EQ(50, testDataSet.getSignedLong(TagId(0x20, 0x21), 0));
    ASSERT_EQ(60, testDataSet.getUnsignedLong(TagId(0x20, 0x22), 0));

}

TEST(dataSetTest, defaults)
{
    Age defaultAge(5, ageUnit_t::years);
    Date defaultDate(2015, 3, 2, 12, 0, 0, 0, 0, 0);
    std::uint32_t defaultUnsigned(150);
    std::uint32_t defaultSigned(-10);
    double defaultDouble(10.0);
    std::string defaultString("defaultstring");
    std::wstring defaultUnicodeString(L"defaultUnicodeString");

    DataSet testDataSet;

    std::unique_ptr<Age> getDefaultAge(testDataSet.getAge(TagId(20, 20), 0, defaultAge));
    ASSERT_EQ(defaultAge.age, getDefaultAge->age);
    ASSERT_EQ(defaultAge.units, getDefaultAge->units);

    std::unique_ptr<Date> getDefaultDate(testDataSet.getDate(TagId(20, 20), 0, defaultDate));
    ASSERT_EQ(defaultDate.year, getDefaultDate->year);
    ASSERT_EQ(defaultDate.month, getDefaultDate->month);
    ASSERT_EQ(defaultDate.day, getDefaultDate->day);
    ASSERT_EQ(defaultDate.hour, getDefaultDate->hour);
    ASSERT_EQ(defaultDate.minutes, getDefaultDate->minutes);
    ASSERT_EQ(defaultDate.seconds, getDefaultDate->seconds);
    ASSERT_EQ(defaultDate.nanoseconds, getDefaultDate->nanoseconds);
    ASSERT_EQ(defaultDate.offsetHours, getDefaultDate->offsetHours);
    ASSERT_EQ(defaultDate.offsetMinutes, getDefaultDate->offsetMinutes);

    ASSERT_EQ(defaultUnsigned, testDataSet.getUnsignedLong(TagId(20, 20), 0, defaultUnsigned));
    ASSERT_EQ(defaultSigned, testDataSet.getSignedLong(TagId(20, 20), 0, defaultSigned));
    ASSERT_FLOAT_EQ(defaultDouble, testDataSet.getDouble(TagId(20, 20), 0, defaultDouble));
    ASSERT_EQ(defaultString, testDataSet.getString(TagId(20, 20), 0, defaultString));
    ASSERT_EQ(defaultUnicodeString, testDataSet.getUnicodeString(TagId(20, 20), 0, defaultUnicodeString));
}

TEST(dataSetTest, testSequence)
{
    for(int transferSyntaxId(0); transferSyntaxId != 4; ++transferSyntaxId)
    {
        std::string transferSyntax;
        switch(transferSyntaxId)
        {
        case 0:
            transferSyntax = "1.2.840.10008.1.2";
            break;
        case 1:
            transferSyntax = "1.2.840.10008.1.2.1";
            break;
        case 2:
            transferSyntax = "1.2.840.10008.1.2.2";
            break;
        case 3:
            transferSyntax = "1.2.840.10008.1.2.5";
            break;
        }

        std::cout << "Sequence test. Transfer syntax: " << transferSyntax << std::endl;

        DataSet testDataSet(transferSyntax);

        {
            DataSet sequence0;
            DataSet sequence1;

            sequence0.setString(TagId(0x10, 0x10), "Test0");
            sequence1.setString(TagId(0x10, 0x10), "Test1");

            testDataSet.setSequenceItem(TagId(tagId_t::ReferencedPerformedProcedureStepSequence_0008_1111), 0, sequence0);
            testDataSet.setSequenceItem(TagId(tagId_t::ReferencedPerformedProcedureStepSequence_0008_1111), 1, sequence1);
        }

        {
            std::unique_ptr<DataSet> sequence0(testDataSet.getSequenceItem(TagId(tagId_t::ReferencedPerformedProcedureStepSequence_0008_1111), 0));
            std::unique_ptr<DataSet> sequence1(testDataSet.getSequenceItem(TagId(tagId_t::ReferencedPerformedProcedureStepSequence_0008_1111), 1));
            ASSERT_EQ("Test0", sequence0->getString(TagId(0x10, 0x10), 0));
            ASSERT_EQ("Test1", sequence1->getString(TagId(0x10, 0x10), 0));
        }

        ReadWriteMemory encodedDataSet;
        MemoryStreamOutput outputStream(encodedDataSet);
        StreamWriter outputWriter(outputStream);
        CodecFactory::save(testDataSet, outputWriter, codecType_t::dicom);

        MemoryStreamInput inputStream(encodedDataSet);
        StreamReader inputReader(inputStream);
        std::unique_ptr<DataSet> readDataSet(CodecFactory::load(inputReader));
        {
            std::unique_ptr<DataSet> sequence0(readDataSet->getSequenceItem(TagId(tagId_t::ReferencedPerformedProcedureStepSequence_0008_1111), 0));
            std::unique_ptr<DataSet> sequence1(readDataSet->getSequenceItem(TagId(tagId_t::ReferencedPerformedProcedureStepSequence_0008_1111), 1));
            ASSERT_EQ("Test0", sequence0->getString(TagId(0x10, 0x10), 0));
            ASSERT_EQ("Test1", sequence1->getString(TagId(0x10, 0x10), 0));
        }
    }
}

TEST(dataSetTest, dataHandler)
{
    {
        DataSet testDataSet;
        {
            std::unique_ptr<WritingDataHandler> handler(testDataSet.getWritingDataHandler(TagId(tagId_t::PatientName_0010_0010), 0, tagVR_t::UL));
            handler->setSize(10);
            ASSERT_EQ(tagVR_t::UL, handler->getDataType());
            ASSERT_EQ(10, handler->getSize());
        }
        std::unique_ptr<ReadingDataHandler> handler(testDataSet.getReadingDataHandler(TagId(tagId_t::PatientName_0010_0010), 0));
        std::unique_ptr<ReadingDataHandler> raw(testDataSet.getReadingDataHandlerRaw(TagId(tagId_t::PatientName_0010_0010), 0));
        ASSERT_EQ(tagVR_t::UL, handler->getDataType());
        ASSERT_EQ(10, handler->getSize());
        ASSERT_EQ(40, raw->getSize());
    }

    {
        DataSet testDataSet;
        {
            std::unique_ptr<WritingDataHandlerNumeric> handler(testDataSet.getWritingDataHandlerNumeric(TagId(tagId_t::RegionLocationMinX0_0018_6018), 0));
            handler->setSignedLong(0, 100);
            handler->setSize(10);
            ASSERT_EQ(tagVR_t::UL, handler->getDataType());
            ASSERT_EQ(10, handler->getSize());
        }
        std::unique_ptr<ReadingDataHandlerNumeric> handler(testDataSet.getReadingDataHandlerNumeric(TagId(tagId_t::RegionLocationMinX0_0018_6018), 0));
        std::unique_ptr<ReadingDataHandler> raw(testDataSet.getReadingDataHandlerRaw(TagId(tagId_t::RegionLocationMinX0_0018_6018), 0));
        ASSERT_EQ(100, handler->getSignedLong(0));
        ASSERT_EQ(tagVR_t::UL, handler->getDataType());
        ASSERT_EQ(10, handler->getSize());
        ASSERT_EQ(40, raw->getSize());
    }

    {
        DataSet testDataSet;
        {
            std::unique_ptr<WritingDataHandlerNumeric> handler(testDataSet.getWritingDataHandlerRaw(TagId(tagId_t::RegionLocationMinX0_0018_6018), 0));
            handler->setSize(16);
            ASSERT_EQ(tagVR_t::UL, handler->getDataType());
            ASSERT_EQ(16, handler->getSize());
        }
        std::unique_ptr<ReadingDataHandlerNumeric> handler(testDataSet.getReadingDataHandlerNumeric(TagId(tagId_t::RegionLocationMinX0_0018_6018), 0));
        std::unique_ptr<ReadingDataHandler> raw(testDataSet.getReadingDataHandlerRaw(TagId(tagId_t::RegionLocationMinX0_0018_6018), 0));
        ASSERT_EQ(tagVR_t::UL, handler->getDataType());
        ASSERT_EQ(4, handler->getSize());
        ASSERT_EQ(16, raw->getSize());
    }

    {
        DataSet testDataSet;
        {
            std::unique_ptr<WritingDataHandler> handler(testDataSet.getWritingDataHandler(TagId(tagId_t::PatientName_0010_0010), 0));
            handler->setSize(2);
            ASSERT_EQ(tagVR_t::PN, handler->getDataType());
            ASSERT_EQ(2, handler->getSize());
        }
        std::unique_ptr<ReadingDataHandler> handler(testDataSet.getReadingDataHandler(TagId(tagId_t::PatientName_0010_0010), 0));
        ASSERT_EQ(tagVR_t::PN, handler->getDataType());
        ASSERT_EQ(2, handler->getSize());
    }

    {
        DataSet testDataSet;
        {
            std::unique_ptr<WritingDataHandlerNumeric> handler(testDataSet.getWritingDataHandlerRaw(TagId(tagId_t::PixelData_7FE0_0010), 0, tagVR_t::OW));
            handler->setSize(20);
            ASSERT_EQ(tagVR_t::OW, handler->getDataType());
            ASSERT_EQ(20, handler->getSize());
        }
        std::unique_ptr<ReadingDataHandlerNumeric> handler(testDataSet.getReadingDataHandlerNumeric(TagId(tagId_t::PixelData_7FE0_0010), 0));
        ASSERT_EQ(tagVR_t::OW, handler->getDataType());
        ASSERT_EQ(10, handler->getSize());
    }
}


TEST(dataSetTest, testEmptySequence)
{
    for(int transferSyntaxId(0); transferSyntaxId != 4; ++transferSyntaxId)
    {
        std::string transferSyntax;
        switch(transferSyntaxId)
        {
        case 0:
            transferSyntax = "1.2.840.10008.1.2";
            break;
        case 1:
            transferSyntax = "1.2.840.10008.1.2.1";
            break;
        case 2:
            transferSyntax = "1.2.840.10008.1.2.2";
            break;
        case 3:
            transferSyntax = "1.2.840.10008.1.2.5";
            break;
        }

        std::cout << "Sequence test. Transfer syntax: " << transferSyntax << std::endl;

        DataSet testDataSet(transferSyntax);

        {
            DataSet sequence0;
            DataSet sequence1;

            testDataSet.setSequenceItem(TagId(tagId_t::ReferencedPerformedProcedureStepSequence_0008_1111), 0, sequence0);
            testDataSet.setSequenceItem(TagId(tagId_t::ReferencedPerformedProcedureStepSequence_0008_1111), 1, sequence1);
        }

        {
            std::unique_ptr<DataSet> sequence0(testDataSet.getSequenceItem(TagId(tagId_t::ReferencedPerformedProcedureStepSequence_0008_1111), 0));
            std::unique_ptr<DataSet> sequence1(testDataSet.getSequenceItem(TagId(tagId_t::ReferencedPerformedProcedureStepSequence_0008_1111), 1));
        }

        ReadWriteMemory encodedDataSet;
        MemoryStreamOutput outputStream(encodedDataSet);
        StreamWriter outputWriter(outputStream);
        CodecFactory::save(testDataSet, outputWriter, codecType_t::dicom);

        MemoryStreamInput inputStream(encodedDataSet);
        StreamReader inputReader(inputStream);
        std::unique_ptr<DataSet> readDataSet(CodecFactory::load(inputReader));
        {
            std::unique_ptr<Tag> sequenceTag(readDataSet->getTag(TagId(tagId_t::ReferencedPerformedProcedureStepSequence_0008_1111)));
            ASSERT_EQ(tagVR_t::SQ, sequenceTag->getDataType());
            ASSERT_THROW(sequenceTag->getSequenceItem(0), MissingItemError);
            ASSERT_THROW(sequenceTag->getSequenceItem(1), MissingItemError);
            ASSERT_THROW(readDataSet->getSequenceItem(TagId(tagId_t::ReferencedPerformedProcedureStepSequence_0008_1111), 0), MissingItemError);
            ASSERT_THROW(readDataSet->getSequenceItem(TagId(tagId_t::ReferencedPerformedProcedureStepSequence_0008_1111), 1), MissingItemError);
        }
    }
}


} // namespace tests

} // namespace imebra
