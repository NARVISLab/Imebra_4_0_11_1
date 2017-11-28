/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dicomCodec.cpp
    \brief Implementation of the class dicomImageCodec.

*/

#include <list>
#include <vector>
#include <string.h>
#include "exceptionImpl.h"
#include "streamReaderImpl.h"
#include "streamWriterImpl.h"
#include "memoryImpl.h"
#include "dicomImageCodecImpl.h"
#include "dataSetImpl.h"
#include "dicomDictImpl.h"
#include "imageImpl.h"
#include "colorTransformsFactoryImpl.h"
#include "codecFactoryImpl.h"
#include "bufferImpl.h"
#include "../include/imebra/exceptions.h"

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
// dicomCodec
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
// Get a DICOM raw or RLE image from a dicom structure
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
std::shared_ptr<image> dicomImageCodec::getImage(const dataSet& dataset, std::shared_ptr<streamReader> pStream, tagVR_t dataType) const
{
    IMEBRA_FUNCTION_START();

    streamReader* pSourceStream = pStream.get();

    // Check for RLE compression
    ///////////////////////////////////////////////////////////
    std::string transferSyntax = dataset.getString(0x0002, 0x0, 0x0010, 0, 0, "1.2.840.10008.1.2");
    bool bRleCompressed = (transferSyntax == "1.2.840.10008.1.2.5");

    // Check for color space and subsampled channels
    ///////////////////////////////////////////////////////////
    std::string colorSpace = dataset.getString(0x0028, 0x0, 0x0004, 0, 0);

    // Retrieve the number of planes
    ///////////////////////////////////////////////////////////
    std::uint8_t channelsNumber=(std::uint8_t)dataset.getUnsignedLong(0x0028, 0x0, 0x0002, 0, 0);

    // Adjust the colorspace and the channels number for old
    //  NEMA files that don't specify those data
    ///////////////////////////////////////////////////////////
    if(colorSpace.empty() && (channelsNumber == 0 || channelsNumber == 1))
    {
        colorSpace = "MONOCHROME2";
        channelsNumber = 1;
    }

    if(colorSpace.empty() && channelsNumber == 3)
    {
        colorSpace = "RGB";
    }

    // Retrieve the image's size
    ///////////////////////////////////////////////////////////
    std::uint32_t imageWidth = dataset.getUnsignedLong(0x0028, 0x0, 0x0011, 0, 0);
    std::uint32_t imageHeight = dataset.getUnsignedLong(0x0028, 0x0, 0x0010, 0, 0);

    if(
            imageWidth > codecFactory::getCodecFactory()->getMaximumImageWidth() ||
            imageHeight > codecFactory::getCodecFactory()->getMaximumImageHeight())
    {
        IMEBRA_THROW(CodecImageTooBigError, "The factory settings prevented the loading of this image. Consider using codecFactory::setMaximumImageSize() to modify the settings");
    }

    if((imageWidth == 0) || (imageHeight == 0))
    {
        IMEBRA_THROW(CodecCorruptedFileError, "The size tags are not available");
    }

    // Check for interleaved planes.
    ///////////////////////////////////////////////////////////
    bool bInterleaved(dataset.getUnsignedLong(0x0028, 0x0, 0x0006, 0, 0, 0) == 0x0);

    // Check for 2's complement
    ///////////////////////////////////////////////////////////
    bool b2Complement = dataset.getUnsignedLong(0x0028, 0x0, 0x0103, 0, 0, 0) != 0x0;

    // Retrieve the allocated/stored/high bits
    ///////////////////////////////////////////////////////////
    std::uint8_t allocatedBits=(std::uint8_t)dataset.getUnsignedLong(0x0028, 0x0, 0x0100, 0, 0);
    std::uint8_t storedBits=(std::uint8_t)dataset.getUnsignedLong(0x0028, 0x0, 0x0101, 0, 0);
    std::uint8_t highBit=(std::uint8_t)dataset.getUnsignedLong(0x0028, 0x0, 0x0102, 0, 0);
    if(highBit < storedBits - 1)
        throw;


    // If the chrominance channels are subsampled, then find
    //  the right image's size
    ///////////////////////////////////////////////////////////
    bool bSubSampledY = channelsNumber > 0x1 && transforms::colorTransforms::colorTransformsFactory::isSubsampledY(colorSpace);
    bool bSubSampledX = channelsNumber > 0x1 && transforms::colorTransforms::colorTransformsFactory::isSubsampledX(colorSpace);

    // Create an image
    ///////////////////////////////////////////////////////////
    bitDepth_t depth;
    if(b2Complement)
    {
        if(highBit >= 16)
        {
            depth = bitDepth_t::depthS32;
        }
        else if(highBit >= 8)
        {
            depth = bitDepth_t::depthS16;
        }
        else
        {
            depth = bitDepth_t::depthS8;
        }
    }
    else
    {
        if(highBit >= 16)
        {
            depth = bitDepth_t::depthU32;
        }
        else if(highBit >= 8)
        {
            depth = bitDepth_t::depthU16;
        }
        else
        {
            depth = bitDepth_t::depthU8;
        }
    }

    std::shared_ptr<image> pImage(std::make_shared<image>(imageWidth, imageHeight, depth, colorSpace, highBit));
    std::shared_ptr<handlers::writingDataHandlerNumericBase> handler = pImage->getWritingDataHandler();
    std::uint32_t tempChannelsNumber = pImage->getChannelsNumber();

    if(handler == 0 || tempChannelsNumber != channelsNumber)
    {
        IMEBRA_THROW(CodecCorruptedFileError, "Cannot allocate the image's buffer");
    }

    // Allocate the dicom channels
    ///////////////////////////////////////////////////////////
    dicomInformation information;
    allocChannels(information, channelsNumber, imageWidth, imageHeight, bSubSampledX, bSubSampledY);

    std::uint32_t mask = (std::uint32_t)( ((std::uint64_t)1 << (highBit + 1)) - 1);
    mask -= (std::uint32_t)(((std::uint64_t)1 << (highBit + 1 - storedBits)) - 1);

    //
    // The image is not compressed
    //
    ///////////////////////////////////////////////////////////
    if(!bRleCompressed)
    {
        std::uint32_t wordSizeBytes = (dataType == tagVR_t::OW) ? 2 : 1;

        // The planes are interleaved
        ///////////////////////////////////////////////////////////
        if(bInterleaved && channelsNumber != 1)
        {
            readUncompressedInterleaved(
                        information,
                        channelsNumber,
                        bSubSampledX,
                        bSubSampledY,
                        pSourceStream,
                        wordSizeBytes,
                        allocatedBits,
                        mask);
        }
        else
        {
            readUncompressedNotInterleaved(
                        information,
                        channelsNumber,
                        pSourceStream,
                        wordSizeBytes,
                        allocatedBits,
                        mask);
        }
    }

    //
    // The image is RLE compressed
    //
    ///////////////////////////////////////////////////////////
    else
    {
        if(bSubSampledX || bSubSampledY)
        {
            IMEBRA_THROW(CodecCorruptedFileError, "Cannot read subsampled RLE images");
        }

        readRLECompressed(information, imageWidth, imageHeight, channelsNumber, pSourceStream, allocatedBits, mask);

    } // ...End of RLE decoding

    // Adjust b2complement buffers
    ///////////////////////////////////////////////////////////
    if(b2Complement)
    {
        std::int32_t checkSign = (std::int32_t)1 << highBit;
        std::int32_t orMask = ((std::int32_t)-1) << highBit;

        for(size_t adjChannels = 0; adjChannels < information.m_channels.size(); ++adjChannels)
        {
            std::int32_t* pAdjBuffer = information.m_channels[adjChannels]->m_pBuffer;
            std::uint32_t adjSize = information.m_channels[adjChannels]->m_bufferSize;
            while(adjSize != 0)
            {
                if(*pAdjBuffer & checkSign)
                {
                    *pAdjBuffer |= orMask;
                }
                ++pAdjBuffer;
                --adjSize;
            }
        }
    }


    // Copy the dicom channels into the image
    ///////////////////////////////////////////////////////////
    std::uint32_t maxSamplingFactorX = bSubSampledX ? 2 : 1;
    std::uint32_t maxSamplingFactorY = bSubSampledY ? 2 : 1;
    for(std::uint32_t copyChannels = 0; copyChannels < channelsNumber; ++copyChannels)
    {
        dicomInformation::ptrChannel dicomChannel = information.m_channels[copyChannels];
        handler->copyFromInt32Interleaved(
                    dicomChannel->m_pBuffer,
                    maxSamplingFactorX /dicomChannel->m_samplingFactorX,
                    maxSamplingFactorY /dicomChannel->m_samplingFactorY,
                    0, 0,
                    dicomChannel->m_width * maxSamplingFactorX / dicomChannel->m_samplingFactorX,
                    dicomChannel->m_height * maxSamplingFactorY / dicomChannel->m_samplingFactorY,
                    copyChannels,
                    imageWidth,
                    imageHeight,
                    channelsNumber);
    }

    // Return OK
    ///////////////////////////////////////////////////////////
    return pImage;

    IMEBRA_FUNCTION_END();

}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Allocate the channels used to read/write an image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomImageCodec::allocChannels(dicomInformation& information, std::uint32_t channelsNumber, std::uint32_t width, std::uint32_t height, bool bSubSampledX, bool bSubSampledY)
{
    IMEBRA_FUNCTION_START();

    if(bSubSampledX && (width & 0x1) != 0)
    {
        ++width;
    }

    if(bSubSampledY && (height & 0x1) != 0)
    {
        ++height;
    }

    information.m_channels.resize(channelsNumber);
    for(std::uint32_t channelNum = 0; channelNum < channelsNumber; ++channelNum)
    {
        std::uint32_t channelWidth = width;
        std::uint32_t channelHeight = height;
        std::uint32_t samplingFactorX = 1;
        std::uint32_t samplingFactorY = 1;
        if(channelNum != 0)
        {
            if(bSubSampledX)
            {
                channelWidth >>= 1;
            }
            if(bSubSampledY)
            {
                channelHeight >>= 1;
            }
        }
        else
        {
            if(bSubSampledX)
            {
                ++samplingFactorX;
            }
            if(bSubSampledY)
            {
                ++samplingFactorY;
            }
        }

        dicomInformation::ptrChannel newChannel(std::make_shared<channel>());
        newChannel->allocate(channelWidth, channelHeight);
        newChannel->m_samplingFactorX = samplingFactorX;
        newChannel->m_samplingFactorY = samplingFactorY;

        information.m_channels[channelNum] = newChannel;
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read an uncompressed interleaved image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomImageCodec::readUncompressedInterleaved(
        dicomInformation& information,
        std::uint32_t channelsNumber,
        bool bSubSampledX,
        bool bSubSampledY,
        streamReader* pSourceStream,
        std::uint32_t wordSizeBytes,
        std::uint8_t allocatedBits,
        std::uint32_t mask
        )
{
    IMEBRA_FUNCTION_START();

    std::uint8_t  bitPointer=0x0;

    std::vector<std::int32_t*> channelsMemory(information.m_channels.size());
    for(size_t copyChannelsPntr = 0; copyChannelsPntr < information.m_channels.size(); ++copyChannelsPntr)
    {
        channelsMemory[copyChannelsPntr] = information.m_channels[copyChannelsPntr]->m_pBuffer;
    }

    // No subsampling here
    ///////////////////////////////////////////////////////////
    if(!bSubSampledX && !bSubSampledY)
    {
        std::uint8_t readBuffer[4];
        for(std::uint32_t totalSize = information.m_channels[0]->m_bufferSize; totalSize != 0; --totalSize)
        {
            for(std::uint32_t scanChannels = 0; scanChannels != channelsNumber; ++scanChannels)
            {
                readPixel(information, pSourceStream, channelsMemory[scanChannels]++, 1, &bitPointer, readBuffer, wordSizeBytes, allocatedBits, mask);
            }
        }
        return;
    }

    std::uint32_t numValuesPerBlock(channelsNumber);
    if(bSubSampledX)
    {
        ++numValuesPerBlock;
    }
    if(bSubSampledY)
    {
        numValuesPerBlock += 2;
    }
    std::vector<std::int32_t> readBlockValues((size_t)numValuesPerBlock);

    // Read the subsampled channels.
    // Find the number of blocks to read
    ///////////////////////////////////////////////////////////
    std::uint32_t adjWidth = information.m_channels[0]->m_width;
    std::uint32_t adjHeight = information.m_channels[0]->m_height;

    std::uint32_t maxSamplingFactorX = bSubSampledX ? 2 : 1;
    std::uint32_t maxSamplingFactorY = bSubSampledY ? 2 : 1;

    std::shared_ptr<memory> readBuffer(std::make_shared<memory>(numValuesPerBlock * ((7 + allocatedBits) >> 3)));

    // Read all the blocks
    ///////////////////////////////////////////////////////////
    std::uint32_t numBlocksY = adjHeight / maxSamplingFactorY;
    std::uint32_t numBlocksX = adjWidth / maxSamplingFactorX;
    for(std::uint32_t blockY(0); blockY != numBlocksY; ++blockY)
    {
        for(std::uint32_t blockX(0); blockX != numBlocksX; ++blockX)
        {
            std::int32_t* readBlockValuesPtr(&(readBlockValues[0]));
            readPixel(information, pSourceStream, readBlockValuesPtr, numValuesPerBlock, &bitPointer, readBuffer->data(), wordSizeBytes, allocatedBits, mask);

            // Read channel 0 (not subsampled)
            ///////////////////////////////////////////////////////////
            *(channelsMemory[0]++) = *readBlockValuesPtr++;
            if(bSubSampledX)
            {
                *(channelsMemory[0]++) = *readBlockValuesPtr++;
            }
            if(bSubSampledY)
            {
                *(channelsMemory[0]+adjWidth-2) = *readBlockValuesPtr++;
                *(channelsMemory[0]+adjWidth-1) = *readBlockValuesPtr++;
            }
            // Read channels 1... (subsampled)
            ///////////////////////////////////////////////////////////
            for(std::uint32_t scanSubSampled = 1; scanSubSampled < channelsNumber; ++scanSubSampled)
            {
                *(channelsMemory[scanSubSampled]++) = *readBlockValuesPtr++;
            }
        }
        if(bSubSampledY)
        {
            channelsMemory[0] += adjWidth;
        }
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write an uncompressed interleaved image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomImageCodec::writeUncompressedInterleaved(
        dicomInformation& information,
        std::uint32_t channelsNumber,
        bool bSubSampledX,
        bool bSubSampledY,
        streamWriter* pDestStream,
        std::uint32_t wordSizeBytes,
        std::uint8_t allocatedBits,
        std::uint32_t mask
        )
{
    IMEBRA_FUNCTION_START();

    std::uint8_t  bitPointer=0x0;

    std::vector<std::int32_t*> channelsMemory(information.m_channels.size());
    for(size_t copyChannelsPntr = 0; copyChannelsPntr < information.m_channels.size(); ++copyChannelsPntr)
    {
        channelsMemory[copyChannelsPntr] = information.m_channels[copyChannelsPntr]->m_pBuffer;
    }

    // No subsampling here
    ///////////////////////////////////////////////////////////
    if(!bSubSampledX && !bSubSampledY)
    {
        for(std::uint32_t totalSize = information.m_channels[0]->m_bufferSize; totalSize != 0; --totalSize)
        {
            for(std::uint32_t scanChannels = 0; scanChannels < channelsNumber; ++scanChannels)
            {
                writePixel(information, pDestStream, *(channelsMemory[scanChannels]++), &bitPointer, wordSizeBytes, allocatedBits, mask);
            }
        }
        flushUnwrittenPixels(information, pDestStream, &bitPointer, wordSizeBytes);
        return;
    }

    // Write the subsampled channels.
    // Find the number of blocks to write
    ///////////////////////////////////////////////////////////
    std::uint32_t adjWidth = information.m_channels[0]->m_width;
    std::uint32_t adjHeight = information.m_channels[0]->m_height;

    std::uint32_t maxSamplingFactorX = bSubSampledX ? 2 : 1;
    std::uint32_t maxSamplingFactorY = bSubSampledY ? 2 : 1;

    // Write all the blocks
    ///////////////////////////////////////////////////////////
    std::uint32_t numBlocksY = adjHeight / maxSamplingFactorY;
    std::uint32_t numBlocksX = adjWidth / maxSamplingFactorX;
    for(std::uint32_t blockY(0); blockY != numBlocksY; ++blockY)
    {
        for(std::uint32_t blockX(0); blockX != numBlocksX; ++blockX)
        {
            // Write channel 0 (not subsampled)
            ///////////////////////////////////////////////////////////
            writePixel(information, pDestStream, *(channelsMemory[0]++), &bitPointer, wordSizeBytes, allocatedBits, mask);
            if(bSubSampledX)
            {
                writePixel(information, pDestStream, *(channelsMemory[0]++), &bitPointer, wordSizeBytes, allocatedBits, mask);
            }
            if(bSubSampledY)
            {
                writePixel(information, pDestStream, *(channelsMemory[0]+adjWidth-2), &bitPointer, wordSizeBytes, allocatedBits, mask);
                writePixel(information, pDestStream, *(channelsMemory[0]+adjWidth-1), &bitPointer, wordSizeBytes, allocatedBits, mask);
            }
            // Write channels 1... (subsampled)
            ///////////////////////////////////////////////////////////
            for(std::uint32_t scanSubSampled = 1; scanSubSampled < channelsNumber; ++scanSubSampled)
            {
                writePixel(information, pDestStream, *(channelsMemory[scanSubSampled]++), &bitPointer, wordSizeBytes, allocatedBits, mask);
            }
        }
        if(bSubSampledY)
        {
            channelsMemory[0] += adjWidth;
        }
    }

    flushUnwrittenPixels(information, pDestStream, &bitPointer, wordSizeBytes);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read an uncompressed not interleaved image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomImageCodec::readUncompressedNotInterleaved(
        dicomInformation& information,
        std::uint32_t channelsNumber,
        streamReader* pSourceStream,
        std::uint32_t wordSizeBytes,
        std::uint8_t allocatedBits,
        std::uint32_t mask
        )
{
    IMEBRA_FUNCTION_START();

    std::uint8_t  bitPointer=0x0;

    std::shared_ptr<memory> readBuffer;
    std::uint32_t lastBufferSize(0);

    // Read all the pixels
    ///////////////////////////////////////////////////////////
    for(std::uint32_t channel = 0; channel < channelsNumber; ++channel)
    {
        if(information.m_channels[channel]->m_bufferSize != lastBufferSize)
        {
            lastBufferSize = information.m_channels[channel]->m_bufferSize;
            readBuffer = std::make_shared<memory>(lastBufferSize * ((7+allocatedBits) >> 3));
        }
        std::int32_t* pMemoryDest = information.m_channels[channel]->m_pBuffer;
        readPixel(information, pSourceStream, pMemoryDest, information.m_channels[channel]->m_bufferSize, &bitPointer, readBuffer->data(), wordSizeBytes, allocatedBits, mask);
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write an uncompressed not interleaved image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomImageCodec::writeUncompressedNotInterleaved(
        dicomInformation& information,
        std::uint32_t channelsNumber,
        streamWriter* pDestStream,
        std::uint32_t wordSizeBytes,
        std::uint8_t allocatedBits,
        std::uint32_t mask
        )
{
    IMEBRA_FUNCTION_START();

    std::uint8_t  bitPointer=0x0;

    // Write all the pixels
    ///////////////////////////////////////////////////////////
    for(std::uint32_t channel = 0; channel < channelsNumber; ++channel)
    {
        std::int32_t* pMemoryDest = information.m_channels[channel]->m_pBuffer;
        for(std::uint32_t scanPixels = information.m_channels[channel]->m_bufferSize; scanPixels != 0; --scanPixels)
        {
            writePixel(information, pDestStream, *pMemoryDest++, &bitPointer, wordSizeBytes, allocatedBits, mask);
        }
    }
    flushUnwrittenPixels(information, pDestStream, &bitPointer, wordSizeBytes);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write a RLE compressed image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomImageCodec::writeRLECompressed(
        dicomInformation& information,
        std::uint32_t imageWidth,
        std::uint32_t imageHeight,
        std::uint32_t channelsNumber,
        streamWriter* pDestStream,
        std::uint8_t allocatedBits,
        std::uint32_t mask
        )
{
    IMEBRA_FUNCTION_START();

    std::uint32_t segmentsOffset[16];
    ::memset(segmentsOffset, 0, sizeof(segmentsOffset));

    // The first phase fills the segmentsOffset pointers, the
    //  second phase writes to the stream.
    ///////////////////////////////////////////////////////////
    for(int phase = 0; phase < 2; ++phase)
    {
        if(phase == 1)
        {
            pDestStream->adjustEndian((std::uint8_t*)segmentsOffset, 4, streamController::lowByteEndian, sizeof(segmentsOffset) / sizeof(segmentsOffset[0]));
            pDestStream->write((std::uint8_t*)segmentsOffset, sizeof(segmentsOffset));
        }

        std::uint32_t segmentNumber = 0;
        std::uint32_t offset = 64;

        for(std::uint32_t scanChannels = 0; scanChannels < channelsNumber; ++scanChannels)
        {
            std::vector<std::uint8_t> rowBytes(imageWidth);

            std::vector<std::uint8_t> differentBytes;
            differentBytes.reserve(imageWidth);

            for(std::int32_t rightShift = (std::int32_t)(((allocatedBits + 7) & 0xfffffff8) - 8); rightShift >= 0; rightShift -= 8)
            {
                std::int32_t* pPixel = information.m_channels[scanChannels]->m_pBuffer;

                if(phase == 0)
                {
                    segmentsOffset[++segmentNumber] = offset;
                    segmentsOffset[0] = segmentNumber;
                }
                else
                {
                    offset = segmentsOffset[++segmentNumber];
                }

                for(std::uint32_t scanY = imageHeight; scanY != 0; --scanY)
                {
                    std::uint8_t* rowBytesPointer = &(rowBytes[0]);

                    for(std::uint32_t scanX = imageWidth; scanX != 0; --scanX)
                    {
                        *(rowBytesPointer++) = (std::uint8_t)((*pPixel & mask) >> rightShift);
                        ++pPixel;
                    }

                    for(size_t scanBytes = 0; scanBytes < imageWidth; /* left empty */)
                    {
                        std::uint8_t currentByte = rowBytes[scanBytes];

                        // Calculate the run-length
                        ///////////////////////////
                        size_t runLength(1);
                        for(; ((scanBytes + runLength) != imageWidth) && rowBytes[scanBytes + runLength] == currentByte; ++runLength)
                        {
                        }

                        // Write the runlength
                        //////////////////////
                        if(runLength > 3)
                        {
                            if(!differentBytes.empty())
                            {
                                offset += (std::uint32_t)writeRLEDifferentBytes(&differentBytes, pDestStream, phase == 1);
                            }
                            if(runLength > 128)
                            {
                                runLength = 128;
                            }
                            offset += 2;
                            scanBytes += runLength;
                            if(phase == 1)
                            {
                                std::uint8_t lengthByte = (std::uint8_t)(1 - runLength);
                                pDestStream->write(&lengthByte, 1);
                                pDestStream->write(&currentByte, 1);
                            }
                            continue;
                        }

                        // Remmember sequence of different bytes
                        ////////////////////////////////////////
                        differentBytes.push_back(currentByte);
                        ++scanBytes;
                    } // for(std::uint32_t scanBytes = 0; scanBytes < imageWidth; )

                    offset += (std::uint32_t)writeRLEDifferentBytes(&differentBytes, pDestStream, phase == 1);

                } // for(std::uint32_t scanY = imageHeight; scanY != 0; --scanY)

                if((offset & 1) != 0)
                {
                    ++offset;
                    if(phase == 1)
                    {
                        const std::uint8_t command = 0x80;
                        pDestStream->write(&command, 1);
                    }
                }

            } // for(std::int32_t rightShift = ((allocatedBits + 7) & 0xfffffff8) -8; rightShift >= 0; rightShift -= 8)

        } // for(int scanChannels = 0; scanChannels < channelsNumber; ++scanChannels)

    } // for(int phase = 0; phase < 2; ++phase)

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write RLE sequence of different bytes
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
size_t dicomImageCodec::writeRLEDifferentBytes(std::vector<std::uint8_t>* pDifferentBytes, streamWriter* pDestStream, bool bWrite)
{
    IMEBRA_FUNCTION_START();

    size_t writtenLength = 0;
    for(size_t offset(0); offset != pDifferentBytes->size();)
    {
        size_t writeSize = pDifferentBytes->size() - offset;
        if(writeSize > 128)
        {
            writeSize = 128;
        }
        writtenLength += writeSize + 1;
        if(bWrite)
        {
            const std::uint8_t writeLength((std::uint8_t)(writeSize - 1));
            pDestStream->write(&writeLength, 1);
            pDestStream->write(&(pDifferentBytes->at(offset)), writeSize);
        }
        offset += writeSize;
    }
    pDifferentBytes->clear();

    // return number of written bytes
    /////////////////////////////////
    return writtenLength;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read a RLE compressed image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomImageCodec::readRLECompressed(
        dicomInformation& information,
        std::uint32_t imageWidth,
        std::uint32_t imageHeight,
        std::uint32_t channelsNumber,
        streamReader* pSourceStream,
        std::uint8_t allocatedBits,
        std::uint32_t mask)
{
    IMEBRA_FUNCTION_START();

    // Copy the RLE header into the segmentsOffset array
    //  and adjust the byte endian to the machine architecture
    ///////////////////////////////////////////////////////////
    std::uint32_t segmentsOffset[16];
    ::memset(segmentsOffset, 0, sizeof(segmentsOffset));
    pSourceStream->read((std::uint8_t*)segmentsOffset, 64);
    pSourceStream->adjustEndian((std::uint8_t*)segmentsOffset, 4, streamController::lowByteEndian, sizeof(segmentsOffset) / sizeof(segmentsOffset[0]));

    //
    // Scan all the RLE segments
    //
    ///////////////////////////////////////////////////////////
    std::uint32_t loopsNumber = channelsNumber;
    std::uint32_t loopSize = imageWidth * imageHeight;

    std::uint32_t currentSegmentOffset = sizeof(segmentsOffset);
    std::uint8_t segmentNumber = 0;
    for(std::uint32_t channel = 0; channel<loopsNumber; ++channel)
    {
        for(std::int32_t leftShift = (std::int32_t)(((allocatedBits + 7) & 0xfffffff8) - 8); leftShift >= 0; leftShift -= 8)
        {
            // Prepare to scan all the RLE segment
            ///////////////////////////////////////////////////////////
            std::uint32_t segmentOffset = segmentsOffset[++segmentNumber]; // Get the offset
            pSourceStream->seekForward(segmentOffset - currentSegmentOffset);
            currentSegmentOffset = segmentOffset;

            std::uint8_t  rleByte = 0;         // RLE code
            std::uint8_t  copyBytes = 0;       // Number of bytes to copy
            std::uint8_t  runByte = 0;         // Byte to use in run-lengths
            std::uint8_t  runLength = 0;       // Number of bytes with the same information (runByte)
            std::uint8_t  copyBytesBuffer[0x81];

            std::int32_t* pChannelMemory = information.m_channels[channel]->m_pBuffer;
            std::uint32_t channelSize = loopSize;
            std::uint8_t* pScanCopyBytes;

            // Read the RLE segment
            ///////////////////////////////////////////////////////////
            pSourceStream->read(&rleByte, 1);
            ++currentSegmentOffset;
            while(channelSize != 0)
            {
                if(rleByte==0x80)
                {
                    pSourceStream->read(&rleByte, 1);
                    ++currentSegmentOffset;
                    continue;
                }

                // Copy the specified number of bytes
                ///////////////////////////////////////////////////////////
                if(rleByte<0x80)
                {
                    copyBytes = ++rleByte;
                    if(copyBytes < channelSize)
                    {
                        pSourceStream->read(copyBytesBuffer, copyBytes + 1);
                        currentSegmentOffset += copyBytes + 1;
                        rleByte = copyBytesBuffer[copyBytes];
                    }
                    else
                    {
                        pSourceStream->read(copyBytesBuffer, copyBytes);
                        currentSegmentOffset += copyBytes;
                    }
                    pScanCopyBytes = copyBytesBuffer;
                    while(copyBytes-- && channelSize != 0)
                    {
                        *pChannelMemory |= ((*pScanCopyBytes++) << leftShift) & mask;
                        ++pChannelMemory;
                        --channelSize;
                    }
                    continue;
                }

                // Copy the same byte several times
                ///////////////////////////////////////////////////////////
                runLength = (std::uint8_t)(1-rleByte);
                if(runLength < channelSize)
                {
                    pSourceStream->read(copyBytesBuffer, 2);
                    currentSegmentOffset += 2;
                    runByte = copyBytesBuffer[0];
                    rleByte = copyBytesBuffer[1];
                }
                else
                {
                    pSourceStream->read(&runByte, 1);
                    ++currentSegmentOffset;
                }
                while(runLength-- && channelSize != 0)
                {
                    *pChannelMemory |= (runByte << leftShift) & mask;
                    ++pChannelMemory;
                    --channelSize;
                }

            } // ...End of the segment scanning loop

        } // ...End of the leftshift calculation

    } // ...Channels scanning loop

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read a single component from a DICOM raw image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomImageCodec::readPixel(
        dicomInformation& information,
        streamReader* pSourceStream,
        std::int32_t* pDest,
        std::uint32_t numPixels,
        std::uint8_t* pBitPointer,
        std::uint8_t* pReadBuffer,
        std::uint32_t wordSizeBytes,
        std::uint8_t allocatedBits,
        std::uint32_t mask)
{
    IMEBRA_FUNCTION_START();

    if(allocatedBits == 8 || allocatedBits == 16 || allocatedBits == 32)
    {
        pSourceStream->read(pReadBuffer, numPixels * (allocatedBits >> 3));
        if(allocatedBits == 8)
        {
            std::uint8_t* pSource(pReadBuffer);
            while(numPixels-- != 0)
            {
                *pDest++ = (std::int32_t)((std::uint32_t)(*pSource++) & mask);
            }
            return;
        }
        pSourceStream->adjustEndian(pReadBuffer, allocatedBits >> 3, streamController::lowByteEndian, numPixels);
        if(allocatedBits == 16)
        {
            std::uint16_t* pSource((std::uint16_t*)(pReadBuffer));
            while(numPixels-- != 0)
            {
                *pDest++ = (std::int32_t)((std::uint32_t)(*pSource++) & mask);
            }
            return;
        }
        std::uint32_t* pSource((std::uint32_t*)(pReadBuffer));
        while(numPixels-- != 0)
        {
            *pDest++ = (std::int32_t)((*pSource++) & mask);
        }
        return;

    }

    while(numPixels-- != 0)
    {
        *pDest = 0;
        for(std::uint8_t bitsToRead = allocatedBits; bitsToRead != 0;)
        {
            if(*pBitPointer == 0)
            {
                if(wordSizeBytes == 2)
                {
                    pSourceStream->read((std::uint8_t*)&information.m_ioWord, sizeof(information.m_ioWord));
                    *pBitPointer = 16;
                }
                else
                {
                    pSourceStream->read(&information.m_ioByte, 1);
                    information.m_ioWord = (std::uint16_t)information.m_ioByte;
                    *pBitPointer = 8;
                }
            }

            if(*pBitPointer <= bitsToRead)
            {
                *pDest |= information.m_ioWord << (allocatedBits - bitsToRead);
                bitsToRead = (std::uint8_t)(bitsToRead - *pBitPointer);
                *pBitPointer = 0;
                continue;
            }

            *pDest |= (information.m_ioWord & (std::uint16_t)(((std::uint32_t) 1 << bitsToRead) - 1)) << (allocatedBits - bitsToRead);
            information.m_ioWord = (std::uint16_t)(information.m_ioWord >> bitsToRead);
            *pBitPointer = (std::uint8_t)(*pBitPointer - bitsToRead);
            bitsToRead = 0;
        }
        *pDest++ &= mask;
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read a single component from a DICOM raw image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomImageCodec::writePixel(
        dicomInformation& information,
        streamWriter* pDestStream,
        std::int32_t pixelValue,
        std::uint8_t*  pBitPointer,
        std::uint32_t wordSizeBytes,
        std::uint8_t allocatedBits,
        std::uint32_t mask)
{
    IMEBRA_FUNCTION_START();

    pixelValue &= mask;

    if(allocatedBits == 8)
    {
        information.m_ioByte = (std::uint8_t)pixelValue;
        pDestStream->write(&information.m_ioByte, 1);
        return;
    }

    if(allocatedBits == 16)
    {
        if(wordSizeBytes == 1)
        {
            information.m_ioWord = pDestStream->adjustEndian((std::uint16_t)pixelValue, streamController::lowByteEndian);
        }
        else
        {
            information.m_ioWord = (std::uint16_t)pixelValue;
        }
        pDestStream->write((std::uint8_t*)&information.m_ioWord, 2);
        return;
    }

    if(allocatedBits == 32)
    {
        if(wordSizeBytes == 1)
        {
            information.m_ioDWord = pDestStream->adjustEndian((std::uint32_t)pixelValue, streamController::lowByteEndian);
        }
        else
        {
            information.m_ioDWord = (std::uint32_t)pixelValue;
        }
        pDestStream->write((std::uint8_t*)&information.m_ioDWord, 4);
        return;
    }

    std::uint8_t maxBits = (std::uint8_t)(wordSizeBytes * 8);

    for(std::uint32_t writeBits = allocatedBits; writeBits != 0;)
    {
        std::uint32_t freeBits = (maxBits - *pBitPointer);
        if(freeBits == maxBits)
        {
            information.m_ioWord = 0;
        }
        if( freeBits <= writeBits )
        {
            information.m_ioWord = (std::uint16_t)(information.m_ioWord | ((pixelValue & (((std::int32_t)1 << freeBits) -1 )) << *pBitPointer));
            *pBitPointer = maxBits;
            writeBits = writeBits - freeBits;
            pixelValue >>= freeBits;
        }
        else
        {
            information.m_ioWord = (std::uint16_t)(information.m_ioWord | ((pixelValue & (((std::int32_t)1 << writeBits) -1 ))<< *pBitPointer));
            *pBitPointer = (std::uint8_t)(*pBitPointer + writeBits);
            writeBits = 0;
        }

        if(*pBitPointer == maxBits)
        {
            if(wordSizeBytes == 2)
            {
                pDestStream->write((std::uint8_t*)&information.m_ioWord, 2);
            }
            else
            {
                information.m_ioByte = (std::uint8_t)information.m_ioWord;
                pDestStream->write(&information.m_ioByte, 1);
            }
            *pBitPointer = 0;
        }
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Used by the writing routines to commit the unwritten
//  bits
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomImageCodec::flushUnwrittenPixels(dicomInformation& information, streamWriter* pDestStream, std::uint8_t* pBitPointer, std::uint32_t wordSizeBytes)
{
    IMEBRA_FUNCTION_START();

    if(*pBitPointer == 0)
    {
        return;
    }
    if(wordSizeBytes == 2)
    {
        pDestStream->write((std::uint8_t*)&information.m_ioWord, 2);
    }
    else if(wordSizeBytes == 4)
    {
        pDestStream->write((std::uint8_t*)&information.m_ioDWord, 4);
    }
    else
    {
        information.m_ioByte = (std::uint8_t)information.m_ioWord;
        pDestStream->write(&information.m_ioByte, 1);
    }
    *pBitPointer = 0;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Insert an image into a Dicom structure
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dicomImageCodec::setImage(
        std::shared_ptr<streamWriter> pDestStream,
        std::shared_ptr<image> pImage,
        const std::string& transferSyntax,
        imageQuality_t /*imageQuality*/,
        tagVR_t dataType,
        std::uint32_t allocatedBits,
        bool bSubSampledX,
        bool bSubSampledY,
        bool bInterleaved,
        bool /*b2Complement*/) const
{
    IMEBRA_FUNCTION_START();

    // First calculate the attributes we want to use.
    // Return an exception if they are different from the
    //  old ones and bDontChangeAttributes is true
    ///////////////////////////////////////////////////////////
    std::uint32_t imageWidth, imageHeight;
    pImage->getSize(&imageWidth, &imageHeight);

    std::string colorSpace = pImage->getColorSpace();
    std::uint32_t highBit = pImage->getHighBit();
    bool bRleCompressed = (transferSyntax == "1.2.840.10008.1.2.5");

    std::shared_ptr<handlers::readingDataHandlerNumericBase> imageHandler = pImage->getReadingDataHandler();
    std::uint32_t channelsNumber = pImage->getChannelsNumber();

    // Copy the image into the dicom channels
    ///////////////////////////////////////////////////////////
    dicomInformation information;
    allocChannels(information, channelsNumber, imageWidth, imageHeight, bSubSampledX, bSubSampledY);
    std::uint32_t maxSamplingFactorX = bSubSampledX ? 2 : 1;
    std::uint32_t maxSamplingFactorY = bSubSampledY ? 2 : 1;
    for(std::uint32_t copyChannels = 0; copyChannels < channelsNumber; ++copyChannels)
    {
        dicomInformation::ptrChannel dicomChannel = information.m_channels[copyChannels];
        imageHandler->copyToInt32Interleaved(
                    dicomChannel->m_pBuffer,
                    maxSamplingFactorX /dicomChannel->m_samplingFactorX,
                    maxSamplingFactorY /dicomChannel->m_samplingFactorY,
                    0, 0,
                    dicomChannel->m_width * maxSamplingFactorX / dicomChannel->m_samplingFactorX,
                    dicomChannel->m_height * maxSamplingFactorY / dicomChannel->m_samplingFactorY,
                    copyChannels,
                    imageWidth,
                    imageHeight,
                    channelsNumber);
    }

    std::uint32_t mask = (std::uint32_t)(((std::uint64_t)1 << (highBit + 1)) - 1);

    if(bRleCompressed)
    {
        writeRLECompressed(
                    information,
                    imageWidth,
                    imageHeight,
                    channelsNumber,
                    pDestStream.get(),
                    (std::uint8_t)allocatedBits,
                    mask);
        return;
    }

    std::uint8_t wordSizeBytes = ((dataType == tagVR_t::OW) || (dataType == tagVR_t::SS) || (dataType == tagVR_t::US)) ? 2 : 1;

    if(bInterleaved || channelsNumber == 1)
    {
        writeUncompressedInterleaved(
                    information,
                    channelsNumber,
                    bSubSampledX, bSubSampledY,
                    pDestStream.get(),
                    wordSizeBytes,
                    (std::uint8_t)allocatedBits,
                    mask);
        return;
    }

    writeUncompressedNotInterleaved(
                information,
                channelsNumber,
                pDestStream.get(),
                wordSizeBytes,
                (std::uint8_t)allocatedBits,
                mask);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns true if the codec can handle the transfer
//  syntax
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
bool dicomImageCodec::canHandleTransferSyntax(const std::string& transferSyntax) const
{
    IMEBRA_FUNCTION_START();

    return(
                transferSyntax == "1.2.840.10008.1.2" ||      // Implicit VR little endian
                transferSyntax == "1.2.840.10008.1.2.1" ||    // Explicit VR little endian
                // transferSyntax=="1.2.840.10008.1.2.1.99" || // Deflated explicit VR little endian
                transferSyntax == "1.2.840.10008.1.2.2" ||    // Explicit VR big endian
                transferSyntax == "1.2.840.10008.1.2.5");     // RLE compression

    IMEBRA_FUNCTION_END();
}


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
//
// Returns true if the transfer syntax has to be
//  encapsulated
//
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
bool dicomImageCodec::encapsulated(const std::string& transferSyntax) const
{
    IMEBRA_FUNCTION_START();

    if(!canHandleTransferSyntax(transferSyntax))
    {
        IMEBRA_THROW(CodecWrongTransferSyntaxError, "Cannot handle the transfer syntax");
    }
    return (transferSyntax == "1.2.840.10008.1.2.5");

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Suggest the number of allocated bits
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dicomImageCodec::suggestAllocatedBits(const std::string& transferSyntax, std::uint32_t highBit) const
{
    IMEBRA_FUNCTION_START();

    if(transferSyntax == "1.2.840.10008.1.2.5")
    {
        return (highBit + 8) & 0xfffffff8;
    }

    return highBit + 1;

    IMEBRA_FUNCTION_END();

}

} // namespace codecs

} // namespace implementation

} // namespace imebra

