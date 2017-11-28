%module imebra

#ifdef SWIGJAVA
	%include <arrays_java.i>
	%include <enums.swg>

	%apply(char *STRING, size_t LENGTH) { (const char *source, size_t sourceSize) };
	%apply(char *STRING, size_t LENGTH) { (char* destination, size_t destinationSize) };
#endif
#ifdef SWIGPYTHON
	%include <carrays.i>
	%include <pybuffer.i>
	%pybuffer_mutable_binary(void *STRING, size_t LENGTH)
	%apply(void *STRING, size_t LENGTH) { (const char *source, size_t sourceSize) };
	%apply(void *STRING, size_t LENGTH) { (char* destination, size_t destinationSize) };
#endif

%rename(assign) operator=;

#define IMEBRA_API

%{
#include <imebra/imebra.h>
%}

%include <std_string.i>
%include <std_wstring.i>

%include <exception.i>
%include <stdint.i>
%include <std_except.i>
%include <std_vector.i>
%include <std_map.i>

%template(FileParts) std::vector<std::string>;
%template(Groups) std::vector<std::uint16_t>;
%template(TagsIds) std::vector<imebra::TagId>;
%template(VOIs) std::vector<imebra::VOIDescription>;

// Declare which methods return an object that should be
// managed by the client.
////////////////////////////////////////////////////////
%newobject imebra::CodecFactory::load;

%newobject imebra::ColorTransformsFactory::getTransform;

%newobject imebra::DataSet::getTag;
%newobject imebra::DataSet::getTagCreate;
%newobject imebra::DataSet::getImage;
%newobject imebra::DataSet::getImageApplyModalityTransform;
%newobject imebra::DataSet::getSequenceItem;
%newobject imebra::DataSet::getLUT;
%newobject imebra::DataSet::getReadingDataHandler;
%newobject imebra::DataSet::getWritingDataHandler;
%newobject imebra::DataSet::getReadingDataHandlerNumeric;
%newobject imebra::DataSet::getWritingDataHandlerNumeric;
%newobject imebra::DataSet::getReadingDataHandlerRaw;
%newobject imebra::DataSet::getWritingDataHandlerRaw;
%newobject imebra::DataSet::getAge;
%newobject imebra::DataSet::getDate;

%newobject imebra::DicomDir::getNewEntry;
%newobject imebra::DicomDir::getFirstRootEntry;
%newobject imebra::DicomDir::updateDataSet;

%newobject imebra::DicomDirEntry::getEntryDataSet;
%newobject imebra::DicomDirEntry::getNextEntry;
%newobject imebra::DicomDirEntry::getFirstChildEntry;

%newobject imebra::DrawBitmap::getBitmap;

%newobject imebra::Image::getReadingDataHandler;
%newobject imebra::Image::getWritingDataHandler;

%newobject imebra::LUT::getReadingDataHandler;
%newobject imebra::LUT::getReadingDataHandler;
%newobject imebra::LUT::getReadingDataHandler;
%newobject imebra::LUT::getReadingDataHandler;

%newobject imebra::ReadingDataHandlerNumeric::getMemory;

%newobject imebra::Tag::getReadingDataHandler;
%newobject imebra::Tag::getWritingDataHandler;
%newobject imebra::Tag::getReadingDataHandlerNumeric;
%newobject imebra::Tag::getReadingDataHandlerRaw;
%newobject imebra::Tag::getWritingDataHandlerNumeric;
%newobject imebra::Tag::getWritingDataHandlerRaw;
%newobject imebra::Tag::getStreamReader;
%newobject imebra::Tag::getStreamWriter;
%newobject imebra::Tag::getSequenceItem;

%newobject imebra::Transform::allocateOutputImage;

%newobject imebra::WritingDataHandlerNumeric::getMemory;


