/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file buffer.h
    \brief Declaration of the buffer class.

*/

#if !defined(imebraBuffer_DE3F98A9_664E_47c0_A29B_B681F9AEB118__INCLUDED_)
#define imebraBuffer_DE3F98A9_664E_47c0_A29B_B681F9AEB118__INCLUDED_

#include "streamControllerImpl.h"
#include "memoryImpl.h"
#include "dataHandlerNumericImpl.h"
#include "../include/imebra/definitions.h"

#include "charsetsListImpl.h"
#include <mutex>

namespace imebra
{

namespace implementation
{

class streamReader;
class streamWriter;
class memory;

/// \addtogroup group_dataset
///
/// @{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief This class manages a memory area containing
///         data in dicom format.
///
/// A buffer also knows the data type of the
///  elements it stores.
/// The data type is in Dicom format (two upper case
///  chars).
///
/// The memory can be accessed through a 
///  \ref handlers::dataHandler derived object
///  obtained by calling the function getDataHandler().
///  
/// Data handlers work on a copy of the buffer, so most
///  of the problem related to the multithreading
///  enviroments are avoided.
///
/// The data handlers supply several functions that
///  allow to access to the data in several formats
///  (strings, numeric, time, and so on).
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class buffer: public std::enable_shared_from_this<buffer>
{

public:
	///////////////////////////////////////////////////////////
	/// \name Constructor
	///
	///////////////////////////////////////////////////////////
	//@{

    /// \brief Constructor.
	///
	///////////////////////////////////////////////////////////
    buffer();

	/// \brief Constructor. Initialize the buffer object and
	///         declare the buffer's content on demand.
	///
	/// On demand content is loaded from the original stream
	///  when the application requires the access to the
	///  buffer.
	///
	/// @param originalStream the stream from which the content
	///                      can be read
	/// @param bufferPosition the first stream's byte that 
	///                      contains the buffer's content
	/// @param bufferLength the buffer's content length, in
	///                      bytes
	/// @param wordLength   the size of a buffer's element,
	///                      in bytes
	/// @param endianType   the stream's endian type
	///
	///////////////////////////////////////////////////////////
    buffer(
        const std::shared_ptr<baseStreamInput>& originalStream,
        size_t bufferPosition,
        size_t bufferLength,
        size_t wordLength,
		streamController::tByteOrdering endianType);

	//@}

	///////////////////////////////////////////////////////////
	/// \name Data handlers
	///
	///////////////////////////////////////////////////////////
	//@{
public:
	/// \brief Retrieve a data handler that can be used to
	///         read, write and resize the memory controlled by 
	///         the buffer.
	///
	/// The data handler will have access to a local copy
	///  of the buffer, then it will not have to worry about
	///  multithreading related problems.
	/// If a writing handler is requested, then the handler's
	///  local buffer will be copied back into the buffer when
	///  the handler will be destroyed.
	///
	/// @param bWrite set to true if you want to write into
	///                the buffer
	/// @param size   this parameter is used only when the
	///                parameter bWrite is set to true and the
	///                buffer is empty: in this case, the
	///                returned buffer will be resized to the
	///                number of elements (NOT bytes) declared
	///                in this parameter
	/// @return a pointer to a dataHandler object
	///
	///////////////////////////////////////////////////////////
    std::shared_ptr<handlers::readingDataHandler> getReadingDataHandler(tagVR_t tagVR) const;

    std::shared_ptr<handlers::writingDataHandler> getWritingDataHandler(tagVR_t tagVR, std::uint32_t size = 0);

	/// \brief Retrieve a raw data handler that can be used to
	///         read, write and resize the memory controlled by 
	///         the buffer.
	///
	/// Raw data handlers always see a collection of bytes,
	///  regardless of the original buffer's type.
	///
	/// The data handler will have access to a local copy
	///  of the buffer, then it will not have to worry about
	///  multithreading related problems.
	/// If a writing handler is requested, then the handler's
	///  local buffer will be copied back into the buffer when
	///  the handler will be destroyed.
	///
	/// @param bWrite set to true if you want to write into
	///                the buffer
	/// @param size   this parameter is used only when the
	///                parameter bWrite is set to true and the
	///                buffer is empty: in this case, the
	///                returned buffer will be resized to the
	///                number of bytes declared in this
	///                parameter
	/// @return a pointer to a dataHandler object
	///
	///////////////////////////////////////////////////////////
    std::shared_ptr<handlers::readingDataHandlerRaw> getReadingDataHandlerRaw(tagVR_t tagVR) const;

    std::shared_ptr<handlers::writingDataHandlerRaw> getWritingDataHandlerRaw(tagVR_t tagVR, std::uint32_t size = 0);

    std::shared_ptr<handlers::readingDataHandlerNumericBase> getReadingDataHandlerNumeric(tagVR_t tagVR) const;

    std::shared_ptr<handlers::writingDataHandlerNumericBase> getWritingDataHandlerNumeric(tagVR_t tagVR, std::uint32_t size = 0);
    //@}

    /// \brief Add a new block of memory to the current data.
    ///
    /// The appended block of memory should not be modified
    /// after it has been appended.
    ///
    /// @param pMemory the memory to append
    ///
    ///////////////////////////////////////////////////////////
    void appendMemory(std::shared_ptr<const memory> pMemory);


	///////////////////////////////////////////////////////////
	/// \name Stream
	///
	///////////////////////////////////////////////////////////
	//@{
	
	/// \brief Return the current buffer's size in bytes
	///
	/// If the buffer is currently loaded then return the
	///  memory's size, otherwise return the size that the
	///  buffer would have when it is loaded.
	///
	/// @return the buffer's size, in bytes
	///////////////////////////////////////////////////////////
    size_t getBufferSizeBytes() const;

	//@}


	///////////////////////////////////////////////////////////
	/// \name Stream
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Return a stream reader connected to the 
	///         buffer's content.
	///
	/// @return          a pointer to a stream reader
	///
	///////////////////////////////////////////////////////////
	std::shared_ptr<streamReader> getStreamReader();

	/// \brief Return a stream writer connected to the 
	///         buffer's content.
	///
	/// The stream works on a local copy of the buffer's data,
	///  then it doesn't have to worry about multithreading
	///  related problems.
	///
	/// @return          a pointer to a stream writer
	///
	///////////////////////////////////////////////////////////
    std::shared_ptr<streamWriter> getStreamWriter(tagVR_t tagVR);

	//@}

    void commit(std::shared_ptr<memory> newMemory, const charsetsList::tCharsetsList& newCharsetsList);

    void commit(std::shared_ptr<memory> newMemory);

	///////////////////////////////////////////////////////////
	/// \name Charsets
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Defines the charsets that should be used by
	///         the object.
	///
	/// The valid charsets are:
	/// - ""
    /// - "ISO_IR 6"
	/// - "ISO_IR 100"
	/// - "ISO_IR 101"
	/// - "ISO_IR 109"
	/// - "ISO_IR 110"
	/// - "ISO_IR 144"
	/// - "ISO_IR 127"
	/// - "ISO_IR 126"
	/// - "ISO_IR 138"
	/// - "ISO_IR 148"
	/// - "ISO_IR 13"
	/// - "ISO_IR 166"
	/// - "ISO 2022 IR 6"
	/// - "ISO 2022 IR 100"
	/// - "ISO 2022 IR 101"
	/// - "ISO 2022 IR 109"
	/// - "ISO 2022 IR 110"
	/// - "ISO 2022 IR 144"
	/// - "ISO 2022 IR 127"
	/// - "ISO 2022 IR 126"
	/// - "ISO 2022 IR 138"
	/// - "ISO 2022 IR 148"
	/// - "ISO 2022 IR 13"
	/// - "ISO 2022 IR 166"
	/// - "ISO 2022 IR 87"
	/// - "ISO 2022 IR 159"
	/// - "ISO 2022 IR 149"
	/// - "ISO_IR 192" (UTF-8)
	/// - "GB18030"
	///
	/// @param pCharsetsList  a list of charsets that can be
	///                        used by the dicom object.
	///                       The default charsets must be 
	///                        the first item in the list
	///
	///////////////////////////////////////////////////////////
    virtual void setCharsetsList(const charsetsList::tCharsetsList& charsets);
	
	/// \brief Retrieve the charsets used by the dicom object.
	///
	/// If during the operation an error is detected (diffetent
	///  objects use different default charsets) then
	///  the exception charsetListExceptionDiffDefault is 
	///  thrown.
	///
	/// @param pCharsetsList  a pointer to a list that will
	///                        be filled with the used 
	///                        charsets
	///
	///////////////////////////////////////////////////////////
    virtual void getCharsetsList(charsetsList::tCharsetsList* pCharsetsList) const;

	//@}

protected:

    /// \brief Returns a memory block containing the buffer
    ///        data.
    ///
    /// If a lazy load is enabled and the data is available on
    /// a stream then load the data into a temporary block of
    /// memory and return it.
    ///
    /// @return a block of memory containing the buffer's data
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<const memory> getLocalMemory() const;

    /// \brief Join all the appended memory blocks into a
    ///        single block.
    ///
    /// @return a single memory block containing all the
    ///         appended data
    ///
    ///////////////////////////////////////////////////////////
    std::shared_ptr<const memory> joinMemory() const;

	//
	// Attributes
	//
	///////////////////////////////////////////////////////////
private:
	// The memory buffer
	///////////////////////////////////////////////////////////
    mutable std::list<std::shared_ptr<const memory> > m_memory;

    mutable std::mutex m_mutex;

protected:
	// The following variables are used to reread the buffer
	//  from the stream.
	///////////////////////////////////////////////////////////
    std::shared_ptr<baseStreamInput> m_originalStream;    // < Original stream
    size_t m_originalBufferPosition; // < Original buffer's position
    size_t m_originalBufferLength;   // < Original buffer's length
    size_t m_originalWordLength;     // < Original word's length (for low/high endian adjustment)
	streamController::tByteOrdering m_originalEndianType; // < Original endian type
	
private:
	// Charset list
	///////////////////////////////////////////////////////////
	charsetsList::tCharsetsList m_charsetsList;

};


/// @}

} // End of namespace implementation

} // End of namespace imebra

#endif // !defined(imebraBuffer_DE3F98A9_664E_47c0_A29B_B681F9AEB118__INCLUDED_)
