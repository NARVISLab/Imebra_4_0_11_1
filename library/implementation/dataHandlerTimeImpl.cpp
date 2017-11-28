/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerTime.cpp
    \brief Implementation of the class dataHandlerTime.

*/

#include <sstream>
#include <iomanip>
#include <stdlib.h>

#include "exceptionImpl.h"
#include "dataHandlerTimeImpl.h"

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
// dataHandlerTime
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

readingDataHandlerTime::readingDataHandlerTime(const memory& parseMemory): readingDataHandlerDateTimeBase(parseMemory, tagVR_t::TM)
{
}

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Get the date/time
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void readingDataHandlerTime::getDate(const size_t index,
         std::uint32_t* pYear,
         std::uint32_t* pMonth,
         std::uint32_t* pDay,
         std::uint32_t* pHour,
         std::uint32_t* pMinutes,
         std::uint32_t* pSeconds,
         std::uint32_t* pNanoseconds,
         std::int32_t* pOffsetHours,
         std::int32_t* pOffsetMinutes) const
{
    IMEBRA_FUNCTION_START();

    *pYear = 0;
    *pMonth = 0;
    *pDay = 0;
    *pHour = 0;
    *pMinutes = 0;
    *pSeconds = 0;
    *pNanoseconds = 0;
    *pOffsetHours = 0;
    *pOffsetMinutes = 0;

    parseTime(getString(index), pHour, pMinutes, pSeconds, pNanoseconds, pOffsetHours, pOffsetMinutes);

    IMEBRA_FUNCTION_END();

}

writingDataHandlerTime::writingDataHandlerTime(const std::shared_ptr<buffer> &pBuffer):
    writingDataHandlerDateTimeBase(pBuffer, tagVR_t::TM, 0, 28)
{

}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set the date/time
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void writingDataHandlerTime::setDate(const size_t index,
         std::uint32_t /* year */,
         std::uint32_t /* month */,
         std::uint32_t /* day */,
         std::uint32_t hour,
         std::uint32_t minutes,
         std::uint32_t seconds,
         std::uint32_t nanoseconds,
         std::int32_t /* offsetHours */,
         std::int32_t /* offsetMinutes */)
{
    IMEBRA_FUNCTION_START();

    std::string timeString = buildTimeSimple(hour, minutes, seconds, nanoseconds);
    setString(index, timeString);

	IMEBRA_FUNCTION_END();
}


} // namespace handlers

} // namespace implementation

} // namespace imebra
