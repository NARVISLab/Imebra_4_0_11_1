/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandler.cpp
    \brief Implementation of the classes ReadingDataHandler & WritingDataHandler.
*/

#include "../include/imebra/writingDataHandlerNumeric.h"
#include "../include/imebra/readingDataHandlerNumeric.h"
#include "../implementation/dataHandlerImpl.h"
#include "../implementation/dataHandlerNumericImpl.h"
#include <cstring>

namespace imebra
{

WritingDataHandlerNumeric::WritingDataHandlerNumeric(std::shared_ptr<implementation::handlers::writingDataHandlerNumericBase> pDataHandler):
    WritingDataHandler(pDataHandler)
{
}

WritingDataHandlerNumeric::~WritingDataHandlerNumeric()
{
}

ReadWriteMemory* WritingDataHandlerNumeric::getMemory() const
{
    std::shared_ptr<imebra::implementation::handlers::writingDataHandlerNumericBase> numericDataHandler = std::dynamic_pointer_cast<imebra::implementation::handlers::writingDataHandlerNumericBase>(m_pDataHandler);
    return new ReadWriteMemory(numericDataHandler->getMemory());
}

void WritingDataHandlerNumeric::assign(const char* source, size_t sourceSize)
{
    std::shared_ptr<imebra::implementation::handlers::writingDataHandlerNumericBase> numericDataHandler = std::dynamic_pointer_cast<imebra::implementation::handlers::writingDataHandlerNumericBase>(m_pDataHandler);
    numericDataHandler->getMemory()->assign((std::uint8_t*) source, sourceSize);
}

char* WritingDataHandlerNumeric::data(size_t* pDataSize) const
{
    std::shared_ptr<imebra::implementation::handlers::writingDataHandlerNumericBase> numericDataHandler = std::dynamic_pointer_cast<imebra::implementation::handlers::writingDataHandlerNumericBase>(m_pDataHandler);
    *pDataSize = numericDataHandler->getMemorySize();
    return (char*)numericDataHandler->getMemoryBuffer();
}

size_t WritingDataHandlerNumeric::data(char* destination, size_t destinationSize) const
{
    std::shared_ptr<imebra::implementation::handlers::writingDataHandlerNumericBase> numericDataHandler = std::dynamic_pointer_cast<imebra::implementation::handlers::writingDataHandlerNumericBase>(m_pDataHandler);
    size_t memorySize = numericDataHandler->getMemorySize();
    if(destination != 0 && destinationSize >= memorySize && memorySize != 0)
    {
        ::memcpy(destination, numericDataHandler->getMemoryBuffer(), memorySize);
    }
    return memorySize;
}

size_t WritingDataHandlerNumeric::getUnitSize() const
{
    std::shared_ptr<imebra::implementation::handlers::writingDataHandlerNumericBase> numericDataHandler = std::dynamic_pointer_cast<imebra::implementation::handlers::writingDataHandlerNumericBase>(m_pDataHandler);
    return numericDataHandler->getUnitSize();
}

bool WritingDataHandlerNumeric::isSigned() const
{
    std::shared_ptr<imebra::implementation::handlers::writingDataHandlerNumericBase> numericDataHandler = std::dynamic_pointer_cast<imebra::implementation::handlers::writingDataHandlerNumericBase>(m_pDataHandler);
    return numericDataHandler->isSigned();
}

bool WritingDataHandlerNumeric::isFloat() const
{
    std::shared_ptr<imebra::implementation::handlers::writingDataHandlerNumericBase> numericDataHandler = std::dynamic_pointer_cast<imebra::implementation::handlers::writingDataHandlerNumericBase>(m_pDataHandler);
    return numericDataHandler->isFloat();
}

void WritingDataHandlerNumeric::copyFrom(const ReadingDataHandlerNumeric& source)
{
    std::shared_ptr<imebra::implementation::handlers::writingDataHandlerNumericBase> numericDataHandler = std::dynamic_pointer_cast<imebra::implementation::handlers::writingDataHandlerNumericBase>(m_pDataHandler);
    return numericDataHandler->copyFrom(std::dynamic_pointer_cast<imebra::implementation::handlers::readingDataHandlerNumericBase>(source.m_pDataHandler));
}

}
