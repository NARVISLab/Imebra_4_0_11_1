/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

#include "../include/imebra/readMemory.h"
#include "../implementation/memoryImpl.h"
#include "../implementation/exceptionImpl.h"
#include "../include/imebra/exceptions.h"
#include <memory.h>

namespace imebra
{

ReadMemory::ReadMemory(): m_pMemory(std::make_shared<const implementation::memory>())
{
}

ReadMemory::ReadMemory(const char* buffer, size_t bufferSize):
    m_pMemory(std::make_shared<const implementation::memory>(new implementation::stringUint8((const std::uint8_t*)buffer, bufferSize)))
{
}

ReadMemory::ReadMemory(std::shared_ptr<const implementation::memory> pMemory): m_pMemory(pMemory)
{
}

ReadMemory::~ReadMemory()
{
}

size_t ReadMemory::size() const
{
    return m_pMemory->size();
}

const char* ReadMemory::data(size_t* pDataSize) const
{
    *pDataSize = m_pMemory->size();
    return (char*)m_pMemory->data();
}

size_t ReadMemory::data(char* destination, size_t destinationSize) const
{
    size_t memorySize = m_pMemory->size();
    if(destination != 0 && destinationSize >= memorySize && memorySize != 0)
    {
        ::memcpy(destination, m_pMemory->data(), memorySize);
    }
    return memorySize;
}

void ReadMemory::regionData(char* destination, size_t destinationSize, size_t sourceOffset) const
{
    IMEBRA_FUNCTION_START();

    size_t memorySize = m_pMemory->size();
    if(m_pMemory->size() < sourceOffset + destinationSize)
    {
        IMEBRA_THROW(MemorySizeError, "The source memory region exceedes the memory size");
    }
    ::memcpy(destination, m_pMemory->data() + sourceOffset, memorySize);

    IMEBRA_FUNCTION_END();
}


bool ReadMemory::empty() const
{
    return m_pMemory->empty();
}

}
