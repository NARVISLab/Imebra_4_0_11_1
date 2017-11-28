/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file imageCodecImpl.cpp
    \brief Implementation of the base class for the image codecs.

*/

#include "imageCodecImpl.h"
#include "exceptionImpl.h"
#include <string.h>


namespace imebra
{

namespace implementation
{

namespace codecs
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Allocate a channel's memory
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void channel::allocate(std::uint32_t width, std::uint32_t height)
{
    IMEBRA_FUNCTION_START();

    m_width = width;
    m_height = height;
    m_bufferSize = width * height;
    m_memory = std::make_shared<memory>(m_bufferSize * sizeof(std::int32_t) );
    m_pBuffer = (std::int32_t*)(m_memory->data());

    ::memset(m_pBuffer, 0, m_bufferSize * sizeof(std::int32_t));

    IMEBRA_FUNCTION_END();
}

} // namespace codecs

} // namespace implementation

} // namespace imebra