%exception {
    try {
        $action
    } catch(const imebra::MissingDataElementError& e) {
        std::string error(imebra::ExceptionsManager::getExceptionTrace());
        SWIG_exception(SWIG_IndexError, error.c_str());
    } catch(const imebra::LutError& e) {
        std::string error(imebra::ExceptionsManager::getExceptionTrace());
        SWIG_exception(SWIG_RuntimeError, error.c_str());
    } catch(const imebra::StreamError& e) {
        std::string error(imebra::ExceptionsManager::getExceptionTrace());
        SWIG_exception(SWIG_IOError, error.c_str());
    } catch(const imebra::DictionaryError& e) {
        std::string error(imebra::ExceptionsManager::getExceptionTrace());
        SWIG_exception(SWIG_ValueError, error.c_str());
    } catch(const imebra::CharsetConversionError& e) {
        std::string error(imebra::ExceptionsManager::getExceptionTrace());
        SWIG_exception(SWIG_RuntimeError, error.c_str());
    } catch(const imebra::CodecError& e) {
        std::string error(imebra::ExceptionsManager::getExceptionTrace());
        SWIG_exception(SWIG_IOError, error.c_str());
    } catch(const imebra::DataHandlerError& e) {
        std::string error(imebra::ExceptionsManager::getExceptionTrace());
        SWIG_exception(SWIG_ValueError, error.c_str());
    } catch(const imebra::DataSetError& e) {
        std::string error(imebra::ExceptionsManager::getExceptionTrace());
        SWIG_exception(SWIG_ValueError, error.c_str());
    } catch(const imebra::DicomDirError& e) {
        std::string error(imebra::ExceptionsManager::getExceptionTrace());
        SWIG_exception(SWIG_RuntimeError, error.c_str());
    } catch(const imebra::HuffmanError& e) {
        std::string error(imebra::ExceptionsManager::getExceptionTrace());
        SWIG_exception(SWIG_IOError, error.c_str());
    } catch(const imebra::ImageError& e) {
        std::string error(imebra::ExceptionsManager::getExceptionTrace());
        SWIG_exception(SWIG_ValueError, error.c_str());
    } catch(const imebra::TransformError& e) {
        std::string error(imebra::ExceptionsManager::getExceptionTrace());
        SWIG_exception(SWIG_ValueError, error.c_str());
    } catch(const imebra::MemoryError& e) {
        std::string error(imebra::ExceptionsManager::getExceptionTrace());
        SWIG_exception(SWIG_MemoryError, error.c_str());
    } catch(const std::runtime_error& e) {
        std::string error(imebra::ExceptionsManager::getExceptionTrace());
        SWIG_exception(SWIG_RuntimeError, error.c_str());
    } catch(const std::exception& e) {
        std::string error(imebra::ExceptionsManager::getExceptionTrace());
        SWIG_exception(SWIG_RuntimeError, error.c_str());
    }
}


%include "../library/include/imebra/tagsEnumeration.h"
%include "../library/include/imebra/tagId.h"
%include "../library/include/imebra/definitions.h"
%include "../library/include/imebra/readMemory.h"
%include "../library/include/imebra/readWriteMemory.h"
%include "../library/include/imebra/memoryPool.h"
%include "../library/include/imebra/baseStreamInput.h"
%include "../library/include/imebra/baseStreamOutput.h"
%include "../library/include/imebra/streamReader.h"
%include "../library/include/imebra/streamWriter.h"
%include "../library/include/imebra/readingDataHandler.h"
%include "../library/include/imebra/readingDataHandlerNumeric.h"
%include "../library/include/imebra/writingDataHandler.h"
%include "../library/include/imebra/writingDataHandlerNumeric.h"
%include "../library/include/imebra/lut.h"
%include "../library/include/imebra/image.h"
%include "../library/include/imebra/tag.h"
%include "../library/include/imebra/dataSet.h"
%include "../library/include/imebra/codecFactory.h"
%include "../library/include/imebra/transform.h"
%include "../library/include/imebra/transformHighBit.h"
%include "../library/include/imebra/transformsChain.h"
%include "../library/include/imebra/modalityVOILUT.h"
%include "../library/include/imebra/VOILUT.h"
%include "../library/include/imebra/colorTransformsFactory.h"
%include "../library/include/imebra/dicomDirEntry.h"
%include "../library/include/imebra/dicomDir.h"
%include "../library/include/imebra/dicomDictionary.h"
%include "../library/include/imebra/drawBitmap.h"
%include "../library/include/imebra/fileStreamInput.h"
%include "../library/include/imebra/fileStreamOutput.h"
%include "../library/include/imebra/memoryStreamInput.h"
%include "../library/include/imebra/memoryStreamOutput.h"



