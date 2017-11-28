/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file data.h
    \brief Declaration of class used to store a single tag.

*/

#if !defined(imebraData_20A41D11_C650_410b_B3AA_CD5B3FF63222__INCLUDED_)
#define imebraData_20A41D11_C650_410b_B3AA_CD5B3FF63222__INCLUDED_

#include <memory>

#include "charsetsListImpl.h"
#include "dataHandlerNumericImpl.h"
#include "../include/imebra/definitions.h"

#include <map>
#include <vector>
#include <mutex>

namespace imebra
{
	class streamReader;
	class streamWriter;

namespace implementation
{

class buffer;
class dataSet;


/// \addtogroup group_dataset
///
/// @{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
/// \brief Represents a single Dicom Tag.
///
/// Dicom tags are stored into a dataSet object.
/// Each tag can be referred by its group ID and tag ID.
///
/// The class data uses the buffer objects to store its
///  %data.
///
/// Each tag can have several buffers (tags defined as
///  sequence in the dicom stream).
///
/// Your application can access to the tag's buffers
///  using a data handler (call getDataHandler()).
/// 
///
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class data
{
public:

    data(tagVR_t tagVR, const charsetsList::tCharsetsList& defaultCharsets);

	///////////////////////////////////////////////////////////
	/// \name Data handlers
	///
	/// Retrieve dataHandler, dataHandlerRaw or a baseStream
	///  that allow to read or modify the buffer's data
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Returns the number of buffers in the tag.
	///
	/// @return the number of buffers in the tag
	///
	///////////////////////////////////////////////////////////
    size_t getBuffersCount() const;

	/// \brief Returns true if the specified buffer exists,
	///         otherwise it returns false.
	///
	/// The buffer can be created by requesting a writing
	///  handler (see handlers::dataHandler).
	///
	/// @param bufferId the zero-based buffer's id the 
	///                 function has to check for
	/// @return true if the buffer exists, false otherwise
	///
	///////////////////////////////////////////////////////////
    bool bufferExists(size_t bufferId) const;

	/// \brief Returns the size of a buffer, in bytes.
	///
	/// If the buffer doesn't exist then returns 0.
	/// See buffer::getBufferSizeBytes() for more info.
	///
	/// @param bufferId the zero-based buffer's id the 
	///                 function has to check for
	/// @return the buffer's size in bytes, or 0 if the buffer
	///          doesn't exist.
	///
	///////////////////////////////////////////////////////////
    size_t getBufferSize(size_t bufferId) const;

    std::shared_ptr<buffer> getBuffer(size_t bufferId) const;

    std::shared_ptr<buffer> getBufferCreate(size_t bufferId);

	/// \brief Get a data handler for the specified buffer.
	///
	/// handlers::dataHandler can be used to read, write 
	///  and resize the buffer.
	///
	/// handlers::dataHandler manages data types depending 
	///  on the buffer's data type: if your application
	///  has to work directly on the buffer's bytes,
	///  then it should obtain a handlers::dataHandlerRaw by
	///  calling getDataHandlerRaw().
	///
	/// Usually a tag has only one buffer (bufferId = 0), 
	///  but when a tag is a sequence or has an undefined
	///  length, then it could store more than one buffer;
	/// in this case your application has to specify the
	///  id of the buffer for which the handlers::dataHandler
	///  is requested.
	///
	/// The application can also specify a data type to use
	///  when a buffer doesn't exist yet: in this case a buffer
	///  with the specified data type will be allocated.
	///
	/// The returned handlers::dataHandler works on a local 
	///  copy of the buffer's data, then it doesn't have to 
	///  worry about multithreading accesses to the buffer's
	///  data.
	///
	/// @param bufferId the zero-based buffer's id the data
	///                 handler must use.
	/// @param bWrite   true if the application wants to write
	///                 something into the buffer
	/// @param tagVR a string that specifies the data
	///                  type to use for the new buffers.
	/// @return a pointer to the data handler for the
	///         requested buffer.
	///
	///////////////////////////////////////////////////////////
    std::shared_ptr<handlers::readingDataHandler> getReadingDataHandler(size_t bufferId) const;

    std::shared_ptr<handlers::writingDataHandler> getWritingDataHandler(size_t bufferId);
	
	/// \brief Get a raw data handler 
	///         (handlers::dataHandlerRaw) for the specified 
	///         buffer.
	///
	/// The raw data handler can be used to read, write and
	///  resize the buffer.
	///
	/// A raw data handler (handlers::dataHandlerRaw) always 
	///  works on a collection of bytes, no matter what the 
	///  buffer's data type is, while the normal 
	///  handlers::dataHandler works on data types depending 
	///  on the buffer's data type.
	///  
	/// A handlers::dataHandler can be obtained by calling
	///  getDataHandler().
	///
	/// Usually a tag has only one buffer (bufferId = 0), 
	///  but when a tag is a sequence or has an undefined
	///  length, then it could store more than one buffer;
	/// in this case your application has to specify the
	///  id of the buffer for which the data handler is
	///  requested.
	///
	/// The application can also specify a data type to use
	///  when a buffer doesn't exist yet: in this case a buffer
	///  with the specified data type will be allocated.
	///
	/// The returned handlers::dataHandlerRaw works on a local 
	///  copy of the buffer's data, then it doesn't have to 
	///  worry about multithreading accesses to the buffer's
	///  data.
	///
	/// @param bufferId the zero-based buffer's id the data
	///                 handler must use.
	/// @param bWrite   true if the application wants to write
	///                 something into the buffer
	/// @param tagVR a string that specifies the data
	///                  type to use for the new buffers.
	/// @return a pointer to the data handler for the
	///         requested buffer.
	///
	///////////////////////////////////////////////////////////
    std::shared_ptr<handlers::readingDataHandlerRaw> getReadingDataHandlerRaw(size_t bufferId) const;

    std::shared_ptr<handlers::writingDataHandlerRaw> getWritingDataHandlerRaw(size_t bufferId);

    std::shared_ptr<handlers::readingDataHandlerNumericBase> getReadingDataHandlerNumeric(size_t bufferId) const;

    std::shared_ptr<handlers::writingDataHandlerNumericBase> getWritingDataHandlerNumeric(size_t bufferId);

    /// \brief Get a streamReader connected to a buffer's data.
	///
	/// @param bufferId   the id of the buffer for which the
	///                    streamReader is required. This 
	///                    parameter is usually 0
	/// @return           the streamReader connected to the 
	///                    buffer's data.
	///
	///////////////////////////////////////////////////////////
    std::shared_ptr<streamReader> getStreamReader(size_t bufferId);

	/// \brief Get a streamWriter connected to a buffer's data.
	///
	/// @param bufferId   the id of the buffer for which the
	///                    streamWriter is required. This 
	///                    parameter is usually 0
	/// @param dataType   the datatype used to create the 
	///                    buffer if it doesn't exist already
	/// @return           the streamWriter connected to the 
	///                    emptied buffer's data.
	///
	///////////////////////////////////////////////////////////
    std::shared_ptr<streamWriter> getStreamWriter(size_t bufferId);

	//@}


	///////////////////////////////////////////////////////////
	/// \name Embedded data sets.
	///
	/// Get or set an embedded dataSet
	///
	///////////////////////////////////////////////////////////
	//@{

	/// \brief Retrieve an embedded data set.
	///
	/// Sequence tags (dicom type=SQ) store embedded dicom 
	///  structures inside.
	///
	/// Sequences allow to nest several dicom structures.
	/// Sequences are used by dicom directories and by some
	///  tags in the normal dataSet.
	///
	/// @param dataSetId  the ID of the dataSet to retrieve.
	///                   Several data sets can be embedded
	///                   in the tag.
	///                   The first dataSet's ID is zero.
	/// @return           a pointer to the retrieved dataSet
	///
	///////////////////////////////////////////////////////////
    std::shared_ptr<dataSet> getSequenceItem(size_t dataSetId) const;

    bool dataSetExists(size_t dataSetId) const;

	/// \brief Set an embedded dataSet to the sequence.
	///
	/// Several data sets can be nested one inside each other.
	/// When a data set is embedded into a tag, then the
	///  tag will have a sequence type (dicom type=SQ).
	///
	/// Sequences are used to build dicom directories and
	///  other tags in normal data sets.
	///
	/// @param dataSetId  The Id of the dataSet inside the tag.
	///                   Tags can store several data sets.
	///                   The first dataSet has an ID of zero
	/// @param pDataSet   A pointer to the dataSet to store
	///                   into the tag
	///
	///////////////////////////////////////////////////////////
    void setSequenceItem(size_t dataSetId, std::shared_ptr<dataSet> pDataSet);

	/// \brief Append an embedded dataSet to the sequence.
	///
	/// Several data sets can be nested one inside each other.
	/// When a data set is embedded into a tag, then the
	///  tag will have a sequence type (dicom type=SQ).
	///
	/// Sequences are used to build dicom directories and
	///  other tags in normal data sets.
	///
	/// @param pDataSet   A pointer to the dataSet to store
	///                   into the tag
	///
	///////////////////////////////////////////////////////////
	void appendDataSet(std::shared_ptr<dataSet> pDataSet);
	
	//@}


	///////////////////////////////////////////////////////////
	/// \name Buffer's data type
	///
	///////////////////////////////////////////////////////////
	//@{

    /// \brief Get the tag's buffer type.
	///
    /// @return the buffer's data type
	///
	///////////////////////////////////////////////////////////
    tagVR_t getDataType() const;

	//@}

    virtual void setCharsetsList(const charsetsList::tCharsetsList& charsetsList);
    virtual void getCharsetsList(charsetsList::tCharsetsList* pCharsetsList) const;

    // Set a buffer
    ///////////////////////////////////////////////////////////
    void setBuffer(size_t bufferId, const std::shared_ptr<buffer>& newBuffer);

protected:

    charsetsList::tCharsetsList m_charsetsList;

    const tagVR_t m_tagVR;

	// Pointers to the internal buffers
	///////////////////////////////////////////////////////////
    typedef std::map<size_t, std::shared_ptr<buffer> > tBuffersMap;
	tBuffersMap m_buffers;

	// Pointers to the embedded datasets
	///////////////////////////////////////////////////////////
	typedef std::shared_ptr<dataSet> ptrDataSet;
	typedef std::vector<ptrDataSet> tEmbeddedDatasetsMap;
	tEmbeddedDatasetsMap m_embeddedDataSets;

    mutable std::mutex m_mutex;
};

/// @}

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraData_20A41D11_C650_410b_B3AA_CD5B3FF63222__INCLUDED_)
