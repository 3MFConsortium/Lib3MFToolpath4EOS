/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: This is the class definition of CToolpathComputationFuture

*/

#include "libeos_toolpathcomputationfuture.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"
#include "libeos_eosto3mfconverter.hpp"
#include "libeos_3mfjobloader.hpp"

using namespace Lib3MF4EOS::Impl;

/*************************************************************************************************************************
 Class definition of CToolpathComputationFuture 
**************************************************************************************************************************/

CToolpathComputationFuture::CToolpathComputationFuture(
	Lib3MF::PModel pInputModel,
	Lib3MF::PWrapper pLib3MFWrapper,
	std::unique_ptr<CTaskCreationFuture> pTaskCreationFuture,
	const std::string& sMaterialPath,
	const CNamespaceRegistry& registry)
	: m_pInputModel(pInputModel), m_pLib3MFWrapper(pLib3MFWrapper), m_pTaskCreationFuture(std::move(pTaskCreationFuture)), m_sMaterialPath(sMaterialPath), m_NamespaceRegistry(registry)
{
	if (m_pInputModel.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "input model is null");
	if (m_pLib3MFWrapper.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "lib3mf wrapper is null");
	if (m_pTaskCreationFuture.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "task creation future is null");
}

CToolpathComputationFuture::~CToolpathComputationFuture()
{
}

void CToolpathComputationFuture::ensureFinished()
{
	if (!m_pTaskCreationFuture->HasFinished()) {
		if (m_pTaskCreationFuture->HasFailed())
			throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHCOMPUTEFAILED, "toolpath computation failed: " + m_pTaskCreationFuture->GetErrorMessage());
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHCOMPUTENOTFINISHED);
	}
}

bool CToolpathComputationFuture::IsRunning()
{
	return m_pTaskCreationFuture->IsRunning();
}

void CToolpathComputationFuture::Abort()
{
	m_pTaskCreationFuture->Abort();
}

bool CToolpathComputationFuture::HasFinished()
{
	return m_pTaskCreationFuture->HasFinished();
}

bool CToolpathComputationFuture::HasFailed()
{
	return m_pTaskCreationFuture->HasFailed();
}

bool CToolpathComputationFuture::HasBeenAborted()
{
	return m_pTaskCreationFuture->HasBeenAborted();
}

std::string CToolpathComputationFuture::GetErrorMessage()
{
	return m_pTaskCreationFuture->GetErrorMessage();
}

void CToolpathComputationFuture::GetProgress(Lib3MF4EOS_uint32 & nCurrentLayer, Lib3MF4EOS_uint32 & nNumberOfLayers, Lib3MF4EOS_uint32 & nCurrentZHeight, Lib3MF4EOS_uint32 & nTargetZHeight)
{
	m_pTaskCreationFuture->GetProgress(nCurrentLayer, nNumberOfLayers, nCurrentZHeight, nTargetZHeight);
}

IEOSRawTask * CToolpathComputationFuture::LoadRawTask()
{
	ensureFinished();
	return m_pTaskCreationFuture->LoadRawTask();
}


void CToolpathComputationFuture::WriteResultTo3MF(Lib3MF::PModel pOutputModel, Lib3MF::PWriter pOutputWriter)
{
	ensureFinished();

	if (pOutputModel.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "output model is null");
	if (pOutputWriter.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "output writer is null");

	pOutputWriter->RegisterCustomNamespace("eos", LIBEOS_NAMESPACE_TOOLPATHPROFILE);
	pOutputWriter->RegisterCustomNamespace("eos3mf", LIBEOS_NAMESPACE_JOBMETADATA);

	SEOS3MFMetadata meta = readModelMetadata(m_pInputModel, m_NamespaceRegistry);
	writeModelMetadata(pOutputModel, meta);

	if (!m_sMaterialPath.empty())
		attachMaterialSetToModel(pOutputModel, m_sMaterialPath);

	std::unique_ptr<IEOSRawTask> pRawTask(m_pTaskCreationFuture->LoadRawTask());
	uint32_t nLayerCount = pRawTask->GetNumberOfLayers();
	uint32_t nFirstLayerIndex = pRawTask->GetFirstLayerIndex();

	CEOSTo3MFConverter converter(m_pLib3MFWrapper, pOutputModel, pOutputWriter);

	collectAndRegisterMeshes(m_pInputModel, converter, m_NamespaceRegistry);

	if (nLayerCount > 0) {
		// The bottom Z needs the heights of the first two layers before the first layer is written.
		std::unique_ptr<IEOSRawTaskLayer> pFirstLayer(pRawTask->ReadLayer(nFirstLayerIndex));
		std::unique_ptr<IEOSRawTaskLayer> pSecondLayer;
		if (nLayerCount > 1)
			pSecondLayer.reset(pRawTask->ReadLayer(nFirstLayerIndex + 1));

		int64_t nSecondLayerZ = pSecondLayer ? pSecondLayer->GetZHeight() : 0;
		converter.setBottomZ(CEOSTo3MFConverter::computeBottomZInMM(nFirstLayerIndex, nLayerCount, pFirstLayer->GetZHeight(), nSecondLayerZ));

		converter.convertLayer(pFirstLayer.get());
		if (pSecondLayer)
			converter.convertLayer(pSecondLayer.get());

		for (uint32_t nLayerOffset = 2; nLayerOffset < nLayerCount; nLayerOffset++) {
			std::unique_ptr<IEOSRawTaskLayer> pEOSLayer(pRawTask->ReadLayer(nFirstLayerIndex + nLayerOffset));
			converter.convertLayer(pEOSLayer.get());
		}
	}

	converter.finish();
}


void CToolpathComputationFuture::WriteResultTo3MFFile(const std::string & sFilePath)
{
	ensureFinished();

	auto pOutModel = m_pLib3MFWrapper->CreateModel();
	auto pOutWriter = pOutModel->QueryWriter("3mf");

	WriteResultTo3MF(pOutModel, pOutWriter);

	pOutWriter->WriteToFile(sFilePath);
}
