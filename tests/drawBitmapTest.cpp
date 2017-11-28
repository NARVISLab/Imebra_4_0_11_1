#include <imebra/imebra.h>
#include "buildImageForTest.h"
#include <gtest/gtest.h>

namespace imebra
{

namespace tests
{

TEST(drawBitmapTest, testDrawBitmapRGB)
{
	for(int monochrome(0); monochrome != 2; ++monochrome)
	{
        for(int highBit = 15; highBit != 2; --highBit)
		{
            std::unique_ptr<Image> testImage(buildImageForTest(
                    401,
                    301,
                    bitDepth_t::depthU32,
                    highBit,
                    401,
                    301,
                    monochrome == 1 ? "MONOCHROME2" :
                                      "RGB",
					50));

            DataSet testDataSet("1.2.840.10008.1.2");
            testDataSet.setImage(0, *testImage, imageQuality_t::high);

            DrawBitmap testDraw;
            std::unique_ptr<ReadWriteMemory> bitmapBuffer(testDraw.getBitmap(*testImage, drawBitmapType_t::drawBitmapRGB, 1));
            size_t bufferSize;
            char* pBuffer(bitmapBuffer->data(&bufferSize));

            std::unique_ptr<ReadingDataHandler> imageHandler(testImage->getReadingDataHandler());

            std::uint32_t red, green, blue;
            size_t index(0);
            for(std::uint32_t scanY = 0; scanY != testImage->getHeight(); ++scanY)
			{
                for(std::uint32_t scanX = 0; scanX != testImage->getWidth(); ++scanX)
				{
                    std::uint8_t displayRed(*pBuffer++);
                    std::uint8_t displayGreen(*pBuffer++);
                    std::uint8_t displayBlue(*pBuffer++);

                    if(monochrome)
                    {
                        red = imageHandler->getUnsignedLong(index++);
                        if(highBit > 7)
                        {
                            red >>= (highBit - 7);
                        }
                        green = blue = red;
                    }
                    else
                    {
                        red = imageHandler->getUnsignedLong(index++);
                        green = imageHandler->getUnsignedLong(index++);
                        blue = imageHandler->getUnsignedLong(index++);
                        if(highBit > 7)
                        {
                            red >>= (highBit - 7);
                            green >>= (highBit - 7);
                            blue >>= (highBit - 7);
                        }
                    }

                    if(highBit < 7)
                    {
                        displayRed >>= (7 - highBit);
                        displayGreen >>= (7 - highBit);
                        displayBlue >>= (7 - highBit);
                    }
                    EXPECT_EQ(red, displayRed);
                    EXPECT_EQ(green, displayGreen);
                    EXPECT_EQ(blue, displayBlue);
				}

			}
		}
	}
}


TEST(drawBitmapTest, testDrawBitmapBGR)
{
    for(int monochrome(0); monochrome != 2; ++monochrome)
    {
        for(int highBit = 15; highBit != 2; --highBit)
        {
            std::unique_ptr<Image> testImage(buildImageForTest(
                    401,
                    301,
                    bitDepth_t::depthU32,
                    highBit,
                    401,
                    301,
                    monochrome == 1 ? "MONOCHROME2" :
                                      "RGB",
                    50));

            DataSet testDataSet("1.2.840.10008.1.2");
            testDataSet.setImage(0, *testImage, imageQuality_t::high);

            DrawBitmap testDraw;
            std::unique_ptr<ReadWriteMemory> bitmapBuffer(testDraw.getBitmap(*testImage, drawBitmapType_t::drawBitmapBGR, 4));
            size_t bufferSize;
            char* pBuffer(bitmapBuffer->data(&bufferSize));

            std::unique_ptr<ReadingDataHandler> imageHandler(testImage->getReadingDataHandler());

            std::uint32_t red, green, blue;
            size_t index(0);
            for(int scanY = 0; scanY != testImage->getHeight(); ++scanY)
            {
                for(int scanX = 0; scanX != testImage->getWidth(); ++scanX)
                {
                    std::uint8_t displayBlue(*pBuffer++);
                    std::uint8_t displayGreen(*pBuffer++);
                    std::uint8_t displayRed(*pBuffer++);

                    if(monochrome)
                    {
                        red = imageHandler->getUnsignedLong(index++);
                        if(highBit > 7)
                        {
                            red >>= (highBit - 7);
                        }
                        green = blue = red;
                    }
                    else
                    {
                        red = imageHandler->getUnsignedLong(index++);
                        green = imageHandler->getUnsignedLong(index++);
                        blue = imageHandler->getUnsignedLong(index++);
                        if(highBit > 7)
                        {
                            red >>= (highBit - 7);
                            green >>= (highBit - 7);
                            blue >>= (highBit - 7);
                        }
                    }

                    if(highBit < 7)
                    {
                        displayRed >>= (7 - highBit);
                        displayGreen >>= (7 - highBit);
                        displayBlue >>= (7 - highBit);
                    }
                    EXPECT_EQ(red, displayRed);
                    EXPECT_EQ(green, displayGreen);
                    EXPECT_EQ(blue, displayBlue);
                }
                pBuffer++; // 4 bytes alignment

            }
        }
    }
}

TEST(drawBitmapTest, testDrawBitmapRGBA)
{
    for(int monochrome(0); monochrome != 2; ++monochrome)
    {
        for(int highBit = 15; highBit != 2; --highBit)
        {
            std::unique_ptr<Image> testImage(buildImageForTest(
                    401,
                    301,
                    bitDepth_t::depthU32,
                    highBit,
                    401,
                    301,
                    monochrome == 1 ? "MONOCHROME2" :
                                      "RGB",
                    50));

            DataSet testDataSet("1.2.840.10008.1.2");
            testDataSet.setImage(0, *testImage, imageQuality_t::high);

            DrawBitmap testDraw;
            std::unique_ptr<ReadWriteMemory> bitmapBuffer(testDraw.getBitmap(*testImage, drawBitmapType_t::drawBitmapRGBA, 1));
            size_t bufferSize;
            char* pBuffer(bitmapBuffer->data(&bufferSize));

            std::unique_ptr<ReadingDataHandler> imageHandler(testImage->getReadingDataHandler());

            std::uint32_t red, green, blue;
            size_t index(0);
            for(int scanY = 0; scanY != testImage->getHeight(); ++scanY)
            {
                for(int scanX = 0; scanX != testImage->getWidth(); ++scanX)
                {
                    std::uint8_t displayRed(*pBuffer++);
                    std::uint8_t displayGreen(*pBuffer++);
                    std::uint8_t displayBlue(*pBuffer++);
                    std::uint8_t displayAlpha(*pBuffer++);

                    if(monochrome)
                    {
                        red = imageHandler->getUnsignedLong(index++);
                        if(highBit > 7)
                        {
                            red >>= (highBit - 7);
                        }
                        green = blue = red;
                    }
                    else
                    {
                        red = imageHandler->getUnsignedLong(index++);
                        green = imageHandler->getUnsignedLong(index++);
                        blue = imageHandler->getUnsignedLong(index++);
                        if(highBit > 7)
                        {
                            red >>= (highBit - 7);
                            green >>= (highBit - 7);
                            blue >>= (highBit - 7);
                        }
                    }

                    if(highBit < 7)
                    {
                        displayRed >>= (7 - highBit);
                        displayGreen >>= (7 - highBit);
                        displayBlue >>= (7 - highBit);
                    }
                    EXPECT_EQ(red, displayRed);
                    EXPECT_EQ(green, displayGreen);
                    EXPECT_EQ(blue, displayBlue);
                    EXPECT_EQ(255, displayAlpha);
                }
            }
        }
    }
}

TEST(drawBitmapTest, testDrawBitmapBGRA)
{
    for(int monochrome(0); monochrome != 2; ++monochrome)
    {
        for(int highBit = 15; highBit != 2; --highBit)
        {
            std::unique_ptr<Image> testImage(buildImageForTest(
                    401,
                    301,
                    bitDepth_t::depthU32,
                    highBit,
                    401,
                    301,
                    monochrome == 1 ? "MONOCHROME2" :
                                      "RGB",
                    50));

            DataSet testDataSet("1.2.840.10008.1.2");
            testDataSet.setImage(0, *testImage, imageQuality_t::high);

            DrawBitmap testDraw;
            std::unique_ptr<ReadWriteMemory> bitmapBuffer(testDraw.getBitmap(*testImage, drawBitmapType_t::drawBitmapBGRA, 4));
            size_t bufferSize;
            char* pBuffer(bitmapBuffer->data(&bufferSize));

            std::unique_ptr<ReadingDataHandler> imageHandler(testImage->getReadingDataHandler());

            std::uint32_t red, green, blue;
            size_t index(0);
            for(int scanY = 0; scanY != testImage->getHeight(); ++scanY)
            {
                for(int scanX = 0; scanX != testImage->getWidth(); ++scanX)
                {
                    std::uint8_t displayBlue(*pBuffer++);
                    std::uint8_t displayGreen(*pBuffer++);
                    std::uint8_t displayRed(*pBuffer++);
                    std::uint8_t displayAlpha(*pBuffer++);

                    if(monochrome)
                    {
                        red = imageHandler->getUnsignedLong(index++);
                        if(highBit > 7)
                        {
                            red >>= (highBit - 7);
                        }
                        green = blue = red;
                    }
                    else
                    {
                        red = imageHandler->getUnsignedLong(index++);
                        green = imageHandler->getUnsignedLong(index++);
                        blue = imageHandler->getUnsignedLong(index++);
                        if(highBit > 7)
                        {
                            red >>= (highBit - 7);
                            green >>= (highBit - 7);
                            blue >>= (highBit - 7);
                        }
                    }

                    if(highBit < 7)
                    {
                        displayRed >>= (7 - highBit);
                        displayGreen >>= (7 - highBit);
                        displayBlue >>= (7 - highBit);
                    }
                    EXPECT_EQ(red, displayRed);
                    EXPECT_EQ(green, displayGreen);
                    EXPECT_EQ(blue, displayBlue);
                    EXPECT_EQ(255, displayAlpha);
                }
            }
        }
    }
}

TEST(drawBitmapTest, testPalette)
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

