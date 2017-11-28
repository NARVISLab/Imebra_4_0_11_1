/*
Copyright 2005 - 2017 by Paolo Brandoli/Binarno s.p.

Imebra is available for free under the GNU General Public License.

The full text of the license is available in the file license.rst
 in the project root folder.

If you do not want to be bound by the GPL terms (such as the requirement 
 that your application must also be GPL), you may purchase a commercial 
 license for Imebra from the Imebra’s website (http://imebra.com).
*/

#include "../include/imebra/dataSet.h"
#include "../include/imebra/tag.h"
#include "../include/imebra/lut.h"
#include "../implementation/dataSetImpl.h"
#include "../implementation/dataHandlerNumericImpl.h"
#include "../implementation/charsetConversionBaseImpl.h"
#include <typeinfo>
#include <memory>

namespace imebra
{

DataSet::DataSet(): m_pDataSet(std::make_shared<imebra::implementation::dataSet>())
{
    implementation::charsetsList::tCharsetsList list;
    list.push_back("ISO 2022 IR 6");
    m_pDataSet->setCharsetsList(list);
}

DataSet::DataSet(const std::string& transferSyntax): m_pDataSet(std::make_shared<imebra::implementation::dataSet>(transferSyntax))
{
    implementation::charsetsList::tCharsetsList list;
    list.push_back("ISO 2022 IR 6");
    m_pDataSet->setCharsetsList(list);
}

DataSet::DataSet(const std::string& transferSyntax, const charsetsList_t& charsets): m_pDataSet(std::make_shared<imebra::implementation::dataSet>(transferSyntax))
{
    implementation::charsetsList::tCharsetsList list;
    for(charsetsList_t::const_iterator scanCharsets(charsets.begin()), endCharsets(charsets.end()); scanCharsets != endCharsets; ++scanCharsets)
    {
        list.push_back(*scanCharsets);
    }
    m_pDataSet->setCharsetsList(list);
}

DataSet::~DataSet()
{
}

DataSet::DataSet(std::shared_ptr<imebra::implementation::dataSet> pDataSet): m_pDataSet(pDataSet)
{
    if(pDataSet == 0)
    {
        m_pDataSet = std::make_shared<imebra::implementation::dataSet>();
    }
}

tagsIds_t DataSet::getTags() const
{
    tagsIds_t returnTags;

    implementation::dataSet::tGroupsIds groups = m_pDataSet->getGroups();
    for(implementation::dataSet::tGroupsIds::const_iterator scanGroups(groups.begin()), endGroups(groups.end());
        scanGroups != endGroups;
        ++scanGroups)
    {
        std::uint32_t orders = m_pDataSet->getGroupsNumber(*scanGroups);
        for(std::uint32_t scanOrders(0); scanOrders != orders; ++scanOrders)
        {
            const implementation::dataSet::tTags tags = m_pDataSet->getGroupTags(*scanGroups, scanOrders);
            for(implementation::dataSet::tTags::const_iterator scanTags(tags.begin()), endTags(tags.end());
                scanTags != endTags;
                ++scanTags)
            {
                returnTags.push_back(TagId(*scanGroups, scanOrders, scanTags->first));
            }
        }
    }

    return returnTags;
}

Tag* DataSet::getTag(const TagId& tagId) const
{
    return new Tag(m_pDataSet->getTag(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId()));
}

Tag* DataSet::getTagCreate(const TagId& tagId, tagVR_t tagVR)
{
    return new Tag(m_pDataSet->getTagCreate(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), tagVR));
}

Tag* DataSet::getTagCreate(const TagId& tagId)
{
    return new Tag(m_pDataSet->getTagCreate(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId()));
}


Image* DataSet::getImage(size_t frameNumber)
{
    return new Image(m_pDataSet->getImage((std::uint32_t)frameNumber));
}

Image* DataSet::getImageApplyModalityTransform(size_t frameNumber)
{
    return new Image(m_pDataSet->getModalityImage((std::uint32_t)frameNumber));
}

void DataSet::setImage(size_t frameNumber, const Image& image, imageQuality_t quality)
{
    m_pDataSet->setImage((std::uint32_t)frameNumber, image.m_pImage, quality);
}

DataSet* DataSet::getSequenceItem(const TagId& tagId, size_t itemId)
{
    return new DataSet(m_pDataSet->getSequenceItem(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), itemId));
}

void DataSet::setSequenceItem(const TagId &tagId, size_t itemId, const DataSet &item)
{
    m_pDataSet->setSequenceItem(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), itemId, item.m_pDataSet);
}

LUT* DataSet::getLUT(const TagId &tagId, size_t itemId)
{
    return new LUT(m_pDataSet->getLut(tagId.getGroupId(), tagId.getTagId(), itemId));
}

vois_t DataSet::getVOIs()
{
    return m_pDataSet->getVOIs();
}

