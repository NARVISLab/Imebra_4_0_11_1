/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file fileStreamOutput.cpp
    \brief Implementation of the file output stream class.

*/

#include "../include/imebra/fileStreamOutput.h"
#include "../implementation/fileStreamImpl.h"

namespace imebra
{

FileStreamOutput::FileStreamOutput(const std::wstring& name): BaseStreamOutput(std::make_shared<implementation::fileStreamOutput>(name))
{
}

FileStreamOutput::FileStreamOutput(const std::string& name): BaseStreamOutput(std::make_shared<implementation::fileStreamOutput>(name))
{
}

FileStreamOutput::~FileStreamOutput()
{
}

}
