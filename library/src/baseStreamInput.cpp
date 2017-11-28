/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file baseStreamInput.cpp
    \brief Implementation of the the base class for the input streams (memory, file, ...).
*/

#include "../include/imebra/baseStreamInput.h"
#include "../implementation/baseStreamImpl.h"
#include "../implementation/exceptionImpl.h"

namespace imebra
{

BaseStreamInput::BaseStreamInput(std::shared_ptr<implementation::baseStreamInput> pStream): m_pStream(pStream)
{
}

BaseStreamInput::~BaseStreamInput()
{
}

}
