#include <imebra/imebra.h>
#include "buildImageForTest.h"
#include <gtest/gtest.h>


namespace imebra
{

namespace tests
{

TEST(transformsChain, emptyChain)
{
    TransformsChain chain;

    ASSERT_TRUE(chain.isEmpty());

    Image rgb(100, 100, bitDepth_t::depthU8, "RGB", 7);

    {
        std::unique_ptr<WritingDataHandler> rgbHandler(rgb.getWritingDataHandler());
        size_t pointer(0);
        for(std::uint32_t y(0); y != 100; ++y)
        {
            for(std::uint32_t x(0); x != 100; ++x)
            {
                rgbHandler->setUnsignedLong(pointer++, y);
                rgbHandler->setUnsignedLong(pointer++, x);
                rgbHandler->setUnsignedLong(pointer++, 255);
            }
        }
    }

    std::unique_ptr<Image> outputImage(chain.allocateOutputImage(rgb, 100, 100));

    chain.runTransform(rgb, 0, 0, 100, 100, *outputImage, 0, 0);

    identicalImages(rgb, *outputImage);
}


TEST(transformsChain, oneTransform)
{
    TransformsChain chain;

    ASSERT_TRUE(chain.isEmpty());

    TransformHighBit highBit;

    chain.addTransform(highBit);

    ASSERT_FALSE(chain.isEmpty());

    Image rgb(100, 100, bitDepth_t::depthU8, "RGB", 7);

    {
        std::unique_ptr<WritingDataHandler> rgbHandler(rgb.getWritingDataHandler());
        size_t pointer(0);
        for(std::uint32_t y(0); y != 100; ++y)
        {
            for(std::uint32_t x(0); x != 100; ++x)
            {
                rgbHandler->setUnsignedLong(pointer++, y);
                rgbHandler->setUnsignedLong(pointer++, x);
                rgbHandler->setUnsignedLong(pointer++, 255);
            }
        }
    }

    std::unique_ptr<Image> outputImage(chain.allocateOutputImage(rgb, 100, 100));

    chain.runTransform(rgb, 0, 0, 100, 100, *outputImage, 0, 0);

    identicalImages(rgb, *outputImage);
}


TEST(transformsChain, twoTransform)
{
    TransformsChain chain;

    ASSERT_TRUE(chain.isEmpty());

    std::unique_ptr<Transform> monochromeToRgb(ColorTransformsFactory::getTransform("MONOCHROME2", "RGB"));
    std::unique_ptr<Transform> rgbToMonochrome(ColorTransformsFactory::getTransform("RGB", "MONOCHROME2"));

    chain.addTransform(*monochromeToRgb);
    chain.addTransform(*rgbToMonochrome);

    ASSERT_FALSE(chain.isEmpty());

    Image monochrome(100, 100, bitDepth_t::depthU8, "MONOCHROME2", 7);

    {
        std::unique_ptr<WritingDataHandler> rgbHandler(monochrome.getWritingDataHandler());
        size_t pointer(0);
        for(std::uint32_t y(0); y != 100; ++y)
        {
            for(std::uint32_t x(0); x != 100; ++x)
            {
                rgbHandler->setUnsignedLong(pointer++, y);
            }
        }
    }

    std::unique_ptr<Image> outputImage(chain.allocateOutputImage(monochrome, 100, 100));

    chain.runTransform(monochrome, 0, 0, 100, 100, *outputImage, 0, 0);

    identicalImages(monochrome, *outputImage);
}

}

}

