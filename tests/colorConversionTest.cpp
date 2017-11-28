#include <imebra/imebra.h>
#include "buildImageForTest.h"
#include <gtest/gtest.h>


namespace imebra
{

namespace tests
{

TEST(colorConversion, RGB2YBRFULL)
{
    for(int ict(0); ict != 2; ++ict)
    {
        Image rgb(1, 1, bitDepth_t::depthU8, "RGB", 7);

        {
            std::unique_ptr<WritingDataHandler> rgbHandler(rgb.getWritingDataHandler());
            rgbHandler->setUnsignedLong(0, 255);
            rgbHandler->setUnsignedLong(1, 255);
            rgbHandler->setUnsignedLong(2, 255);
        }

        std::string destinationColorSpace(ict == 0 ? "YBR_FULL" : "YBR_ICT");
        std::unique_ptr<Transform> rgb2ybr(ColorTransformsFactory::getTransform("RGB", destinationColorSpace));

        Image ybr(1, 1, bitDepth_t::depthU8, destinationColorSpace, 7);
        rgb2ybr->runTransform(rgb, 0, 0, 1, 1, ybr, 0, 0);

        std::unique_ptr<ReadingDataHandler> ybr1Handler(ybr.getReadingDataHandler());
        ASSERT_EQ(255, ybr1Handler->getSignedLong(0));
        ASSERT_EQ(128, ybr1Handler->getSignedLong(1));
        ASSERT_EQ(128, ybr1Handler->getSignedLong(2));
    }
}


TEST(colorConversion, RGB2YBRRCT)
{
    Image rgb(255, 255, bitDepth_t::depthU8, "RGB", 7);

    {
        std::unique_ptr<WritingDataHandler> rgbHandler(rgb.getWritingDataHandler());
        for(int y = 0; y != 255; ++y)
        {
            for(int x = 0; x != 255; ++x)
            {
                rgbHandler->setUnsignedLong((x + y * 50) * 3, x);
                rgbHandler->setUnsignedLong((x + y * 50) * 3 + 1, y);
                rgbHandler->setUnsignedLong((x + y * 50) * 3 + 2, (y + x) / 2);
            }
        }
    }


    std::unique_ptr<Transform> rgb2ybr(ColorTransformsFactory::getTransform("RGB", "YBR_RCT"));
    std::unique_ptr<Image> ybr(rgb2ybr->allocateOutputImage(rgb, 255, 255));
    rgb2ybr->runTransform(rgb, 0, 0, 255, 255, *ybr, 0, 0);

    std::unique_ptr<Transform> ybr2rgb(ColorTransformsFactory::getTransform("YBR_RCT", "RGB"));
    std::unique_ptr<Image> rgb1(ybr2rgb->allocateOutputImage(*ybr, 255, 255));
    ybr2rgb->runTransform(*ybr, 0, 0, 255, 255, *rgb1, 0, 0);

    ASSERT_GE(1, compareImages(rgb, *rgb1)); // Account for ceiling/floor adjustment
}


TEST(colorConversion, RGB2YBRPARTIAL)
{
    Image rgb(3, 1, bitDepth_t::depthU8, "RGB", 7);

    {
        std::unique_ptr<WritingDataHandler> rgbHandler(rgb.getWritingDataHandler());
        rgbHandler->setUnsignedLong(0, 255);
        rgbHandler->setUnsignedLong(1, 255);
        rgbHandler->setUnsignedLong(2, 255);
        rgbHandler->setUnsignedLong(3, 0);
        rgbHandler->setUnsignedLong(4, 0);
        rgbHandler->setUnsignedLong(5, 0);
        rgbHandler->setUnsignedLong(6, 255);
        rgbHandler->setUnsignedLong(7, 0);
        rgbHandler->setUnsignedLong(8, 0);
    }

    std::unique_ptr<Transform> rgb2ybr(ColorTransformsFactory::getTransform("RGB", "YBR_PARTIAL"));

    Image ybr(3, 1, bitDepth_t::depthU8, "YBR_PARTIAL", 7);
    rgb2ybr->runTransform(rgb, 0, 0, 3, 1, ybr, 0, 0);

    std::unique_ptr<ReadingDataHandler> ybr1Handler(ybr.getReadingDataHandler());
    ASSERT_EQ(235, ybr1Handler->getSignedLong(0));
    ASSERT_EQ(128, ybr1Handler->getSignedLong(1));
    ASSERT_EQ(128, ybr1Handler->getSignedLong(2));
    ASSERT_EQ(16, ybr1Handler->getSignedLong(3));
    ASSERT_EQ(128, ybr1Handler->getSignedLong(4));
    ASSERT_EQ(128, ybr1Handler->getSignedLong(5));
    ASSERT_EQ(81, ybr1Handler->getSignedLong(6));
    ASSERT_EQ(91, ybr1Handler->getSignedLong(7));
    ASSERT_EQ(240, ybr1Handler->getSignedLong(8));
}


TEST(colorConversion, RGB2MONOCHROME2)
{
    Image rgb(2, 1, bitDepth_t::depthU8, "RGB", 7);

    {
        std::unique_ptr<WritingDataHandler> rgbHandler(rgb.getWritingDataHandler());
        rgbHandler->setUnsignedLong(0, 255);
        rgbHandler->setUnsignedLong(1, 255);
        rgbHandler->setUnsignedLong(2, 255);
        rgbHandler->setUnsignedLong(3, 0);
        rgbHandler->setUnsignedLong(4, 0);
        rgbHandler->setUnsignedLong(5, 0);
    }

    std::unique_ptr<Transform> toMonochrome2(ColorTransformsFactory::getTransform("RGB", "MONOCHROME2"));

    Image monochrome2(2, 1, bitDepth_t::depthU8, "MONOCHROME2", 7);
    toMonochrome2->runTransform(rgb, 0, 0, 2, 1, monochrome2, 0, 0);

    std::unique_ptr<ReadingDataHandler> monochromeHandler(monochrome2.getReadingDataHandler());
    ASSERT_EQ(255, monochromeHandler->getSignedLong(0));
    ASSERT_EQ(0, monochromeHandler->getSignedLong(1));
}


TEST(colorConversion, RGB2MONOCHROME1)
{
    Image rgb(2, 1, bitDepth_t::depthU8, "RGB", 7);

    {
        std::unique_ptr<WritingDataHandler> rgbHandler(rgb.getWritingDataHandler());
        rgbHandler->setUnsignedLong(0, 255);
        rgbHandler->setUnsignedLong(1, 255);
        rgbHandler->setUnsignedLong(2, 255);
        rgbHandler->setUnsignedLong(3, 0);
        rgbHandler->setUnsignedLong(4, 0);
        rgbHandler->setUnsignedLong(5, 0);
    }

    std::unique_ptr<Transform> toMonochrome1(ColorTransformsFactory::getTransform("RGB", "MONOCHROME1"));

    Image monochrome1(2, 1, bitDepth_t::depthU8, "MONOCHROME1", 7);
    toMonochrome1->runTransform(rgb, 0, 0, 2, 1, monochrome1, 0, 0);

    std::unique_ptr<ReadingDataHandler> monochromeHandler(monochrome1.getReadingDataHandler());
    ASSERT_EQ(0, monochromeHandler->getSignedLong(0));
    ASSERT_EQ(255, monochromeHandler->getSignedLong(1));
}


TEST(colorConversion, MONOCHROME12MONOCHROME2)
{
    Image monochrome1(3, 1, bitDepth_t::depthU8, "MONOCHROME1", 7);

    {
        std::unique_ptr<WritingDataHandler> monochrome1Handler(monochrome1.getWritingDataHandler());
        monochrome1Handler->setUnsignedLong(0, 255);
        monochrome1Handler->setUnsignedLong(1, 254);
        monochrome1Handler->setUnsignedLong(2, 253);
    }

    std::unique_ptr<Transform> toMonochrome2(ColorTransformsFactory::getTransform("MONOCHROME1", "MONOCHROME2"));

    Image monochrome2(3, 1, bitDepth_t::depthU8, "MONOCHROME2", 7);
    toMonochrome2->runTransform(monochrome1, 0, 0, 3, 1, monochrome2, 0, 0);

    std::unique_ptr<ReadingDataHandler> monochrome2Handler(monochrome2.getReadingDataHandler());
    ASSERT_EQ(0, monochrome2Handler->getSignedLong(0));
    ASSERT_EQ(1, monochrome2Handler->getSignedLong(1));
    ASSERT_EQ(2, monochrome2Handler->getSignedLong(2));
}


TEST(colorConversion, MONOCHROME12RGB)
{
    Image monochrome1(3, 1, bitDepth_t::depthU8, "MONOCHROME1", 7);

    {
        std::unique_ptr<WritingDataHandler> monochrome1Handler(monochrome1.getWritingDataHandler());
        monochrome1Handler->setUnsignedLong(0, 255);
        monochrome1Handler->setUnsignedLong(1, 254);
        monochrome1Handler->setUnsignedLong(2, 253);
    }

    std::unique_ptr<Transform> toRGB(ColorTransformsFactory::getTransform("MONOCHROME1", "RGB"));

    Image rgb(3, 1, bitDepth_t::depthU8, "RGB", 7);
    toRGB->runTransform(monochrome1, 0, 0, 3, 1, rgb, 0, 0);

    std::unique_ptr<ReadingDataHandler> rgbHandler(rgb.getReadingDataHandler());
    ASSERT_EQ(0, rgbHandler->getSignedLong(0));
    ASSERT_EQ(0, rgbHandler->getSignedLong(1));
    ASSERT_EQ(0, rgbHandler->getSignedLong(2));
    ASSERT_EQ(1, rgbHandler->getSignedLong(3));
    ASSERT_EQ(1, rgbHandler->getSignedLong(4));
    ASSERT_EQ(1, rgbHandler->getSignedLong(5));
    ASSERT_EQ(2, rgbHandler->getSignedLong(6));
    ASSERT_EQ(2, rgbHandler->getSignedLong(7));
    ASSERT_EQ(2, rgbHandler->getSignedLong(8));
}


TEST(colorConversion, MONOCHROME22RGB)
{
    Image monochrome2(3, 1, bitDepth_t::depthU8, "MONOCHROME2", 7);

    {
        std::unique_ptr<WritingDataHandler> monochrome2Handler(monochrome2.getWritingDataHandler());
        monochrome2Handler->setUnsignedLong(0, 255);
        monochrome2Handler->setUnsignedLong(1, 254);
        monochrome2Handler->setUnsignedLong(2, 253);
    }

    std::unique_ptr<Transform> toRGB(ColorTransformsFactory::getTransform("MONOCHROME2", "RGB"));

    Image rgb(3, 1, bitDepth_t::depthU8, "RGB", 7);
    toRGB->runTransform(monochrome2, 0, 0, 3, 1, rgb, 0, 0);

    std::unique_ptr<ReadingDataHandler> rgbHandler(rgb.getReadingDataHandler());
    ASSERT_EQ(255, rgbHandler->getSignedLong(0));
    ASSERT_EQ(255, rgbHandler->getSignedLong(1));
    ASSERT_EQ(255, rgbHandler->getSignedLong(2));
    ASSERT_EQ(254, rgbHandler->getSignedLong(3));
    ASSERT_EQ(254, rgbHandler->getSignedLong(4));
    ASSERT_EQ(254, rgbHandler->getSignedLong(5));
    ASSERT_EQ(253, rgbHandler->getSignedLong(6));
    ASSERT_EQ(253, rgbHandler->getSignedLong(7));
    ASSERT_EQ(253, rgbHandler->getSignedLong(8));
}


TEST(colorConversion, MONOCHROME2YBRFULL)
{
    for(int ict(0); ict != 2; ++ict)
    {
        Image monochrome2(3, 1, bitDepth_t::depthU8, "MONOCHROME2", 7);

        {
            std::unique_ptr<WritingDataHandler> monochrome2Handler(monochrome2.getWritingDataHandler());
            monochrome2Handler->setUnsignedLong(0, 255);
            monochrome2Handler->setUnsignedLong(1, 254);
            monochrome2Handler->setUnsignedLong(2, 253);
        }

        std::string destinationColorSpace(ict == 0 ? "YBR_FULL" : "YBR_ICT");
        std::unique_ptr<Transform> toYBR(ColorTransformsFactory::getTransform("MONOCHROME2", destinationColorSpace));

        Image ybr(3, 1, bitDepth_t::depthU8, destinationColorSpace, 7);
        toYBR->runTransform(monochrome2, 0, 0, 3, 1, ybr, 0, 0);

        std::unique_ptr<ReadingDataHandler> ybrHandler(ybr.getReadingDataHandler());
        ASSERT_EQ(255, ybrHandler->getSignedLong(0));
        ASSERT_EQ(128, ybrHandler->getSignedLong(1));
        ASSERT_EQ(128, ybrHandler->getSignedLong(2));
        ASSERT_EQ(254, ybrHandler->getSignedLong(3));
        ASSERT_EQ(128, ybrHandler->getSignedLong(4));
        ASSERT_EQ(128, ybrHandler->getSignedLong(5));
        ASSERT_EQ(253, ybrHandler->getSignedLong(6));
        ASSERT_EQ(128, ybrHandler->getSignedLong(7));
        ASSERT_EQ(128, ybrHandler->getSignedLong(8));
    }
}


TEST(colorConversion, YBRFULL2MONOCHROME2)
{
    for(int ict(0); ict != 2; ++ict)
    {
        std::string sourceColorSpace(ict == 0 ? "YBR_FULL" : "YBR_ICT");
        Image ybr(3, 1, bitDepth_t::depthU8, sourceColorSpace, 7);

        {
            std::unique_ptr<WritingDataHandler> ybrHandler(ybr.getWritingDataHandler());
            ybrHandler->setUnsignedLong(0, 255);
            ybrHandler->setUnsignedLong(1, 128);
            ybrHandler->setUnsignedLong(2, 128);
            ybrHandler->setUnsignedLong(3, 254);
            ybrHandler->setUnsignedLong(4, 128);
            ybrHandler->setUnsignedLong(5, 128);
            ybrHandler->setUnsignedLong(6, 253);
            ybrHandler->setUnsignedLong(7, 128);
            ybrHandler->setUnsignedLong(8, 128);
        }

        std::unique_ptr<Transform> toMonochrome2(ColorTransformsFactory::getTransform(sourceColorSpace, "MONOCHROME2"));

        Image monochrome2(3, 1, bitDepth_t::depthU8, "MONOCHROME2", 7);
        toMonochrome2->runTransform(ybr, 0, 0, 3, 1, monochrome2, 0, 0);

        std::unique_ptr<ReadingDataHandler> monochromeHandler(monochrome2.getReadingDataHandler());
        ASSERT_EQ(255, monochromeHandler->getSignedLong(0));
        ASSERT_EQ(254, monochromeHandler->getSignedLong(1));
        ASSERT_EQ(253, monochromeHandler->getSignedLong(2));
    }
}


TEST(colorConversion, YBRFULL2RGB)
{
    for(int ict(0); ict != 2; ++ict)
    {
        std::string sourceColorSpace(ict == 0 ? "YBR_FULL" : "YBR_ICT");
        Image ybr(1, 1, bitDepth_t::depthU8, sourceColorSpace, 7);

        {
            std::unique_ptr<WritingDataHandler> ybrHandler(ybr.getWritingDataHandler());
            ybrHandler->setUnsignedLong(0, 255);
            ybrHandler->setUnsignedLong(1, 128);
            ybrHandler->setUnsignedLong(2, 128);
        }

        std::unique_ptr<Transform> ybr2rgb(ColorTransformsFactory::getTransform(sourceColorSpace, "RGB"));

        Image rgb1(1, 1, bitDepth_t::depthU8, "RGB", 7);
        ybr2rgb->runTransform(ybr, 0, 0, 1, 1, rgb1, 0, 0);
        std::unique_ptr<ReadingDataHandler> rgb1Handler(rgb1.getReadingDataHandler());

        ASSERT_EQ(255, rgb1Handler->getSignedLong(0));
        ASSERT_EQ(255, rgb1Handler->getSignedLong(1));
        ASSERT_EQ(255, rgb1Handler->getSignedLong(2));
    }
}


TEST(colorConversion, YBRPARTIAL2RGB)
{
    Image ybr(5, 1, bitDepth_t::depthU8, "YBR_PARTIAL", 7);

    {
        std::unique_ptr<WritingDataHandler> ybrHandler(ybr.getWritingDataHandler());
        ybrHandler->setUnsignedLong(0, 235);
        ybrHandler->setUnsignedLong(1, 128);
        ybrHandler->setUnsignedLong(2, 128);
        ybrHandler->setUnsignedLong(3, 255);
        ybrHandler->setUnsignedLong(4, 128);
        ybrHandler->setUnsignedLong(5, 128);
        ybrHandler->setUnsignedLong(6, 16);
        ybrHandler->setUnsignedLong(7, 128);
        ybrHandler->setUnsignedLong(8, 128);
        ybrHandler->setUnsignedLong(9, 0);
        ybrHandler->setUnsignedLong(10, 128);
        ybrHandler->setUnsignedLong(11, 128);
        ybrHandler->setUnsignedLong(12, 81);
        ybrHandler->setUnsignedLong(13, 91);
        ybrHandler->setUnsignedLong(14, 240);
    }

    std::unique_ptr<Transform> ybr2rgb(ColorTransformsFactory::getTransform("YBR_PARTIAL", "RGB"));

    Image rgb1(5, 1, bitDepth_t::depthU8, "RGB", 7);
    ybr2rgb->runTransform(ybr, 0, 0, 5, 1, rgb1, 0, 0);
    std::unique_ptr<ReadingDataHandler> rgb1Handler(rgb1.getReadingDataHandler());

    ASSERT_EQ(255, rgb1Handler->getSignedLong(0));
    ASSERT_EQ(255, rgb1Handler->getSignedLong(1));
    ASSERT_EQ(255, rgb1Handler->getSignedLong(2));
    ASSERT_EQ(255, rgb1Handler->getSignedLong(3));
    ASSERT_EQ(255, rgb1Handler->getSignedLong(4));
    ASSERT_EQ(255, rgb1Handler->getSignedLong(5));
    ASSERT_EQ(0, rgb1Handler->getSignedLong(6));
    ASSERT_EQ(0, rgb1Handler->getSignedLong(7));
    ASSERT_EQ(0, rgb1Handler->getSignedLong(8));
    ASSERT_EQ(0, rgb1Handler->getSignedLong(9));
    ASSERT_EQ(0, rgb1Handler->getSignedLong(10));
    ASSERT_EQ(0, rgb1Handler->getSignedLong(11));
    ASSERT_EQ(254, rgb1Handler->getSignedLong(12));
    ASSERT_EQ(0, rgb1Handler->getSignedLong(13));
    ASSERT_EQ(1, rgb1Handler->getSignedLong(14));
}

TEST(colorConversion, factoryTest)
{
    ASSERT_FALSE(ColorTransformsFactory::canSubsample("RGB"));
    ASSERT_FALSE(ColorTransformsFactory::canSubsample("MONOCHROME2"));
    ASSERT_FALSE(ColorTransformsFactory::canSubsample("MONOCHROME1"));
    ASSERT_FALSE(ColorTransformsFactory::canSubsample("PALETTE COLOR"));
    ASSERT_TRUE(ColorTransformsFactory::canSubsample("YBR_FULL"));
    ASSERT_TRUE(ColorTransformsFactory::canSubsample("YBR_PARTIAL"));

    ASSERT_FALSE(ColorTransformsFactory::isMonochrome("RGB"));
    ASSERT_TRUE(ColorTransformsFactory::isMonochrome("MONOCHROME2"));
    ASSERT_TRUE(ColorTransformsFactory::isMonochrome("MONOCHROME1"));
    ASSERT_FALSE(ColorTransformsFactory::isMonochrome("PALETTE COLOR"));
    ASSERT_FALSE(ColorTransformsFactory::isMonochrome("YBR_FULL"));
    ASSERT_FALSE(ColorTransformsFactory::isMonochrome("YBR_PARTIAL"));

    ASSERT_EQ(3, ColorTransformsFactory::getNumberOfChannels("RGB"));
    ASSERT_EQ(1, ColorTransformsFactory::getNumberOfChannels("MONOCHROME2"));
    ASSERT_EQ(1, ColorTransformsFactory::getNumberOfChannels("MONOCHROME1"));
    ASSERT_EQ(1, ColorTransformsFactory::getNumberOfChannels("PALETTE COLOR"));
    ASSERT_EQ(3, ColorTransformsFactory::getNumberOfChannels("YBR_FULL"));
    ASSERT_EQ(3, ColorTransformsFactory::getNumberOfChannels("YBR_PARTIAL"));

    ASSERT_EQ("YBR_PARTIAL_422", ColorTransformsFactory::makeSubsampled("YBR_PARTIAL", true, false));
    ASSERT_EQ("YBR_PARTIAL_420", ColorTransformsFactory::makeSubsampled("YBR_PARTIAL", true, true));
    ASSERT_EQ("YBR_PARTIAL", ColorTransformsFactory::makeSubsampled("YBR_PARTIAL", false, false));

    ASSERT_EQ("YBR_FULL", ColorTransformsFactory::normalizeColorSpace("YBR_FULL_422"));
    ASSERT_TRUE(ColorTransformsFactory::isSubsampledX("YBR_FULL_422"));
    ASSERT_FALSE(ColorTransformsFactory::isSubsampledY("YBR_FULL_422"));
    ASSERT_TRUE(ColorTransformsFactory::isSubsampledY("YBR_FULL_420"));
    ASSERT_EQ("PALETTE COLOR", ColorTransformsFactory::normalizeColorSpace("PALETTE COLOR"));
}

}

}

