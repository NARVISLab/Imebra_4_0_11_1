/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file stream.cpp
    \brief Implementation of the stream class.

*/

#include "exceptionImpl.h"
#include "fileStreamImpl.h"
#include "charsetConversionImpl.h"
#include "../include/imebra/exceptions.h"

#include <sstream>
#include <errno.h>

namespace imebra
{

namespace implementation
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// Stream
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Open a file (unicode)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void fileStream::openFile(const std::wstring& fileName, std::ios_base::openmode mode)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::mutex> lock(m_mutex);

    if(m_openFile != 0)
    {
        if(::fclose(m_openFile) != 0)
        {
            IMEBRA_THROW(StreamCloseError, "Error while closing the file");
        }
        m_openFile = 0;
    }

    std::wstring strMode;

    int tempMode = mode & (~std::ios::binary);

    if(tempMode == (int)(std::ios::in | std::ios::out))
    {
        strMode = L"r+";
    }

    if(tempMode == (int)(std::ios::in | std::ios::out | std::ios::app))
    {
        strMode = L"a+";
    }

    if(tempMode == (int)(std::ios::in | std::ios::out | std::ios::trunc))
    {
        strMode = L"w+";
    }

    if(tempMode == (int)(std::ios::out) || tempMode == (int)(std::ios::out | std::ios::trunc))
    {
        strMode = L"w";
    }

    if(tempMode == (int)(std::ios::out | std::ios::app))
    {
        strMode = L"a";
    }

    if(tempMode == (int)(std::ios::in))
    {
        strMode = L"r";
    }

    strMode += L"b";

#if defined(IMEBRA_WINDOWS)
     errno_t errorCode = ::_wfopen_s(&m_openFile, fileName.c_str(), strMode.c_str());
     if (errorCode != 0)
     {
         m_openFile = 0;
     }
#else
    // Convert the filename to UTF8
    defaultCharsetConversion toUtf8("ISO-IR 192");
    std::string utf8FileName(toUtf8.fromUnicode(fileName));

    // Convert the filemode to UTF8
    std::string utf8Mode(toUtf8.fromUnicode(strMode));

    m_openFile = ::fopen(utf8FileName.c_str(), utf8Mode.c_str());
    int errorCode = errno;
#endif
    if(m_openFile == 0)
    {
        std::ostringstream errorMessage;
        errorMessage << "stream::openFile failure - error code: " << errorCode;
        IMEBRA_THROW(StreamOpenError, "stream::openFile failure - error code: " << errorCode);
    }

    IMEBRA_FUNCTION_END();
}

void fileStream::close()
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::mutex> lock(m_mutex);

    if(m_openFile != 0)
    {
        if(::fclose(m_openFile) != 0)
        {
            IMEBRA_THROW(StreamCloseError, "Error while closing the file");
        }
        m_openFile = 0;
    }

    IMEBRA_FUNCTION_END();
}




///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Destructor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
fileStream::~fileStream()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if(m_openFile != 0)
    {
        ::fclose(m_openFile);
        m_openFile = 0;
    }
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Open a file (ansi)
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
fileStreamInput::fileStreamInput(const std::string& fileName)
{
    IMEBRA_FUNCTION_START();

	std::wstring wFileName;
	size_t fileNameSize(fileName.size());
	wFileName.resize(fileNameSize);
	for(size_t copyChars = 0; copyChars != fileNameSize; ++copyChars)
	{
		wFileName[copyChars] = (wchar_t)fileName[copyChars];
	}
    openFile(wFileName, std::ios::in);

	IMEBRA_FUNCTION_END();
}

fileStreamInput::fileStreamInput(const std::wstring& fileName)
{
    IMEBRA_FUNCTION_START();

    openFile(fileName, std::ios::in);

    IMEBRA_FUNCTION_END();
}

fileStreamOutput::fileStreamOutput(const std::string& fileName)
{
    IMEBRA_FUNCTION_START();

    std::wstring wFileName;
    size_t fileNameSize(fileName.size());
    wFileName.resize(fileNameSize);
    for(size_t copyChars = 0; copyChars != fileNameSize; ++copyChars)
    {
        wFileName[copyChars] = (wchar_t)fileName[copyChars];
    }
    openFile(wFileName, std::ios::out);

    IMEBRA_FUNCTION_END();
}

fileStreamOutput::fileStreamOutput(const std::wstring &fileName)
{
    IMEBRA_FUNCTION_START();

    openFile(fileName, std::ios::out);

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Write raw data into the stream
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void fileStreamOutput::write(size_t startPosition, const std::uint8_t* pBuffer, size_t bufferLength)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::mutex> lock(m_mutex);

    ::fseek(m_openFile, (long)startPosition, SEEK_SET);
	if(ferror(m_openFile) != 0)
	{
        IMEBRA_THROW(StreamWriteError, "stream::seek failure");
	}

	if(::fwrite(pBuffer, 1, bufferLength, m_openFile) != bufferLength)
	{
        IMEBRA_THROW(StreamWriteError, "stream::write failure");
	}

	IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Read raw data from the stream
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
size_t fileStreamInput::read(size_t startPosition, std::uint8_t* pBuffer, size_t bufferLength)
{
    IMEBRA_FUNCTION_START();

    std::lock_guard<std::mutex> lock(m_mutex);

    ::fseek(m_openFile, (long)startPosition, SEEK_SET);
	if(ferror(m_openFile) != 0)
	{
		return 0;
	}

    size_t readBytes = (size_t)::fread(pBuffer, 1, bufferLength, m_openFile);
	if(ferror(m_openFile) != 0)
	{
        IMEBRA_THROW(StreamReadError, "stream::read failure");
	}
	return readBytes;

	IMEBRA_FUNCTION_END();
}

} // namespace implementation

} // namespace imebra
