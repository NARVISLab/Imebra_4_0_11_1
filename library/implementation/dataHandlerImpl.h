/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandler.h
    \brief Declaration of the base class used by all the data handlers.

*/

#if !defined(imebraDataHandler_6F85D344_DEF8_468d_BF73_AC5BB17FD22A__INCLUDED_)
#define imebraDataHandler_6F85D344_DEF8_468d_BF73_AC5BB17FD22A__INCLUDED_

#include "../include/imebra/definitions.h"
#include "../include/imebra/exceptions.h"
#include "exceptionImpl.h"
#include <memory>
#include "charsetsListImpl.h"


namespace imebra
{

namespace implementation
{

class memory;
class buffer;

/// \namespace handlers
/// \brief All the implementations of the data handlers
///         are defined in this namespace.
///
///////////////////////////////////////////////////////////
namespace handlers
{

///
/// \brief The readingDataHandler parses the content of
///        a tag's buffer and makes it accessible as
///        numeric value, string, time or age.
///
///////////////////////////////////////////////////////////
class readingDataHandler
{
public:
    ///
    /// \brief Constructor.
    /// \param The data type of the tag exposed by this
    ///         readingDataHandler.
    ///
    ///////////////////////////////////////////////////////////
    readingDataHandler(tagVR_t dataType);

    virtual ~readingDataHandler();

    tagVR_t getDataType() const;

    /// \brief Retrieve the data handler's local buffer buffer
    ///         size (in elements).
    ///
    /// @return the buffer's size in elements
    ///
    ///////////////////////////////////////////////////////////
    virtual size_t getSize() const = 0;

    /// \brief Retrieve the buffer's element referenced by the
    ///         zero-based index specified in the parameter and
    ///         returns it as a signed long value.
    ///
    /// @param index   the zero base index of the buffer's
    ///                 element to retrieve
    /// @return the value of the data element referenced by
    ///          the index, transformed into a signed long, or
    ///          0 if the index is out of range
    ///
    ///////////////////////////////////////////////////////////
    virtual std::int32_t getSignedLong(const size_t index) const = 0;

    /// \brief Retrieve the buffer's element referenced by the
    ///         zero-based index specified in the parameter and
    ///         returns it as an unsigned long value.
    ///
    /// @param index   the zero base index of the buffer's
    ///                 element to retrieve
    /// @return the value of the data element referenced by
    ///          the index, transformed into an unsigned long,
    ///          or 0 if the index is out of range
    ///
    ///////////////////////////////////////////////////////////
    virtual std::uint32_t getUnsignedLong(const size_t index) const = 0;

    /// \brief Retrieve the buffer's element referenced by the
    ///         zero-based index specified in the parameter and
    ///         returns it as a double floating point value.
    ///
    /// @param index   the zero base index of the buffer's
    ///                 element to retrieve
    /// @return the value of the data element referenced by
    ///          the index, transformed into a double floating
    ///          point, or 0 if the index is out of range
    ///
    ///////////////////////////////////////////////////////////
    virtual double getDouble(const size_t index) const = 0;

    /// \brief Retrieve the buffer's element referenced by the
    ///         zero-based index specified in the parameter and
    ///         returns it as a string value.
    ///
    /// @param index   the zero base index of the buffer's
    ///                 element to retrieve
    /// @return the value of the data element referenced by
    ///          the index, transformed into a string, or
    ///          0 if the index is out of range
    ///
    ///////////////////////////////////////////////////////////
    virtual std::string getString(const size_t index) const = 0;

    /// \brief Retrieve the buffer's element referenced by the
    ///         zero-based index specified in the parameter and
    ///         returns it as an unicode string value.
    ///
    /// @param index   the zero base index of the buffer's
    ///                 element to retrieve
    /// @return the value of the data element referenced by
    ///          the index, transformed into an unicode string,
    ///          or 0 if the index is out of range
    ///
    ///////////////////////////////////////////////////////////
    virtual std::wstring getUnicodeString(const size_t index) const = 0;

