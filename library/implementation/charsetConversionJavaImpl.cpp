/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file charsetConversionJava.cpp
    \brief Implementation of the charsetConversion class using the JVM.

*/

#if defined(IMEBRA_USE_JAVA)

#include "configurationImpl.h"
#include "streamControllerImpl.h"

#include "exceptionImpl.h"
#include "charsetConversionJavaImpl.h"
#include <memory.h>

namespace imebra
{

JavaVM* m_javaVM = 0;

extern "C"
{

    //------------------------------------------------------------------------
    JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* aVm, void* aReserved)
    {
        // cache java VM
        m_javaVM = aVm;

        return JNI_VERSION_1_6;
    }

} // extern "C"


JavaVM* get_imebra_javaVM()
{
    return m_javaVM;
}

///////////////////////////////////////////////////////////
//
// Constructor
//
///////////////////////////////////////////////////////////
charsetConversionJava::charsetConversionJava(const std::string& dicomName)
{
    IMEBRA_FUNCTION_START();

    const charsetInformation& info = getDictionary().getCharsetInformation(dicomName);
    m_tableName = info.m_javaRegistration;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Destructor
//
///////////////////////////////////////////////////////////
charsetConversionJava::~charsetConversionJava()
{
}


///////////////////////////////////////////////////////////
//
// Convert a string from unicode to multibyte
//
///////////////////////////////////////////////////////////
std::string charsetConversionJava::fromUnicode(const std::wstring& unicodeString) const
{
    IMEBRA_FUNCTION_START();

    if(unicodeString.empty())
    {
        return std::string();
    }

    bool bDetach(false);
    JNIEnv* env = getJavaEnv(&bDetach);

    std::string bytes;
    bytes.resize(unicodeString.length() * sizeof(wchar_t));
    ::memcpy(&bytes[0], &(unicodeString[0]), bytes.size());
    streamController::adjustEndian((std::uint8_t*)&(bytes[0]), sizeof(wchar_t), streamController::highByteEndian, unicodeString.size());
    jstring javaString = getNativeJavaString(env, bytes, sizeof(wchar_t) == 2 ? "UTF-16BE" : "UTF-32BE");

    std::string returnValue;

    if(javaString != 0)
    {
        returnValue = getBytesFromString(env, javaString, m_tableName.c_str());
        env->DeleteLocalRef(javaString);
    }

    if(bDetach)
    {
        get_imebra_javaVM()->DetachCurrentThread();
    }

    if(returnValue == "?" && unicodeString != L"?")
    {
        return "";
    }

    if(returnValue == "\x22\x44" && unicodeString != L"\xbf" && m_tableName == "JIS_X0212-1990")
    {
        return "";
    }

    return returnValue;

    IMEBRA_FUNCTION_END();
}


///////////////////////////////////////////////////////////
//
// Convert a string from multibyte to unicode
//
///////////////////////////////////////////////////////////
std::wstring charsetConversionJava::toUnicode(const std::string& asciiString) const
{
    IMEBRA_FUNCTION_START();

    if(asciiString.empty())
    {
        return std::wstring();
    }

    bool bDetach(false);
    JNIEnv* env = getJavaEnv(&bDetach);

    std::wstring returnValue;
    jstring javaString = getNativeJavaString(env, asciiString, m_tableName.c_str());
    if(javaString != 0)
    {
        std::string bytes = getBytesFromString(env, javaString, sizeof(wchar_t) == 2 ? "UTF-16BE" : "UTF-32BE");
        if(bytes.size() != 0)
        {
            returnValue.resize(bytes.size() / sizeof(wchar_t));
            ::memcpy(&(returnValue[0]), &(bytes[0]), bytes.size());
            streamController::adjustEndian((std::uint8_t*)&(returnValue[0]), sizeof(wchar_t), streamController::highByteEndian, returnValue.size());
        }
        env->DeleteLocalRef(javaString);
    }

    if(bDetach)
    {
        get_imebra_javaVM()->DetachCurrentThread();
    }

    return returnValue;

    IMEBRA_FUNCTION_END();
}


/**
 The function creates a byte array, copies the native C string into the byte array,
 and finally invokes the String(byte[] bytes) constructor to create the resulting jstring object.
 Class_java_lang_String is a global reference to the java.lang.String class, and MID_String_init
 is the method ID of the string constructor.
 Because this is a utility function, we make sure to delete the local reference to the byte array
  created temporarily to store the characters.
*/
jstring charsetConversionJava::getNativeJavaString(JNIEnv *env, const std::string& str, const char* tableName)
{
    IMEBRA_FUNCTION_START();

    jclass Class_java_lang_String = env->FindClass("java/lang/String");
    jmethodID MID_String_init = env->GetMethodID(Class_java_lang_String, "<init>", "([BLjava/lang/String;)V");

    if (env->EnsureLocalCapacity(2) < 0)
    {
        throw;
    }

    size_t len = str.size();
    jbyteArray bytes = env->NewByteArray(len);
    if (bytes != 0)
    {
        jstring jTableName = env->NewStringUTF(tableName);
        if(jTableName != 0)
        {
            env->SetByteArrayRegion(bytes, 0, len, (jbyte *)&(str[0]));
            jstring result = (jstring)env->NewObject(Class_java_lang_String, MID_String_init, bytes, jTableName);
            env->DeleteLocalRef(jTableName);
            env->DeleteLocalRef(bytes);
            if(result == 0)
            {
                throw;
            }
            return result;
        }
        env->DeleteLocalRef(bytes);
    }

    throw;
    return 0;

    IMEBRA_FUNCTION_END();
}





/**
 Use the String.getBytes method to convert a jstring to the appropriate native encoding.
 The following utility function translates a jstring to a locale-specific native C string
*/
std::string charsetConversionJava::getBytesFromString(JNIEnv *env, jstring jstr, const char* tableName)
{
    IMEBRA_FUNCTION_START();

    std::string result;
    jclass Class_java_lang_String = env->FindClass("java/lang/String");
    jmethodID MID_String_getBytes = env->GetMethodID(Class_java_lang_String, "getBytes", "(Ljava/lang/String;)[B");

    jbyteArray bytes = 0;
    jthrowable exc;
    if (env->EnsureLocalCapacity(2) < 0)
    {
        return ""; /* out of memory error */
    }

    jstring jTableName = env->NewStringUTF(tableName);
    if(jTableName != 0)
    {
        bytes = (jbyteArray)env->CallObjectMethod(jstr, MID_String_getBytes, jTableName);
        exc = env->ExceptionOccurred();
        if (!exc && bytes != 0)
        {
            jint len = env->GetArrayLength(bytes);
            if(len != 0)
            {
                result.resize(len);
                env->GetByteArrayRegion(bytes, 0, len, (jbyte *)&(result[0]));
            }
        }
        else
        {
            env->DeleteLocalRef(exc);
        }
        if(bytes != 0)
        {
            env->DeleteLocalRef(bytes);
        }
        env->DeleteLocalRef(jTableName);
    }
    return result;

    IMEBRA_FUNCTION_END();
}


JNIEnv* charsetConversionJava::getJavaEnv(bool* bDetach)
{
    IMEBRA_FUNCTION_START();

    JavaVM* javaVM = get_imebra_javaVM();
    JNIEnv* env;

    // double check it's all ok
    int getEnvStat = javaVM->GetEnv((void **)&env, JNI_VERSION_1_6);
    if (getEnvStat == JNI_EDETACHED)
    {
#ifdef __ANDROID__
        if (javaVM->AttachCurrentThread(&env, 0) == 0)
#else
        if (javaVM->AttachCurrentThread((void**)&env, 0) == 0)
#endif
        {
            *bDetach = true;
            return env;
        }
    }
    else if (getEnvStat == JNI_OK)
    {
        *bDetach = false;
        return env;
    }
    return 0;

    IMEBRA_FUNCTION_END();

}


} // namespace imebra

#endif
