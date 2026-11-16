/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
	* Redistributions of source code must retain the above copyright
	  notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.
	* Neither the name of the 3MF Consortium nor the
	  names of its contributors may be used to endorse or promote products
	  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE 3MF CONSORTIUM BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#include "common_resourcepackage.hpp"

#include "common_utils.hpp"
#include "common_portablezipwriter.hpp"

#include "Libraries/libzip/zip.h"
#include "Libraries/PugiXML/pugixml.hpp"

#include "lib3mftoolpath4eos_interfaceexception.hpp"

#include <map>

#include <sstream>

#define ROOT_ZIP_READCHUNKSIZE 65536
#define ROOT_PACKAGEFILENAME "package.xml"

namespace AMCCommon {

	// Throws a resource package related error as a LibEOS interface exception.
	static void throwResourcePackageError(const std::string& sMessage)
	{
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDSTREAMDATA, sMessage);
	}

	// Asserts that the given pointer is not null.
	static void assertResourcePackagePointer(const void* pPointer, const std::string& sContext)
	{
		if (pPointer == nullptr)
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "null pointer: " + sContext);
	}


	class CResourcePackage_ZIPFilePtr {
	private:
		zip_file_t* m_pFile;
	public:

		CResourcePackage_ZIPFilePtr(zip_file_t* pFile)
			: m_pFile(pFile)
		{
			assertResourcePackagePointer(m_pFile, "zip file pointer");

		}

		~CResourcePackage_ZIPFilePtr()
		{
			if (m_pFile != nullptr) {
				zip_fclose(m_pFile);
			}
			m_pFile = nullptr;
		}

	};


	class CResourcePackageZIP {

	private:
		zip_error_t m_ZIPError;
		zip_t* m_ZIParchive;
		zip_source_t* m_ZIPsource;
		std::map <std::string, uint64_t> m_ZIPEntries;
		std::string m_sZIPDebugName;
		
	public:

		CResourcePackageZIP(const uint64_t nZIPStreamBufferSize, const uint8_t* pZIPStreamBuffer, const std::string & sZIPDebugName)
			: m_ZIParchive(nullptr), m_ZIPsource(nullptr), m_sZIPDebugName (sZIPDebugName)

		{
			assertResourcePackagePointer(pZIPStreamBuffer, "zip stream buffer");

			m_ZIPError.str = nullptr;
			m_ZIPError.sys_err = 0;
			m_ZIPError.zip_err = 0;

			zip_error_init(&m_ZIPError);

			m_ZIPsource = zip_source_buffer_create(pZIPStreamBuffer, (size_t)nZIPStreamBufferSize, 0, &m_ZIPError);

			if (m_ZIPsource == nullptr)
				throwResourcePackageError("could not read ZIP file: " + sZIPDebugName);

			m_ZIParchive = zip_open_from_source(m_ZIPsource, ZIP_RDONLY | ZIP_CHECKCONS, &m_ZIPError);
			if (m_ZIParchive == nullptr)
				throwResourcePackageError("could not open ZIP file: " + sZIPDebugName);

			int64_t nEntryCount = zip_get_num_entries(m_ZIParchive, ZIP_FL_UNCHANGED);
			if (nEntryCount < 0)
				throwResourcePackageError("could not get ZIP entries: " + sZIPDebugName);

			for (int64_t nIndex = 0; nIndex < nEntryCount; nIndex++) {
				const char* pszName = zip_get_name(m_ZIParchive, (uint64_t)nIndex, ZIP_FL_ENC_GUESS);
				m_ZIPEntries.insert(std::make_pair(pszName, nIndex));
			}

		}


		~CResourcePackageZIP()
		{
			if (m_ZIParchive != nullptr)
				zip_close(m_ZIParchive);

			if (m_ZIPsource != nullptr)
				zip_source_close(m_ZIPsource);

			zip_error_fini(&m_ZIPError);

			m_ZIPsource = nullptr;
			m_ZIParchive = nullptr;

		}


		bool hasFile(const std::string& sName)
		{
			auto iIter = m_ZIPEntries.find(sName);
			return (iIter != m_ZIPEntries.end());
		}


		void unzipFileEx(const std::string& sName, uint8_t* pBuffer, const uint64_t nBufferSize)
		{
			auto iIter = m_ZIPEntries.find(sName);
			if (iIter == m_ZIPEntries.end())
				throwResourcePackageError("ZIP entry not found: " + m_sZIPDebugName + "|" + sName);

			zip_stat_t Stat;
			int32_t nResult = zip_stat_index(m_ZIParchive, iIter->second, ZIP_FL_UNCHANGED, &Stat);
			if (nResult != 0)
				throwResourcePackageError("could not stat ZIP entry: " + m_sZIPDebugName + "|" + sName);

			uint64_t nSize = Stat.size;

			zip_file_t* pFile = zip_fopen_index(m_ZIParchive, iIter->second, ZIP_FL_UNCHANGED);
			if (pFile == nullptr)
				throwResourcePackageError("could not open ZIP entry: " + m_sZIPDebugName + "|" + sName);

			CResourcePackage_ZIPFilePtr pZIPFilePtr(pFile);
			
			if (nSize > 0) {

				assertResourcePackagePointer(pBuffer, "read buffer");
				if (nBufferSize < nSize)
					throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_BUFFERTOOSMALL, "buffer too small: " + m_sZIPDebugName + "|" + sName);

				uint64_t cbBytesLeft = nSize;
				uint64_t cbBytesRead = 0;

				uint8_t* pData = pBuffer;
				while (cbBytesLeft > 0) {
					uint32_t cbBytesToRead;
					if (cbBytesLeft > ROOT_ZIP_READCHUNKSIZE)
						cbBytesToRead = ROOT_ZIP_READCHUNKSIZE;
					else
						cbBytesToRead = (uint32_t)cbBytesLeft;
					cbBytesLeft -= cbBytesToRead;

					uint64_t readSize = zip_fread(pFile, pData, cbBytesToRead);
					if (readSize < 0)
						throwResourcePackageError("could not read ZIP stream: " + m_sZIPDebugName + "|" + sName);
					cbBytesRead += readSize;

					if (readSize != (uint64_t)cbBytesToRead)
						break;

					pData += readSize;
				}


				if ((uint64_t)cbBytesRead != nSize) {
					throwResourcePackageError("could not read full ZIP data: " + m_sZIPDebugName + "|" + sName);
				}

			}

		}


		void unzipFile(const std::string& sName, std::vector<uint8_t>& Buffer)
		{

			auto iIter = m_ZIPEntries.find(sName);
			if (iIter == m_ZIPEntries.end())
				throwResourcePackageError("ZIP entry not found: " + m_sZIPDebugName + "|" + sName);

			zip_stat_t Stat;
			int32_t nResult = zip_stat_index(m_ZIParchive, iIter->second, ZIP_FL_UNCHANGED, &Stat);
			if (nResult != 0)
				throwResourcePackageError("could not stat ZIP entry: " + m_sZIPDebugName + "|" + sName);

			uint64_t nSize = Stat.size;

			zip_file_t* pFile = zip_fopen_index(m_ZIParchive, iIter->second, ZIP_FL_UNCHANGED);
			if (pFile == nullptr)
				throwResourcePackageError("could not open ZIP entry: " + m_sZIPDebugName + "|" + sName);

			CResourcePackage_ZIPFilePtr pZIPFilePtr(pFile);

			Buffer.resize(nSize);
			if (nSize > 0) {

				uint64_t cbBytesLeft = nSize;
				uint64_t cbBytesRead = 0;

				uint8_t* pData = Buffer.data();
				while (cbBytesLeft > 0) {
					uint32_t cbBytesToRead;
					if (cbBytesLeft > ROOT_ZIP_READCHUNKSIZE)
						cbBytesToRead = ROOT_ZIP_READCHUNKSIZE;
					else
						cbBytesToRead = (uint32_t)cbBytesLeft;
					cbBytesLeft -= cbBytesToRead;

					uint64_t readSize = zip_fread(pFile, pData, cbBytesToRead);
					if (readSize < 0)
						throwResourcePackageError("could not read ZIP stream: " + m_sZIPDebugName + "|" + sName);
					cbBytesRead += readSize;

					if (readSize != (uint64_t)cbBytesToRead)
						break;

					pData += readSize;
				}


				if ((uint64_t)cbBytesRead != nSize) {
					throwResourcePackageError("could not read full ZIP data: " + m_sZIPDebugName + "|" + sName);
				}

			}
		}


	};
	


	CResourcePackageEntry::CResourcePackageEntry(const std::string& sUUID,  const std::string& sName, const std::string& sFileName, const std::string& sExtension, const std::string& sContentType, uint32_t nSize)
			: m_sName (sName), m_sFileName (sFileName), m_sContentType (sContentType), m_nSize (nSize), m_sUUID (AMCCommon::CUtils::normalizeUUIDString (sUUID)), m_sExtension (sExtension)
	{

		if (!AMCCommon::CUtils::stringCharactersAreValid(m_sName, false, true, true, true, true, true, true))
			throwResourcePackageError("invalid package entry name: " + m_sName);

		if (!m_sExtension.empty()) {
			if (!AMCCommon::CUtils::stringCharactersAreValid(m_sExtension, false, true, true, true, true, true, true))
				throwResourcePackageError("invalid package entry extension: " + m_sExtension);
		}

		if (!AMCCommon::CUtils::stringCharactersAreValid(m_sFileName, false, true, true, true, true, true, true))
			throwResourcePackageError("invalid package entry filename: " + m_sFileName);

	}

	std::string CResourcePackageEntry::getName()
	{
		return m_sName;
	}

	std::string CResourcePackageEntry::getFileName()
	{
		return m_sFileName;
	}

	std::string CResourcePackageEntry::getContentType()
	{
		return m_sContentType;
	}

	std::string CResourcePackageEntry::getUUID()
	{
		return m_sUUID;
	}

	uint32_t CResourcePackageEntry::getSize()
	{
		return m_nSize;
	}

	std::string CResourcePackageEntry::getExtension()
	{
		return m_sExtension;
	}



    PResourcePackage CResourcePackage::makeFromStream(AMCCommon::PImportStream pStream, const std::string& sPackageDebugName, const std::vector<std::string>& acceptedSchemaNamespaces)
	{
		return makeFromStream(pStream.get(), sPackageDebugName, acceptedSchemaNamespaces);
	}

	PResourcePackage CResourcePackage::makeFromStream(AMCCommon::CImportStream* pStream, const std::string& sPackageDebugName, const std::vector<std::string>& acceptedSchemaNamespaces)
	{
		assertResourcePackagePointer(pStream, "import stream");

		return std::make_shared<CResourcePackage>(pStream, sPackageDebugName, acceptedSchemaNamespaces);
	}

	PResourcePackage CResourcePackage::makeEmpty(const std::string& sPackageDebugName)
	{
		return std::make_shared<CResourcePackage>(sPackageDebugName);
	}

	CResourcePackage::CResourcePackage(const std::string& sPackageDebugName)
		: m_sPackageDebugName (sPackageDebugName)
	{
		m_pResourcePackageZIP = nullptr;
	}


	CResourcePackage::CResourcePackage(AMCCommon::CImportStream* pStream, const std::string& sPackageDebugName, const std::vector<std::string>& acceptedSchemaNamespaces)
		: m_sPackageDebugName (sPackageDebugName)
	{
		assertResourcePackagePointer(pStream, "import stream");

		pStream->readIntoMemory(m_ZIPBuffer);

		if (m_ZIPBuffer.size () == 0)
			throwResourcePackageError("could not parse resource index: " + m_sPackageDebugName);

		m_pResourcePackageZIP = std::make_shared<CResourcePackageZIP>(m_ZIPBuffer.size(), m_ZIPBuffer.data(), sPackageDebugName);

		if (!m_pResourcePackageZIP->hasFile(ROOT_PACKAGEFILENAME))
			throwResourcePackageError("could not find resource index: " + m_sPackageDebugName);

		std::vector<uint8_t> DistBuffer;
		m_pResourcePackageZIP->unzipFile(ROOT_PACKAGEFILENAME, DistBuffer);

		if (DistBuffer.size() == 0)
			throwResourcePackageError("empty resource index: " + m_sPackageDebugName);

		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_buffer(DistBuffer.data(), DistBuffer.size());
		if (!result)
			throwResourcePackageError("could not parse resource index: " + m_sPackageDebugName);

		auto rootNode = doc.child("package");
		if (rootNode.empty())
			throwResourcePackageError("missing resource index: " + m_sPackageDebugName);

		auto xmlnsAttrib = rootNode.attribute("xmlns");
		if (xmlnsAttrib.empty())
			throwResourcePackageError("missing XML schema: " + m_sPackageDebugName);

		std::string xmlns(xmlnsAttrib.as_string());
		bool bSchemaAccepted = false;
		std::string sAcceptedList;
		for (const auto& sAccepted : acceptedSchemaNamespaces) {
			if (xmlns == sAccepted) {
				bSchemaAccepted = true;
				break;
			}
			if (!sAcceptedList.empty())
				sAcceptedList += ", ";
			sAcceptedList += sAccepted;
		}
		if (!bSchemaAccepted)
			throwResourcePackageError("invalid XML schema: " + m_sPackageDebugName + " (found \"" + xmlns + "\", expected one of: " + sAcceptedList + ")");

		auto entryNodes = rootNode.children("entry");
		for (pugi::xml_node entryNode : entryNodes) {
			auto nameAttrib = entryNode.attribute("name");
			if (nameAttrib.empty())
				throwResourcePackageError("missing resource name: " + m_sPackageDebugName);

			auto extensionAttrib = entryNode.attribute("extension");

			auto fileNameAttrib = entryNode.attribute("filename");
			if (fileNameAttrib.empty())
				throwResourcePackageError("missing resource filename: " + m_sPackageDebugName);

			auto sizeAttrib = entryNode.attribute("size");
			if (sizeAttrib.empty())
				throwResourcePackageError("missing resource size: " + m_sPackageDebugName);

			auto contenttypeAttrib = entryNode.attribute("contenttype");
			if (contenttypeAttrib.empty())
				throwResourcePackageError("missing resource content type: " + m_sPackageDebugName);

			std::string sName = nameAttrib.as_string();
			std::string sFileName = fileNameAttrib.as_string();
			std::string sContentType = contenttypeAttrib.as_string();
			std::string sUUID = AMCCommon::CUtils::createUUID();
			std::string sExtension = extensionAttrib.as_string();
			uint32_t nSize = sizeAttrib.as_uint();

			auto pEntry = std::make_shared <CResourcePackageEntry> (sUUID, sName, sFileName, sExtension, sContentType, nSize);
			m_Entries.push_back(pEntry);
			m_NameMap.insert(std::make_pair(sName, pEntry));
			m_UUIDMap.insert(std::make_pair (sUUID, pEntry));

		}


	}

	CResourcePackage::~CResourcePackage()
	{

	}


	uint64_t CResourcePackage::getEntryCount()
	{
		return m_Entries.size();
	}


	PResourcePackageEntry CResourcePackage::getEntry(uint64_t nIndex)
	{
		if (nIndex >= m_Entries.size())
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "invalid resource index: " + m_sPackageDebugName);

		return m_Entries[nIndex];
	}

	PResourcePackageEntry CResourcePackage::findEntryByUUID(const std::string& sUUID, const bool bHasToExist)
	{
		auto iIter = m_UUIDMap.find(AMCCommon::CUtils::normalizeUUIDString (sUUID));
		if (iIter == m_UUIDMap.end()) {
			if (bHasToExist)
				throwResourcePackageError("resource entry not found: " + m_sPackageDebugName + "/" + sUUID);

			return nullptr;
		}

		return iIter->second;
	}


	PResourcePackageEntry CResourcePackage::findEntryByName(const std::string& sName, const bool bHasToExist)
	{
		auto iIter = m_NameMap.find(sName);
		if (iIter == m_NameMap.end()) {
			if (bHasToExist)
				throwResourcePackageError("resource entry not found: " + m_sPackageDebugName + "/" + sName);

			return nullptr;
		}

		return iIter->second;
	}





	void CResourcePackage::readEntry(const std::string& sName, std::vector<uint8_t>& Buffer)
	{
		std::lock_guard<std::mutex> lockGuard(m_Mutex);

		auto iIter = m_NameMap.find(sName);
		if (iIter == m_NameMap.end())
			throwResourcePackageError("resource entry not found: " + m_sPackageDebugName + "/" + sName);

		auto sFileName = iIter->second->getFileName ();
		m_pResourcePackageZIP->unzipFile(sFileName, Buffer);

	}


	void CResourcePackage::readEntryEx(const std::string& sName, uint8_t* pBuffer, const uint64_t nBufferSize)
	{
		std::lock_guard<std::mutex> lockGuard(m_Mutex);

		auto iIter = m_NameMap.find(sName);
		if (iIter == m_NameMap.end())
			throwResourcePackageError("resource entry not found: " + m_sPackageDebugName + "/" + sName);

		auto sFileName = iIter->second->getFileName();
		m_pResourcePackageZIP->unzipFileEx(sFileName, pBuffer, nBufferSize);

	}



	class CResourcePackageBuilderZIP {
	private:
		AMCCommon::PExportStream m_pExportStream;

		AMCCommon::PPortableZIPWriter m_pZIPWriter;

		std::string m_sSchemaNamespace;

		std::stringstream m_PackageStreamData;

	public:
		CResourcePackageBuilderZIP(AMCCommon::PExportStream pExportStream, const std::string& sSchemaNamespace)
			: m_pExportStream(pExportStream), m_sSchemaNamespace (sSchemaNamespace)
		{
			if (pExportStream.get() == nullptr)
				throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "invalid export stream");

			m_pZIPWriter = std::make_shared<AMCCommon::CPortableZIPWriter>(pExportStream, true);


			m_PackageStreamData << "<package xmlns=\"" << sSchemaNamespace << "\">" << std::endl;

		}

		virtual ~CResourcePackageBuilderZIP()
		{
			m_pZIPWriter = nullptr;
		}

		void addEntry(const std::string& sName, const std::string& sFileName, const std::string& sExtension, const std::string& sContentType, const std::vector<uint8_t> entryData)
		{					

			m_PackageStreamData << "  <entry name=\"" << sName << "\" filename=\""<< sFileName <<"\" extension=\""<< sExtension << "\" size=\""<< entryData.size ()<<"\" contenttype=\""<< sContentType  << "\"/>" << std::endl;

			auto pEntry = m_pZIPWriter->createEntry(sFileName, AMCCommon::CUtils::getUnixTimeStamp());
			if (entryData.size() > 0)
				pEntry->writeBuffer(entryData.data(), entryData.size());
			m_pZIPWriter->closeEntry();
			
		}

		void finalize()
		{
			m_PackageStreamData << "</package>" << std::endl;
			std::string sPackageXMLString = m_PackageStreamData.str();

			auto pEntry = m_pZIPWriter->createEntry("package.xml", AMCCommon::CUtils::getUnixTimeStamp());
			if (sPackageXMLString.length() > 0)
				pEntry->writeBuffer((void*)sPackageXMLString.c_str(), sPackageXMLString.length());
			m_pZIPWriter->closeEntry();

			m_pZIPWriter = nullptr;
		}

	};


	CResourcePackageBuilder::CResourcePackageBuilder(AMCCommon::PExportStream pExportStream, const std::string& sSchemaNamespace)
	{
		m_pBuilderZIP = std::make_shared<CResourcePackageBuilderZIP>(pExportStream, sSchemaNamespace);
	}

	CResourcePackageBuilder::~CResourcePackageBuilder()
	{

	}


	void CResourcePackageBuilder::addEntry(const std::string& sName, const std::string& sFileName, const std::string& sExtension, const std::string& sContentType, const std::vector<uint8_t> entryData)
	{
		m_pBuilderZIP->addEntry(sName, sFileName, sExtension, sContentType, entryData);
	}

	void CResourcePackageBuilder::finalize()
	{
		m_pBuilderZIP->finalize();
	}


}
