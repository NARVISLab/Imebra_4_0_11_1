/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file jpegStreamCodec.cpp
    \brief Implementation of the class jpegStreamCodec.

*/

#include "exceptionImpl.h"
#include "streamReaderImpl.h"
#include "streamWriterImpl.h"
#include "huffmanTableImpl.h"
#include "jpegStreamCodecImpl.h"
#include "jpegImageCodecImpl.h"
#include "dataSetImpl.h"
#include "codecFactoryImpl.h"
#include "../include/imebra/exceptions.h"
#include <vector>
#include <stdlib.h>
#include <string.h>

namespace imebra
{

namespace implementation
{

namespace codecs
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default luminance or RGB quantization table
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegStdLuminanceQuantTbl[] =
{
    16,  11,  10,  16,  24,  40,  51,  61,
    12,  12,  14,  19,  26,  58,  60,  55,
    14,  13,  16,  24,  40,  57,  69,  56,
    14,  17,  22,  29,  51,  87,  80,  62,
    18,  22,  37,  56,  68, 109, 103,  77,
    24,  35,  55,  64,  81, 104, 113,  92,
    49,  64,  78,  87, 103, 121, 120, 101,
    72,  92,  95,  98, 112, 100, 103,  99
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default chrominance quantization table
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegStdChrominanceQuantTbl[] =
{
    17,  18,  24,  47,  99,  99,  99,  99,
    18,  21,  26,  66,  99,  99,  99,  99,
    24,  26,  56,  99,  99,  99,  99,  99,
    47,  66,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default scale factors for FDCT/IDCT calculation
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const float JpegDctScaleFactor[]=
{
    (float)1.0,
    (float)1.387039845,
    (float)1.306562965,
    (float)1.175875602,
    (float)1.0,
    (float)0.785694958,
    (float)0.541196100,
    (float)0.275899379
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default Huffman table for DC values of luminance channel
// (Values per length)
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegBitsDcLuminance[]=
{ 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default Huffman table for DC values of luminance channel
// (Values to code)
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegValDcLuminance[]=
{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default Huffman table for DC values of chrominance
//  channel (Values per length)
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegBitsDcChrominance[]=
{ 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default Huffman table for DC values of chrominance
//  channel (Values to code)
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegValDcChrominance[]=
{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default Huffman table for AC values of luminance channel
// (Values per length)
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegBitsAcLuminance[]=
{ 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d };


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default Huffman table for AC values of luminance channel
// (Values to code)
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegValAcLuminance[]=
{
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
    0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
    0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
    0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
    0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
    0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
    0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
    0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
    0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
    0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
    0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
    0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Default Huffman table for AC values of chrominance
//  channel (Values per length)
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegBitsAcChrominance[] =
{ 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77 };


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Default Huffman table for AC values of chrominance
//  channel (Values to code)
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegValAcChrominance[] =
{
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
    0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
    0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
    0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
    0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
    0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
    0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
    0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
    0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
    0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
    0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Translate zig-zag order in 8x8 blocks to raw order
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
static const std::uint32_t JpegDeZigZagOrder[]=
{
    0, 1, 8, 16, 9, 2, 3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

#define JPEG_DECOMPRESSION_BITS_PRECISION 14

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegCodec
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Constructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
jpegStreamCodec::jpegStreamCodec()
{
    IMEBRA_FUNCTION_START();

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write a jpeg stream
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void jpegStreamCodec::writeStream(std::shared_ptr<streamWriter> pStream, std::shared_ptr<dataSet> pDataSet) const
{
    IMEBRA_FUNCTION_START();

    // Retrieve the transfer syntax
    ///////////////////////////////////////////////////////////
    std::string transferSyntax = pDataSet->getString(0x0002, 0x0, 0x0010, 0, 0);

    // The buffer can be written as it is
    ///////////////////////////////////////////////////////////
    jpegImageCodec imageCodec;
    if(imageCodec.canHandleTransferSyntax(transferSyntax))
    {
        std::shared_ptr<data> imageData = pDataSet->getTag(0x7fe0, 0, 0x0010);

        std::uint32_t firstBufferId(0);
        std::uint32_t endBufferId(1);
        if(imageData->bufferExists(1))
        {
            pDataSet->getFrameBufferIds(0, &firstBufferId, &endBufferId);
        }
        for(std::uint32_t scanBuffers = firstBufferId; scanBuffers != endBufferId; ++scanBuffers)
        {
            std::shared_ptr<handlers::readingDataHandlerRaw> readHandler = imageData->getReadingDataHandlerRaw(scanBuffers);
            const std::uint8_t* readBuffer = readHandler->getMemoryBuffer();
            pStream->write(readBuffer, readHandler->getSize());
        }
        return;

    }

    // Get the image then write it
    ///////////////////////////////////////////////////////////
    std::shared_ptr<image> decodedImage = pDataSet->getImage(0);
    imageCodec.setImage(pStream, decodedImage, "1.2.840.10008.1.2.4.50", imageQuality_t::high, tagVR_t::OB, 8, true, true, false, false);

    IMEBRA_FUNCTION_END();

}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Build a DICOM dataset from a jpeg file
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void jpegStreamCodec::readStream(std::shared_ptr<streamReader> pSourceStream, std::shared_ptr<dataSet> pDataSet, std::uint32_t /* maxSizeBufferLoad = 0xffffffff */) const
{
    IMEBRA_FUNCTION_START();

    streamReader* pStream = pSourceStream.get();

    // Store the stream's position.
    // This will be used later, in order to reread all the
    //  stream's content and store it into the dataset
    ///////////////////////////////////////////////////////////
    size_t startPosition = pStream->position();

    try
    {
        // Read the Jpeg signature
        ///////////////////////////////////////////////////////////
        std::uint8_t jpegSignature[2];
        try
        {
            pStream->read(jpegSignature, 2);
        }
        catch(StreamEOFError&)
        {
            IMEBRA_THROW(CodecWrongFormatError, "readStream detected a wrong format");
        }

        // If the jpeg signature is wrong, then return an error
        //  condition
        ///////////////////////////////////////////////////////////
        std::uint8_t checkSignature[2]={(std::uint8_t)0xff, (std::uint8_t)0xd8};
        if(::memcmp(jpegSignature, checkSignature, 2) != 0)
        {
            IMEBRA_THROW(CodecWrongFormatError, "detected a wrong format");
        }
    }
    catch(StreamEOFError&)
    {
        IMEBRA_THROW(CodecWrongFormatError, "detected a wrong format");
    }

    // Used to read discharged chars
    ///////////////////////////////////////////////////////////
    std::uint8_t entryByte;

    // Read all the tags in the stream
    ///////////////////////////////////////////////////////////
    jpeg::jpegInformation information;
    while(!information.m_bEndOfImage)
    {
        // If a tag has been found, then parse it
        ///////////////////////////////////////////////////////////
        pStream->read(&entryByte, 1);
        if(entryByte != 0xff)
        {
            continue;
        }
        do
        {
            pStream->read(&entryByte, 1);
        } while(entryByte == 0xff);

        if(entryByte != 0)
        {
            std::shared_ptr<jpeg::tag> pTag;
            tTagsMap::const_iterator findTag = m_tagsMap.find(entryByte);
            if(findTag != m_tagsMap.end())
            {
                pTag = findTag->second;
            }
            else
            {
                pTag = m_tagsMap.find(0xff)->second;
            }

            // Parse the tag
            ///////////////////////////////////////////////////////////
            pTag->readTag(pStream, &information, entryByte);
        }
    }

    //
    // Build the dataset
    //
    ///////////////////////////////////////////////////////////

    // Color space
    ///////////////////////////////////////////////////////////
    if(information.m_channelsMap.size() == 1)
        pDataSet->setString(0x0028, 0, 0x0004, 0, "MONOCHROME2");
    else
        pDataSet->setString(0x0028, 0, 0x0004, 0, "YBR_FULL");

    // Transfer syntax
    ///////////////////////////////////////////////////////////
    switch(information.m_process)
    {
    case 0x00:
        pDataSet->setString(0x0002, 0, 0x0010, 0, "1.2.840.10008.1.2.4.50");
        break;
    case 0x01:
        pDataSet->setString(0x0002, 0, 0x0010, 0, "1.2.840.10008.1.2.4.51");
        break;
    case 0x03:
        pDataSet->setString(0x0002, 0, 0x0010, 0, "1.2.840.10008.1.2.4.57");
        break;
    case 0x07:
        pDataSet->setString(0x0002, 0, 0x0010, 0, "1.2.840.10008.1.2.4.57");
        break;
    default:
        IMEBRA_THROW(JpegCodecCannotHandleSyntaxError, "Jpeg SOF not supported");
    }

    // Number of planes
    ///////////////////////////////////////////////////////////
    pDataSet->setUnsignedLong(0x0028, 0, 0x0002, 0, (std::uint32_t)information.m_channelsMap.size());

    // Image's width
    /////////////////////////////////////////////////////////////////
    pDataSet->setUnsignedLong(0x0028, 0, 0x0011, 0, information.m_imageWidth);

    // Image's height
    /////////////////////////////////////////////////////////////////
    pDataSet->setUnsignedLong(0x0028, 0, 0x0010, 0, information.m_imageHeight);

    // Number of frames
    /////////////////////////////////////////////////////////////////
    pDataSet->setUnsignedLong(0x0028, 0, 0x0008, 0, 1);

    // Pixel representation
    /////////////////////////////////////////////////////////////////
    pDataSet->setUnsignedLong(0x0028, 0x0, 0x0103, 0, 0);

    // Allocated, stored bits and high bit
    /////////////////////////////////////////////////////////////////
    pDataSet->setUnsignedLong(0x0028, 0x0, 0x0100, 0, information.m_precision);
    pDataSet->setUnsignedLong(0x0028, 0x0, 0x0101, 0, information.m_precision);
    pDataSet->setUnsignedLong(0x0028, 0x0, 0x0102, 0, information.m_precision - 1);

    // Interleaved (more than 1 channel in the channels list)
    /////////////////////////////////////////////////////////////////
    if(information.m_channelsMap.size() != 1)
    {
        pDataSet->setUnsignedLong(0x0028, 0x0, 0x0006, 0, (information.m_channelsList[0] != 0 && information.m_channelsList[1]) != 0 ? 1 : 0);
    }

    // Insert the basic offset table
    ////////////////////////////////////////////////////////////////
    std::shared_ptr<handlers::writingDataHandlerRaw> offsetHandler=pDataSet->getWritingDataHandlerRaw(0x7fe0, 0, 0x0010, 0, tagVR_t::OB);
    offsetHandler->setSize(4);
    ::memset(offsetHandler->getMemoryBuffer(), 0, offsetHandler->getSize());

    // Reread all the stream's content and write it into the dataset
    ////////////////////////////////////////////////////////////////
    size_t finalPosition = pStream->position();
    size_t streamLength = (std::uint32_t)(finalPosition - startPosition);
    pStream->seek(startPosition);

    std::shared_ptr<handlers::writingDataHandlerRaw> imageHandler = pDataSet->getWritingDataHandlerRaw(0x7fe0, 0, 0x0010, 1, tagVR_t::OB);
    if(imageHandler != 0 && streamLength != 0)
    {
        imageHandler->setSize(streamLength);
        pStream->read(imageHandler->getMemoryBuffer(), streamLength);
    }

    IMEBRA_FUNCTION_END();
}


} // namespace codecs

} // namespace implementation

} // namespace imebra



