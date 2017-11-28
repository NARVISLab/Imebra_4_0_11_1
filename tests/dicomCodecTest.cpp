#include <imebra/imebra.h>
#include "buildImageForTest.h"
#include "testsSettings.h"
#include <gtest/gtest.h>
#include <limits>

namespace imebra
{

namespace tests
{

TEST(dicomCodecTest, testDicom)
{
    char* colorSpaces[] = {"MONOCHROME2", "RGB", "YBR_FULL", "YBR_FULL_422", "YBR_FULL_420"};

    std::uint32_t highBitStep = 1;
    std::uint32_t interleavedStart = 0;
    std::uint32_t signStep = 1;

    std::uint32_t sizeX(201);
    std::uint32_t sizeY(151);

    for(int transferSyntaxId(0); transferSyntaxId != 4; ++transferSyntaxId)
	{
        for(std::uint32_t interleaved(interleavedStart); interleaved != 2; ++interleaved)
		{
            for(std::uint32_t sign=0; sign != 2; sign += signStep)
			{
                for(std::uint32_t highBit(0); highBit != 32; highBit += highBitStep)
				{
                    for(unsigned int colorSpaceIndex(0); colorSpaceIndex != sizeof(colorSpaces)/sizeof(colorSpaces[0]); ++colorSpaceIndex)
                    {
                        std::string colorSpace(colorSpaces[colorSpaceIndex]);
                        if((highBit > 24 || transferSyntaxId == 3 || interleaved == 0) &&
                                (ColorTransformsFactory::isSubsampledX(colorSpace) || ColorTransformsFactory::isSubsampledX(colorSpace)))
                        {
                            continue;
                        }

                        bitDepth_t depth(sign == 0 ? bitDepth_t::depthU8 : bitDepth_t::depthS8);
                        if(highBit > 7)
                        {
                            depth = (sign == 0 ? bitDepth_t::depthU16 : bitDepth_t::depthS16);
                        }
                        if(highBit > 15)
                        {
                            depth = (sign == 0 ? bitDepth_t::depthU32 : bitDepth_t::depthS32);
                        }

                        std::unique_ptr<Image> dicomImage0, dicomImage1, dicomImage2;

                        if(ColorTransformsFactory::isSubsampledY(colorSpace) || ColorTransformsFactory::isSubsampledX(colorSpace))
                        {
                            dicomImage0.reset(buildSubsampledImage(
                                    sizeX,
                                    sizeY,
                                    depth,
                                    highBit,
                                    30,
                                    20,
                                    colorSpace));
                            dicomImage1.reset(buildSubsampledImage(
                                    sizeX,
                                    sizeY,
                                    depth,
                                    highBit,
                                    30,
                                    20,
                                    colorSpace));
                            dicomImage2.reset(buildSubsampledImage(
                                    sizeX,
                                    sizeY,
                                    depth,
                                    highBit,
                                    30,
                                    20,
                                    colorSpace));
                        }
                        else
                        {
                            dicomImage0.reset(buildImageForTest(
                                    sizeX,
                                    sizeY,
                                    depth,
                                    highBit,
                                    30,
                                    20,
                                    colorSpace,
                                    1));
                            dicomImage1.reset(buildImageForTest(
                                    sizeX,
                                    sizeY,
                                    depth,
                                    highBit,
                                    30,
                                    20,
                                    colorSpace,
                                    100));
                            dicomImage2.reset(buildImageForTest(
                                    sizeX,
                                    sizeY,
                                    depth,
                                    highBit,
                                    30,
                                    20,
                                    colorSpace,
                                    150));

                        }

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

                        std::cout << "Dicom test. Transfer syntax: " << transferSyntax;
                        std::cout << " interleaved: " << interleaved;
                        std::cout << " sign: " << sign;
                        std::cout << " highBit: " << highBit << std::endl;
                        std::cout << " colorSpace: " << colorSpace << std::endl;

                        imageQuality_t quality = imageQuality_t::veryHigh;
                        if(ColorTransformsFactory::isSubsampledY(colorSpace))
                        {
                            quality = imageQuality_t::belowMedium;
                        }
                        else if(ColorTransformsFactory::isSubsampledX(colorSpace))
                        {
                            quality = imageQuality_t::medium;
                        }

                        ReadWriteMemory streamMemory;
                        {
                            DataSet testDataSet(transferSyntax);
                            std::unique_ptr<WritingDataHandler> writingDataHandler(testDataSet.getWritingDataHandler(TagId(0x0010, 0x0010), 0));
                            writingDataHandler->setString(0, "AAAaa");
                            writingDataHandler->setString(1, "BBBbbb");
                            writingDataHandler->setString(2, "");
                            writingDataHandler.reset();
                            testDataSet.setDouble(TagId(tagId_t::TimeRange_0008_1163), 50.6);
                            if(ColorTransformsFactory::getNumberOfChannels(colorSpace) > 1)
                            {
                                testDataSet.setUnsignedLong(TagId(imebra::tagId_t::PlanarConfiguration_0028_0006), 1 - interleaved);
                            }
                            testDataSet.setImage(0, *dicomImage0, quality);
                            testDataSet.setImage(1, *dicomImage1, quality);
                            testDataSet.setImage(2, *dicomImage2, quality);

                            DataSet sequenceItem;
                            sequenceItem.setString(TagId(tagId_t::PatientName_0010_0010), "test test");

                            testDataSet.setSequenceItem(TagId(tagId_t::ReferencedPerformedProcedureStepSequence_0008_1111), 0, sequenceItem);

                            MemoryStreamOutput writeStream(streamMemory);
                            StreamWriter writer(writeStream);
                            CodecFactory::save(testDataSet, writer, codecType_t::dicom);
                        }

                        for(unsigned int lazyLoad(0); lazyLoad != 2; ++lazyLoad)
                        {
                            MemoryStreamInput readStream(streamMemory);
                            StreamReader reader(readStream);
                            std::unique_ptr<DataSet> testDataSet(CodecFactory::load(reader, lazyLoad == 0 ? std::numeric_limits<size_t>::max() : 1));

                            EXPECT_EQ(0, testDataSet->getUnsignedLong(TagId(tagId_t::FileMetaInformationVersion_0002_0001), 0));
                            EXPECT_EQ(1, testDataSet->getUnsignedLong(TagId(tagId_t::FileMetaInformationVersion_0002_0001), 1));
                            EXPECT_THROW(testDataSet->getUnsignedLong(TagId(tagId_t::FileMetaInformationVersion_0002_0001), 2), MissingItemError);
                            EXPECT_EQ(tagVR_t::OB, testDataSet->getDataType(TagId(tagId_t::FileMetaInformationVersion_0002_0001)));

                            EXPECT_EQ(std::string("AAAaa"), testDataSet->getString(TagId(imebra::tagId_t::PatientName_0010_0010), 0));
                            EXPECT_EQ(std::string("BBBbbb"), testDataSet->getString(TagId(imebra::tagId_t::PatientName_0010_0010), 1));
                            EXPECT_EQ(std::string(""), testDataSet->getString(TagId(imebra::tagId_t::PatientName_0010_0010), 2));
                            EXPECT_FLOAT_EQ(50.6, testDataSet->getDouble(TagId(tagId_t::TimeRange_0008_1163), 0));

                            std::unique_ptr<DataSet> sequenceItem(testDataSet->getSequenceItem(TagId(tagId_t::ReferencedPerformedProcedureStepSequence_0008_1111), 0));
                            EXPECT_EQ("test test", sequenceItem->getString(TagId(tagId_t::PatientName_0010_0010), 0));
                            EXPECT_THROW(sequenceItem->getUnsignedLong(TagId(tagId_t::FileMetaInformationVersion_0002_0001), 0), MissingGroupError);

                            std::unique_ptr<Image> checkImage0(testDataSet->getImage(0));
                            std::unique_ptr<Image> checkImage1(testDataSet->getImage(1));
                            std::unique_ptr<Image> checkImage2(testDataSet->getImage(2));

                            if(checkImage0->getChannelsNumber() == 1)
                            {
                                ASSERT_THROW(testDataSet->getTag(TagId(tagId_t::PlanarConfiguration_0028_0006)), MissingDataElementError);
                            }
                            else
                            {
                                EXPECT_EQ(1 - interleaved, testDataSet->getSignedLong(TagId(imebra::tagId_t::PlanarConfiguration_0028_0006), 0));
                            }

                            ASSERT_TRUE(identicalImages(*checkImage0, *dicomImage0));
                            ASSERT_TRUE(identicalImages(*checkImage1, *dicomImage1));
                            ASSERT_TRUE(identicalImages(*checkImage2, *dicomImage2));

                        }
                    }
				}
			}
		}
	} // transferSyntaxId
}


TEST(dicomCodecTest, testDicom32bit)
{
    for(int transferSyntaxId(0); transferSyntaxId != 4; ++transferSyntaxId)
    {
        std::string colorSpace("MONOCHROME2");

        std::unique_ptr<Image> dicomImage(new Image(3, 1, bitDepth_t::depthU32, colorSpace, 31));
        {
            std::unique_ptr<WritingDataHandlerNumeric> write(dicomImage->getWritingDataHandler());
            write->setUnsignedLong(0, std::numeric_limits<std::uint32_t>::max());
            write->setUnsignedLong(1, std::numeric_limits<std::uint32_t>::max() / 2);
            write->setUnsignedLong(2, 0);
        }

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

        std::cout << "Dicom test. Transfer syntax: " << transferSyntax;
        std::cout << " maxValue: " << std::numeric_limits<std::uint32_t>::max() << std::endl;

        imageQuality_t quality = imageQuality_t::veryHigh;
        if(ColorTransformsFactory::isSubsampledY(colorSpace))
        {
            quality = imageQuality_t::belowMedium;
        }
        else if(ColorTransformsFactory::isSubsampledX(colorSpace))
        {
            quality = imageQuality_t::medium;
        }

        ReadWriteMemory streamMemory;
        {
            DataSet testDataSet(transferSyntax);
            testDataSet.setImage(0, *dicomImage, imageQuality_t::veryHigh);

            MemoryStreamOutput writeStream(streamMemory);
            StreamWriter writer(writeStream);
            CodecFactory::save(testDataSet, writer, codecType_t::dicom);
        }

        MemoryStreamInput readStream(streamMemory);
        StreamReader reader(readStream);
        std::unique_ptr<DataSet> testDataSet(CodecFactory::load(reader, std::numeric_limits<size_t>::max()));

        std::unique_ptr<Image> checkImage(testDataSet->getImage(0));

        std::unique_ptr<ReadingDataHandlerNumeric> read(checkImage->getReadingDataHandler());
        EXPECT_EQ(std::numeric_limits<std::uint32_t>::max(), read->getUnsignedLong(0));
        EXPECT_EQ(std::numeric_limits<std::uint32_t>::max() / 2, read->getUnsignedLong(1));
        EXPECT_EQ(0, read->getUnsignedLong(2));
    }
}


TEST(dicomCodecTest, testImplicitPrivateTags)
{
    ReadWriteMemory streamMemory;
    {
        DataSet testDataSet("1.2.840.10008.1.2");
        testDataSet.setString(TagId(tagId_t::PatientName_0010_0010), "Patient name");
        testDataSet.setString(TagId(std::uint16_t(11), std::uint16_t(2)), "Private tag", tagVR_t::ST);

        MemoryStreamOutput writeStream(streamMemory);
        StreamWriter writer(writeStream);
        CodecFactory::save(testDataSet, writer, codecType_t::dicom);
    }

    MemoryStreamInput readStream(streamMemory);
    StreamReader reader(readStream);
    std::unique_ptr<DataSet> testDataSet(CodecFactory::load(reader, std::numeric_limits<size_t>::max()));

    EXPECT_EQ("Patient name", testDataSet->getString(TagId(tagId_t::PatientName_0010_0010), 0));

    std::unique_ptr<ReadingDataHandlerNumeric> privateHandler(testDataSet->getReadingDataHandlerNumeric(TagId(std::uint16_t(11), std::uint16_t(2)), 0));
    EXPECT_EQ(tagVR_t::UN, privateHandler->getDataType());

    size_t length;
    std::string privateString(privateHandler->data(&length));
    EXPECT_EQ("Private tag ", privateString); // Even length


}

} // namespace tests

} // namespace imebra

