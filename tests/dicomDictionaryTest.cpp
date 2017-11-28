#include <imebra/imebra.h>
#include <gtest/gtest.h>

namespace imebra
{

namespace tests
{

TEST(dicomDictionaryTest, getTagInfo)
{
    ASSERT_TRUE(DicomDictionary::getTagName(TagId(tagId_t::PatientName_0010_0010)).find("Patient") != std::string::npos);
    ASSERT_TRUE(DicomDictionary::getUnicodeTagName(TagId(tagId_t::PatientName_0010_0010)).find(L"Patient") != std::string::npos);
    ASSERT_EQ(tagVR_t::PN, DicomDictionary::getTagType(TagId(tagId_t::PatientName_0010_0010)));
    ASSERT_EQ(64, DicomDictionary::getMaxSize(tagVR_t::PN));
    ASSERT_EQ(4, DicomDictionary::getWordSize(tagVR_t::UL));
}


} // namespace tests

} // namespace imebra
