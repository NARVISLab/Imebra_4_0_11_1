/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

#include "../include/imebra/lut.h"
#include "../implementation/dataHandlerNumericImpl.h"
#include "../implementation/LUTImpl.h"

namespace imebra
{

LUT::LUT(std::shared_ptr<implementation::lut> pLut): m_pLut(pLut)
{
}

LUT::~LUT()
{
}

std::wstring LUT::getDescription() const
{
    return m_pLut->getDescription();
}

ReadingDataHandlerNumeric* LUT::getReadingDataHandler() const
{
    return new ReadingDataHandlerNumeric(m_pLut->getReadingDataHandler());
}

size_t LUT::getBits() const
{
    return m_pLut->getBits();
}

size_t LUT:: getSize() const
{
    return m_pLut->getSize();
}

std::int32_t LUT::getFirstMapped() const
{
    return m_pLut->getFirstMapped();
}

std::uint32_t LUT::getMappedValue(std::int32_t index) const
{
    return m_pLut->getMappedValue(index);
}


}

