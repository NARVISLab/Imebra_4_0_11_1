#include <imebra/imebra.h>
#include "buildImageForTest.h"
#include <gtest/gtest.h>


namespace imebra
{

namespace tests
{

TEST(voilut, voilutUnsigned8)
{
    Image unsigned8(6, 1, bitDepth_t::depthU8, "MONOCHROME2", 7);
    {
        std::unique_ptr<WritingDataHandler> unsigned8Handler(unsigned8.getWritingDataHandler());
        unsigned8Handler->setUnsignedLong(0, 0);
        unsigned8Handler->setUnsignedLong(1, 10);
        unsigned8Handler->setUnsignedLong(2, 20);
        unsigned8Handler->setUnsignedLong(3, 30);
        unsigned8Handler->setUnsignedLong(4, 40);
        unsigned8Handler->setUnsignedLong(5, 50);
    }

    VOILUT voilut;
    voilut.setCenterWidth(0, 50);
    ASSERT_EQ(0, voilut.getCenter());
    ASSERT_EQ(50, voilut.getWidth());

    std::unique_ptr<Image> unsigned8Out(voilut.allocateOutputImage(unsigned8, 6, 1));
    voilut.runTransform(unsigned8, 0, 0, 6, 1, *unsigned8Out, 0, 0);

    std::unique_ptr<ReadingDataHandler> unsigned8Handler(unsigned8Out->getReadingDataHandler());

    ASSERT_EQ(128, unsigned8Handler->getUnsignedLong(0));
    ASSERT_EQ(179, unsigned8Handler->getUnsignedLong(1));
    ASSERT_EQ(230, unsigned8Handler->getUnsignedLong(2));
    ASSERT_EQ(255, unsigned8Handler->getUnsignedLong(3));
    ASSERT_EQ(255, unsigned8Handler->getUnsignedLong(4));
    ASSERT_EQ(255, unsigned8Handler->getUnsignedLong(5));

    voilut.setCenterWidth(70, 50);

    Image signed16Out(6, 1, bitDepth_t::depthS16, "MONOCHROME2", 15);
    voilut.runTransform(unsigned8, 0, 0, 6, 1, signed16Out, 0, 0);

    std::unique_ptr<ReadingDataHandler> signed16Handler(signed16Out.getReadingDataHandler());

    ASSERT_EQ(-32768, signed16Handler->getSignedLong(0));
    ASSERT_EQ(-32768, signed16Handler->getSignedLong(1));
    ASSERT_EQ(-32768, signed16Handler->getSignedLong(2));
    ASSERT_EQ(-32768, signed16Handler->getSignedLong(3));
    ASSERT_EQ(-32768, signed16Handler->getSignedLong(4));
    ASSERT_EQ(-26213, signed16Handler->getSignedLong(5));

}


TEST(voilut, voilutUnsigned8OptimalVOI)
{
    Image unsigned8(6, 1, bitDepth_t::depthU8, "MONOCHROME2", 7);
    {
        std::unique_ptr<WritingDataHandler> unsigned8Handler(unsigned8.getWritingDataHandler());
        unsigned8Handler->setUnsignedLong(0, 0);
        unsigned8Handler->setUnsignedLong(1, 10);
        unsigned8Handler->setUnsignedLong(2, 20);
        unsigned8Handler->setUnsignedLong(3, 30);
        unsigned8Handler->setUnsignedLong(4, 40);
        unsigned8Handler->setUnsignedLong(5, 50);
    }

    VOILUT voilut;
    voilut.applyOptimalVOI(unsigned8, 0, 0, 6, 1);
    ASSERT_EQ(25, voilut.getCenter());
    ASSERT_EQ(50, voilut.getWidth());

    std::unique_ptr<Image> unsigned8Out(voilut.allocateOutputImage(unsigned8, 6, 1));
    voilut.runTransform(unsigned8, 0, 0, 6, 1, *unsigned8Out, 0, 0);

    std::unique_ptr<ReadingDataHandler> unsigned8Handler(unsigned8Out->getReadingDataHandler());

    ASSERT_EQ(0, unsigned8Handler->getUnsignedLong(0));
    ASSERT_EQ(51, unsigned8Handler->getUnsignedLong(1));
    ASSERT_EQ(102, unsigned8Handler->getUnsignedLong(2));
    ASSERT_EQ(154, unsigned8Handler->getUnsignedLong(3));
    ASSERT_EQ(205, unsigned8Handler->getUnsignedLong(4));
    ASSERT_EQ(255, unsigned8Handler->getUnsignedLong(5));

    Image signed16Out(6, 1, bitDepth_t::depthS16, "MONOCHROME2", 15);
    voilut.runTransform(unsigned8, 0, 0, 6, 1, signed16Out, 0, 0);

    std::unique_ptr<ReadingDataHandler> signed16Handler(signed16Out.getReadingDataHandler());

    ASSERT_EQ(-32767, signed16Handler->getSignedLong(0));
    ASSERT_EQ(-19660, signed16Handler->getSignedLong(1));
    ASSERT_EQ(-6553, signed16Handler->getSignedLong(2));
    ASSERT_EQ(6554, signed16Handler->getSignedLong(3));
    ASSERT_EQ(19661, signed16Handler->getSignedLong(4));
    ASSERT_EQ(32767, signed16Handler->getSignedLong(5));

}


TEST(voilut, voilutUnsigned8LUT)
{
    Image unsigned8(6, 1, bitDepth_t::depthU8, "MONOCHROME2", 7);
    {
        std::unique_ptr<WritingDataHandler> unsigned8Handler(unsigned8.getWritingDataHandler());
        unsigned8Handler->setUnsignedLong(0, 0);
        unsigned8Handler->setUnsignedLong(1, 1);
        unsigned8Handler->setUnsignedLong(2, 2);
        unsigned8Handler->setUnsignedLong(3, 3);
        unsigned8Handler->setUnsignedLong(4, 4);
        unsigned8Handler->setUnsignedLong(5, 5);
    }

    DataSet testDataSet;
    std::unique_ptr<Tag> sequenceTag(testDataSet.getTagCreate(TagId(tagId_t::VOILUTSequence_0028_3010)));
    DataSet lutItem;
    {
        std::unique_ptr<WritingDataHandlerNumeric> descriptor(lutItem.getWritingDataHandlerNumeric(TagId(tagId_t::LUTDescriptor_0028_3002), 0, tagVR_t::US));
        std::unique_ptr<WritingDataHandlerNumeric> data(lutItem.getWritingDataHandlerNumeric(TagId(tagId_t::LUTData_0028_3006), 0, tagVR_t::US));
        descriptor->setUnsignedLong(0, 3);
        descriptor->setUnsignedLong(1, 2);
        descriptor->setUnsignedLong(2, 16);

        data->setUnsignedLong(0, 100);
        data->setUnsignedLong(1, 200);
        data->setUnsignedLong(2, 300);
    }
    sequenceTag->setSequenceItem(0, lutItem);
    std::unique_ptr<LUT> lut(testDataSet.getLUT(TagId(tagId_t::VOILUTSequence_0028_3010), 0));


    VOILUT voilut;
    voilut.setLUT(*lut);

    std::unique_ptr<Image> paletteOut(voilut.allocateOutputImage(unsigned8, 6, 1));
    voilut.runTransform(unsigned8, 0, 0, 6, 1, *paletteOut, 0, 0);

    std::unique_ptr<ReadingDataHandler> paletteHandler(paletteOut->getReadingDataHandler());

    ASSERT_EQ(100, paletteHandler->getUnsignedLong(0));
    ASSERT_EQ(100, paletteHandler->getUnsignedLong(1));
    ASSERT_EQ(100, paletteHandler->getUnsignedLong(2));
    ASSERT_EQ(200, paletteHandler->getUnsignedLong(3));
    ASSERT_EQ(300, paletteHandler->getUnsignedLong(4));
    ASSERT_EQ(300, paletteHandler->getUnsignedLong(5));

}

}

}