    TransformsChain transformsChain;
    DrawBitmap drawBitmap(transformsChain);

    std::unique_ptr<ReadWriteMemory> drawMemory(drawBitmap.getBitmap(*(testDataSet.getImageApplyModalityTransform(0)), drawBitmapType_t::drawBitmapRGB, 1));

    size_t dataSize(0);
    unsigned char* pMemory((unsigned char*)drawMemory->data(&dataSize));
    size_t pointer(0);
    for(std::uint32_t checkY = 0; checkY < sizeY; ++checkY)
    {
        for(std::uint32_t checkX = 0; checkX < sizeX; ++checkX)
        {
            std::uint32_t r = (std::uint32_t)*(pMemory++);
            std::uint32_t g = (std::uint32_t)*(pMemory++);
            std::uint32_t b = (std::uint32_t)*(pMemory++);

            ASSERT_EQ(((pointer & 0xff) + 10) & 0xff, r);
            ASSERT_EQ(((pointer & 0xff) + 21) & 0xff, g);
            ASSERT_EQ(((pointer & 0xff) + 32) & 0xff, b);

            ++pointer;
        }
    }
}


// A buffer initialized to a default data type should use the data type OB
TEST(drawBitmapTest, testPalette16bit)
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
            redData->setUnsignedLong(fillPalette, ((fillPalette + 10) & 0xff) * 256);
            greenData->setUnsignedLong(fillPalette, ((fillPalette + 21) & 0xff) * 256);
            blueData->setUnsignedLong(fillPalette, ((fillPalette + 32) & 0xff) * 256);
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


    TransformsChain transformsChain;
    DrawBitmap drawBitmap(transformsChain);

    std::unique_ptr<ReadWriteMemory> drawMemory(drawBitmap.getBitmap(*(testDataSet.getImageApplyModalityTransform(0)), drawBitmapType_t::drawBitmapRGB, 1));

    size_t pointer(0);
    size_t dataSize(0);
    unsigned char* pMemory((unsigned char*)drawMemory->data(&dataSize));

    for(std::uint32_t checkY = 0; checkY < sizeY; ++checkY)
    {
        for(std::uint32_t checkX = 0; checkX < sizeX; ++checkX)
        {
            std::uint32_t r = (std::uint32_t)*(pMemory++);
            std::uint32_t g = (std::uint32_t)*(pMemory++);
            std::uint32_t b = (std::uint32_t)*(pMemory++);

            ASSERT_EQ(((pointer & 0xff) + 10) & 0xff, r);
            ASSERT_EQ(((pointer & 0xff) + 21) & 0xff, g);
            ASSERT_EQ(((pointer & 0xff) + 32) & 0xff, b);

            ++pointer;
        }
    }
}



} // namespace tests

} // namespace imebra
