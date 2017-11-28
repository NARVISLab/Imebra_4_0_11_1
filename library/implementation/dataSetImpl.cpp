/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataSet.cpp
    \brief Implementation of the class dataSet.

*/

#include "exceptionImpl.h"
#include "streamReaderImpl.h"
#include "streamWriterImpl.h"
#include "memoryStreamImpl.h"
#include "dataSetImpl.h"
#include "dataHandlerNumericImpl.h"
#include "dicomDictImpl.h"
#include "codecFactoryImpl.h"
#include "streamCodecImpl.h"
#include "imageCodecImpl.h"
#include "imageImpl.h"
#include "LUTImpl.h"
#include "colorTransformsFactoryImpl.h"
#include "transformsChainImpl.h"
#include "transformHighBitImpl.h"
#include "modalityVOILUTImpl.h"
#include "bufferImpl.h"
#include <iostream>
#include <string.h>


namespace imebra
{

namespace implementation
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// dataSet
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

dataSet::dataSet(): m_itemOffset(0)
{
}

dataSet::dataSet(const std::string& transferSyntax): m_itemOffset(0)
{
    setString(0x0002, 0x0, 0x0010, 0, transferSyntax);
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the requested tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<data> dataSet::getTag(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId) const
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    tGroups::const_iterator findGroup(m_groups.find(groupId));
    if(findGroup == m_groups.end())
    {
        IMEBRA_THROW(MissingGroupError, "The requested group is missing");
    }

    if(findGroup->second.size() <= order)
    {
        IMEBRA_THROW(MissingGroupError, "The requested group is missing");
    }

    const tTags& tagsMap = findGroup->second.at(order);
    tTags::const_iterator findTag(tagsMap.find(tagId));
    if(findTag == tagsMap.end())
    {
        IMEBRA_THROW(MissingTagError, "The requested tag is missing");
    }
    return findTag->second;

	IMEBRA_FUNCTION_END();
}


std::shared_ptr<data> dataSet::getTagCreate(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    if(m_groups[groupId].size() <= order)
    {
        m_groups[groupId].resize(order + 1);
    }

    if(m_groups[groupId][order][tagId] == 0)
    {
        m_groups[groupId][order][tagId] = std::make_shared<data>(tagVR, m_charsetsList);
    }

    return m_groups[groupId][order][tagId];

    IMEBRA_FUNCTION_END();
}

std::shared_ptr<data> dataSet::getTagCreate(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId)
{
    IMEBRA_FUNCTION_START();

    return getTagCreate(groupId, order, tagId, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}

bool dataSet::bufferExists(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        return getTag(groupId, order, tagId)->bufferExists(bufferId);
    }
    catch(const MissingDataElementError&)
    {
        return false;
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the image from the structure
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<image> dataSet::getImage(std::uint32_t frameNumber) const
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

	// Retrieve the transfer syntax
	///////////////////////////////////////////////////////////
    std::string transferSyntax = getString(0x0002, 0x0, 0x0010, 0, 0, "1.2.840.10008.1.2");

	// Get the right codec
	///////////////////////////////////////////////////////////
    std::shared_ptr<const codecs::imageCodec> pCodec(codecs::codecFactory::getCodecFactory()->getImageCodec(transferSyntax));

    try
    {
        std::shared_ptr<implementation::data> imageTag = getTag(0x7fe0, 0x0, 0x0010);

        const tagVR_t imageStreamDataType(imageTag->getDataType());

        // Get the number of frames
        ///////////////////////////////////////////////////////////
        std::uint32_t numberOfFrames = getUnsignedLong(0x0028, 0, 0x0008, 0, 0, 1);

        if(frameNumber >= numberOfFrames)
        {
            IMEBRA_THROW(DataSetImageDoesntExistError, "The requested image doesn't exist");
        }

        // Placeholder for the stream containing the image
        ///////////////////////////////////////////////////////////
        std::shared_ptr<streamReader> imageStream;

        // Retrieve the second item in the image's tag.
        // If the second item is present, then a multiframe
        //  image is present.
        ///////////////////////////////////////////////////////////
        bool bDontNeedImagesPositions = false;
        {
            if(imageTag->bufferExists(1))
            {
                std::uint32_t firstBufferId(0), endBufferId(0);
                size_t totalLength(0);
                if(imageTag->getBufferSize(0) == 0 && numberOfFrames + 1 == imageTag->getBuffersCount())
                {
                    firstBufferId = frameNumber + 1;
                    endBufferId = firstBufferId + 1;
                    totalLength = imageTag->getBufferSize(firstBufferId);
                }
                else
                {
                    totalLength = getFrameBufferIds(frameNumber, &firstBufferId, &endBufferId);
                }
                if(firstBufferId == endBufferId - 1)
                {
                    imageStream = imageTag->getStreamReader(firstBufferId);
                }
                else
                {
                    std::shared_ptr<memory> temporaryMemory(std::make_shared<memory>(totalLength));
                    const std::uint8_t* pDest = temporaryMemory->data();
                    for(std::uint32_t scanBuffers = firstBufferId; scanBuffers != endBufferId; ++scanBuffers)
                    {
                        std::shared_ptr<handlers::readingDataHandlerRaw> bufferHandler = imageTag->getReadingDataHandlerRaw(scanBuffers);
                        const std::uint8_t* pSource = bufferHandler->getMemoryBuffer();
                        ::memcpy((void*)pDest, (void*)pSource, bufferHandler->getSize());
                        pDest += bufferHandler->getSize();
                    }
                    std::shared_ptr<baseStreamInput> compositeStream(std::make_shared<memoryStreamInput>(temporaryMemory));
                    imageStream = std::make_shared<streamReader>(compositeStream);
                }
                bDontNeedImagesPositions = true;
            }
        }

        // If the image cannot be found, then probably we are
        //  handling an old dicom format.
        // Then try to read the image from the next group with
        //  id=0x7fe
        ///////////////////////////////////////////////////////////
        if(imageStream == 0)
        {
            try
            {
                imageStream = getStreamReader(0x7fe0, (std::uint16_t)frameNumber, 0x0010, 0x0);
                bDontNeedImagesPositions = true;
            }
            catch(const MissingDataElementError&)
            {
                // Nothing to do
            }
        }

        // We are dealing with an old dicom format that doesn't
        //  include the image offsets and stores all the images
        //  in one buffer
        ///////////////////////////////////////////////////////////
        if(imageStream == 0)
        {
            imageStream = imageTag->getStreamReader(0x0);

            // Reset an internal array that keeps track of the
            //  images position
            ///////////////////////////////////////////////////////////
            if(m_imagesPositions.size() != numberOfFrames)
            {
                m_imagesPositions.resize(numberOfFrames);

                for(std::uint32_t resetImagesPositions = 0; resetImagesPositions != numberOfFrames; ++resetImagesPositions)
                {
                    m_imagesPositions[resetImagesPositions] = 0;
                }

            }

            // Read all the images before the desidered one so we set
            //  reading position in the stream
            ///////////////////////////////////////////////////////////
            for(std::uint32_t readImages = 0; readImages < frameNumber; readImages++)
            {
                size_t offsetPosition = m_imagesPositions[readImages];
                if(offsetPosition == 0)
                {
                    pCodec->getImage(*this, imageStream, imageStreamDataType);
                    m_imagesPositions[readImages] = imageStream->position();
                    continue;
                }
                if((m_imagesPositions[readImages + 1] == 0) || (readImages == (frameNumber - 1)))
                {
                    imageStream->seek(offsetPosition);
                }
            }
        }

        double pixelDistanceX = getDouble(0x0028, 0x0, 0x0030, 0, 0, 1);
        double pixelDistanceY = getDouble(0x0028, 0x0, 0x0030, 0, 1, 1);

        std::shared_ptr<image> pImage;
        pImage = pCodec->getImage(*this, imageStream, imageStreamDataType);

        if(!bDontNeedImagesPositions && m_imagesPositions.size() > frameNumber)
        {
            m_imagesPositions[frameNumber] = imageStream->position();
        }

        // If the image has been returned correctly, then set
        //  the image's size
        ///////////////////////////////////////////////////////////
        if(pImage != 0)
        {
            std::uint32_t width, height;
            pImage->getSize(&width, &height);
            pImage->setSizeMm(pixelDistanceX*(double)width, pixelDistanceY*(double)height);
        }

        if(pImage->getColorSpace() == "PALETTE COLOR")
        {
            std::shared_ptr<lut> red(std::make_shared<lut>(getReadingDataHandlerNumeric(0x0028, 0x0, 0x1101, 0), getReadingDataHandlerNumeric(0x0028, 0x0, 0x1201, 0), L"", pImage->isSigned()));
            std::shared_ptr<lut> green(std::make_shared<lut>(getReadingDataHandlerNumeric(0x0028, 0x0, 0x1102, 0), getReadingDataHandlerNumeric(0x0028, 0x0, 0x1202, 0), L"", pImage->isSigned()));
            std::shared_ptr<lut> blue(std::make_shared<lut>(getReadingDataHandlerNumeric(0x0028, 0x0, 0x1103, 0), getReadingDataHandlerNumeric(0x0028, 0x0, 0x1203, 0), L"", pImage->isSigned()));
            std::shared_ptr<palette> imagePalette(std::make_shared<palette>(red, green, blue));
            pImage->setPalette(imagePalette);
        }

        return pImage;
    }
    catch(const MissingDataElementError&)
    {
        IMEBRA_THROW(DataSetImageDoesntExistError, "The requested image doesn't exist");
    }

	IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get an image from the dataset and apply the modality
//  transform.
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<image> dataSet::getModalityImage(std::uint32_t frameNumber) const
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::shared_ptr<image> originalImage = getImage(frameNumber);

    std::shared_ptr<transforms::colorTransforms::colorTransformsFactory> colorFactory(transforms::colorTransforms::colorTransformsFactory::getColorTransformsFactory());
    if(originalImage == 0 || !colorFactory->isMonochrome(originalImage->getColorSpace()))
    {
        return originalImage;
    }

    std::shared_ptr<transforms::modalityVOILUT> modalityVOILUT(std::make_shared<transforms::modalityVOILUT>(std::static_pointer_cast<const dataSet>(shared_from_this())));

    // Convert to MONOCHROME2 if a modality transform is not present
    ////////////////////////////////////////////////////////////////
    if(modalityVOILUT->isEmpty())
    {
        std::shared_ptr<transforms::transform> monochromeColorTransform(colorFactory->getTransform(originalImage->getColorSpace(), "MONOCHROME2"));
        if(monochromeColorTransform != 0)
        {
            std::uint32_t width, height;
            originalImage->getSize(&width, &height);
            std::shared_ptr<image> outputImage = monochromeColorTransform->allocateOutputImage(originalImage->getDepth(),
                                                                                               originalImage->getColorSpace(),
                                                                                               originalImage->getHighBit(),
                                                                                               originalImage->getPalette(),
                                                                                               width, height);
            monochromeColorTransform->runTransform(originalImage, 0, 0, width, height, outputImage, 0, 0);
            return outputImage;
        }

        return originalImage;
    }

    // Apply the modality VOI/LUT transform
    ///////////////////////////////////////
    std::uint32_t width, height;
    originalImage->getSize(&width, &height);
    std::shared_ptr<image> outputImage = modalityVOILUT->allocateOutputImage(originalImage->getDepth(),
                                                                             originalImage->getColorSpace(),
                                                                             originalImage->getHighBit(),
                                                                             originalImage->getPalette(),
                                                                             width, height);
    modalityVOILUT->runTransform(originalImage, 0, 0, width, height, outputImage, 0, 0);
    return outputImage;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Insert an image into the dataset
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setImage(std::uint32_t frameNumber, std::shared_ptr<image> pImage, imageQuality_t quality)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

	// bDontChangeAttributes is true if some images already
	//  exist in the dataset and we must save the new image
	//  using the attributes already stored
	///////////////////////////////////////////////////////////
    std::uint32_t numberOfFrames = getUnsignedLong(0x0028, 0, 0x0008, 0, 0, 0);
	if(frameNumber != numberOfFrames)
	{
        IMEBRA_THROW(DataSetWrongFrameError, "The frames must be inserted in sequence");
	}
	bool bDontChangeAttributes = (numberOfFrames != 0);
    std::string transferSyntax = getString(0x0002, 0x0, 0x0010, 0, 0, "1.2.840.10008.1.2");

	// Select the right codec
	///////////////////////////////////////////////////////////
    std::shared_ptr<const codecs::imageCodec> saveCodec(codecs::codecFactory::getCodecFactory()->getImageCodec(transferSyntax));

	// Do we have to save the basic offset table?
	///////////////////////////////////////////////////////////
    std::uint16_t groupId(0x7fe0), tagId(0x0010); // The tag where the image must be stored
    bool bEncapsulated = saveCodec->encapsulated(transferSyntax) || bufferExists(groupId, 0x0, tagId, 0x1);

	// Set the subsampling flags
	///////////////////////////////////////////////////////////
    bool bSubSampledX = (std::uint32_t)quality > (std::uint32_t)imageQuality_t::high;
    bool bSubSampledY = (std::uint32_t)quality > (std::uint32_t)imageQuality_t::medium;
	if( !transforms::colorTransforms::colorTransformsFactory::canSubsample(pImage->getColorSpace()) )
	{
		bSubSampledX = bSubSampledY = false;
	}

    bool b2complement = pImage->isSigned();
    std::uint32_t channelsNumber = pImage->getChannelsNumber();
    std::uint8_t allocatedBits = (std::uint8_t)(saveCodec->suggestAllocatedBits(transferSyntax, pImage->getHighBit()));
    bool bInterleaved = (getUnsignedLong(0x0028, 0x0, 0x0006, 0, 0, channelsNumber > 1 ? 0 : 1) == 0x0);

	// If the attributes cannot be changed, then check the
	//  attributes already stored in the dataset
	///////////////////////////////////////////////////////////
	if(bDontChangeAttributes)
	{
        std::string currentColorSpace = getString(0x0028, 0x0, 0x0004, 0, 0);
        if(
                transforms::colorTransforms::colorTransformsFactory::normalizeColorSpace(pImage->getColorSpace()) !=
                    transforms::colorTransforms::colorTransformsFactory::normalizeColorSpace(currentColorSpace) ||
                bSubSampledX != transforms::colorTransforms::colorTransformsFactory::isSubsampledX(currentColorSpace) ||
                bSubSampledY != transforms::colorTransforms::colorTransformsFactory::isSubsampledY(currentColorSpace) ||
                b2complement != (getUnsignedLong(0x0028, 0, 0x0103, 0, 0) != 0) ||
                allocatedBits != (std::uint8_t)getUnsignedLong(0x0028, 0x0, 0x0100, 0, 0) ||
                channelsNumber != getUnsignedLong(0x0028, 0x0, 0x0002, 0, 0))
        {
            IMEBRA_THROW(DataSetDifferentFormatError, "An image already exists in the dataset and has different attributes");
        }
	}

	// Select the data type OB if not already set in the
	//  dataset
	///////////////////////////////////////////////////////////
    tagVR_t dataHandlerType;
    if(transferSyntax == "1.2.840.10008.1.2")
    {
        dataHandlerType = dicomDictionary::getDicomDictionary()->getTagType(0x7FE0, 0x0010);
    }
    else
    {
        dataHandlerType = (bEncapsulated || allocatedBits <= 8) ? tagVR_t::OB : tagVR_t::OW;
    }

	// Encapsulated mode. Check if we have the offsets table
	///////////////////////////////////////////////////////////
    std::uint32_t firstBufferId(0);
    if(bEncapsulated)
	{
        try
        {
            if(bufferExists(groupId, 0x0, tagId, 0))
            {
                dataHandlerType = getDataType(groupId, 0x0, tagId);
            }
            else
            {
                std::shared_ptr<handlers::readingDataHandlerRaw> imageHandler0 = getReadingDataHandlerRaw(groupId, 0x0, tagId, 0x0);
                size_t bufferSize(imageHandler0->getSize());

                if(bufferSize != 0 && !bufferExists(groupId, 0x0, tagId, 0x1))
                {
                    // The first image must be moved forward, in order to
                    //  make some room for the offset table
                    ///////////////////////////////////////////////////////////
                    dataHandlerType = imageHandler0->getDataType();
                    std::shared_ptr<handlers::writingDataHandlerRaw> moveFirstImage = getWritingDataHandlerRaw(groupId, 0x0, tagId, 0x1, dataHandlerType);
                    moveFirstImage->setSize(bufferSize);
                    ::memcpy(moveFirstImage->getMemoryBuffer(), imageHandler0->getMemoryBuffer(), bufferSize);
                }
            }
        }
        catch(const MissingDataElementError&)
        {
            // Nothing to do. No image has been stored yet
        }

		firstBufferId = getFirstAvailFrameBufferId();
	}

	// Get a stream to save the image
    ///////////////////////////////////////////////////////////
    {
        std::shared_ptr<streamWriter> outputStream;
        std::shared_ptr<memory> uncompressedImage(std::make_shared<memory>());
        if(bEncapsulated)
        {
            outputStream = getStreamWriter(groupId, 0, tagId, firstBufferId, dataHandlerType);
        }
        else
        {
            std::shared_ptr<memoryStreamOutput> memStream(std::make_shared<memoryStreamOutput>(uncompressedImage));
            outputStream = std::make_shared<streamWriter>(memStream);
        }

        // Save the image in the stream
        ///////////////////////////////////////////////////////////
        saveCodec->setImage(
            outputStream,
            pImage,
            transferSyntax,
            quality,
            dataHandlerType,
            allocatedBits,
            bSubSampledX, bSubSampledY,
            bInterleaved,
            b2complement);

        outputStream->flushDataBuffer();

        if(!bEncapsulated)
        {
            getTagCreate(groupId, 0, tagId, dataHandlerType)->getBufferCreate(firstBufferId)->appendMemory(uncompressedImage);
        }
    }

	// The images' positions calculated by getImage are not
	//  valid now. They must be recalculated.
	///////////////////////////////////////////////////////////
	m_imagesPositions.clear();

	// Write the attributes in the dataset
	///////////////////////////////////////////////////////////
	if(!bDontChangeAttributes)
	{
        std::shared_ptr<handlers::writingDataHandler> dataHandlerTransferSyntax = getWritingDataHandler(0x0002, 0x0, 0x0010, 0x0);
        dataHandlerTransferSyntax->setString(0, transferSyntax);

        std::string colorSpace = pImage->getColorSpace();
        setString(0x0028, 0x0, 0x0004, 0, transforms::colorTransforms::colorTransformsFactory::makeSubsampled(colorSpace, bSubSampledX, bSubSampledY));
        if(channelsNumber > 1)
        {
            setUnsignedLong(0x0028, 0x0, 0x0006, 0, bInterleaved ? 0 : 1);
        }
        setUnsignedLong(0x0028, 0x0, 0x0100, 0, allocatedBits);            // allocated bits
        setUnsignedLong(0x0028, 0x0, 0x0101, 0, pImage->getHighBit() + 1); // stored bits
        setUnsignedLong(0x0028, 0x0, 0x0102, 0, pImage->getHighBit());     // high bit
        setUnsignedLong(0x0028, 0x0, 0x0103, 0, b2complement ? 1 : 0);
        setUnsignedLong(0x0028, 0x0, 0x0002, 0, channelsNumber);
		std::uint32_t imageWidth, imageHeight;
		pImage->getSize(&imageWidth, &imageHeight);
        setUnsignedLong(0x0028, 0x0, 0x0011, 0, imageWidth);
        setUnsignedLong(0x0028, 0x0, 0x0010, 0, imageHeight);

        if(colorSpace == "PALETTE COLOR")
		{
            IMEBRA_THROW(DataSetImagePaletteColorIsReadOnly, "Cannot set images with color space PALETTE COLOR");
		}

		double imageSizeMmX, imageSizeMmY;
		pImage->getSizeMm(&imageSizeMmX, &imageSizeMmY);

	}

	// Update the number of frames
	///////////////////////////////////////////////////////////
	numberOfFrames = frameNumber + 1;
    setUnsignedLong(0x0028, 0, 0x0008, 0, numberOfFrames );

	// Update the offsets tag with the image's offsets
	///////////////////////////////////////////////////////////
	if(!bEncapsulated)
	{
		return;
	}

	std::uint32_t calculatePosition(0);
    std::shared_ptr<data> tag(getTag(groupId, 0, tagId));
    for(std::uint32_t scanBuffers = 1; scanBuffers < firstBufferId; ++scanBuffers)
	{
        calculatePosition += (std::uint32_t)tag->getBufferSize(scanBuffers);
		calculatePosition += 8;
	}
    std::shared_ptr<handlers::writingDataHandlerRaw> offsetHandler(getWritingDataHandlerRaw(groupId, 0, tagId, 0, dataHandlerType));
    offsetHandler->setSize(4 * (frameNumber + 1));
    std::shared_ptr<handlers::readingDataHandlerRaw> originalOffsetHandler(getReadingDataHandlerRaw(groupId, 0, tagId, 0));
    originalOffsetHandler->copyTo(offsetHandler->getMemoryBuffer(), offsetHandler->getSize());
	std::uint8_t* pOffsetFrame(offsetHandler->getMemoryBuffer() + (frameNumber * 4));
	*( (std::uint32_t*)pOffsetFrame  ) = calculatePosition;
	streamController::adjustEndian(pOffsetFrame, 4, streamController::lowByteEndian, 1);

	IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Get the offset, in bytes, of the specified frame
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataSet::getFrameOffset(std::uint32_t frameNumber) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        // Retrieve the buffer containing the offsets
        ///////////////////////////////////////////////////////////
        std::shared_ptr<handlers::readingDataHandlerRaw> framesPointer = getReadingDataHandlerRaw(0x7fe0, 0x0, 0x0010, 0);

        // Get the offset table's size, in number of offsets
        ///////////////////////////////////////////////////////////
        std::uint32_t offsetsCount = (std::uint32_t)(framesPointer->getSize() / sizeof(std::uint32_t));

        // If the requested frame doesn't exist then return
        //  0xffffffff (the maximum value)
        ///////////////////////////////////////////////////////////
        if(frameNumber >= offsetsCount && frameNumber != 0)
        {
            return std::numeric_limits<std::uint32_t>::max();
        }

        // Return the requested offset. If the requested frame is
        //  the first and is offset is not specified, then return
        //  0 (the first position)
        ///////////////////////////////////////////////////////////
        if(frameNumber < offsetsCount)
        {
            std::uint32_t* pOffsets = (std::uint32_t*)(framesPointer->getMemoryBuffer());
            std::uint32_t returnOffset(pOffsets[frameNumber]);
            streamController::adjustEndian((std::uint8_t*)&returnOffset, 4, streamController::lowByteEndian);
            return returnOffset;
        }
        return 0;
    }
    catch(const MissingDataElementError&)
    {
        return 0xffffffff;
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Return the buffer that starts at the specified offset
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataSet::getFrameBufferId(std::uint32_t offset) const
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<data> imageTag = getTag(0x7fe0, 0, 0x0010);

    // Start from the buffer n. 1 (the buffer 0 contains
    //  the offset table
    ///////////////////////////////////////////////////////////
    std::uint32_t scanBuffers(1);

    if(offset == std::numeric_limits<std::uint32_t>::max())
    {
        while(imageTag->bufferExists(scanBuffers))
        {
            ++scanBuffers;
        }
        return scanBuffers;
    }

    while(offset != 0)
    {
        // Calculate the total size of the buffer, including
        //  its descriptor (tag group and id and length)
        ///////////////////////////////////////////////////////////
        std::uint32_t bufferSize = (std::uint32_t)imageTag->getBufferSize(scanBuffers);;
        bufferSize += 4; // one WORD for the group id, one WORD for the tag id
        bufferSize += 4; // one DWORD for the tag length
        if(bufferSize > offset)
        {
            IMEBRA_THROW(DataSetImageDoesntExistError, "Image not in the offset table");
        }
        offset -= bufferSize;
        ++scanBuffers;
    }

    return scanBuffers;

	IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the first buffer and the end buffer occupied by an
//  image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
size_t dataSet::getFrameBufferIds(std::uint32_t frameNumber, std::uint32_t* pFirstBuffer, std::uint32_t* pEndBuffer) const
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    try
    {
        std::uint32_t startOffset = getFrameOffset(frameNumber);
        std::uint32_t endOffset = getFrameOffset(frameNumber + 1);

        if(startOffset == std::numeric_limits<std::int32_t>::max())
        {
            IMEBRA_THROW(DataSetImageDoesntExistError, "Image not in the table offset");
        }

        *pFirstBuffer = getFrameBufferId(startOffset);
        *pEndBuffer = getFrameBufferId(endOffset);

        std::shared_ptr<data> imageTag;
        try
        {
            imageTag = getTag(0x7fe0, 0, 0x0010);
        }
        catch(MissingDataElementError& e)
        {
            return 0;
        }

        size_t totalSize(0);
        for(std::uint32_t scanBuffers(*pFirstBuffer); scanBuffers != *pEndBuffer; ++scanBuffers)
        {
            totalSize += imageTag->getBufferSize(scanBuffers);
        }
        return totalSize;
    }
    catch(const MissingDataElementError&)
    {
        IMEBRA_THROW(DataSetCorruptedOffsetTableError, "The basic offset table is corrupted");
    }

	IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Return the id of the first available buffer that can
//  be used to store a new frame
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataSet::getFirstAvailFrameBufferId() const
{
    IMEBRA_FUNCTION_START();

    std::uint32_t availableId(1);
    while(bufferExists(0x7fe0, 0, 0x0010, availableId))
    {
        ++availableId;
    }

	return availableId;

	IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve a sequence item as a dataset
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<dataSet> dataSet::getSequenceItem(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t itemId) const
{
    IMEBRA_FUNCTION_START();

    return getTag(groupId, order, tagId)->getSequenceItem(itemId);

	IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set a sequence item
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setSequenceItem(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t itemId, std::shared_ptr<dataSet> pDataSet)
{
    IMEBRA_FUNCTION_START();

    getTagCreate(groupId, order, tagId, tagVR_t::SQ)->setSequenceItem(itemId, pDataSet);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve a LUT from the data set
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<lut> dataSet::getLut(std::uint16_t groupId, std::uint16_t tagId, size_t lutId) const
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::shared_ptr<dataSet> embeddedLUT = getSequenceItem(groupId, 0, tagId, lutId);
    std::shared_ptr<handlers::readingDataHandlerNumericBase> descriptorHandle = embeddedLUT->getReadingDataHandlerNumeric(0x0028, 0x0, 0x3002, 0x0);
    std::shared_ptr<handlers::readingDataHandlerNumericBase> dataHandle = embeddedLUT->getReadingDataHandlerNumeric(0x0028, 0x0, 0x3006, 0x0);

    std::shared_ptr<lut> pLUT = std::make_shared<lut>(
        descriptorHandle,
        dataHandle,
        embeddedLUT->getUnicodeString(0x0028, 0x0, 0x3003, 0, 0, L""),
        getUnsignedLong(0x0028, 0, 0x0103, 0, 0, 0) != 0);
	return pLUT;

	IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Returns the settings for the values of interest
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
vois_t dataSet::getVOIs()
{
    IMEBRA_FUNCTION_START();

    vois_t vois;

    try
    {
        for(size_t voiIndex(0); ; ++voiIndex)
        {
            VOIDescription voi;
            voi.center = getDouble(0x0028, 0, 0x1050, 0, voiIndex);
            voi.width = getDouble(0x0028, 0, 0x1051, 0, voiIndex);
            voi.description = getUnicodeString(0x0028, 0, 0x1055, 0, voiIndex, L"");
            vois.push_back(voi);
        }
    }
    catch(const MissingDataElementError&)
    {
        // VOI not present
    }

    return vois;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get a tag as a signed long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::int32_t dataSet::getSignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return getReadingDataHandler(groupId, order, tagId, bufferId)->getSignedLong(elementNumber);

	IMEBRA_FUNCTION_END();
}

std::int32_t dataSet::getSignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, std::int32_t defaultValue) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        return getSignedLong(groupId, order, tagId, bufferId, elementNumber);
    }
    catch(const MissingDataElementError&)
    {
        return defaultValue;
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set a tag as a signed long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setSignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, std::int32_t newValue, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::shared_ptr<handlers::writingDataHandler> dataHandler = getWritingDataHandler(groupId, order, tagId, bufferId, tagVR);
    dataHandler->setSize(1);
    dataHandler->setSignedLong(0, newValue);

	IMEBRA_FUNCTION_END();
}


void dataSet::setSignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, std::int32_t newValue)
{
    IMEBRA_FUNCTION_START();

    setSignedLong(groupId, order, tagId, bufferId, newValue, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the requested tag as an unsigned long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataSet::getUnsignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return getReadingDataHandler(groupId, order, tagId, bufferId)->getUnsignedLong(elementNumber);

	IMEBRA_FUNCTION_END();
}

std::uint32_t dataSet::getUnsignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, std::uint32_t defaultValue) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        return getUnsignedLong(groupId, order, tagId, bufferId, elementNumber);
    }
    catch(const MissingDataElementError&)
    {
        return defaultValue;
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the requested tag as an unsigned long
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setUnsignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, std::uint32_t newValue, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::shared_ptr<handlers::writingDataHandler> dataHandler = getWritingDataHandler(groupId, order, tagId, bufferId, tagVR);
    dataHandler->setSize(1);
    dataHandler->setUnsignedLong(0, newValue);

	IMEBRA_FUNCTION_END();
}


void dataSet::setUnsignedLong(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, std::uint32_t newValue)
{
    IMEBRA_FUNCTION_START();

    setUnsignedLong(groupId, order, tagId, bufferId, newValue, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the requested tag as a double
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
double dataSet::getDouble(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return getReadingDataHandler(groupId, order, tagId, bufferId)->getDouble(elementNumber);

	IMEBRA_FUNCTION_END();
}

double dataSet::getDouble(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, double defaultValue) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        return getDouble(groupId, order, tagId, bufferId, elementNumber);
    }
    catch(const MissingDataElementError&)
    {
        return defaultValue;
    }

    IMEBRA_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the requested tag as a double
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setDouble(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, double newValue, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::shared_ptr<handlers::writingDataHandler> dataHandler = getWritingDataHandler(groupId, order, tagId, bufferId, tagVR);
    dataHandler->setSize(1);
    dataHandler->setDouble(0, newValue);

	IMEBRA_FUNCTION_END();
}


void dataSet::setDouble(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, double newValue)
{
    IMEBRA_FUNCTION_START();

    setDouble(groupId, order, tagId, bufferId, newValue, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the requested tag as a string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::string dataSet::getString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return getReadingDataHandler(groupId, order, tagId, bufferId)->getString(elementNumber);

	IMEBRA_FUNCTION_END();
}

std::string dataSet::getString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, const std::string& defaultValue) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        return getString(groupId, order, tagId, bufferId, elementNumber);
    }
    catch(const MissingDataElementError&)
    {
        return defaultValue;
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the requested tag as an unicode string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::wstring dataSet::getUnicodeString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber) const
{
    IMEBRA_FUNCTION_START();

    return getReadingDataHandler(groupId, order, tagId, bufferId)->getUnicodeString(elementNumber);

	IMEBRA_FUNCTION_END();
}

std::wstring dataSet::getUnicodeString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, const std::wstring& defaultValue) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        return getUnicodeString(groupId, order, tagId, bufferId, elementNumber);
    }
    catch(const MissingDataElementError&)
    {
        return defaultValue;
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the requested tag as a string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::string& newString, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::shared_ptr<handlers::writingDataHandler> dataHandler = getWritingDataHandler(groupId, order, tagId, bufferId, tagVR);
    dataHandler->setSize(1);
    dataHandler->setString(0, newString);

	IMEBRA_FUNCTION_END();
}


