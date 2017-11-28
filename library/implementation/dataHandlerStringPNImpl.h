/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dataHandlerStringPN.h
    \brief Declaration of the class dataHandlerStringPN.

*/

#if !defined(imebraDataHandlerStringPN_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_)
#define imebraDataHandlerStringPN_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_

#include "dataHandlerStringUnicodeImpl.h"


namespace imebra
{

namespace implementation
{

namespace handlers
{

/*!
\brief Handles the Dicom type "PN" (person name).

This class separates the component groups of the name.

The person name can be formed by three groups:
 one or more groups can be absent.
- the first components group contains a character
  representation of the person name
- the second components group contains an ideographic
  representation of the person name
- the third components group contains a phonetic
  representation of the patient name

Inside a components group, the name components
 (first name, middle name, surname, etc) must be
 separated by a ^.

This class doesn't insert or parse the ^ separator
 which must be inserted and handled by the calling
 application, but handles the = separator which
 separates the components groups.\n
This means that to read or set all the patient name
 you have to move the pointer to the internal element
 by using setPointer(), incPointer() or skip().

For instance, to set the name "Left^Arrow" both
 with a character and an ideographic representation you
 have to use the following code:

\code
myDataSet->getDataHandler(group, 0, tag, 0, true, "PN");
myDataSet->setSize(2);
myDataSet->setUnicodeString(L"Left^Arrow");
myDataSet->incPointer();
myDataSet->setUnicodeString(L"<-"); // :-)
\endcode

*/
class readingDataHandlerStringPN : public readingDataHandlerStringUnicode
{
public:
    readingDataHandlerStringPN(const memory& parseMemory, const charsetsList::tCharsetsList& charsets);
};

class writingDataHandlerStringPN: public writingDataHandlerStringUnicode
{
public:
    writingDataHandlerStringPN(const std::shared_ptr<buffer>& pBuffer, const charsetsList::tCharsetsList& charsets);

    virtual void validate() const;
};

} // namespace handlers

} // namespace implementation

} // namespace imebra

#endif // !defined(imebraDataHandlerStringPN_367AAE47_6FD7_4107_AB5B_25A355C5CB6E__INCLUDED_)
