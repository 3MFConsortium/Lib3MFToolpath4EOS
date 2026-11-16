/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: This is the class declaration of CToolpathComputationFuture

*/


#ifndef __LIBEOS_TOOLPATHCOMPUTATIONFUTURE
#define __LIBEOS_TOOLPATHCOMPUTATIONFUTURE

#include "lib3mftoolpath4eos_interfaces.hpp"

#include "libeos_future.hpp"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4250)
#endif

#include "libeos_taskcreationfuture.hpp"
#include "libeos_namespaceregistry.hpp"
#include <memory>

namespace Lib3MF4EOS {
namespace Impl {


/*************************************************************************************************************************
 Class declaration of CToolpathComputationFuture 
**************************************************************************************************************************/

class CToolpathComputationFuture : public virtual IToolpathComputationFuture, public virtual CFuture {
private:
	Lib3MF::PModel m_pInputModel;
	Lib3MF::PWrapper m_pLib3MFWrapper;
	std::unique_ptr<CTaskCreationFuture> m_pTaskCreationFuture;
	std::string m_sMaterialPath;
	CNamespaceRegistry m_NamespaceRegistry;

	void ensureFinished();

public:

	CToolpathComputationFuture(
		Lib3MF::PModel pInputModel,
		Lib3MF::PWrapper pLib3MFWrapper,
		std::unique_ptr<CTaskCreationFuture> pTaskCreationFuture,
		const std::string& sMaterialPath,
		const CNamespaceRegistry& registry);

	virtual ~CToolpathComputationFuture();

	bool IsRunning() override;
	void Abort() override;
	bool HasFinished() override;
	bool HasFailed() override;
	bool HasBeenAborted() override;
	std::string GetErrorMessage() override;

	void GetProgress(Lib3MF4EOS_uint32 & nCurrentLayer, Lib3MF4EOS_uint32 & nNumberOfLayers, Lib3MF4EOS_uint32 & nCurrentZHeight, Lib3MF4EOS_uint32 & nTargetZHeight) override;

	void WriteResultTo3MFFile(const std::string & sFilePath) override;
	void WriteResultTo3MF(Lib3MF::PModel pOutputModel, Lib3MF::PWriter pOutputWriter) override;
	IEOSRawTask * LoadRawTask() override;
};

} // namespace Impl
} // namespace Lib3MF4EOS

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif // __LIBEOS_TOOLPATHCOMPUTATIONFUTURE