ReadingDataHandler* DataSet::getReadingDataHandler(const TagId& tagId, size_t bufferId) const
{
    return new ReadingDataHandler(m_pDataSet->getReadingDataHandler(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId));
}

WritingDataHandler* DataSet::getWritingDataHandler(const TagId& tagId, size_t bufferId, tagVR_t tagVR)
{
    return new WritingDataHandler(m_pDataSet->getWritingDataHandler(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId, tagVR));
}

WritingDataHandler* DataSet::getWritingDataHandler(const TagId& tagId, size_t bufferId)
{
    return new WritingDataHandler(m_pDataSet->getWritingDataHandler(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId));
}

ReadingDataHandlerNumeric* DataSet::getReadingDataHandlerNumeric(const TagId& tagId, size_t bufferId) const
{
    return new ReadingDataHandlerNumeric(m_pDataSet->getReadingDataHandlerNumeric(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId));
}

ReadingDataHandlerNumeric* DataSet::getReadingDataHandlerRaw(const TagId& tagId, size_t bufferId) const
{
    std::shared_ptr<implementation::handlers::readingDataHandlerNumericBase> numericHandler = m_pDataSet->getReadingDataHandlerRaw(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId);
    return new ReadingDataHandlerNumeric(numericHandler);
}

WritingDataHandlerNumeric* DataSet::getWritingDataHandlerNumeric(const TagId& tagId, size_t bufferId, tagVR_t tagVR)
{
    return new WritingDataHandlerNumeric(m_pDataSet->getWritingDataHandlerNumeric(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId, tagVR));
}

WritingDataHandlerNumeric* DataSet::getWritingDataHandlerNumeric(const TagId& tagId, size_t bufferId)
{
    return new WritingDataHandlerNumeric(m_pDataSet->getWritingDataHandlerNumeric(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId));
}

WritingDataHandlerNumeric* DataSet::getWritingDataHandlerRaw(const TagId& tagId, size_t bufferId, tagVR_t tagVR)
{
    std::shared_ptr<implementation::handlers::writingDataHandlerNumericBase> numericHandler = m_pDataSet->getWritingDataHandlerRaw(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId, tagVR);
    return new WritingDataHandlerNumeric(numericHandler);
}

WritingDataHandlerNumeric* DataSet::getWritingDataHandlerRaw(const TagId& tagId, size_t bufferId)
{
    std::shared_ptr<implementation::handlers::writingDataHandlerNumericBase> numericHandler = m_pDataSet->getWritingDataHandlerRaw(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId);
    return new WritingDataHandlerNumeric(numericHandler);
}

bool DataSet::bufferExists(const TagId& tagId, size_t bufferId)
{
    return m_pDataSet->bufferExists(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), bufferId);
}

std::int32_t DataSet::getSignedLong(const TagId& tagId, size_t elementNumber) const
{
    return m_pDataSet->getSignedLong(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber);
}

std::int32_t DataSet::getSignedLong(const TagId& tagId, size_t elementNumber, std::int32_t defaultValue) const
{
    return m_pDataSet->getSignedLong(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber, defaultValue);
}

void DataSet::setSignedLong(const TagId& tagId, std::int32_t newValue, tagVR_t tagVR)
{
    m_pDataSet->setSignedLong(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, newValue, tagVR);
}

void DataSet::setSignedLong(const TagId& tagId, std::int32_t newValue)
{
    m_pDataSet->setSignedLong(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, newValue);
}

std::uint32_t DataSet::getUnsignedLong(const TagId& tagId, size_t elementNumber) const
{
    return m_pDataSet->getUnsignedLong(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber);
}

std::uint32_t DataSet::getUnsignedLong(const TagId& tagId, size_t elementNumber, std::uint32_t defaultValue) const
{
    return m_pDataSet->getUnsignedLong(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber, defaultValue);
}

void DataSet::setUnsignedLong(const TagId& tagId, std::uint32_t newValue, tagVR_t tagVR)
{
    m_pDataSet->setUnsignedLong(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, newValue, tagVR);
}

void DataSet::setUnsignedLong(const TagId& tagId, std::uint32_t newValue)
{
    m_pDataSet->setUnsignedLong(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, newValue);
}

double DataSet::getDouble(const TagId& tagId, size_t elementNumber) const
{
    return m_pDataSet->getDouble(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber);
}

double DataSet::getDouble(const TagId& tagId, size_t elementNumber, double defaultValue) const
{
    return m_pDataSet->getDouble(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber, defaultValue);
}

void DataSet::setDouble(const TagId& tagId, double newValue, tagVR_t tagVR)
{
    m_pDataSet->setDouble(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, newValue, tagVR);
}

void DataSet::setDouble(const TagId& tagId, double newValue)
{
    m_pDataSet->setDouble(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, newValue);
}

std::string DataSet::getString(const TagId& tagId, size_t elementNumber) const
{
    return m_pDataSet->getString(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber);
}

