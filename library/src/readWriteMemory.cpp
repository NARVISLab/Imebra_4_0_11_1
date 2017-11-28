/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

#include "../include/imebra/readWriteMemory.h"
#include "../implementation/memoryImpl.h"
#include <memory.h>

namespace imebra
{

ReadWriteMemory::ReadWriteMemory(): ReadMemory()
{
}

ReadWriteMemory::ReadWriteMemory(size_t initialSize)
{
    m_pMemory = std::make_shared<const implementation::memory>(initialSize);
}

ReadWriteMemory::ReadWriteMemory(const ReadMemory &sourceMemory)
{
    m_pMemory = std::make_shared<const implementation::memory>(sourceMemory.size());
    std::const_pointer_cast<implementation::memory>(m_pMemory)->copyFrom(sourceMemory.m_pMemory);
}

ReadWriteMemory::ReadWriteMemory(const char* buffer, size_t bufferSize)
{
    m_pMemory = std::make_shared<const implementation::memory>(new implementation::stringUint8((const std::uint8_t*)buffer, bufferSize));
}

ReadWriteMemory::ReadWriteMemory(std::shared_ptr<implementation::memory> pMemory)
{
    m_pMemory = pMemory;
}

ReadWriteMemory::~ReadWriteMemory()
{
}

void ReadWriteMemory::copyFrom(const ReadMemory& sourceMemory)
{
    std::const_pointer_cast<implementation::memory>(m_pMemory)->copyFrom(sourceMemory.m_pMemory);
}

void ReadWriteMemory::clear()
{
    std::const_pointer_cast<implementation::memory>(m_pMemory)->clear();
}

void ReadWriteMemory::resize(size_t newSize)
{
    std::const_pointer_cast<implementation::memory>(m_pMemory)->resize(newSize);
}

void ReadWriteMemory::reserve(size_t reserveSize)
{
    std::const_pointer_cast<implementation::memory>(m_pMemory)->reserve(reserveSize);
}

char* ReadWriteMemory::data(size_t* pDataSize) const
{
    std::shared_ptr<implementation::memory> pMemory = std::const_pointer_cast<implementation::memory>(m_pMemory);
    *pDataSize = pMemory->size();
    return (char*)pMemory->data();
}

void ReadWriteMemory::assign(const char* source, size_t sourceSize)
{
    std::const_pointer_cast<implementation::memory>(m_pMemory)->assign((const std::uint8_t*)source, sourceSize);
}

void ReadWriteMemory::assignRegion(const char* source, size_t sourceSize, size_t destinationOffset)
{
    std::const_pointer_cast<implementation::memory>(m_pMemory)->assignRegion((const std::uint8_t*)source, sourceSize, destinationOffset);
}

}
