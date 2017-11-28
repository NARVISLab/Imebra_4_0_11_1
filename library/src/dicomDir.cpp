/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

/*! \file dicomDir.cpp
    \brief Implementation of the classes that parse/create a DICOMDIR
        structure (DicomDir and DicomDirEntry).

*/

#include "../include/imebra/dicomDir.h"
#include "../include/imebra/dataSet.h"
#include "../include/imebra/dicomDirEntry.h"
#include "../implementation/dicomDirImpl.h"

namespace imebra
{

DicomDir::DicomDir()
{
    DataSet emptyDataSet;
    m_pDicomDir = std::make_shared<imebra::implementation::dicomDir>();
}

DicomDir::DicomDir(const DataSet& fromDataSet): m_pDicomDir(std::make_shared<imebra::implementation::dicomDir>(fromDataSet.m_pDataSet))
{
}

DicomDir::~DicomDir()
{
}

DicomDirEntry* DicomDir::getNewEntry(directoryRecordType_t recordType)
{
    return new DicomDirEntry(m_pDicomDir->getNewRecord(recordType));
}

DicomDirEntry* DicomDir::getFirstRootEntry() const
{
    std::shared_ptr<implementation::directoryRecord> pRootRecord(m_pDicomDir->getFirstRootRecord());
    if(pRootRecord == 0)
    {
        return 0;
    }
    return new DicomDirEntry(pRootRecord);
}

void DicomDir::setFirstRootEntry(const DicomDirEntry& firstEntryRecord)
{
    m_pDicomDir->setFirstRootRecord(firstEntryRecord.m_pDirectoryRecord);
}

DataSet* DicomDir::updateDataSet()
{
    return new DataSet(m_pDicomDir->buildDataSet());
}

}
