/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file streamReader.cpp
    \brief Implementation of the the class used to read the streams.

*/

#include "../include/imebra/streamReader.h"
#include "../include/imebra/baseStreamInput.h"
#include "../implementation/streamReaderImpl.h"

namespace imebra
{

StreamReader::StreamReader(std::shared_ptr<implementation::streamReader> pReader): m_pReader(pReader)
{
}

StreamReader::StreamReader(const BaseStreamInput& stream): m_pReader(std::make_shared<implementation::streamReader>(stream.m_pStream))
{
}


StreamReader::StreamReader(const BaseStreamInput& stream, size_t virtualStart, size_t virtualLength): m_pReader(std::make_shared<implementation::streamReader>(stream.m_pStream, virtualStart, virtualLength))
{
}

StreamReader::~StreamReader()
{
}

}
