/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file baseStream.h
    \brief Declaration of the the base class for the streams (memory, file, ...)
            used by the imebra library.

*/

#if !defined(imebraBaseStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
#define imebraBaseStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_

#include <memory>
#include "exceptionImpl.h"
#include <vector>
#include <map>
#include <stdexcept>


namespace imebra
{

namespace implementation
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This class represents an input stream.
///
/// Specialized classes derived from this class can
///  read from files stored on the computer's disks, on the
///  network or in memory.
///
/// The application can read or write into the stream
///  by using the streamReader or the streamWriter.
///
/// While this class can be used across several threads,
///  the streamReader and the streamWriter can be used in
///  one thread only. This is not a big deal, since one
///  stream can be connected to several streamReaders and
///  streamWriters.
///
/// The library supplies two specialized streams derived
///  from this class:
/// - imebra::stream (used to read or write into physical
///    files)
/// - imebra::memoryStream (used to read or write into
///    imebra::memory objects)
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class baseStreamInput
{

public:
    virtual ~baseStreamInput();

	/// \brief Read raw data from the stream.
	///
	/// The function is multithreading-safe and is called by
	///  the streamReader class when its buffer has to be
	///  refilled.
	///
	/// @param startPosition  the position in the file from
	///                        which the data has to be read
	/// @param pBuffer        a pointer to the memory where the
	///                        read data has to be placed
	/// @param bufferLength   the number of bytes to read from
	///                        the file
	/// @return the number of bytes read from the file. When
	///          it is 0 then the end of the file has been
	///          reached
	///
	///////////////////////////////////////////////////////////
    virtual size_t read(size_t startPosition, std::uint8_t* pBuffer, size_t bufferLength) = 0;
};


class baseStreamOutput
{

public:
    virtual ~baseStreamOutput();

    /// \brief Writes raw data into the stream.
    ///
    /// The function is multithreading-safe and is called by
    ///  the streamWriter class when its buffer has to be
    ///  flushed.
    ///
    /// @param startPosition  the position in the file where
    ///                        the data has to be written
    /// @param pBuffer        pointer to the data that has to
    ///                        be written
    /// @param bufferLength   number of bytes in the data
    ///                        buffer that has to be written
    ///
    ///////////////////////////////////////////////////////////
    virtual void write(size_t startPosition, const std::uint8_t* pBuffer, size_t bufferLength) = 0;

};

} // namespace implementation

} // namespace imebra


#endif // !defined(imebraBaseStream_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