std::string DataSet::getString(const TagId& tagId, size_t elementNumber, const std::string& defaultValue) const
{
    return m_pDataSet->getString(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber, defaultValue);
}

std::wstring DataSet::getUnicodeString(const TagId& tagId, size_t elementNumber) const
{
    return m_pDataSet->getUnicodeString(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber);
}

std::wstring DataSet::getUnicodeString(const TagId& tagId, size_t elementNumber, const std::wstring& defaultValue) const
{
    return m_pDataSet->getUnicodeString(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber, defaultValue);
}

void DataSet::setString(const TagId& tagId, const std::string& newString, tagVR_t tagVR)
{
    m_pDataSet->setString(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, newString, tagVR);
}

void DataSet::setString(const TagId& tagId, const std::string& newString)
{
    m_pDataSet->setString(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, newString);
}

void DataSet::setUnicodeString(const TagId& tagId, const std::wstring& newString, tagVR_t tagVR)
{
    m_pDataSet->setUnicodeString(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, newString, tagVR);
}

void DataSet::setUnicodeString(const TagId& tagId, const std::wstring& newString)
{
    m_pDataSet->setUnicodeString(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, newString);
}

void DataSet::setAge(const TagId& tagId, const Age& age)
{
    m_pDataSet->setAge(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, age.age, age.units);
}

Age* DataSet::getAge(const TagId& tagId, size_t elementNumber) const
{
    imebra::ageUnit_t units;
    std::uint32_t age = m_pDataSet->getAge(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber, &units);
    return new Age(age, units);
}

Age* DataSet::getAge(const TagId& tagId, size_t elementNumber, const Age& defaultValue) const
{
    imebra::ageUnit_t units;
    std::uint32_t age = m_pDataSet->getAge(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber, &units, defaultValue.age, defaultValue.units);
    return new Age(age, units);
}

void DataSet::setDate(const TagId& tagId, const Date& date, tagVR_t tagVR)
{
    m_pDataSet->setDate(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0,
                        (std::uint32_t) date.year,
                        (std::uint32_t) date.month,
                        (std::uint32_t) date.day,
                        (std::uint32_t) date.hour,
                        (std::uint32_t) date.minutes,
                        (std::uint32_t) date.seconds,
                        (std::uint32_t) date.nanoseconds,
                        (std::int32_t) date.offsetHours,
                        (std::int32_t) date.offsetMinutes, tagVR);
}

void DataSet::setDate(const TagId& tagId, const Date& date)
{
    m_pDataSet->setDate(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0,
                        (std::uint32_t) date.year,
                        (std::uint32_t) date.month,
                        (std::uint32_t) date.day,
                        (std::uint32_t) date.hour,
                        (std::uint32_t) date.minutes,
                        (std::uint32_t) date.seconds,
                        (std::uint32_t) date.nanoseconds,
                        (std::int32_t) date.offsetHours,
                        (std::int32_t) date.offsetMinutes);
}

Date* DataSet::getDate(const TagId& tagId, size_t elementNumber) const
{
    std::uint32_t year, month, day, hour, minutes, seconds, nanoseconds;
    std::int32_t offsetHours, offsetMinutes;
    m_pDataSet->getDate(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber,
                        &year, &month, &day, &hour, &minutes, &seconds, &nanoseconds, &offsetHours, &offsetMinutes);

    return new Date(
                (unsigned int)year,
                (unsigned int)month,
                (unsigned int)day,
                (unsigned int)hour,
                (unsigned int)minutes,
                (unsigned int)seconds,
                (unsigned int)nanoseconds,
                (int)offsetHours,
                (int)offsetMinutes);
}

Date* DataSet::getDate(const TagId& tagId, size_t elementNumber, const Date& defaultValue) const
{
    std::uint32_t year, month, day, hour, minutes, seconds, nanoseconds;
    std::int32_t offsetHours, offsetMinutes;
    m_pDataSet->getDate(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId(), 0, elementNumber,
                        &year, &month, &day, &hour, &minutes, &seconds, &nanoseconds, &offsetHours, &offsetMinutes,
                        defaultValue.year, defaultValue.month, defaultValue.day,
                        defaultValue.hour, defaultValue.minutes, defaultValue.seconds, defaultValue.nanoseconds,
                        defaultValue.offsetHours, defaultValue.offsetMinutes);

    return new Date(
                (unsigned int)year,
                (unsigned int)month,
                (unsigned int)day,
                (unsigned int)hour,
                (unsigned int)minutes,
                (unsigned int)seconds,
                (unsigned int)nanoseconds,
                (int)offsetHours,
                (int)offsetMinutes);
}


tagVR_t DataSet::getDataType(const TagId& tagId) const
{
    return m_pDataSet->getDataType(tagId.getGroupId(), tagId.getGroupOrder(), tagId.getTagId());
}

}
