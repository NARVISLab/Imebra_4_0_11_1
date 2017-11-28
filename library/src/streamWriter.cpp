/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file streamWriter.cpp
    \brief Implementation of the the class used to write the streams.

*/

#include "../include/imebra/streamWriter.h"
#include "../include/imebra/baseStreamOutput.h"

#include "../implementation/streamWriterImpl.h"

namespace imebra
{

StreamWriter::StreamWriter(std::shared_ptr<implementation::streamWriter> pWriter): m_pWriter(pWriter)
{
}

StreamWriter::StreamWriter(const BaseStreamOutput& stream): m_pWriter(std::make_shared<implementation::streamWriter>(stream.m_pStream))
{
}

StreamWriter::StreamWriter(const BaseStreamOutput& stream, size_t virtualStart, size_t virtualLength): m_pWriter(std::make_shared<implementation::streamWriter>(stream.m_pStream, virtualStart, virtualLength))
{
}

StreamWriter::~StreamWriter()
{
}

}
