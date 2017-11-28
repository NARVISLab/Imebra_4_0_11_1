/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file VOILUT.cpp
    \brief Implementation of VOILUT.

*/


#include "../include/imebra/VOILUT.h"
#include "../include/imebra/dataSet.h"
#include "../implementation/VOILUTImpl.h"
#include "../include/imebra/lut.h"

namespace imebra
{

VOILUT::VOILUT(): Transform(std::make_shared<imebra::implementation::transforms::VOILUT>())
{
}

VOILUT::~VOILUT()
{
}

void VOILUT::setCenterWidth(double center, double width)
{
    ((imebra::implementation::transforms::VOILUT*)m_pTransform.get())->setCenterWidth(center, width);
}

void VOILUT::setLUT(const LUT &lut)
{
    ((imebra::implementation::transforms::VOILUT*)m_pTransform.get())->setLUT(lut.m_pLut);
}

void VOILUT::applyOptimalVOI(const Image& inputImage, std::uint32_t topLeftX, std::uint32_t topLeftY, std::uint32_t width, std::uint32_t height)
{
    ((imebra::implementation::transforms::VOILUT*)m_pTransform.get())->applyOptimalVOI(inputImage.m_pImage, topLeftX, topLeftY, width, height);
}

double VOILUT::getCenter() const
{
    double center, width;
    ((imebra::implementation::transforms::VOILUT*)m_pTransform.get())->getCenterWidth(&center, &width);
    return center;
}

double VOILUT::getWidth() const
{
    double center, width;
    ((imebra::implementation::transforms::VOILUT*)m_pTransform.get())->getCenterWidth(&center, &width);
    return width;
}

}
