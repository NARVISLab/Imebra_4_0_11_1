/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file jpegCodecBaseImpl.cpp
    \brief Implementation of the class jpegCodecBaseImpl.

*/

#include "exceptionImpl.h"
#include "huffmanTableImpl.h"
#include "jpegCodecBaseImpl.h"
#include "imageCodecImpl.h"
#include "streamWriterImpl.h"
#include "streamReaderImpl.h"
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

namespace jpeg
{
jpegChannel::jpegChannel():
        m_quantTable(0),
        m_blockMcuX(0),
        m_blockMcuY(0),
        m_blockMcuXY(0),
        m_lastDCValue(0),
        m_defaultDCValue(0),
        m_losslessPositionX(0),
        m_losslessPositionY(0),
        m_unprocessedAmplitudesCount(0),
        m_unprocessedAmplitudesPredictor(0),
        m_huffmanTableDC(0),
        m_huffmanTableAC(0),
        m_pActiveHuffmanTableDC(0),
        m_pActiveHuffmanTableAC(0),
        m_valuesMask(0)
{
}

jpegChannel::~jpegChannel()
{
}

}
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Constructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
jpegCodecBase::jpegCodecBase()
{
    IMEBRA_FUNCTION_START();

    // Register all the tag classes
    ///////////////////////////////////////////////////////////

    // Unknown tag must be registered
    ///////////////////////////////////////////////////////////
    registerTag(unknown, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagUnknown>()));

    // Register SOF
    ///////////////////////////////////////////////////////////
    registerTag(sof0, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sof1, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sof2, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sof3, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sof5, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sof6, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sof7, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sof9, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sofA, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sofB, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sofD, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sofE, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));
    registerTag(sofF, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOF>()));

    // Register DHT
    ///////////////////////////////////////////////////////////
    registerTag(dht, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagDHT>()));

    // Register DQT
    ///////////////////////////////////////////////////////////
    registerTag(dqt, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagDQT>()));

    // Register SOS
    ///////////////////////////////////////////////////////////
    registerTag(sos, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagSOS>()));

    // Register EOI
    ///////////////////////////////////////////////////////////
    registerTag(eoi, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagEOI>()));

    // Register RST
    ///////////////////////////////////////////////////////////
    registerTag(rst0, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagRST>()));
    registerTag(rst1, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagRST>()));
    registerTag(rst2, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagRST>()));
    registerTag(rst3, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagRST>()));
    registerTag(rst4, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagRST>()));
    registerTag(rst5, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagRST>()));
    registerTag(rst6, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagRST>()));
    registerTag(rst7, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagRST>()));

    // Register DRI
    ///////////////////////////////////////////////////////////
    registerTag(dri, std::shared_ptr<jpeg::tag>(std::make_shared<jpeg::tagDRI>()));

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Register a tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void jpegCodecBase::registerTag(tTagId tagId, std::shared_ptr<jpeg::tag> pTag)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<jpeg::tag> test = pTag;
    m_tagsMap[(std::uint8_t)tagId]=pTag;

    IMEBRA_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write a single jpeg tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void jpegCodecBase::writeTag(streamWriter* pDestinationStream, tTagId tagId, jpeg::jpegInformation& information) const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<jpeg::tag> pTag;
    tTagsMap::const_iterator findTag = m_tagsMap.find((std::uint8_t)tagId);
    if(findTag == m_tagsMap.end())
    {
        return;
    }
    const std::uint8_t ff(0xff);
    std::uint8_t byteTagId(tagId);
    pDestinationStream->write(&ff, 1);
    pDestinationStream->write(&byteTagId, 1);
    findTag->second->writeTag(pDestinationStream, information);

    IMEBRA_FUNCTION_END();
}


namespace jpeg
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegInformation
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

jpegInformation::jpegInformation()
{
    reset(imageQuality_t::veryHigh);
}


