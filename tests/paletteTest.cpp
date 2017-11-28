#include <imebra/imebra.h>
#include <gtest/gtest.h>

namespace imebra
{

namespace tests
{


// A buffer initialized to a default data type should use the data type OB
TEST(paletteTest, testPalette)
{
    DataSet testDataSet("1.2.840.10008.1.2.1");

    {
        std::unique_ptr<WritingDataHandlerNumeric> redDescriptor(testDataSet.getWritingDataHandlerNumeric(TagId(tagId_t::RedPaletteColorLookupTableDescriptor_0028_1101), 0, tagVR_t::US));
        std::unique_ptr<WritingDataHandlerNumeric> redData(testDataSet.getWritingDataHandlerNumeric(TagId(tagId_t::RedPaletteColorLookupTableData_0028_1201), 0, tagVR_t::US));
        redDescriptor->setUnsignedLong(0, 256);
        redDescriptor->setUnsignedLong(1, 0);
        redDescriptor->setUnsignedLong(2, 8);

        std::unique_ptr<WritingDataHandlerNumeric> greenDescriptor(testDataSet.getWritingDataHandlerNumeric(TagId(tagId_t::GreenPaletteColorLookupTableDescriptor_0028_1102), 0, tagVR_t::US));
        std::unique_ptr<WritingDataHandlerNumeric> greenData(testDataSet.getWritingDataHandlerNumeric(TagId(tagId_t::GreenPaletteColorLookupTableData_0028_1202), 0, tagVR_t::US));
        greenDescriptor->setUnsignedLong(0, 256);
        greenDescriptor->setUnsignedLong(1, 0);
        greenDescriptor->setUnsignedLong(2, 8);

        std::unique_ptr<WritingDataHandlerNumeric> blueDescriptor(testDataSet.getWritingDataHandlerNumeric(TagId(tagId_t::BluePaletteColorLookupTableDescriptor_0028_1103), 0, tagVR_t::US));
        std::unique_ptr<WritingDataHandlerNumeric> blueData(testDataSet.getWritingDataHandlerNumeric(TagId(tagId_t::BluePaletteColorLookupTableData_0028_1203), 0, tagVR_t::US));
        blueDescriptor->setUnsignedLong(0, 256);
        blueDescriptor->setUnsignedLong(1, 0);
        blueDescriptor->setUnsignedLong(2, 8);

        for(size_t fillPalette(0); fillPalette != 256; fillPalette += 2)
        {
            redData->setUnsignedLong(fillPalette / 2, ((fillPalette + 10) & 0xff) | (((fillPalette + 11) & 0xff) << 8));
            greenData->setUnsignedLong(fillPalette / 2, ((fillPalette + 21) & 0xff) | (((fillPalette + 22) & 0xff) << 8));
            blueData->setUnsignedLong(fillPalette / 2, ((fillPalette + 32) & 0xff) | (((fillPalette + 33) & 0xff) << 8));
        }
    }

    size_t sizeX(300), sizeY(300);
    Image paletteImage(sizeX, sizeY, bitDepth_t::depthU8, "MONOCHROME2", 7);

    {
        std::unique_ptr<WritingDataHandlerNumeric> imageHandler(paletteImage.getWritingDataHandler());

        size_t pointer(0);

        // Make 3 bands (RGB)
        for(std::uint32_t y=0; y<sizeY; ++y)
        {
            for(std::uint32_t x=0; x<sizeX; ++x)
            {
                imageHandler->setUnsignedLong(pointer, pointer & 0xff);
                ++pointer;
            }
        }
    }

    testDataSet.setImage(0, paletteImage, imageQuality_t::veryHigh);
    testDataSet.setString(TagId(tagId_t::PhotometricInterpretation_0028_0004), "PALETTE COLOR");

    std::unique_ptr<Transform> colorTransform(ColorTransformsFactory::getTransform("PALETTE COLOR", "RGB"));
    std::unique_ptr<Image> storedImage(testDataSet.getImage(0));
    std::unique_ptr<Image> rgbImage(colorTransform->allocateOutputImage(*storedImage, sizeX, sizeY));

    colorTransform->runTransform(*storedImage, 0, 0, sizeX, sizeY, *rgbImage, 0, 0);

    ASSERT_EQ(sizeX, rgbImage->getWidth());
    ASSERT_EQ(sizeY, rgbImage->getHeight());

    std::unique_ptr<ReadingDataHandlerNumeric> rgbHandler(rgbImage->getReadingDataHandler());

    size_t pointer = 0;
    size_t rgbPointer = 0;
    for(std::uint32_t checkY = 0; checkY < sizeY; ++checkY)
    {
        for(std::uint32_t checkX = 0; checkX < sizeX; ++checkX)
        {
            std::uint32_t r = rgbHandler->getUnsignedLong(rgbPointer++);
            std::uint32_t g = rgbHandler->getUnsignedLong(rgbPointer++);
            std::uint32_t b = rgbHandler->getUnsignedLong(rgbPointer++);

            ASSERT_EQ(((pointer & 0xff) + 10) & 0xff, r);
            ASSERT_EQ(((pointer & 0xff) + 21) & 0xff, g);
            ASSERT_EQ(((pointer & 0xff) + 32) & 0xff, b);

            ++pointer;
        }
    }
}


// A buffer initialized to a default data type should use the data type OB
TEST(paletteTest, testPalette16bit)
{
    DataSet testDataSet("1.2.840.10008.1.2.1");

    {
        std::unique_ptr<WritingDataHandlerNumeric> redDescriptor(testDataSet.getWritingDataHandlerNumeric(TagId(tagId_t::RedPaletteColorLookupTableDescriptor_0028_1101), 0, tagVR_t::US));
        std::unique_ptr<WritingDataHandlerNumeric> redData(testDataSet.getWritingDataHandlerNumeric(TagId(tagId_t::RedPaletteColorLookupTableData_0028_1201), 0, tagVR_t::US));
        redDescriptor->setUnsignedLong(0, 256);
        redDescriptor->setUnsignedLong(1, 0);
        redDescriptor->setUnsignedLong(2, 16);

        std::unique_ptr<WritingDataHandlerNumeric> greenDescriptor(testDataSet.getWritingDataHandlerNumeric(TagId(tagId_t::GreenPaletteColorLookupTableDescriptor_0028_1102), 0, tagVR_t::US));
        std::unique_ptr<WritingDataHandlerNumeric> greenData(testDataSet.getWritingDataHandlerNumeric(TagId(tagId_t::GreenPaletteColorLookupTableData_0028_1202), 0, tagVR_t::US));
        greenDescriptor->setUnsignedLong(0, 256);
        greenDescriptor->setUnsignedLong(1, 0);
        greenDescriptor->setUnsignedLong(2, 16);

        std::unique_ptr<WritingDataHandlerNumeric> blueDescriptor(testDataSet.getWritingDataHandlerNumeric(TagId(tagId_t::BluePaletteColorLookupTableDescriptor_0028_1103), 0, tagVR_t::US));
        std::unique_ptr<WritingDataHandlerNumeric> blueData(testDataSet.getWritingDataHandlerNumeric(TagId(tagId_t::BluePaletteColorLookupTableData_0028_1203), 0, tagVR_t::US));
        blueDescriptor->setUnsignedLong(0, 256);
        blueDescriptor->setUnsignedLong(1, 0);
        blueDescriptor->setUnsignedLong(2, 16);

        for(size_t fillPalette(0); fillPalette != 256; fillPalette++)
        {
            redData->setUnsignedLong(fillPalette, (fillPalette + 10) & 0xff);
            greenData->setUnsignedLong(fillPalette, (fillPalette + 21) & 0xff);
            blueData->setUnsignedLong(fillPalette, (fillPalette + 32) & 0xff);
        }
    }

    size_t sizeX(300), sizeY(300);
    Image paletteImage(sizeX, sizeY, bitDepth_t::depthU8, "MONOCHROME2", 7);

    {
        std::unique_ptr<WritingDataHandlerNumeric> imageHandler(paletteImage.getWritingDataHandler());

        size_t pointer(0);

        // Make 3 bands (RGB)
        for(std::uint32_t y=0; y<sizeY; ++y)
        {
            for(std::uint32_t x=0; x<sizeX; ++x)
            {
                imageHandler->setUnsignedLong(pointer, pointer & 0xff);
                ++pointer;
            }
        }
    }

    testDataSet.setImage(0, paletteImage, imageQuality_t::veryHigh);
    testDataSet.setString(TagId(tagId_t::PhotometricInterpretation_0028_0004), "PALETTE COLOR");

    std::unique_ptr<Transform> colorTransform(ColorTransformsFactory::getTransform("PALETTE COLOR", "RGB"));
    std::unique_ptr<Image> storedImage(testDataSet.getImage(0));
    std::unique_ptr<Image> rgbImage(colorTransform->allocateOutputImage(*storedImage, sizeX, sizeY));

    colorTransform->runTransform(*storedImage, 0, 0, sizeX, sizeY, *rgbImage, 0, 0);

    ASSERT_EQ(sizeX, rgbImage->getWidth());
    ASSERT_EQ(sizeY, rgbImage->getHeight());

    std::unique_ptr<ReadingDataHandlerNumeric> rgbHandler(rgbImage->getReadingDataHandler());

    size_t pointer = 0;
    size_t rgbPointer = 0;
    for(std::uint32_t checkY = 0; checkY < sizeY; ++checkY)
    {
        for(std::uint32_t checkX = 0; checkX < sizeX; ++checkX)
        {
            std::uint32_t r = rgbHandler->getUnsignedLong(rgbPointer++);
            std::uint32_t g = rgbHandler->getUnsignedLong(rgbPointer++);
            std::uint32_t b = rgbHandler->getUnsignedLong(rgbPointer++);

            ASSERT_EQ(((pointer & 0xff) + 10) & 0xff, r);
            ASSERT_EQ(((pointer & 0xff) + 21) & 0xff, g);
            ASSERT_EQ(((pointer & 0xff) + 32) & 0xff, b);

            ++pointer;
        }
    }
}


} // namespace tests

} // namespace imebra
