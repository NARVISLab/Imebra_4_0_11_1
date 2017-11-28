/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file image_swig.h
    \brief Implementation of the class Image for SWIG.

*/

#include "../include/imebra/image.h"
#include "../implementation/imageImpl.h"
#include "../implementation/dataHandlerImpl.h"
#include "../implementation/dataHandlerNumericImpl.h"

namespace imebra
{

Image::Image(
        std::uint32_t width,
        std::uint32_t height,
        bitDepth_t depth,
        const std::string& colorSpace,
        std::uint32_t highBit):
    m_pImage(std::make_shared<implementation::image>(width, height, depth, colorSpace, highBit))
{
}

Image::Image(std::shared_ptr<implementation::image> pImage): m_pImage(pImage)
{
}

Image::~Image()
{
}

double Image::getWidthMm() const
{
    double width, height;
    m_pImage->getSizeMm(&width, &height);
    return width;
}

double Image::getHeightMm() const
{
    double width, height;
    m_pImage->getSizeMm(&width, &height);
    return height;
}

void Image::setSizeMm(double width, double height)
{
    m_pImage->setSizeMm(width, height);
}

std::uint32_t Image::getWidth() const
{
    std::uint32_t width, height;
    m_pImage->getSize(&width, &height);
    return width;
}

std::uint32_t Image::getHeight() const
{
    std::uint32_t width, height;
    m_pImage->getSize(&width, &height);
    return height;
}

ReadingDataHandlerNumeric* Image::getReadingDataHandler() const
{
    return new ReadingDataHandlerNumeric(m_pImage->getReadingDataHandler());
}

WritingDataHandlerNumeric* Image::getWritingDataHandler()
{
    return new WritingDataHandlerNumeric(m_pImage->getWritingDataHandler());
}

std::string Image::getColorSpace() const
{
    return m_pImage->getColorSpace();
}

std::uint32_t Image::getChannelsNumber() const
{
    return m_pImage->getChannelsNumber();
}

bitDepth_t Image::getDepth() const
{
    return m_pImage->getDepth();
}

std::uint32_t Image::getHighBit() const
{
    return m_pImage->getHighBit();
}

}
