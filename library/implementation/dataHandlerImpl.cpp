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
    \brief Implementation of the base class for the data handlers.

*/

#include "../include/imebra/exceptions.h"
#include "exceptionImpl.h"
#include "dataHandlerImpl.h"
#include "memoryImpl.h"
#include "dicomDictImpl.h"

namespace imebra
{

namespace implementation
{

namespace handlers
{


readingDataHandler::readingDataHandler(tagVR_t dataType): m_dataType(dataType)
{
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the data 's type
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
tagVR_t readingDataHandler::getDataType() const
{
    return m_dataType;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the date
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void readingDataHandler::getDate(const size_t /* index */,
        std::uint32_t* /* pYear */,
        std::uint32_t* /* pMonth */,
        std::uint32_t* /* pDay */,
        std::uint32_t* /* pHour */,
        std::uint32_t* /* pMinutes */,
        std::uint32_t* /* pSeconds */,
        std::uint32_t* /* pNanoseconds */,
        std::int32_t* /* pOffsetHours */,
        std::int32_t* /* pOffsetMinutes */) const
{
    IMEBRA_FUNCTION_START();

    IMEBRA_THROW(DataHandlerConversionError, "Cannot convert VR "<< dicomDictionary::getDicomDictionary()->enumDataTypeToString(getDataType()) << " to Date");

    IMEBRA_FUNCTION_END();
}

std::uint32_t readingDataHandler::getAge(const size_t /* index */, ageUnit_t * /* pUnit */) const
{
    IMEBRA_FUNCTION_START();

    IMEBRA_THROW(DataHandlerConversionError, "Cannot convert VR "<< dicomDictionary::getDicomDictionary()->enumDataTypeToString(getDataType()) << " to Age");

    IMEBRA_FUNCTION_END();
}


writingDataHandler::writingDataHandler(const std::shared_ptr<buffer> &pBuffer, tagVR_t dataType, const uint8_t paddingByte):
    m_dataType(dataType), m_buffer(pBuffer), m_paddingByte(paddingByte)
{
}

writingDataHandler::~writingDataHandler()
{
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Retrieve the data 's type
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
tagVR_t writingDataHandler::getDataType() const
{
    return m_dataType;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the date
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void writingDataHandler::setDate(const size_t /* index */,
        std::uint32_t /* year */,
        std::uint32_t /* month */,
        std::uint32_t /* day */,
        std::uint32_t /* hour */,
        std::uint32_t /*minutes */,
        std::uint32_t /*seconds */,
        std::uint32_t /*nanoseconds */,
		std::int32_t /*offsetHours */,
		std::int32_t /*offsetMinutes */)
{
    IMEBRA_FUNCTION_START();

    IMEBRA_THROW(DataHandlerConversionError, "Cannot convert Date to VR "<< dicomDictionary::getDicomDictionary()->enumDataTypeToString(getDataType()));

    IMEBRA_FUNCTION_END();
}

void writingDataHandler::setAge(const size_t /* index */, const std::uint32_t /* age */, const ageUnit_t /* unit */)
{
    IMEBRA_FUNCTION_START();

    IMEBRA_THROW(DataHandlerConversionError, "Cannot convert Age to VR "<< dicomDictionary::getDicomDictionary()->enumDataTypeToString(getDataType()));

    IMEBRA_FUNCTION_END();
}

} // namespace handlers

} // namespace implementation

} // namespace imebra