void jpegInformation::reset(imageQuality_t compQuality)
{
    IMEBRA_FUNCTION_START();

    // Factor used to calculate the quantization tables used
    //  for the compression
    ///////////////////////////////////////////////////////////
    float compQuantization = (float)compQuality / (float)imageQuality_t::medium;

    // Resets the channels list
    ///////////////////////////////////////////////////////////
    memset(m_channelsList, 0, sizeof(m_channelsList));

    // Allocate the huffman tables
    ///////////////////////////////////////////////////////////
    for(int resetHuffmanTables = 0; resetHuffmanTables<16; ++resetHuffmanTables)
    {
        std::shared_ptr<huffmanTable> huffmanDC(std::make_shared<huffmanTable>(9));
        m_pHuffmanTableDC[resetHuffmanTables] = huffmanDC;

        std::shared_ptr<huffmanTable> huffmanAC(std::make_shared<huffmanTable>(9));
        m_pHuffmanTableAC[resetHuffmanTables] = huffmanAC;
    }

    eraseChannels();

    m_bEndOfImage = false;

    m_imageWidth = m_imageHeight = 0;

    m_precision = 8;
    m_valuesMask = ((std::int32_t)1 << m_precision)-1;

    m_process = 0;

    m_mcuPerRestartInterval = 0;

    m_mcuLastRestart = 0;

    m_spectralIndexStart = 0;
    m_spectralIndexEnd = 63;

    m_bLossless = false;

    // The number of MCUs (horizontal, vertical, total)
    ///////////////////////////////////////////////////////////
    m_mcuNumberX = 0;
    m_mcuNumberY = 0;
    m_mcuNumberTotal = 0;

    m_maxSamplingFactorX = 0;
    m_maxSamplingFactorY = 0;

    m_mcuProcessed = 0;
    m_mcuProcessedX = 0;
    m_mcuProcessedY = 0;
    m_eobRun = 0;

    m_jpegImageWidth = 0;
    m_jpegImageHeight = 0;

    // Reset the QT tables
    ///////////////////////////////////////////////////////////
    for(int resetQT = 0; resetQT<16; ++resetQT)
    {
        const std::uint32_t* pSourceTable = (resetQT == 0) ? JpegStdLuminanceQuantTbl : JpegStdChrominanceQuantTbl;

        std::uint8_t tableIndex = 0;
        for(std::uint8_t row = 0; row < 8; ++row)
        {
            for(std::uint8_t col = 0; col < 8; ++col)
            {
                std::uint32_t quant = (std::uint32_t) ((float)(pSourceTable[tableIndex]) * compQuantization);
                if(quant < 1)
                {
                    quant = 1;
                }
                if(quant > 255)
                {
                    quant = 255;
                }
                m_quantizationTable[resetQT][tableIndex++] = quant;
            }
        }
        recalculateQuantizationTables(resetQT);
    }

    // Reset the huffman tables
    ///////////////////////////////////////////////////////////
    for(int resetHT=0; resetHT < 16; ++resetHT)
    {
        m_pHuffmanTableDC[resetHT]->reset();
        m_pHuffmanTableAC[resetHT]->reset();
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Erase all the allocated channels
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void jpegInformation::eraseChannels()
{
    IMEBRA_FUNCTION_START();

    m_channelsMap.clear();
    memset(m_channelsList, 0, sizeof(m_channelsList));

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Allocate the channels. This function is called when a
//  SOF tag is found
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void jpegInformation::allocChannels()
{
    IMEBRA_FUNCTION_START();

    m_maxSamplingFactorX=1L;
    m_maxSamplingFactorY=1L;

    m_valuesMask = ((std::int32_t)1 << m_precision)-1;

    // Find the maximum sampling factor
    ///////////////////////////////////////////////////////////
    for(tChannelsMap::iterator channelsIterator0=m_channelsMap.begin(); channelsIterator0!=m_channelsMap.end(); ++channelsIterator0)
    {
        std::shared_ptr<jpeg::jpegChannel> pChannel=channelsIterator0->second;

        if(pChannel->m_samplingFactorX>m_maxSamplingFactorX)
            m_maxSamplingFactorX=pChannel->m_samplingFactorX;
        if(pChannel->m_samplingFactorY>m_maxSamplingFactorY)
            m_maxSamplingFactorY=pChannel->m_samplingFactorY;
    }

    if(m_bLossless)
    {
        m_jpegImageWidth=(m_imageWidth+(m_maxSamplingFactorX-1))/m_maxSamplingFactorX;
        m_jpegImageWidth*=m_maxSamplingFactorX;
        m_jpegImageHeight=(m_imageHeight+(m_maxSamplingFactorY-1))/m_maxSamplingFactorY;
        m_jpegImageHeight*=m_maxSamplingFactorY;
    }
    else
    {
        m_jpegImageWidth=(m_imageWidth+((m_maxSamplingFactorX<<3)-1))/(m_maxSamplingFactorX<<3);
        m_jpegImageWidth*=(m_maxSamplingFactorX<<3);
        m_jpegImageHeight=(m_imageHeight+((m_maxSamplingFactorY<<3)-1))/(m_maxSamplingFactorY<<3);
        m_jpegImageHeight*=(m_maxSamplingFactorY<<3);
    }

    // Allocate the channels' buffers
    ///////////////////////////////////////////////////////////
    for(tChannelsMap::iterator channelsIterator1=m_channelsMap.begin(); channelsIterator1 != m_channelsMap.end(); ++channelsIterator1)
    {
        std::shared_ptr<jpeg::jpegChannel> pChannel=channelsIterator1->second;
        pChannel->m_defaultDCValue = m_bLossless ? ((std::int32_t)1<<(m_precision - 1)) : 0;
        pChannel->m_lastDCValue = pChannel->m_defaultDCValue;

        pChannel->allocate(
                    m_jpegImageWidth*(std::uint32_t)pChannel->m_samplingFactorX/m_maxSamplingFactorX,
                    m_jpegImageHeight*(std::uint32_t)pChannel->m_samplingFactorY/m_maxSamplingFactorY);
        pChannel->m_valuesMask = m_valuesMask;
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Find the MCU's size
// This function is called when a SOS tag is found
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void jpegInformation::findMcuSize()
{
    IMEBRA_FUNCTION_START();

    // Find the maximum sampling factor for all the channels
    ///////////////////////////////////////////////////////////
    std::uint32_t maxSamplingFactorChannelsX=1;
    std::uint32_t maxSamplingFactorChannelsY=1;
    for(tChannelsMap::iterator allChannelsIterator=m_channelsMap.begin(); allChannelsIterator!=m_channelsMap.end(); ++allChannelsIterator)
    {
        std::shared_ptr<jpeg::jpegChannel> pChannel = allChannelsIterator->second;

        if(pChannel->m_samplingFactorX > maxSamplingFactorChannelsX)
            maxSamplingFactorChannelsX = pChannel->m_samplingFactorX;
        if(pChannel->m_samplingFactorY > maxSamplingFactorChannelsY)
            maxSamplingFactorChannelsY = pChannel->m_samplingFactorY;
    }


    // Find the minimum and maximum sampling factor in the scan
    ///////////////////////////////////////////////////////////
    std::uint32_t maxSamplingFactorX=1;
    std::uint32_t maxSamplingFactorY=1;
    std::uint32_t minSamplingFactorX=256;
    std::uint32_t minSamplingFactorY=256;

    jpeg::jpegChannel* pChannel; // Used in the lòops
    for(jpeg::jpegChannel** channelsIterator = m_channelsList; *channelsIterator != 0; ++channelsIterator)
    {
        pChannel = *channelsIterator;

        if(pChannel->m_samplingFactorX > maxSamplingFactorX)
            maxSamplingFactorX = pChannel->m_samplingFactorX;
        if(pChannel->m_samplingFactorY > maxSamplingFactorY)
            maxSamplingFactorY = pChannel->m_samplingFactorY;
        if(pChannel->m_samplingFactorX < minSamplingFactorX)
            minSamplingFactorX = pChannel->m_samplingFactorX;
        if(pChannel->m_samplingFactorY < minSamplingFactorY)
            minSamplingFactorY = pChannel->m_samplingFactorY;
    }

    // Find the number of blocks per MCU per channel
    ///////////////////////////////////////////////////////////
    for(jpeg::jpegChannel** channelsIterator = m_channelsList; *channelsIterator != 0; ++channelsIterator)
    {
        pChannel=*channelsIterator;

        pChannel->m_blockMcuX = pChannel->m_samplingFactorX / minSamplingFactorX;
        pChannel->m_blockMcuY = pChannel->m_samplingFactorY / minSamplingFactorY;
        pChannel->m_blockMcuXY = pChannel->m_blockMcuX * pChannel->m_blockMcuY;
        pChannel->m_losslessPositionX = 0;
        pChannel->m_losslessPositionY = 0;
        pChannel->m_unprocessedAmplitudesCount = 0;
        pChannel->m_unprocessedAmplitudesPredictor = 0;
        pChannel->m_lastDCValue = pChannel->m_defaultDCValue;
    }

    // Find the MCU size, in image's pixels
    ///////////////////////////////////////////////////////////
    if(m_bLossless)
    {
        m_mcuNumberX = m_jpegImageWidth * minSamplingFactorX / maxSamplingFactorChannelsX;
        m_mcuNumberY = m_jpegImageHeight * minSamplingFactorY / maxSamplingFactorChannelsY;
    }
    else
    {
        std::uint32_t xBoundary = 8 * maxSamplingFactorChannelsX / minSamplingFactorX;
        std::uint32_t yBoundary = 8 * maxSamplingFactorChannelsY / minSamplingFactorY;

        m_mcuNumberX = (m_imageWidth + xBoundary - 1) / xBoundary;
        m_mcuNumberY = (m_imageHeight + yBoundary - 1) / yBoundary;
    }
    m_mcuNumberTotal = m_mcuNumberX*m_mcuNumberY;
    m_mcuProcessed = 0;
    m_mcuProcessedX = 0;
    m_mcuProcessedY = 0;


    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Calculate the quantization tables with the correction
//  factor needed by the IDCT/FDCT
//
//
///////////////////////////////////////////////////////////;
///////////////////////////////////////////////////////////
void jpegInformation::recalculateQuantizationTables(int table)
{
    IMEBRA_FUNCTION_START();

    // Adjust the tables for compression/decompression
    ///////////////////////////////////////////////////////////
    std::uint8_t tableIndex = 0;
    for(std::uint8_t row = 0; row<8; ++row)
    {
        for(std::uint8_t col = 0; col<8; ++col)
        {
            m_decompressionQuantizationTable[table][tableIndex]=(long long)((float)((m_quantizationTable[table][tableIndex])<<JPEG_DECOMPRESSION_BITS_PRECISION)*JpegDctScaleFactor[col]*JpegDctScaleFactor[row]);
            m_compressionQuantizationTable[table][tableIndex]=1.0f/((float)((m_quantizationTable[table][tableIndex])<<3)*JpegDctScaleFactor[col]*JpegDctScaleFactor[row]);
            ++tableIndex;
        }
    }

    IMEBRA_FUNCTION_END();
}



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegChannel
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

void jpegChannel::processUnprocessedAmplitudes()
{
    IMEBRA_FUNCTION_START();

    if(m_unprocessedAmplitudesCount == 0)
    {
        return;
    }

    std::int32_t* pDest = m_pBuffer + (m_losslessPositionY * m_width + m_losslessPositionX);
    std::int32_t* pSource = m_unprocessedAmplitudesBuffer;

    // Find missing pixels
    std::int32_t missingPixels = (std::int32_t)m_width - (std::int32_t)m_losslessPositionX + (std::int32_t)m_width * ((std::int32_t)m_height - (std::int32_t)m_losslessPositionY - 1);
    if(missingPixels < (std::int32_t)m_unprocessedAmplitudesCount)
    {
        IMEBRA_THROW(CodecCorruptedFileError, "Excess data in the lossless jpeg stream");
    }

    if(m_unprocessedAmplitudesPredictor == 0)
    {
        while(m_unprocessedAmplitudesCount != 0)
        {
            --m_unprocessedAmplitudesCount;
            *(pDest++) = *(pSource++) & m_valuesMask;
            if(++m_losslessPositionX == m_width)
            {
                m_losslessPositionX = 0;
                ++m_losslessPositionY;
            }
        }
        m_lastDCValue = *(pDest - 1);
        return;
    }

    int applyPrediction;
    std::int32_t* pPreviousLine = pDest - m_width;
    std::int32_t* pPreviousLineColumn = pDest - m_width - 1;
    while(m_unprocessedAmplitudesCount != 0)
    {
        --m_unprocessedAmplitudesCount;
        applyPrediction = (int)m_unprocessedAmplitudesPredictor;
        if(m_losslessPositionY == 0)
        {
            applyPrediction = 1;
        }
        else if(m_losslessPositionX == 0)
        {
            applyPrediction = 2;
        }
        switch(applyPrediction)
        {
        case 1:
            m_lastDCValue += *(pSource++);
            break;
        case 2:
            m_lastDCValue = *(pSource++) + *pPreviousLine;
            break;
        case 3:
            m_lastDCValue = *(pSource++) + *pPreviousLineColumn;
            break;
        case 4:
            m_lastDCValue += *(pSource++) + *pPreviousLine - *pPreviousLineColumn;
            break;
        case 5:
            m_lastDCValue += *(pSource++) + ((*pPreviousLine - *pPreviousLineColumn)>>1);
            break;
        case 6:
            m_lastDCValue -= *pPreviousLineColumn;
            m_lastDCValue >>= 1;
            m_lastDCValue += *(pSource++) + *pPreviousLine;
            break;
        case 7:
            m_lastDCValue += *pPreviousLine;
            m_lastDCValue >>= 1;
            m_lastDCValue += *(pSource++);
            break;
        default:
            IMEBRA_THROW(CodecCorruptedFileError, "Wrong predictor index in lossless jpeg stream");
        }

        m_lastDCValue &= m_valuesMask;
        *pDest++ = m_lastDCValue;

        ++pPreviousLine;
        ++pPreviousLineColumn;
        if(++m_losslessPositionX == m_width)
        {
            ++m_losslessPositionY;
            m_losslessPositionX = 0;
        }
    }
    IMEBRA_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegCodecTag
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
// Write the tag's length
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void tag::writeLength(streamWriter* pStream, std::uint16_t length) const
{
    IMEBRA_FUNCTION_START();

    length = (std::uint16_t)(length + sizeof(length));
    pStream->adjustEndian((std::uint8_t*)&length, sizeof(length), streamController::highByteEndian);
    pStream->write((std::uint8_t*)&length, sizeof(length));

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read the tag's length
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t tag::readLength(streamReader* pStream) const
{
    IMEBRA_FUNCTION_START();

    std::uint16_t length = 0;
    pStream->read((std::uint8_t*)&length, sizeof(length));
    pStream->adjustEndian((std::uint8_t*)&length, sizeof(length), streamController::highByteEndian);
    if(length > 1)
        length = (std::uint16_t)(length - 2);
    return (std::uint32_t)length;

    IMEBRA_FUNCTION_END();
}



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagUnknown
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
// Write the tag's content
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void tagUnknown::writeTag(streamWriter* pStream, jpegInformation& /* information */) const
{
    IMEBRA_FUNCTION_START();

    writeLength(pStream, 0);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read the tag's content
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void tagUnknown::readTag(streamReader* pStream, jpegInformation* /* pInformation */, std::uint8_t /* tagEntry */) const
{
    IMEBRA_FUNCTION_START();

    std::uint32_t tagLength = readLength(pStream);
    pStream->seekForward(tagLength);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagSOF
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
// Write the tag's content
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void tagSOF::writeTag(streamWriter* pStream, jpegInformation& information) const
{
    IMEBRA_FUNCTION_START();

    // Calculate the components number
    ///////////////////////////////////////////////////////////
    std::uint8_t componentsNumber = (std::uint8_t)(information.m_channelsMap.size());

    // Write the tag's length
    ///////////////////////////////////////////////////////////
    writeLength(pStream, (std::uint16_t)(6 + componentsNumber * 3));

    // Write the precision, in bits
    ///////////////////////////////////////////////////////////
    std::uint8_t precisionBits=(std::uint8_t)information.m_precision;
    pStream->write(&precisionBits, 1);

    // Write the image's size, in pixels
    ///////////////////////////////////////////////////////////
    std::uint16_t imageWidth=(std::uint16_t)information.m_imageWidth;
    std::uint16_t imageHeight=(std::uint16_t)information.m_imageHeight;
    pStream->adjustEndian((std::uint8_t*)&imageHeight, 2, streamController::highByteEndian);
    pStream->adjustEndian((std::uint8_t*)&imageWidth, 2, streamController::highByteEndian);
    pStream->write((std::uint8_t*)&imageHeight, 2);
    pStream->write((std::uint8_t*)&imageWidth, 2);

    // write the components number
    ///////////////////////////////////////////////////////////
    pStream->write((std::uint8_t*)&componentsNumber, 1);

    // Write all the components specifications
    ///////////////////////////////////////////////////////////
    std::uint8_t componentId;
    std::uint8_t componentSamplingFactor;
    std::uint8_t componentQuantTable;

    for(jpeg::jpegInformation::tChannelsMap::const_iterator channelsIterator = information.m_channelsMap.begin(); channelsIterator != information.m_channelsMap.end(); ++channelsIterator)
    {
        ptrChannel pChannel=channelsIterator->second;

        componentId = channelsIterator->first;
        componentSamplingFactor = (std::uint8_t)( ((pChannel->m_samplingFactorX & 0xf) << 4) | (pChannel->m_samplingFactorY & 0xf) );
        componentQuantTable = (std::uint8_t)pChannel->m_quantTable;

        pStream->write((std::uint8_t*)&componentId, 1);
        pStream->write((std::uint8_t*)&componentSamplingFactor, 1);
        pStream->write((std::uint8_t*)&componentQuantTable, 1);
    }

    IMEBRA_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the tag's content
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagSOF::readTag(streamReader* pStream, jpegInformation* pInformation, std::uint8_t tagEntry) const
{
    IMEBRA_FUNCTION_START();

    // tag dedicated stream (throws if we attempt to read past
    //  the tag bytes
    //////////////////////////////////////////////////////////
    const std::uint32_t tagLength = readLength(pStream);
    std::shared_ptr<streamReader> tagReader(pStream->getReader(tagLength));

    pInformation->m_bLossless = (tagEntry==0xc3) || (tagEntry==0xc7);
    pInformation->m_process = (std::uint8_t)(tagEntry - 0xc0);

    // Read the precision, in bits
    ///////////////////////////////////////////////////////////
    std::uint8_t precisionBits;
    tagReader->read(&precisionBits, 1);
    pInformation->m_precision = precisionBits;

    // Read the image's size, in pixels
    ///////////////////////////////////////////////////////////
    std::uint16_t imageWidth, imageHeight;
    tagReader->read((std::uint8_t*)&imageHeight, 2);
    tagReader->read((std::uint8_t*)&imageWidth, 2);
    tagReader->adjustEndian((std::uint8_t*)&imageHeight, 2, streamController::highByteEndian);
    tagReader->adjustEndian((std::uint8_t*)&imageWidth, 2, streamController::highByteEndian);

    if(
            precisionBits < 8 ||
            precisionBits > 16 ||
            imageWidth > codecFactory::getCodecFactory()->getMaximumImageWidth() ||
            imageHeight > codecFactory::getCodecFactory()->getMaximumImageHeight())
    {
        IMEBRA_THROW(CodecImageTooBigError, "The factory settings prevented the loading of this image. Consider using codecFactory::setMaximumImageSize() to modify the settings");
    }

    pInformation->m_imageWidth = imageWidth;
    pInformation->m_imageHeight = imageHeight;

    // Read the components number
    ///////////////////////////////////////////////////////////
    pInformation->eraseChannels();
    std::uint8_t componentsNumber;
    tagReader->read(&componentsNumber, 1);

    // Get all the components specifications
    ///////////////////////////////////////////////////////////
    std::uint8_t componentId;
    std::uint8_t componentSamplingFactor;
    std::uint8_t componentQuantTable;
    for(std::uint8_t scanComponents(0); scanComponents<componentsNumber; ++scanComponents)
    {
        tagReader->read(&componentId, 1);
        tagReader->read(&componentSamplingFactor, 1);
        tagReader->read(&componentQuantTable, 1);

        ptrChannel pChannel(std::make_shared<jpeg::jpegChannel>());
        pChannel->m_quantTable = (int)componentQuantTable;
        if(pChannel->m_quantTable >= 16)
        {
            IMEBRA_THROW(CodecCorruptedFileError, "Corrupted quantization table index in SOF tag");
        }
        pChannel->m_samplingFactorX = componentSamplingFactor >> 4;
        pChannel->m_samplingFactorY = componentSamplingFactor & 0x0f;
        if(
                (pChannel->m_samplingFactorX != 1 &&
                 pChannel->m_samplingFactorX != 2 &&
                 pChannel->m_samplingFactorX != 4) ||
                (pChannel->m_samplingFactorY != 1 &&
                 pChannel->m_samplingFactorY != 2 &&
                 pChannel->m_samplingFactorY != 4)
                )
        {
            IMEBRA_THROW(CodecCorruptedFileError, "Wrong sampling factor in SOF tag");
        }
        pInformation->m_channelsMap[componentId] = pChannel;
    }

    // Recalculate the MCUs' attributes
    ///////////////////////////////////////////////////////////
    pInformation->allocChannels();

    IMEBRA_FUNCTION_END_MODIFY(StreamEOFError, CodecCorruptedFileError);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagDHT
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
// Write the DHT entry
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagDHT::writeTag(streamWriter* pStream, jpegInformation& information) const
{
    IMEBRA_FUNCTION_START();

    // Used to write bytes into the stream
    /////////////////////////////////////////////////////////////////
    std::uint8_t byte;

    // There are two phases:
    //  the first phase calculates the tag's length, the second one
    //  write the tables' definitions
    /////////////////////////////////////////////////////////////////
    std::uint16_t tagLength = 0;
    for(int phase = 0; phase < 2; ++phase)
    {
        // Write the tag's length
        /////////////////////////////////////////////////////////////////
        if(phase == 1)
            writeLength(pStream, tagLength);

        // Scan all the used tables
        /////////////////////////////////////////////////////////////////
        for(int tableNum = 0; tableNum < 16; ++tableNum)
        {
            // Scan for DC and AC tables
            /////////////////////////////////////////////////////////////////
            for(int DcAc = 0; DcAc < 2; ++DcAc)
            {
                // bAdd is true if the huffman table is used by a channel
                /////////////////////////////////////////////////////////////////
                bool bAdd=false;

                for(jpeg::jpegInformation::tChannelsMap::const_iterator channelsIterator = information.m_channelsMap.begin(); !bAdd && channelsIterator != information.m_channelsMap.end(); ++channelsIterator)
                {
                    ptrChannel pChannel=channelsIterator->second;
                    bAdd= DcAc==0 ? (tableNum == pChannel->m_huffmanTableDC) : (tableNum == pChannel->m_huffmanTableAC);
                }

                // If the table is used by at least one channel, then write
                //  its definition
                /////////////////////////////////////////////////////////////////
                if(!bAdd)
                {
                    continue;
                }
                std::shared_ptr<huffmanTable> pHuffman;

                if(DcAc==0)
                {
                    pHuffman = information.m_pHuffmanTableDC[tableNum];
                }
                else
                {
                    pHuffman = information.m_pHuffmanTableAC[tableNum];
                }

                // Calculate the tag's length
                /////////////////////////////////////////////////////////////////
                if(phase == 0)
                {
                    pHuffman->incValueFreq(0x100);
                    pHuffman->calcHuffmanCodesLength(16);
                    // Remove the value 0x100 now
                    pHuffman->removeLastCode();

                    pHuffman->calcHuffmanTables();
                    tagLength = (std::uint16_t)(tagLength + 17);
                    for(int scanLength = 0; scanLength < 16;)
                    {
                        tagLength = (std::uint16_t)(tagLength + (pHuffman->getValuesPerLength(++scanLength)));
                    }
                    continue;
                }

                // Write the huffman table
                /////////////////////////////////////////////////////////////////

                // Write the table ID
                /////////////////////////////////////////////////////////////////
                std::uint8_t tableID=(std::uint8_t)((DcAc<<4) | tableNum);
                pStream->write(&tableID, 1);

                // Write the values per length.
                /////////////////////////////////////////////////////////////////
                int scanLength;
                for(scanLength=0; scanLength<16;)
                {
                    byte = (std::uint8_t)(pHuffman->getValuesPerLength(++scanLength));
                    pStream->write(&byte, 1);
                }

                // Write the table values
                /////////////////////////////////////////////////////////////////
                std::uint32_t valueIndex = 0;
                for(scanLength = 0; scanLength < 16; ++scanLength)
                {
                    for(std::uint32_t scanValues = 0; scanValues < pHuffman->getValuesPerLength(scanLength+1); ++scanValues)
                    {
                        byte = (std::uint8_t)(pHuffman->getOrderedValue(valueIndex++));
                        pStream->write(&byte, 1);
                    }
                }
            } // DcAc
        } // tableNum
    } // phase

    IMEBRA_FUNCTION_END();
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the DHT entry
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagDHT::readTag(streamReader* pStream, jpegInformation* pInformation, std::uint8_t /* tagEntry */) const
{
    IMEBRA_FUNCTION_START();

    // tag dedicated stream (throws if we attempt to read past
    //  the tag bytes)
    //////////////////////////////////////////////////////////
    const std::uint32_t tagLength = readLength(pStream);
    std::shared_ptr<streamReader> tagReader(pStream->getReader(tagLength));

    // Used to read bytes from the stream
    /////////////////////////////////////////////////////////////////
    std::uint8_t byte;

    try
    {
        // Read all the defined tables
        /////////////////////////////////////////////////////////////////
        while(!tagReader->endReached())
        {
            // Read the table's ID
            /////////////////////////////////////////////////////////////////
            tagReader->read(&byte, 1);

            // Get a pointer to the right table
            /////////////////////////////////////////////////////////////////
            std::shared_ptr<huffmanTable> pHuffman;
            if((byte & 0xf0) == 0)
                pHuffman = pInformation->m_pHuffmanTableDC[byte & 0xf];
            else
                pHuffman = pInformation->m_pHuffmanTableAC[byte & 0xf];

            // Reset the table
            /////////////////////////////////////////////////////////////////
            pHuffman->reset();

            // Read the number of codes per length
            /////////////////////////////////////////////////////////////////
            for(std::uint32_t scanLength=0; scanLength != 16; )
            {
                tagReader->read(&byte, 1);
                pHuffman->setValuesPerLength(++scanLength, (std::uint32_t)byte);
            }

            // Used to store the values into the table
            /////////////////////////////////////////////////////////////////
            std::uint32_t valueIndex = 0;

            // Read all the values and store them into the huffman table
            /////////////////////////////////////////////////////////////////
            for(std::uint32_t scanLength = 0; scanLength != 16; ++scanLength)
            {
                for(std::uint32_t scanValues = 0; scanValues != pHuffman->getValuesPerLength(scanLength + 1); ++scanValues)
                {
                    tagReader->read(&byte, 1);
                    pHuffman->addOrderedValue(valueIndex++, (std::uint32_t)byte);
                }
            }

            // Calculate the huffman tables
            /////////////////////////////////////////////////////////////////
            pHuffman->calcHuffmanTables();
        }
    }
    catch(const HuffmanCreateTableError& e)
    {
        IMEBRA_THROW(CodecCorruptedFileError, e.what());
    }

    IMEBRA_FUNCTION_END_MODIFY(StreamEOFError, CodecCorruptedFileError);

}




/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagSOS
//
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write the SOS entry
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagSOS::writeTag(streamWriter* pStream, jpegInformation& information) const
{
    IMEBRA_FUNCTION_START();

    // Calculate the components number
    /////////////////////////////////////////////////////////////////
    std::uint8_t componentsNumber(0);
    while(information.m_channelsList[componentsNumber] != 0)
    {
        ++componentsNumber;
    }

    // Write the tag's length
    /////////////////////////////////////////////////////////////////
    writeLength(pStream, (std::uint16_t)(4 + 2 * componentsNumber));

    // Write the component's number
    /////////////////////////////////////////////////////////////////
    pStream->write(&componentsNumber, 1);

    // Scan all the channels in the current scan
    /////////////////////////////////////////////////////////////////
    jpeg::jpegChannel* pChannel; // used in the loop
    for(jpeg::jpegChannel** listIterator = information.m_channelsList; *listIterator != 0; ++listIterator)
    {
        pChannel = *listIterator;

        std::uint8_t channelId(0);

        pChannel->m_lastDCValue = pChannel->m_defaultDCValue;

        // Find the channel's ID
        /////////////////////////////////////////////////////////////////
        for(jpeg::jpegInformation::tChannelsMap::const_iterator mapIterator = information.m_channelsMap.begin(); mapIterator != information.m_channelsMap.end(); ++mapIterator)
        {
            if(mapIterator->second.get() == pChannel)
            {
                channelId=mapIterator->first;
                break;
            }
        }

        // Write the channel's ID
        /////////////////////////////////////////////////////////////////
        pStream->write(&channelId, 1);

        // Write the ac/dc tables ID
        /////////////////////////////////////////////////////////////////
        std::uint8_t acdc = (std::uint8_t)(((pChannel->m_huffmanTableDC & 0xf)<<4) | (pChannel->m_huffmanTableAC & 0xf));

        pStream->write(&acdc, 1);
    }

    std::uint8_t byte;

    // Write the spectral index start
    /////////////////////////////////////////////////////////////////
    byte=(std::uint8_t)information.m_spectralIndexStart;
    pStream->write(&byte, 1);

    // Write the spectral index end
    /////////////////////////////////////////////////////////////////
    byte=(std::uint8_t)information.m_spectralIndexEnd;
    pStream->write(&byte, 1);

    // Write the hi/lo bit
    /////////////////////////////////////////////////////////////////
    byte = 0;
    pStream->write(&byte, 1);

    IMEBRA_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the SOS entry
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagSOS::readTag(streamReader* pStream, jpegInformation* pInformation, std::uint8_t /* tagEntry */) const
{
    IMEBRA_FUNCTION_START();

    // tag dedicated stream (throws if we attempt to read past
    //  the tag bytes)
    //////////////////////////////////////////////////////////
    const std::uint32_t tagLength = readLength(pStream);
    std::shared_ptr<streamReader> tagReader(pStream->getReader(tagLength));

    pInformation->m_eobRun = 0;
    memset(pInformation->m_channelsList, 0, sizeof(pInformation->m_channelsList));

    std::uint8_t componentsNumber;
    tagReader->read(&componentsNumber, 1);

    std::uint8_t byte;
    for(std::uint8_t scanComponents = 0; scanComponents != componentsNumber; ++scanComponents)
    {
        tagReader->read(&byte, 1);

        jpeg::jpegInformation::tChannelsMap::const_iterator findChannel = pInformation->m_channelsMap.find(byte);
        if(findChannel == pInformation->m_channelsMap.end())
        {
            IMEBRA_THROW(CodecCorruptedFileError, "Corrupted SOS tag found");
        }
        ptrChannel pChannel = findChannel->second;

        pChannel->processUnprocessedAmplitudes();

        tagReader->read(&byte, 1);

        pChannel->m_huffmanTableDC=byte>>4;
        pChannel->m_huffmanTableAC=byte & 0xf;
        pChannel->m_pActiveHuffmanTableDC = pInformation->m_pHuffmanTableDC[pChannel->m_huffmanTableDC].get();
        pChannel->m_pActiveHuffmanTableAC = pInformation->m_pHuffmanTableAC[pChannel->m_huffmanTableAC].get();

        pChannel->m_lastDCValue = pChannel->m_defaultDCValue;

        pInformation->m_channelsList[scanComponents] = pChannel.get();

    }

    tagReader->read(&byte, 1);
    pInformation->m_spectralIndexStart = byte;

    tagReader->read(&byte, 1);
    pInformation->m_spectralIndexEnd = byte;

    tagReader->read(&byte, 1);
    if(byte != 0)
    {
        IMEBRA_THROW(CodecCorruptedFileError, "Progressive JPEG not supported");
    }

    pInformation->findMcuSize();

    IMEBRA_FUNCTION_END_MODIFY(StreamEOFError, CodecCorruptedFileError);

}



/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagDQT
//
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write the DQT tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagDQT::writeTag(streamWriter* pStream, jpegInformation& information) const
{
    IMEBRA_FUNCTION_START();

    // Read the tag's length
    /////////////////////////////////////////////////////////////////
    std::uint16_t tagLength = 0;

    std::uint8_t  tablePrecision;
    std::uint8_t  tableValue8;
    std::uint16_t tableValue16;

    for(int phase = 0; phase < 2; ++phase)
    {
        if(phase != 0)
        {
            writeLength(pStream, tagLength);
        }
        for(std::uint8_t tableId = 0; tableId < 16; ++tableId)
        {
            // bAdd is true if the huffman table is used by a channel
            /////////////////////////////////////////////////////////////////
            bool bAdd=false;

            for(jpeg::jpegInformation::tChannelsMap::iterator channelsIterator = information.m_channelsMap.begin(); !bAdd && channelsIterator != information.m_channelsMap.end(); ++channelsIterator)
            {
                ptrChannel pChannel=channelsIterator->second;
                bAdd=pChannel->m_quantTable==tableId;
            }

            if(!bAdd)
            {
                continue;
            }
            // Calculate the table's precision
            bool b16Bits = information.m_precision > 8;
            for(int tableIndex = 0; !b16Bits && (tableIndex < 64); ++tableIndex)
            {
                if(information.m_quantizationTable[tableId][tableIndex] >= 256)
                {
                    b16Bits=true;
                }
            }

            if(phase == 0)
            {
                tagLength = (std::uint16_t)(tagLength + 1 + (b16Bits ? 128 : 64));
            }
            else
            {
                tablePrecision = (std::uint8_t)(tableId | (b16Bits ? 0x10 : 0));
                pStream->write(&tablePrecision, 1);
                if(b16Bits)
                {
                    for(int tableIndex = 0; tableIndex < 64; ++tableIndex)
                    {
                        tableValue16 = (std::uint16_t)information.m_quantizationTable[tableId][JpegDeZigZagOrder[tableIndex]];
                        pStream->adjustEndian((std::uint8_t*)&tableValue16, 2, streamController::highByteEndian);
                        pStream->write((std::uint8_t*)&tableValue16, 2);
                    }
                }
                else
                {
                    for(int tableIndex = 0; tableIndex < 64; ++tableIndex)
                    {
                        tableValue8=(std::uint8_t)information.m_quantizationTable[tableId][JpegDeZigZagOrder[tableIndex]];
                        pStream->write(&tableValue8, 1);
                    }
                }
            }

            information.recalculateQuantizationTables(tableId);
        }
    }

    IMEBRA_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the DQT tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagDQT::readTag(streamReader* pStream, jpegInformation* pInformation, std::uint8_t /* tagEntry */) const
{
    IMEBRA_FUNCTION_START();

    // tag dedicated stream (throws if we attempt to read past
    //  the tag bytes)
    //////////////////////////////////////////////////////////
    const std::uint32_t tagLength = readLength(pStream);
    std::shared_ptr<streamReader> tagReader(pStream->getReader(tagLength));

    std::uint8_t  tablePrecision;
    std::uint8_t  tableValue8;
    std::uint16_t tableValue16;
    while(!tagReader->endReached())
    {
        tagReader->read(&tablePrecision, 1);

        // Read a DQT table
        /////////////////////////////////////////////////////////////////
        for(int tableIndex = 0; tableIndex < 64; ++tableIndex)
        {
            // 16 bits precision
            /////////////////////////////////////////////////////////////////
            if((tablePrecision & 0xf0) != 0)
            {
                tagReader->read((std::uint8_t*)&tableValue16, 2);
                tagReader->adjustEndian((std::uint8_t*)&tableValue16, 2, streamController::highByteEndian);
                pInformation->m_quantizationTable[tablePrecision & 0x0f][JpegDeZigZagOrder[tableIndex]]=tableValue16;
            }

            // 8 bits precision
            /////////////////////////////////////////////////////////////////
            else
            {
                tagReader->read(&tableValue8, 1);
                pInformation->m_quantizationTable[tablePrecision & 0x0f][JpegDeZigZagOrder[tableIndex]]=tableValue8;
            }

        } // ----- End of table reading

        pInformation->recalculateQuantizationTables(tablePrecision & 0x0f);
    }

    IMEBRA_FUNCTION_END_MODIFY(StreamEOFError, CodecCorruptedFileError);

}



/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagDRI
//
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write the DRI tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagDRI::writeTag(streamWriter* pStream, jpegInformation& information) const
{
    IMEBRA_FUNCTION_START();

    // Write the tag's length
    /////////////////////////////////////////////////////////////////
    writeLength(pStream, 2);

    // Write the MCU per restart interval
    /////////////////////////////////////////////////////////////////
    std::uint16_t unitsPerRestartInterval = information.m_mcuPerRestartInterval;
    pStream->adjustEndian((std::uint8_t*)&unitsPerRestartInterval, 2, streamController::highByteEndian);
    pStream->write((std::uint8_t*)&unitsPerRestartInterval, 2);

    IMEBRA_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the DRI tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagDRI::readTag(streamReader* pStream, jpegInformation* pInformation, std::uint8_t /* tagEntry */) const
{
    IMEBRA_FUNCTION_START();

    // tag dedicated stream (throws if we attempt to read past
    //  the tag bytes)
    //////////////////////////////////////////////////////////
    const std::uint32_t tagLength = readLength(pStream);
    std::shared_ptr<streamReader> tagReader(pStream->getReader(tagLength));

    std::uint16_t unitsPerRestartInterval;
    tagReader->read((std::uint8_t*)&unitsPerRestartInterval, 2);
    tagReader->adjustEndian((std::uint8_t*)&unitsPerRestartInterval, 2, streamController::highByteEndian);
    pInformation->m_mcuPerRestartInterval=unitsPerRestartInterval;

    IMEBRA_FUNCTION_END_MODIFY(StreamEOFError, CodecCorruptedFileError);

}



/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagRST
//
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write the RST tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagRST::writeTag(streamWriter* /* pStream */, jpegInformation& /* information */) const
{
    return;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the RST tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagRST::readTag(streamReader* /* pStream */, jpegInformation* pInformation, std::uint8_t tagEntry) const
{
    IMEBRA_FUNCTION_START();

    // Reset the channels last dc value
    /////////////////////////////////////////////////////////////////
    for(jpeg::jpegChannel** channelsIterator = pInformation->m_channelsList; *channelsIterator != 0; ++channelsIterator)
    {
        (*channelsIterator)->processUnprocessedAmplitudes();
        (*channelsIterator)->m_lastDCValue = (*channelsIterator)->m_defaultDCValue;
    }

    // Calculate the mcu processed counter
    /////////////////////////////////////////////////////////////////
    if(pInformation->m_mcuPerRestartInterval > 0)
    {
        std::uint32_t doneRestartInterval=(pInformation->m_mcuProcessed + pInformation->m_mcuPerRestartInterval-1) / pInformation->m_mcuPerRestartInterval-1;
        std::uint8_t doneRestartIntervalID=(std::uint8_t)(doneRestartInterval & 0x7);
        std::uint8_t foundRestartIntervalID=tagEntry & 0x7;
        if(foundRestartIntervalID<doneRestartIntervalID)
        doneRestartInterval += 8;
        doneRestartInterval -= doneRestartIntervalID;
        doneRestartInterval += foundRestartIntervalID;
        pInformation->m_mcuProcessed=(doneRestartInterval+1) * pInformation->m_mcuPerRestartInterval;
        pInformation->m_mcuProcessedY = pInformation->m_mcuProcessed / pInformation->m_mcuNumberX;
        pInformation->m_mcuProcessedX = pInformation->m_mcuProcessed - (pInformation->m_mcuProcessedY * pInformation->m_mcuNumberX);
        pInformation->m_mcuLastRestart = pInformation->m_mcuProcessed;

        // Update the lossless pixel's counter in the channels
        /////////////////////////////////////////////////////////////////
        for(jpeg::jpegChannel** channelsIterator = pInformation->m_channelsList; *channelsIterator != 0; ++channelsIterator)
        {
            jpeg::jpegChannel* pChannel(*channelsIterator);
            pChannel->m_losslessPositionX = pInformation->m_mcuProcessedX / pChannel->m_blockMcuX;
            pChannel->m_losslessPositionY = pInformation->m_mcuProcessedY / pChannel->m_blockMcuY;
        }
    }

    pInformation->m_eobRun = 0;

    IMEBRA_FUNCTION_END();
}



/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
//
// jpegCodecTagEOI
//
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Write the EOI tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagEOI::writeTag(streamWriter* pStream, jpegInformation& /* information */) const
{
    IMEBRA_FUNCTION_START();

    writeLength(pStream, 0);

    IMEBRA_FUNCTION_END();
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
//
//
// Read the EOI tag
//
//
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void tagEOI::readTag(streamReader* /* pStream */, jpegInformation* pInformation, std::uint8_t /* tagEntry */) const
{
    pInformation->m_bEndOfImage=true;
}

} // namespace jpeg

} // namespace codecs

} // namespace implementation

} // namespace imebra