void dataSet::setString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::string& newString)
{
    IMEBRA_FUNCTION_START();

    setString(groupId, order, tagId, bufferId, newString, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the requested tag as a string
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setUnicodeString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::wstring& newString, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::shared_ptr<handlers::writingDataHandler> dataHandler = getWritingDataHandler(groupId, order, tagId, bufferId, tagVR);
    dataHandler->setSize(1);
    dataHandler->setUnicodeString(0, newString);

	IMEBRA_FUNCTION_END();
}


void dataSet::setUnicodeString(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, const std::wstring& newString)
{
    IMEBRA_FUNCTION_START();

    setUnicodeString(groupId, order, tagId, bufferId, newString, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}


void dataSet::setAge(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, std::uint32_t age, ageUnit_t units)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::shared_ptr<handlers::writingDataHandler> dataHandler = getWritingDataHandler(groupId, order, tagId, bufferId, tagVR_t::AS);
    dataHandler->setSize(1);
    dataHandler->setAge(0, age, units);

    IMEBRA_FUNCTION_END();
}

std::uint32_t dataSet::getAge(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, ageUnit_t* pUnits) const
{
    IMEBRA_FUNCTION_START();

    return getReadingDataHandler(groupId, order, tagId, bufferId)->getAge(elementNumber, pUnits);

    IMEBRA_FUNCTION_END();
}

std::uint32_t dataSet::getAge(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId,
                              size_t elementNumber, ageUnit_t* pUnits,
                              std::uint32_t defaultAge, ageUnit_t defaultUnits) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        return getAge(groupId, order, tagId, bufferId, elementNumber, pUnits);
    }
    catch(const MissingDataElementError&)
    {
        *pUnits = defaultUnits;
        return defaultAge;
    }

    IMEBRA_FUNCTION_END();

}


