/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dicomImageCodec.h
    \brief Declaration of the class dicomImageCodec.

*/

#if !defined(imebraDicomImageCodec_CC44A2C5_2B8C_42c1_9704_3F9C582643B9__INCLUDED_)
#define imebraDicomImageCodec_CC44A2C5_2B8C_42c1_9704_3F9C582643B9__INCLUDED_

#include "imageCodecImpl.h"
#include "dataImpl.h"
#include "dataSetImpl.h"
#include "streamControllerImpl.h"

/// \def IMEBRA_DATASET_MAX_DEPTH
/// \brief Max number of datasets embedded into each 
///        others.
///
/// This value is used to prevent a stack when reading a
///  malformed dataset that has too many datasets embedded
///   into each others.
///
///////////////////////////////////////////////////////////
#define IMEBRA_DATASET_MAX_DEPTH 16 


namespace imebra
{

namespace implementation
{

namespace codecs
{

/// \addtogroup group_codecs
///
/// @{

struct dicomInformation
{
    std::uint32_t m_ioDWord;
    std::uint16_t m_ioWord;
    std::uint8_t  m_ioByte;

    typedef std::shared_ptr<channel> ptrChannel;
    std::vector<ptrChannel> m_channels;
};

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief The Dicom codec.
///
/// This class is used to decode and encode a DICOM image.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class dicomImageCodec : public imageCodec
{
public:
	// Get an image from a dicom structure
	///////////////////////////////////////////////////////////
    virtual std::shared_ptr<image> getImage(const dataSet& dataset, std::shared_ptr<streamReader> pSourceStream, tagVR_t dataType) const;

	// Write an image into a dicom structure
	///////////////////////////////////////////////////////////
	virtual void setImage(
		std::shared_ptr<streamWriter> pDestStream,
		std::shared_ptr<image> pImage,
        const std::string& transferSyntax,
        imageQuality_t imageQuality,
        tagVR_t dataType,
        std::uint32_t allocatedBits,
		bool bSubSampledX,
		bool bSubSampledY,
		bool bInterleaved,
        bool b2Complement) const;

	// Returns true if the codec can handle the transfer
	//  syntax
	///////////////////////////////////////////////////////////
    virtual bool canHandleTransferSyntax(const std::string& transferSyntax) const;

	// Returns true if the transfer syntax has to be
	//  encapsulated
	//
	///////////////////////////////////////////////////////////
    virtual bool encapsulated(const std::string& transferSyntax) const;

	// Returns the suggested allocated bits
	///////////////////////////////////////////////////////////
    virtual std::uint32_t suggestAllocatedBits(const std::string& transferSyntax, std::uint32_t highBit) const;

protected:
	// Read an uncompressed interleaved image
	///////////////////////////////////////////////////////////
    static void readUncompressedInterleaved(
            dicomInformation& information,
            std::uint32_t channelsNumber,
            bool bSubSampledX,
            bool bSubSampledY,
            streamReader* pSourceStream,
            std::uint32_t wordSizeBytes,
            std::uint8_t allocatedBits,
            std::uint32_t mask
            );

	// Write an uncompressed interleaved image
	///////////////////////////////////////////////////////////
    static void writeUncompressedInterleaved(
            dicomInformation& information,
            std::uint32_t channelsNumber,
            bool bSubSampledX,
            bool bSubSampledY,
            streamWriter* pDestStream,
            std::uint32_t wordSizeBytes,
            std::uint8_t allocatedBits,
            std::uint32_t mask
            );

	// Read an uncompressed not interleaved image
	///////////////////////////////////////////////////////////
    static void readUncompressedNotInterleaved(
            dicomInformation& information,
            std::uint32_t channelsNumber,
            streamReader* pSourceStream,
            std::uint32_t wordSizeBytes,
            std::uint8_t allocatedBits,
            std::uint32_t mask
            );

	// Write an uncompressed not interleaved image
	///////////////////////////////////////////////////////////
    static void writeUncompressedNotInterleaved(
            dicomInformation& information,
            std::uint32_t channelsNumber,
            streamWriter* pDestStream,
            std::uint32_t wordSizeBytes,
            std::uint8_t allocatedBits,
            std::uint32_t mask
            );

	// Write an RLE compressed image
	///////////////////////////////////////////////////////////
    static void writeRLECompressed(
            dicomInformation& information,
            std::uint32_t imageWidth,
            std::uint32_t imageHeight,
            std::uint32_t channelsNumber,
            streamWriter* pDestStream,
            std::uint8_t allocatedBits,
            std::uint32_t mask
            );

    // Write RLE sequence of different bytes
    ///////////////////////////////////////////////////////////
    static size_t writeRLEDifferentBytes(std::vector<std::uint8_t>* pDifferentBytes, streamWriter* pDestStream, bool bWrite);

	// Read an RLE compressed image
	///////////////////////////////////////////////////////////
    static void readRLECompressed(
            dicomInformation& information,
            std::uint32_t imageWidth,
            std::uint32_t imageHeight,
            std::uint32_t channelsNumber,
            streamReader* pSourceStream,
            std::uint8_t allocatedBits,
            std::uint32_t mask);


	// Read a single pixel of a RAW dicom image
	///////////////////////////////////////////////////////////
    static void readPixel(
                    dicomInformation& information,
					streamReader* pSourceStream,
					std::int32_t* pDest,
					std::uint32_t numPixels,
					std::uint8_t* bitPointer,
					std::uint8_t* pReadBuffer,
                    std::uint32_t wordSizeBytes,
                    std::uint8_t allocatedBits,
                    std::uint32_t mask);

	// Write a single pixel of a RAW dicom image
	///////////////////////////////////////////////////////////
    static void writePixel(
                    dicomInformation& information,
					streamWriter* pDestStream,
					std::int32_t pixelValue,
					std::uint8_t*  pBitPointer,
                    std::uint32_t wordSizeBytes,
                    std::uint8_t allocatedBits,
                    std::uint32_t mask);

	// Flush the unwritten bytes of an uncompressed image
	///////////////////////////////////////////////////////////
    static void flushUnwrittenPixels(dicomInformation& information, streamWriter* pDestStream, std::uint8_t* pBitPointer, std::uint32_t wordSizeBytes);

    static void allocChannels(dicomInformation& information, std::uint32_t channelsNumber, std::uint32_t width, std::uint32_t height, bool bSubSampledX, bool bSubSampledY);
};


/// @}

} // namespace codecs

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraDicomImageCodec_CC44A2C5_2B8C_42c1_9704_3F9C582643B9__INCLUDED_)
