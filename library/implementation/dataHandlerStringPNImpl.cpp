/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerStringPN.cpp
    \brief Implementation of the class dataHandlerStringPN.

*/

#include "dataHandlerStringPNImpl.h"

namespace imebra
{

namespace implementation
{

namespace handlers
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// dataHandlerStringPN
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

readingDataHandlerStringPN::readingDataHandlerStringPN(const memory& parseMemory, const charsetsList::tCharsetsList& charsets):
    readingDataHandlerStringUnicode(parseMemory, charsets, tagVR_t::PN, L'=', 0x20)
{
}

writingDataHandlerStringPN::writingDataHandlerStringPN(const std::shared_ptr<buffer>& pBuffer, const charsetsList::tCharsetsList& charsets):
    writingDataHandlerStringUnicode(pBuffer, charsets, tagVR_t::PN, L'=', 0, 0, 0x20)
{
}

void writingDataHandlerStringPN::validate() const
{
    IMEBRA_FUNCTION_START();

    if(m_strings.size() > 3)
    {
        IMEBRA_THROW(DataHandlerInvalidDataError, "A patient name can contain maximum 3 groups");
    }
    for(size_t scanGroups(0); scanGroups != m_strings.size(); ++scanGroups)
    {
        if(m_strings[scanGroups].size() > 64)
        {
            IMEBRA_THROW(DataHandlerInvalidDataError, "A patient name group can contain maximum 64 chars");
        }
    }

    writingDataHandlerStringUnicode::validate();

    IMEBRA_FUNCTION_END();
}

} // namespace handlers

} // namespace implementation

} // namespace imebra