void dataSet::setDate(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, std::uint32_t year, std::uint32_t month, std::uint32_t day, std::uint32_t hour, std::uint32_t minutes, std::uint32_t seconds, std::uint32_t nanoseconds, std::int32_t offsetHours, std::int32_t offsetMinutes, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    std::shared_ptr<handlers::writingDataHandler> dataHandler = getWritingDataHandler(groupId, order, tagId, bufferId, tagVR);
    dataHandler->setSize(1);
    dataHandler->setDate(0, year, month, day, hour, minutes, seconds, nanoseconds, offsetHours, offsetMinutes);

    IMEBRA_FUNCTION_END();
}

void dataSet::setDate(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, std::uint32_t year, std::uint32_t month, std::uint32_t day, std::uint32_t hour, std::uint32_t minutes, std::uint32_t seconds, std::uint32_t nanoseconds, std::int32_t offsetHours, std::int32_t offsetMinutes)
{
    IMEBRA_FUNCTION_START();

    setDate(groupId, order, tagId, bufferId, year, month, day, hour, minutes, seconds, nanoseconds, offsetHours, offsetMinutes, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}

void dataSet::getDate(uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber, std::uint32_t *pYear, std::uint32_t *pMonth, std::uint32_t *pDay, std::uint32_t *pHour, std::uint32_t *pMinutes, std::uint32_t *pSeconds, std::uint32_t *pNanoseconds, std::int32_t *pOffsetHours, std::int32_t *pOffsetMinutes) const
{
    IMEBRA_FUNCTION_START();

    return getReadingDataHandler(groupId, order, tagId, bufferId)->getDate(elementNumber, pYear, pMonth, pDay, pHour, pMinutes, pSeconds, pNanoseconds, pOffsetHours, pOffsetMinutes);

    IMEBRA_FUNCTION_END();
}

void dataSet::getDate(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, size_t elementNumber,
    std::uint32_t* pYear,
    std::uint32_t* pMonth,
    std::uint32_t* pDay,
    std::uint32_t* pHour,
    std::uint32_t* pMinutes,
    std::uint32_t* pSeconds,
    std::uint32_t* pNanoseconds,
    std::int32_t* pOffsetHours,
    std::int32_t* pOffsetMinutes,
    std::uint32_t defaultYear,
    std::uint32_t defaultMonth,
    std::uint32_t defaultDay,
    std::uint32_t defaultHour,
    std::uint32_t defaultMinutes,
    std::uint32_t defaultSeconds,
    std::uint32_t defaultNanoseconds,
    std::int32_t defaultOffsetHours,
    std::int32_t defaultOffsetMinutes) const
{
    IMEBRA_FUNCTION_START();

    try
    {
        return getDate(groupId, order, tagId, bufferId, elementNumber,
                            pYear, pMonth, pDay, pHour, pMinutes, pSeconds, pNanoseconds, pOffsetHours, pOffsetMinutes);
    }
    catch(const MissingDataElementError&)
    {
        *pYear = defaultYear;
        *pMonth = defaultMonth;
        *pDay = defaultDay;
        *pHour = defaultHour;
        *pMinutes = defaultMinutes;
        *pSeconds = defaultSeconds;
        *pNanoseconds = defaultNanoseconds;
        *pOffsetHours = defaultOffsetHours;
        *pOffsetMinutes = defaultOffsetMinutes;
    }

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get a data handler for the requested tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<handlers::readingDataHandler> dataSet::getReadingDataHandler(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    return getTag(groupId, order, tagId)->getReadingDataHandler(bufferId);

	IMEBRA_FUNCTION_END();
}


std::shared_ptr<handlers::writingDataHandler> dataSet::getWritingDataHandler(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<data> tag(getTagCreate(groupId, order, tagId, tagVR));

    return tag->getWritingDataHandler(bufferId);

    IMEBRA_FUNCTION_END();
}

std::shared_ptr<handlers::writingDataHandler> dataSet::getWritingDataHandler(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId)
{
    IMEBRA_FUNCTION_START();

    return getWritingDataHandler(groupId, order, tagId, bufferId, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get a stream reader that works on the specified tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<streamReader> dataSet::getStreamReader(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    return getTag(groupId, order, tagId)->getStreamReader(bufferId);

	IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve a stream writer for the specified tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<streamWriter> dataSet::getStreamWriter(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, tagVR_t dataType)
{
    IMEBRA_FUNCTION_START();

    return getTagCreate(groupId, order, tagId, dataType)->getStreamWriter(bufferId);

	IMEBRA_FUNCTION_END();
}

std::shared_ptr<streamWriter> dataSet::getStreamWriter(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId)
{
    IMEBRA_FUNCTION_START();

    return getStreamWriter(groupId, order, tagId, bufferId, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get a raw data handler for the requested tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::shared_ptr<handlers::readingDataHandlerRaw> dataSet::getReadingDataHandlerRaw(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    return getTag(groupId, order, tagId)->getReadingDataHandlerRaw(bufferId);

	IMEBRA_FUNCTION_END();
}


std::shared_ptr<handlers::writingDataHandlerRaw> dataSet::getWritingDataHandlerRaw(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<data> tag = getTagCreate(groupId, order, tagId, tagVR);

    return tag->getWritingDataHandlerRaw(bufferId);

    IMEBRA_FUNCTION_END();
}


std::shared_ptr<handlers::writingDataHandlerRaw> dataSet::getWritingDataHandlerRaw(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId)
{
    IMEBRA_FUNCTION_START();

    return getWritingDataHandlerRaw(groupId, order, tagId, bufferId, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}


std::shared_ptr<handlers::readingDataHandlerNumericBase> dataSet::getReadingDataHandlerNumeric(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId) const
{
    IMEBRA_FUNCTION_START();

    return getTag(groupId, order, tagId)->getReadingDataHandlerNumeric(bufferId);

    IMEBRA_FUNCTION_END();
}


std::shared_ptr<handlers::writingDataHandlerNumericBase> dataSet::getWritingDataHandlerNumeric(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId, tagVR_t tagVR)
{
    IMEBRA_FUNCTION_START();

    std::shared_ptr<data> tag = getTagCreate(groupId, order, tagId, tagVR);

    return tag->getWritingDataHandlerNumeric(bufferId);

    IMEBRA_FUNCTION_END();
}


std::shared_ptr<handlers::writingDataHandlerNumericBase> dataSet::getWritingDataHandlerNumeric(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId, size_t bufferId)
{
    IMEBRA_FUNCTION_START();

    return getWritingDataHandlerNumeric(groupId, order, tagId, bufferId, dicomDictionary::getDicomDictionary()->getTagType(groupId, tagId));

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the data type of a tag
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
tagVR_t dataSet::getDataType(std::uint16_t groupId, std::uint32_t order, std::uint16_t tagId) const
{
    IMEBRA_FUNCTION_START();

    return getTag(groupId, order, tagId)->getDataType();

	IMEBRA_FUNCTION_END();
}

void dataSet::updateCharsetTag()
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    charsetsList::tCharsetsList charsets;
	getCharsetsList(&charsets);
    std::shared_ptr<handlers::writingDataHandler> charsetHandler(getWritingDataHandler(0x0008, 0, 0x0005, 0));
	charsetHandler->setSize((std::uint32_t)(charsets.size()));
	std::uint32_t pointer(0);
	for(charsetsList::tCharsetsList::iterator scanCharsets = charsets.begin(); scanCharsets != charsets.end(); ++scanCharsets)
	{
        charsetHandler->setString(pointer++, *scanCharsets);
	}

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Update the list of the used charsets
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::updateTagsCharset()
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    charsetsList::tCharsetsList charsets;
    try
    {
        std::shared_ptr<handlers::readingDataHandler> charsetHandler(getReadingDataHandler(0x0008, 0, 0x0005, 0));
        for(std::uint32_t pointer(0); pointer != charsetHandler->getSize(); ++pointer)
		{
            charsets.push_back(charsetHandler->getString(pointer));
		}
	}
    catch(const MissingDataElementError&)
    {
        // TODO
    }

    setCharsetsList(charsets);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the item's position in the stream
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void dataSet::setItemOffset(std::uint32_t offset)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

	m_itemOffset = offset;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the item's position in the stream
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
std::uint32_t dataSet::getItemOffset() const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);

	return m_itemOffset;
}

void dataSet::getCharsetsList(charsetsList::tCharsetsList* pCharsetsList) const
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    for(tGroups::const_iterator scanGroups(m_groups.begin()), endGroups(m_groups.end()); scanGroups != endGroups; ++scanGroups)
    {
        for(tGroupsList::const_iterator scanGroupsList(scanGroups->second.begin()), endGroupsList(scanGroups->second.end()); scanGroupsList != endGroupsList; ++scanGroupsList)
        {
            for(tTags::const_iterator scanTags((*scanGroupsList).begin()), endTags((*scanGroupsList).end()); scanTags != endTags; ++scanTags)
            {
                charsetsList::tCharsetsList charsets;
                scanTags->second->getCharsetsList(&charsets);
                charsetsList::updateCharsets(&charsets, pCharsetsList);

            }
        }
    }

    IMEBRA_FUNCTION_END();
}

dataSet::tGroupsIds dataSet::getGroups() const
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    dataSet::tGroupsIds groups;

    for(tGroups::const_iterator scanGroups(m_groups.begin()), endGroups(m_groups.end()); scanGroups != endGroups; ++scanGroups)
    {
        groups.insert(scanGroups->first);
    }

    return groups;

    IMEBRA_FUNCTION_END();
}

std::uint32_t dataSet::getGroupsNumber(uint16_t groupId) const
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    dataSet::tGroups::const_iterator findGroup(m_groups.find(groupId));

    if(findGroup == m_groups.end())
    {
        return 0;
    }

    return (std::uint32_t)(findGroup->second.size());

    IMEBRA_FUNCTION_END();
}

const dataSet::tTags& dataSet::getGroupTags(std::uint16_t groupId, size_t groupOrder) const
{
    IMEBRA_FUNCTION_START();

    static const dataSet::tTags emptyTags;

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    tGroups::const_iterator findGroup(m_groups.find(groupId));
    if(findGroup == m_groups.end() || findGroup->second.size() <= groupOrder)
    {
        return emptyTags;
    }

    return findGroup->second.at(groupOrder);

    IMEBRA_FUNCTION_END();
}

void dataSet::setCharsetsList(const charsetsList::tCharsetsList& charsetsList)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::recursive_mutex> lock(m_mutex);

    m_charsetsList = charsetsList;
    for(tGroups::iterator scanGroups(m_groups.begin()), endGroups(m_groups.end()); scanGroups != endGroups; ++scanGroups)
    {
        for(tGroupsList::iterator scanGroupsList(scanGroups->second.begin()), endGroupsList(scanGroups->second.end()); scanGroupsList != endGroupsList; ++scanGroupsList)
        {
            for(tTags::iterator scanTags((*scanGroupsList).begin()), endTags((*scanGroupsList).end()); scanTags != endTags; ++scanTags)
            {
                scanTags->second->setCharsetsList(charsetsList);
            }
        }
    }

    IMEBRA_FUNCTION_END();
}

} // namespace implementation

} // namespace imebra
