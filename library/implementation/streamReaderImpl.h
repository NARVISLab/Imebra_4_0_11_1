/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file streamReader.h
    \brief Declaration of the the class used to read the streams.

*/

#if !defined(imebraStreamReader_F6221390_BC44_4B83_B5BB_3485222FF1DD__INCLUDED_)
#define imebraStreamReader_F6221390_BC44_4B83_B5BB_3485222FF1DD__INCLUDED_

#include "streamControllerImpl.h"
#include "../include/imebra/exceptions.h"
#include <exception>

namespace imebra
{

namespace implementation
{

class JpegEoiFound: public std::runtime_error
{
public:
    JpegEoiFound(const std::string& message): std::runtime_error(message)
    {}
};

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief Represents a stream reader.
///        A stream reader can read data from a stream.
///        Several stream readers can share a single
///        baseStream derived object.
///
/// The stream reader object is not multithread safe, but
///  one single stream can have several streamReader
///  objects (in different threads) connected to it.
///
/// A stream reader can also be connected only to a part
///  of a stream: when this feature is used, then the
///  streamReader's client thinks that he is using a
///  whole stream, while the reader limits its view
///  to allowed stream's bytes only.
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class streamReader: public streamController
{
public:
    /// \brief Build a streamReader and connect it to an
    ///         existing stream.
    ///
    ///////////////////////////////////////////////////////////
    streamReader(std::shared_ptr<baseStreamInput> pControlledStream);

    /// \brief Build a streamReader and connect it to a part
    ///         of an existing stream.
	///
	/// When the streamReader is connected to a part of a
	///  stream then all the its functions will act on the
	///  viewable stream's part only.
	///
	/// @param pControlledStream  the stream that will be
	///                            controlled by the reader
	/// @param virtualStart       the first stream's byte
	///                            visible to the reader
	/// @param virtualLength      the number of bytes visible
	///                            to the reader. A value of 0
	///                            means that all the bytes
	///                            are visible
	///
	///////////////////////////////////////////////////////////
    streamReader(std::shared_ptr<baseStreamInput> pControlledStream, size_t virtualStart, size_t virtualLength);

    std::shared_ptr<baseStreamInput> getControlledStream();

    /// \brief Returns a new streamReader object that starts
    ///        at the current stream location and continues
    ///        for the specified amount of bytes.
    ///
    /// @param virtualLength the amount of bytes that can be
    ///                      read from the new streamReader.
    ///                      The called streamReader will
    ///                      advance the read position past
    ///                      the end position of the new
    ///                      streamReader
    std::shared_ptr<streamReader> getReader(size_t virtualLength);

    /// \brief Read raw data from the stream.
	///
	/// The number of bytes specified in the parameter
	///  bufferLength will be read from the stream and copied
	///  into a buffer referenced by the parameter pBuffer.
	/// The buffer's size must be equal or greater than
	///  the number of bytes to read.
	///
	/// The functions throws a streamExceptionRead
	///  exception if an error occurs.
	///
	/// @param pBuffer   a pointer to the buffer where the
	///                  read data must be copied.
	///                  the buffer's size must be at least
	///                  equal to the size indicated in the
	///                  bufferLength parameter.
	/// @param bufferLength the number of bytes to read from
	///                  the stream.
	///
	///////////////////////////////////////////////////////////
    void read(std::uint8_t* pBuffer, size_t bufferLength);

    size_t readSome(std::uint8_t* pBuffer, size_t bufferLength);

	/// \brief Returns true if the last byte in the stream
	///         has already been read.
	///
	/// @return true if the last byte in the stream has already
	///          been read
	///
	///////////////////////////////////////////////////////////
	bool endReached();

	/// \brief Seek the stream's read position.
	///
	/// The read position is moved to the specified byte in the
	///  stream.
	/// Subsequent calls to the read operations like read(),
	///  readBits(), readBit(), addBit() and readByte() will
	///  read data from the position specified here.
	///
	/// @param newPosition the new position to use for read
	///                   operations, in bytes from the
    ///                   beginning of the stream
	///
	///////////////////////////////////////////////////////////
    void seek(size_t newPosition);

    void seekForward(std::uint32_t newPosition);

	/// \brief Read the specified amount of bits from the
	///         stream.
	///
	/// The functions uses a special bit pointer to keep track
	///  of the bytes that haven't been completly read.
	///
	/// The function throws a streamExceptionRead exception if
	///  an error occurs.
	///
	/// @param bitsNum   the number of bits to read.
	///                  The function can read 32 bits maximum
	/// @return an integer containing the fetched bits, right
	///                   aligned
	///
	///////////////////////////////////////////////////////////
    inline std::uint32_t readBits(size_t bitsNum)
	{
        IMEBRA_FUNCTION_START();

        const size_t bufferSize(8);

		// All the requested bits are already in the buffer.
		// Just return them.
		///////////////////////////////////////////////////////////
		if(bitsNum <= m_inBitsNum)
		{
            std::uint32_t returnValue((m_inBitsBuffer & 0x00ff) >> (bufferSize - bitsNum));
			m_inBitsBuffer <<= bitsNum;
			m_inBitsNum -= bitsNum;
			return returnValue;
		}

		// Fill a local variable with the read bits
		///////////////////////////////////////////////////////////
		std::uint32_t returnValue(0);

		// Some bits are in the bits buffer, copy them and reset
		//  the bits buffer
		///////////////////////////////////////////////////////////
		if(m_inBitsNum != 0)
		{
			bitsNum -= m_inBitsNum;
            returnValue = ((std::uint32_t)((m_inBitsBuffer & 0xff) >> (bufferSize - m_inBitsNum))) << bitsNum;
		}

		// Read the requested number of bits
		///////////////////////////////////////////////////////////
		for(;;)
		{
			if(bitsNum <= 8)
			{
                m_inBitsBuffer = readByte();
				returnValue |= (m_inBitsBuffer >> (bufferSize - bitsNum));
				m_inBitsBuffer <<= bitsNum;
				m_inBitsNum = 8 - bitsNum;
				return returnValue;
			}

			bitsNum -= 8;
			returnValue |= ((std::uint32_t)readByte()) << bitsNum;
		}

        IMEBRA_FUNCTION_END();
    }

	/// \brief Read one bit from the stream.
	///
	/// The returned buffer will store the value 0 or 1,
	///  depending on the value of the read bit.
	///
	/// The function throws a streamExceptionRead if an error
	///  occurs.
	///
	/// @return the value of the read bit (1 or 0)
    ///
	///////////////////////////////////////////////////////////
	inline std::uint32_t readBit()
	{
        IMEBRA_FUNCTION_START();

        if(m_inBitsNum == 0)
        {
            m_inBitsBuffer = readByte();
            m_inBitsNum = 8;
        }
        --m_inBitsNum;
        m_inBitsBuffer <<= 1;
        return (m_inBitsBuffer >> 8) & 1;

		IMEBRA_FUNCTION_END();
	}


	/// \brief Read one bit from the stream and add its value
	///         to the specified buffer.
	///
	/// The buffer pointed by the pBuffer parameter is
	///  left-shifted before the read bit is inserted in the
	///  least significant bit of the buffer.
	///
	/// The function throws a streamExceptionRead if an error
	///  occurs.
	///
	/// @param pBuffer   a pointer to a std::uint32_t value that
	///                   will be left shifted and filled
	///                   with the read bit.
    ///
	///////////////////////////////////////////////////////////
	inline void addBit(std::uint32_t* const pBuffer)
	{
        IMEBRA_FUNCTION_START();

        (*pBuffer) <<= 1;
                
        if(m_inBitsNum == 0)
        {
            m_inBitsBuffer = readByte();
            m_inBitsNum = 8;
        }
        m_inBitsBuffer <<= 1;
        --m_inBitsNum;
        *pBuffer |= (m_inBitsBuffer >> 8) & 1;

		IMEBRA_FUNCTION_END();
	}

	/// \brief Reset the bit pointer used by readBits(),
	///         readBit() and addBit().
	///
	/// A subsequent call to readBits(), readBit and
	///  addBit() will read data from a byte-aligned boundary.
	///
	///////////////////////////////////////////////////////////
	inline void resetInBitsBuffer()
	{
		m_inBitsNum = 0;
	}

	/// \brief Read a single byte from the stream, parsing it
	///         if m_pTagByte is not zero.
	///
	/// The read byte is stored in the buffer pointed by the
	///  parameter pBuffer.
	///
	/// If m_pTagByte is zero, then the function reads a byte
	///  and returns true.
	///
	/// If m_pTagByte is not zero, then the function skips
	///  all the bytes in the stream that have the value 0xFF.
	/// If the function doesn't have to skip any 0xFF bytes,
	///  then the function just read a byte and returns true.
	///
	/// If one or more 0xFF bytes have been skipped, then
	///  the function returns a value depending on the byte
	///  that follows the 0xFF run:
	/// - if the byte is 0, then the function fill the pBuffer
	///    parameter with a value 0xFF and returns true
	/// - if the byte is not 0, then that value is stored in
	///    the location pointed by m_pTagByte and the function
	///    returns false.
	///
	/// This mechanism is used to parse the jpeg tags in a
	///  stream.
	///
	/// @return          the read byte
	///
	///////////////////////////////////////////////////////////
	inline std::uint8_t readByte()
	{
        IMEBRA_FUNCTION_START();

        // Update the data buffer if it is empty
		///////////////////////////////////////////////////////////
        if(m_dataBufferCurrent == m_dataBufferEnd && fillDataBuffer() == 0)
        {
            IMEBRA_THROW(StreamEOFError, "Attempt to read past the end of the file");
        }

		// Read one byte. Return immediatly if the tags are not
		//  activated
		///////////////////////////////////////////////////////////
        if(m_dataBuffer[m_dataBufferCurrent] != 0xff || !m_bJpegTags)
        {
            return m_dataBuffer[m_dataBufferCurrent++];
        }
        do
        {
            if(++m_dataBufferCurrent == m_dataBufferEnd && fillDataBuffer() == 0)
            {
                IMEBRA_THROW(StreamEOFError, "Attempt to read past the end of the file");
            }
        }while(m_dataBuffer[m_dataBufferCurrent] == 0xff);

        if(m_dataBuffer[m_dataBufferCurrent] != 0)
        {
            if(m_dataBuffer[m_dataBufferCurrent] == 0xd9)
            {
                IMEBRA_THROW(JpegEoiFound, "Jpeg End of Image tag found");
            }
            IMEBRA_THROW(StreamJpegTagInStreamError, "Corrupted jpeg stream");
        }
        ++m_dataBufferCurrent;

        return 0xff;

        IMEBRA_FUNCTION_END();
    }

private:
	/// \brief Read data from the file into the data buffer.
	///
	/// The function reads as many bytes as possible until the
	///  data buffer is full or the controlled stream cannot
	///  supply any more byte.
	///
	///////////////////////////////////////////////////////////
    size_t fillDataBuffer();

	/// \brief Read data from the file into the data buffer.
	///
	///////////////////////////////////////////////////////////
    size_t fillDataBuffer(std::uint8_t* pDestinationBuffer, size_t readLength);

private:
    std::shared_ptr<baseStreamInput> m_pControlledStream;

    unsigned int m_inBitsBuffer;
    size_t m_inBitsNum;

};

///@}

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraStreamReader_F6221390_BC44_4B83_B5BB_3485222FF1DD__INCLUDED_)
