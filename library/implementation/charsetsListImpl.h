/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file charsetsList.h
    \brief Declaration of the the base class for the classes that need to be
            aware of the Dicom charsets.

*/

#if !defined(imebraCharsetsList_DE3F98A9_664E_47c0_A29B_B681F9AEB118__INCLUDED_)
#define imebraCharsetsList_DE3F98A9_664E_47c0_A29B_B681F9AEB118__INCLUDED_

#include <stdexcept>
#include <string>
#include <list>

namespace imebra
{

namespace implementation
{

/// \addtogroup group_dataset
///
/// @{


/// \name charsetsList
/// \brief The classes used to convert between different
///         charsets are declared in this namespace.
///////////////////////////////////////////////////////////
namespace charsetsList
{

/// \typedef std::list<std::wstring> tCharsetsList
/// \brief Defines a list of widechar strings.
///
/// It is used to set or retrieve a list of charsets
///
///////////////////////////////////////////////////////////
typedef std::list<std::string> tCharsetsList;

void updateCharsets(const tCharsetsList* pCharsetsList, tCharsetsList* pDestinationCharsetsList);

} // namespace charsetsList

/// @}

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraCharsetsList_DE3F98A9_664E_47c0_A29B_B681F9AEB118__INCLUDED_)

