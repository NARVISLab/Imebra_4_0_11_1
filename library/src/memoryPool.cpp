/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

#include "../include/imebra/memoryPool.h"

#include "../implementation/memoryImpl.h"

namespace imebra
{

void MemoryPool::flush()
{
    implementation::memoryPoolGetter::getMemoryPoolGetter().getMemoryPoolLocal().flush();
}

size_t MemoryPool::getUnusedMemorySize()
{
    return implementation::memoryPoolGetter::getMemoryPoolGetter().getMemoryPoolLocal().getUnusedMemorySize();
}

void MemoryPool::setMemoryPoolSize(size_t minMemoryBlockSize, size_t maxMemoryPoolSize)
{
    implementation::memoryPoolGetter::getMemoryPoolGetter().getMemoryPoolLocal().setMinMaxMemory(minMemoryBlockSize, maxMemoryPoolSize);
}

}