    /// \brief Retrieve the buffer's element referenced by the
    ///         zero-based index specified in the parameter and
    ///         returns it as a date/time value.
    ///
    /// @param index   the zero base index of the buffer's
    ///                 element to retrieve
    /// @param pYear   a pointer to a value that will be filled
    ///                 with the UTC date's year
    /// @param pMonth  a pointer to a value that will be filled
    ///                 with the UTC date's month
    /// @param pDay    a pointer to a value that will be filled
    ///                 with the UTC date's day of the month
    /// @param pHour   a pointer to a value that will be filled
    ///                 with the UTC hour
    /// @param pMinutes a pointer to a value that will be
    ///                 filled with the UTC minutes
    /// @param pSeconds a pointer to a value that will be
    ///                 filled with the UTC seconds
    /// @param pNanoseconds a pointer to a value that will be
    ///                 filled with the UTC nanosecods
    /// @param pOffsetHours a pointer to a value that will be
    ///                 filled with the difference between the
    ///                 date time zone and the UTC time zone
    /// @param pOffsetMinutes a pointer to a value that will be
    ///                 filled with the difference between the
    ///                 date time zone and the UTC time zone
    ///
    ///////////////////////////////////////////////////////////
    virtual void getDate(const size_t index,
        std::uint32_t* pYear,
        std::uint32_t* pMonth,
        std::uint32_t* pDay,
        std::uint32_t* pHour,
        std::uint32_t* pMinutes,
        std::uint32_t* pSeconds,
        std::uint32_t* pNanoseconds,
        std::int32_t* pOffsetHours,
        std::int32_t* pOffsetMinutes) const;

    /// \brief Retrieve the age value and its unit from the
    ///         buffer handled by this handler.
    ///
    /// @param index the zero based index of the age value to
    ///               modify
    /// @param pUnit a pointer to a ageUnit_t that will be
    ///               filled with the unit information related
    ///               to the returned value
    /// @return the age, expressed in the unit written in the
    ///               location referenced by the parameter
    ///               pUnit
    ///
    ///////////////////////////////////////////////////////////
    virtual std::uint32_t getAge(const size_t index, ageUnit_t* pUnit) const;

private:
    const tagVR_t m_dataType;
};

class writingDataHandler
{
public:
    writingDataHandler(const std::shared_ptr<buffer>& pBuffer, tagVR_t dataType, const std::uint8_t paddingByte);

    virtual ~writingDataHandler();

    tagVR_t getDataType() const;

    virtual size_t getSize() const = 0;

    /// \brief Set the buffer's size, in elements.
    ///
    /// The function resize the data handler's local buffer
    ///  to the requested number of elements.
    ///
    /// @param elementsNumber the requested buffer's size,
    ///                        in data elements
    ///
    ///////////////////////////////////////////////////////////
    virtual void setSize(const size_t elementsNumber) = 0;

    /// \brief Set the buffer's element referenced by the
    ///         zero-based index specified in the parameter
    ///         to a date/time value.
    ///
    /// @param index   the zero base index of the buffer's
    ///                 element to be set
    /// @param year   the UTC date's year
    /// @param month  the UTC date's month
    /// @param day    the UTC date's day of the month
    /// @param hour   the UTC hour
    /// @param minutes the UTC minutes
    /// @param seconds the UTC seconds
    /// @param nanoseconds the UTC nanosecods
    /// @param offsetHours the difference between the date time
    ///                zone and the UTC time zone
    /// @param offsetMinutes the difference between the date
    ///                time zone and the UTC time zone
    ///
    ///////////////////////////////////////////////////////////
    virtual void setDate(const size_t index,
        std::uint32_t year,
        std::uint32_t month,
        std::uint32_t day,
        std::uint32_t hour,
        std::uint32_t minutes,
        std::uint32_t seconds,
        std::uint32_t nanoseconds,
        std::int32_t offsetHours,
        std::int32_t offsetMinutes);

