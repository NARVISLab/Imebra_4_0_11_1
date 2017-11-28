/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement
 that your application must also be GPL), you may purchase a commercial
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

#include "../include/imebra/exceptions.h"
#include "../implementation/exceptionImpl.h"

namespace imebra
{

std::string ExceptionsManager::getExceptionTrace()
{
    std::string message(implementation::exceptionsManagerGetter::getExceptionsManagerGetter().getExceptionsManager().getMessage());
    return message;
}

LutError::LutError(const std::string& message): std::runtime_error(message)
{}

LutCorruptedError::LutCorruptedError(const std::string& message): LutError(message)
{}

MissingDataElementError::MissingDataElementError(const std::string& message): std::runtime_error(message)
{}

MissingGroupError::MissingGroupError(const std::string& message): MissingDataElementError(message)
{}

MissingTagError::MissingTagError(const std::string& message): MissingDataElementError(message)
{}

MissingBufferError::MissingBufferError(const std::string& message): MissingDataElementError(message)
{}

MissingItemError::MissingItemError(const std::string& message): MissingDataElementError(message)
{}

StreamError::StreamError(const std::string& message): std::runtime_error(message)
{}

StreamOpenError::StreamOpenError(const std::string& message): StreamError(message)
{}

StreamReadError::StreamReadError(const std::string& message): StreamError(message)
{}

StreamWriteError::StreamWriteError(const std::string& message): StreamError(message)
{}

StreamCloseError::StreamCloseError(const std::string& message): StreamError(message)
{}

DictionaryError::DictionaryError(const std::string& message): std::runtime_error(message)
{}

DictionaryUnknownTagError::DictionaryUnknownTagError(const std::string &message): DictionaryError(message)
{}

DictionaryUnknownDataTypeError::DictionaryUnknownDataTypeError(const std::string &message): DictionaryError(message)
{}

CharsetConversionError::CharsetConversionError(const std::string& message): std::runtime_error(message)
{}

CharsetConversionNoTableError::CharsetConversionNoTableError(const std::string& message): CharsetConversionError(message)
{}

CharsetConversionNoSupportedTableError::CharsetConversionNoSupportedTableError(const std::string& message): CharsetConversionError(message)
{}

CharsetListDiffDefaultError::CharsetListDiffDefaultError(const std::string& message): CharsetConversionError(message)
{}

CodecError::CodecError(const std::string& message): std::runtime_error(message)
{}

CodecWrongFormatError::CodecWrongFormatError(const std::string& message): CodecError(message)
{}

CodecCorruptedFileError::CodecCorruptedFileError(const std::string& message): CodecError(message)
{}

CodecWrongTransferSyntaxError::CodecWrongTransferSyntaxError(const std::string& message): CodecError(message)
{}

CodecImageTooBigError::CodecImageTooBigError(const std::string& message): CodecError(message)
{}

InvalidSequenceItemError::InvalidSequenceItemError(const std::string &message): CodecError(message)
{
}

DataHandlerError::DataHandlerError(const std::string& message): std::runtime_error(message)
{}

DataHandlerConversionError::DataHandlerConversionError(const std::string &message): DataHandlerError(message)
{}

DataHandlerCorruptedBufferError::DataHandlerCorruptedBufferError(const std::string &message): DataHandlerError(message)
{}

DataHandlerInvalidDataError::DataHandlerInvalidDataError(const std::string &message): DataHandlerError(message)
{}

DataSetError::DataSetError(const std::string& message): std::runtime_error(message)
{}

DataSetDifferentFormatError::DataSetDifferentFormatError(const std::string& message): DataSetError(message)
{}

DataSetUnknownTransferSyntaxError::DataSetUnknownTransferSyntaxError(const std::string& message): DataSetError(message)
{}

DataSetWrongFrameError::DataSetWrongFrameError(const std::string& message): DataSetError(message)
{}

DataSetOldFormatError::DataSetOldFormatError(const std::string& message): DataSetError(message)
{}

DataSetImageDoesntExistError::DataSetImageDoesntExistError(const std::string& message): DataSetError(message)
{}

DataSetImagePaletteColorIsReadOnly::DataSetImagePaletteColorIsReadOnly(const std::string &message): DataSetError(message)
{}

DataSetCorruptedOffsetTableError::DataSetCorruptedOffsetTableError(const std::string& message): DataSetError(message)
{}

DicomDirError::DicomDirError(const std::string& message): std::runtime_error(message)
{}

DicomDirCircularReferenceError::DicomDirCircularReferenceError(const std::string& message): DicomDirError(message)
{}

DicomDirUnknownDirectoryRecordTypeError::DicomDirUnknownDirectoryRecordTypeError(const std::string& message): DicomDirError(message)
{}

HuffmanError::HuffmanError(const std::string& message): std::runtime_error(message)
{}

HuffmanCreateTableError::HuffmanCreateTableError(const std::string& message): HuffmanError(message)
{}

HuffmanReadError::HuffmanReadError(const std::string& message): HuffmanError(message)
{}

HuffmanWriteError::HuffmanWriteError(const std::string& message): HuffmanError(message)
{}

ImageError::ImageError(const std::string& message): std::runtime_error(message)
{}

ImageUnknownDepthError::ImageUnknownDepthError(const std::string& message): ImageError(message)
{}

ImageUnknownColorSpaceError::ImageUnknownColorSpaceError(const std::string& message): ImageError(message)
{}

ImageInvalidSizeError::ImageInvalidSizeError(const std::string& message): ImageError(message)
{}

TransformError::TransformError(const std::string& message): std::runtime_error(message)
{}

TransformInvalidAreaError::TransformInvalidAreaError(const std::string& message): TransformError(message)
{}

TransformDifferentHighBitError::TransformDifferentHighBitError(const std::string &message): TransformError(message)
{}

ColorTransformError::ColorTransformError(const std::string& message): TransformError(message)
{}

ColorTransformWrongColorSpaceError::ColorTransformWrongColorSpaceError(const std::string& message): ColorTransformError(message)
{}

ColorTransformsFactoryError::ColorTransformsFactoryError(const std::string& message): TransformError(message)
{}

ColorTransformsFactoryNoTransformError::ColorTransformsFactoryNoTransformError(const std::string& message): ColorTransformsFactoryError(message)
{}

TransformHighBitError::TransformHighBitError(const std::string& message): TransformError(message)
{}

TransformHighBitDifferentColorSpacesError::TransformHighBitDifferentColorSpacesError(const std::string& message): TransformHighBitError(message)
{}

StreamEOFError::StreamEOFError(const std::string& message): StreamError(message)
{}

StreamJpegTagInStreamError::StreamJpegTagInStreamError(const std::string& message): StreamError(message)
{}

ModalityVOILUTError::ModalityVOILUTError(const std::string& message): TransformError(message)
{}

DicomCodecError::DicomCodecError(const std::string& message): CodecError(message)
{}

DicomCodecDepthLimitReachedError::DicomCodecDepthLimitReachedError(const std::string&message): DicomCodecError(message)
{}

JpegCodecError::JpegCodecError(const std::string& message): CodecError(message)
{}

JpegCodecCannotHandleSyntaxError::JpegCodecCannotHandleSyntaxError(const std::string& message): JpegCodecError(message)
{}

MemoryError::MemoryError(const std::string& message): std::runtime_error(message)
{}

MemorySizeError::MemorySizeError(const std::string& message): MemoryError(message)
{}


}
