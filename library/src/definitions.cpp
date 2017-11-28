/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

#include "../include/imebra/definitions.h"
#include "../implementation/exceptionImpl.h"

namespace imebra
{

Age::Age(uint32_t initialAge, ageUnit_t initialUnits):
    age(initialAge), units(initialUnits)
{

}

double Age::years() const
{
    switch(units)
    {
    case ageUnit_t::years:
        return (double)age;
    case ageUnit_t::months:
        return age / (double)12;
    case ageUnit_t::weeks:
        return age / 52.142857;
    case ageUnit_t::days:
        return age / (double)365;
    default:
        throw;
    }
}

Date::Date(
        const unsigned int initialYear,
        const unsigned int initialMonth,
        const unsigned int initialDay,
        const unsigned int initialHour,
        const unsigned int initialMinutes,
        const unsigned int initialSeconds,
        const unsigned int initialNanoseconds,
        const signed int initialOffsetHours,
        const signed int initialOffsetMinutes):
    year(initialYear),
    month(initialMonth),
    day(initialDay),
    hour(initialHour),
    minutes(initialMinutes),
    seconds(initialSeconds),
    nanoseconds(initialNanoseconds),
    offsetHours(initialOffsetHours),
    offsetMinutes(initialOffsetMinutes)
{

}

}