    /// \brief Set the value of the age string and specify
    ///         its unit.
    ///
    /// @param index the zero based index of the age value to
    ///               read
    /// @param age   the age to be written in the buffer
    /// @param unit  the units used for the parameter age
    ///
    ///////////////////////////////////////////////////////////
    virtual void setAge(const size_t index, const std::uint32_t age, const ageUnit_t unit);

    /// \brief Set the buffer's element referenced by the
    ///         zero-based index specified in the parameter
    ///         to a signed long value.
    ///
    /// @param index   the zero base index of the buffer's
    ///                 element to be set
    /// @param value the value to write into the
    ///				  %data element.
    ///
    ///////////////////////////////////////////////////////////
    virtual void setSignedLong(const size_t index, const std::int32_t value) =0;

    /// \brief Set the buffer's element referenced by the
    ///         zero-based index specified in the parameter
    ///         to an unsigned long value.
    ///
    /// @param index   the zero base index of the buffer's
    ///                 element to be set
    /// @param value the value to write into the
    ///				  %data element.
    ///
    ///////////////////////////////////////////////////////////
    virtual void setUnsignedLong(const size_t index, const std::uint32_t value) =0;

    /// \brief Set the buffer's element referenced by the
    ///         zero-based index specified in the parameter
    ///         to a double floating point value.
    ///
    /// @param index   the zero base index of the buffer's
    ///                 element to be set
    /// @param value the value to write into the
    ///				  %data element.
    ///
    ///////////////////////////////////////////////////////////
    virtual void setDouble(const size_t index, const double value) =0;

    /// \brief Set the buffer's element referenced by the
    ///         zero-based index specified in the parameter
    ///         to a string value. See also setUnicodeString().
    ///
    /// @param index   the zero base index of the buffer's
    ///                 element to be set
    /// @param value the value to write into the
    ///				  %data element.
    ///////////////////////////////////////////////////////////
    virtual void setString(const size_t index, const std::string& value) =0;

    /// \brief Set the buffer's element referenced by the
    ///         zero-based index specified in the parameter
    ///         to a string value. See also setString().
    ///
    /// @param index   the zero base index of the buffer's
    ///                 element to be set
    /// @param value the value to write into the %data
    ///				  element.
    ///////////////////////////////////////////////////////////
    virtual void setUnicodeString(const size_t index, const std::wstring& value) =0;

protected:
    const tagVR_t m_dataType;

    // Pointer to the connected buffer
    ///////////////////////////////////////////////////////////
    std::shared_ptr<buffer> m_buffer;

    const std::uint8_t m_paddingByte;
};

///
/// \brief Throws an exception if the content of the strings container
///        does not respect the rules (max size, unit size,
///        no multiple strings if separator is not allowed).
///
template <typename stringContainer>
void validateStringContainer(const stringContainer& strings, size_t maxSize, size_t unitSize, bool bSeparator)
{
    IMEBRA_FUNCTION_START();

    if(!bSeparator && strings.size() > 1)
    {
        IMEBRA_THROW(DataHandlerInvalidDataError, "Multiple strings not allowed");
    }

    if(unitSize != 0)
    {
        for(size_t scanStrings(0); scanStrings != strings.size(); ++scanStrings)
        {
            if(strings.at(scanStrings).size() != unitSize)
            {
                IMEBRA_THROW(DataHandlerInvalidDataError, "Strings must be " << unitSize << " bytes long");
            }
        }
    }

    if(maxSize != 0)
    {
        for(size_t scanStrings(0); scanStrings != strings.size(); ++scanStrings)
        {
            if(strings.at(scanStrings).size() > maxSize)
            {
                IMEBRA_THROW(DataHandlerInvalidDataError, "Element size is " << strings.at(scanStrings).size() << " but should be maximum " << maxSize << " bytes");
            }
        }
    }

    IMEBRA_FUNCTION_END();
}


} // namespace handlers

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraDataHandler_6F85D344_DEF8_468d_BF73_AC5BB17FD22A__INCLUDED_)
