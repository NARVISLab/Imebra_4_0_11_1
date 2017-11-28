#include <gtest/gtest.h>
#include <imebra/imebra.h>

namespace imebra
{

namespace tests
{

// A buffer initialized to a default data type should use the data type OB
TEST(bitTransformTest, bitShift)
{
    std::uint32_t width = 41;
    std::uint32_t height = 13;
    Image bits8Image(width, height, bitDepth_t::depthU8, "RGB", 7);
    Image bits16Image(width, height, bitDepth_t::depthU16, "RGB", 15);
    Image bits4Image(width, height, bitDepth_t::depthU8, "RGB", 3);
    {
        std::unique_ptr<WritingDataHandler> imageHandler(bits8Image.getWritingDataHandler());

        // Make 3 bands (RGB)
        size_t elementNumber(0);
        for(std::uint32_t y=0; y<height; ++y)
        {
            for(std::uint32_t x=0; x<width; ++x)
            {
                std::int32_t r, g, b;
                std::uint32_t value = y * 255 / height;
                r = g = 0;
                b = value;
                if(x < width - width/3)
                {
                    r = 0;
                    g = value;
                    b = 0;
                }
                if(x < width / 3)
                {
                    r = value;
                    g = 0;
                    b = 0;
                }
                imageHandler->setUnsignedLong(elementNumber++, r);
                imageHandler->setUnsignedLong(elementNumber++, g);
                imageHandler->setUnsignedLong(elementNumber++, b);
            }
        }
    }

    TransformHighBit transform;
    transform.runTransform(bits8Image, 0, 0, width, height, bits16Image, 0, 0);
    transform.runTransform(bits8Image, 0, 0, width, height, bits4Image, 0, 0);


    std::unique_ptr<ReadingDataHandler> bits8Handler(bits8Image.getReadingDataHandler());
    std::unique_ptr<ReadingDataHandler> bits16Handler(bits16Image.getReadingDataHandler());
    std::unique_ptr<ReadingDataHandler> bits4Handler(bits4Image.getReadingDataHandler());
    size_t elementNumber = 0;
    for(std::uint32_t checkY = 0; checkY < height; ++checkY)
    {
        for(std::uint32_t checkX = 0; checkX < width; ++checkX)
        {
            std::int32_t r, g, b;
            std::uint32_t value = checkY * 255 / height;
            r = g = 0;
            b = value;
            if(checkX < width - width/3)
            {
                r = 0;
                g = value;
                b = 0;
            }
            if(checkX < width / 3)
            {
                r = value;
                g = 0;
                b = 0;
            }

            std::int32_t value0r = bits8Handler->getUnsignedLong(elementNumber);
            std::int32_t value1r = bits16Handler->getUnsignedLong(elementNumber);
            std::int32_t value2r = bits4Handler->getUnsignedLong(elementNumber++);

            std::int32_t value0g = bits8Handler->getUnsignedLong(elementNumber);
            std::int32_t value1g = bits16Handler->getUnsignedLong(elementNumber);
            std::int32_t value2g = bits4Handler->getUnsignedLong(elementNumber++);

            std::int32_t value0b = bits8Handler->getUnsignedLong(elementNumber);
            std::int32_t value1b = bits16Handler->getUnsignedLong(elementNumber);
            std::int32_t value2b = bits4Handler->getUnsignedLong(elementNumber++);

            EXPECT_EQ(value0r, r);
            EXPECT_EQ(value0g, g);
            EXPECT_EQ(value0b, b);

            EXPECT_EQ(value0r, (value1r>>8));
            EXPECT_EQ(value0g, (value1g>>8));
            EXPECT_EQ(value0b, (value1b>>8));

            EXPECT_EQ((value0r >> 4), value2r);
            EXPECT_EQ((value0g >> 4), value2g);
            EXPECT_EQ((value0b >> 4), value2b);

        }
    }
}

TEST(bitTransformTest, allocateOutputImage)
{
    std::uint32_t width = 2;
    std::uint32_t height = 2;
    Image bits8Image(width, height, bitDepth_t::depthU8, "RGB", 7);
    Image bits16Image(width, height, bitDepth_t::depthU16, "MONOCHROME2", 15);
    Image bits4Image(width, height, bitDepth_t::depthU8, "YBR_FULL", 3);

    TransformHighBit transform;
    std::unique_ptr<Image> allocated8bits(transform.allocateOutputImage(bits8Image, 1, 1));
    std::unique_ptr<Image> allocated16bits(transform.allocateOutputImage(bits16Image, 1, 1));
    std::unique_ptr<Image> allocated4bits(transform.allocateOutputImage(bits4Image, 1, 1));

    ASSERT_EQ(7, allocated8bits->getHighBit());
    ASSERT_EQ("RGB", allocated8bits->getColorSpace());
    ASSERT_EQ(15, allocated16bits->getHighBit());
    ASSERT_EQ("MONOCHROME2", allocated16bits->getColorSpace());
    ASSERT_EQ(3, allocated4bits->getHighBit());
    ASSERT_EQ("YBR_FULL", allocated4bits->getColorSpace());

}

} // namespace tests

} // namespace imebra
