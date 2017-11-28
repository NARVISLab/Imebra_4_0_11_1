/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file charsetConversionJava.h
    \brief Declaration of the class used to convert a string between different
            charsets using the JVM.

The class hides the platform specific implementations and supplies a common
 interface for the charsets translations.

*/

#if !defined(imebraCharsetConversionJAVA_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
#define imebraCharsetConversionJAVA_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_

#if defined(IMEBRA_USE_JAVA)

#include <jni.h>
#include <string>
#include "charsetConversionBaseImpl.h"


namespace imebra
{

class charsetConversionJava: public charsetConversionBase
{
public:
    charsetConversionJava(const std::string& dicomName);
    virtual ~charsetConversionJava();

    virtual std::string fromUnicode(const std::wstring& unicodeString) const;

    virtual std::wstring toUnicode(const std::string& asciiString) const;

protected:
    static jstring getNativeJavaString(JNIEnv *env, const std::string& str, const char* tableName);
    static std::string getBytesFromString(JNIEnv *env, jstring jstr, const char* tableName);

    static JNIEnv* getJavaEnv(bool* bDetach);

    std::string m_tableName;
};

typedef charsetConversionJava defaultCharsetConversion;

} // namespace imebra

#endif // IMEBRA_USE_JAVA

#endif // !defined(imebraCharsetConversionJAVA_3146DA5A_5276_4804_B9AB_A3D54C6B123A__INCLUDED_)
