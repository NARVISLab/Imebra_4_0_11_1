/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file transform.cpp
    \brief Implementation of the Transform class.

*/

#include "../include/imebra/transform.h"
#include "../include/imebra/image.h"
#include "../implementation/transformImpl.h"

namespace imebra
{

Transform::~Transform()
{
}

Transform::Transform(std::shared_ptr<imebra::implementation::transforms::transform> pTransform): m_pTransform(pTransform)
{}

bool Transform::isEmpty() const
{
    return m_pTransform == 0 || m_pTransform->isEmpty();
}

Image* Transform::allocateOutputImage(const Image& inputImage, std::uint32_t width, std::uint32_t height) const
{
    return new Image(m_pTransform->allocateOutputImage(inputImage.m_pImage->getDepth(),
                                                   inputImage.m_pImage->getColorSpace(),
                                                   inputImage.m_pImage->getHighBit(),
                                                   inputImage.m_pImage->getPalette(),
                                                   width, height));
}

void Transform::runTransform(
            const Image& inputImage,
            std::uint32_t inputTopLeftX, std::uint32_t inputTopLeftY, std::uint32_t inputWidth, std::uint32_t inputHeight,
            Image& outputImage,
            std::uint32_t outputTopLeftX, std::uint32_t outputTopLeftY) const
{
    m_pTransform->runTransform(inputImage.m_pImage,
        inputTopLeftX,
        inputTopLeftY,
        inputWidth,
        inputHeight,
        outputImage.m_pImage,
        outputTopLeftX,
        outputTopLeftY);
}

}
