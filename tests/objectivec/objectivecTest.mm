#include <gtest/gtest.h>
#include <imebra/imebra.h>
#include <imebra/objectivec/imebra_strings.h>
#include <imebra/objectivec/imebra_image.h>
#include "../buildImageForTest.h"

namespace imebra
{

namespace tests
{

// Test NSString conversion functions
TEST(objectivec, stringToNSStringTest)
{
    // Try a cyrillic/arabic patient name
    std::string patientName0 = "??\xD0\xA1\xD0\xBC\xD1\x8B\xD1\x81\xD0\xBB\x20\xD0\xB2\xD1\x81\xD0\xB5\xD0\xB9";
    std::string patientName1 = "\xD0\xA1\xD0\xBC\xD1\x8B\xD1\x81\xD0\xBB\x20\xD0\xB2\xD1\x81\xD0\xB5\xD0\xB9";

    ReadWriteMemory streamMemory;
    {
        charsetsList_t charsets;
        charsets.push_back("ISO_IR 6");
        DataSet testDataSet("1.2.840.10008.1.2.1", charsets);

        {
            std::unique_ptr<WritingDataHandler> handler(testDataSet.getWritingDataHandler(TagId(0x10, 0x10), 0));

            handler->setString(0, patientName0);
            handler->setString(1, patientName1);
        }

        MemoryStreamOutput writeStream(streamMemory);
        StreamWriter writer(writeStream);
        CodecFactory::save(testDataSet, writer, codecType_t::dicom);
    }

    {
        MemoryStreamInput readStream(streamMemory);
        StreamReader reader(readStream);
        std::unique_ptr<DataSet> testDataSet(CodecFactory::load(reader));

        std::string patientName0(testDataSet->getString(TagId(0x0010, 0x0010), 0));
        std::string patientName1(testDataSet->getString(TagId(0x0010, 0x0010), 1));

        NSString* nsPatientName0 = stringToNSString(patientName0);
        NSString* nsPatientName1 = stringToNSString(patientName1);

        EXPECT_TRUE([nsPatientName0 isEqualToString:@"??\xD0\xA1\xD0\xBC\xD1\x8B\xD1\x81\xD0\xBB\x20\xD0\xB2\xD1\x81\xD0\xB5\xD0\xB9"]);
        EXPECT_TRUE([nsPatientName1 isEqualToString:@"\xD0\xA1\xD0\xBC\xD1\x8B\xD1\x81\xD0\xBB\x20\xD0\xB2\xD1\x81\xD0\xB5\xD0\xB9"]);

        [nsPatientName0 release];
        [nsPatientName1 release];
    }
}


// Test NSString conversion functions
TEST(objectivec, NSStringToStringTest)
{
    ReadWriteMemory streamMemory;
    {
        charsetsList_t charsets;
        charsets.push_back("ISO_IR 6");
        DataSet testDataSet("1.2.840.10008.1.2.1", charsets);

        {
            std::unique_ptr<WritingDataHandler> handler(testDataSet.getWritingDataHandler(TagId(0x10, 0x10), 0));

            handler->setString(0, NSStringToString(@"Test 1"));
            handler->setString(1, NSStringToString(@"Test 2"));
        }

        MemoryStreamOutput writeStream(streamMemory);
        StreamWriter writer(writeStream);
        CodecFactory::save(testDataSet, writer, codecType_t::dicom);
    }

    {
        MemoryStreamInput readStream(streamMemory);
        StreamReader reader(readStream);
        std::unique_ptr<DataSet> testDataSet(CodecFactory::load(reader));

        std::string patientName0(testDataSet->getString(TagId(0x0010, 0x0010), 0));
        std::string patientName1(testDataSet->getString(TagId(0x0010, 0x0010), 1));

        EXPECT_EQ("Test 1", patientName0);
        EXPECT_EQ("Test 2", patientName1);

    }
}


#if defined(__APPLE__)
// Test NSImage
TEST(objectivec, images)
{
    std::string transferSyntax = "1.2.840.10008.1.2.4.50";
    DataSet dataset(transferSyntax);

    std::uint32_t width = 600;
    std::uint32_t height = 400;

    std::unique_ptr<Image> baselineImage(buildImageForTest(width, height, bitDepth_t::depthU8, 7, 30, 20, "RGB", 50));

    std::unique_ptr<Transform> colorTransform(ColorTransformsFactory::getTransform("RGB", "YBR_FULL"));
    std::unique_ptr<Image> ybrImage(colorTransform->allocateOutputImage(*baselineImage, width, height));
    colorTransform->runTransform(*baselineImage, 0, 0, width, height, *ybrImage, 0, 0);

    TransformsChain chain;
    DrawBitmap drawBitmap(chain);
    NSImage* nsImage = getImebraImage(*ybrImage, drawBitmap);

    NSData *imageData = [nsImage TIFFRepresentation];
    NSBitmapImageRep *imageRep = [NSBitmapImageRep imageRepWithData:imageData];
    NSDictionary *imageProps = [NSDictionary dictionaryWithObject:[NSNumber numberWithFloat:1.0] forKey:NSImageCompressionFactor];
    imageData = [imageRep representationUsingType:NSJPEGFileType properties:imageProps];

    ReadWriteMemory dataMemory((const char*)[imageData bytes], [imageData length]);
    MemoryStreamInput dataStream(dataMemory);
    StreamReader dataReader(dataStream);

    std::unique_ptr<DataSet> loadedDataSet(CodecFactory::load(dataReader));
    std::unique_ptr<Image> loadedImage(loadedDataSet->getImage(0));

    // Compare the buffers. A little difference is allowed
    double differenceYBR = compareImages(*ybrImage, *loadedImage);
    ASSERT_LE(differenceYBR, 1);
}
#endif

} // namespace tests

} // namespace imebra
