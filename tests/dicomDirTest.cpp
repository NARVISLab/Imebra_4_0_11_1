#include <imebra/imebra.h>
#include <gtest/gtest.h>

namespace imebra
{

namespace tests
{

TEST(dicomDirTest, createDicomDir)
{
    DicomDir newDicomDir;

    std::unique_ptr<DicomDirEntry> rootRecord(newDicomDir.getNewEntry(directoryRecordType_t::patient));

    std::unique_ptr<DataSet> rootRecordDataSet(rootRecord->getEntryDataSet());
    rootRecordDataSet->setUnicodeString(TagId(tagId_t::PatientName_0010_0010), L"Surname");
    newDicomDir.setFirstRootEntry(*rootRecord);

    std::unique_ptr<DicomDirEntry> nextRecord(newDicomDir.getNewEntry(directoryRecordType_t::patient));
    std::unique_ptr<DataSet> nextRecordDataSet(nextRecord->getEntryDataSet());
    nextRecordDataSet->setUnicodeString(TagId(tagId_t::PatientName_0010_0010), L"Surname 1");
    rootRecord->setNextEntry(*nextRecord);

    std::unique_ptr<DicomDirEntry> imageRecord(newDicomDir.getNewEntry(directoryRecordType_t::image));
    std::unique_ptr<DataSet> imageRecordDataSet(imageRecord->getEntryDataSet());
    imageRecordDataSet->setString(TagId(tagId_t::SOPInstanceUID_0008_0018), "1.2.840.34.56.78999654.235");
    fileParts_t parts;
    parts.push_back("folder");
    parts.push_back("file.dcm");
    imageRecord->setFileParts(parts);

    nextRecord->setFirstChildEntry(*imageRecord);

    std::unique_ptr<DataSet> dicomDirDataSet(newDicomDir.updateDataSet());

    ReadWriteMemory streamMemory;
    MemoryStreamOutput memStream(streamMemory);
    StreamWriter writer(memStream);
    CodecFactory::save(*dicomDirDataSet, writer, codecType_t::dicom);

    MemoryStreamInput inputMemStream(streamMemory);
    StreamReader reader(inputMemStream);
    std::unique_ptr<DataSet> readDataSet(CodecFactory::load(reader));

    const DicomDir testDicomDir(*readDataSet);
    std::unique_ptr<DicomDirEntry> testRootRecord(testDicomDir.getFirstRootEntry());
    std::unique_ptr<DataSet> testRootRecordDataSet(testRootRecord->getEntryDataSet());
    EXPECT_EQ(directoryRecordType_t::patient, testRootRecord->getType());
    EXPECT_EQ("PATIENT", testRootRecord->getTypeString());
    EXPECT_EQ(std::wstring(L"Surname"), testRootRecordDataSet->getUnicodeString(TagId(tagId_t::PatientName_0010_0010), 0));

    std::unique_ptr<DicomDirEntry> testNextRecord(testRootRecord->getNextEntry());
    std::unique_ptr<DataSet> testNextRecordDataSet(testNextRecord->getEntryDataSet());
    EXPECT_EQ(directoryRecordType_t::patient, testNextRecord->getType());
    EXPECT_EQ(std::wstring(L"Surname 1"), testNextRecordDataSet->getUnicodeString(TagId(tagId_t::PatientName_0010_0010), 0));
    EXPECT_EQ(0, testNextRecord->getNextEntry());

    std::unique_ptr<DicomDirEntry> testImageRecord(testNextRecord->getFirstChildEntry());
    std::unique_ptr<DataSet> testImageRecordDataSet(testImageRecord->getEntryDataSet());
    EXPECT_EQ(directoryRecordType_t::image, testImageRecord->getType());
    EXPECT_EQ(std::string("1.2.840.34.56.78999654.235"), testImageRecordDataSet->getString(TagId(tagId_t::SOPInstanceUID_0008_0018), 0));
    EXPECT_EQ(std::string("folder"), testImageRecord->getFileParts().at(0));
    EXPECT_EQ(std::string("file.dcm"), testImageRecord->getFileParts().at(1));
    EXPECT_EQ(0, testImageRecord->getFirstChildEntry());
}


TEST(dicomDirTest, emptyDicomDir)
{
    DicomDir newDicomDir;

    std::unique_ptr<DataSet> dicomDirDataSet(newDicomDir.updateDataSet());

    ReadWriteMemory streamMemory;
    MemoryStreamOutput memStream(streamMemory);
    StreamWriter writer(memStream);
    CodecFactory::save(*dicomDirDataSet, writer, codecType_t::dicom);

    MemoryStreamInput inputMemStream(streamMemory);
    StreamReader reader(inputMemStream);
    std::unique_ptr<DataSet> readDataSet(CodecFactory::load(reader));

    const DicomDir testDicomDir(*readDataSet);
    EXPECT_EQ(0, testDicomDir.getFirstRootEntry());
}

} // namespace tests

} // namespace imebra
