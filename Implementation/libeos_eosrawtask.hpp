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

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE 3MF CONSORTIUM BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Abstract: This is the class declaration of CEOSRawTask

*/


#ifndef __LIBEOS_EOSRAWTASK
#define __LIBEOS_EOSRAWTASK

#include "lib3mftoolpath4eos_interfaces.hpp"

// Parent classes
#include "libeos_base.hpp"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4250)
#endif

// Include custom headers here.
#include "libeos_eossdk.hpp"
#include <mutex>

#include "../Common/common_workingdirectory.hpp"

namespace Lib3MF4EOS {
namespace Impl {


/*************************************************************************************************************************
 Class declaration of CEOSRawTask 
**************************************************************************************************************************/

class CEOSRawTask : public virtual IEOSRawTask, public virtual CBase {
private:

    AMCCommon::PWorkingDirectory m_pTaskDirectory;
    PEOSSDK m_pSDK;
	std::string m_sAbsoluteTaskFileName;

	EOS_FILE_HANDLE m_hTaskFile;

	void queryLayerRange(uint64_t& nLayerCount, uint64_t& nFirstLayerIndex);

public:

    CEOSRawTask(PEOSSDK pSDK, AMCCommon::PWorkingDirectory pTaskDirectory, const std::string & sAbsoluteTaskFileName);

	virtual ~CEOSRawTask();

	std::string GetAbsoluteFilename() override;

	std::string GetTaskDirectory() override;

	Lib3MF4EOS_uint32 GetNumberOfLayers() override;

	Lib3MF4EOS_uint32 GetFirstLayerIndex() override;

	void Release() override;

    IEOSRawTaskLayer* ReadLayer(const Lib3MF4EOS_uint32 nLayerIndex) override;

};

} // namespace Impl
} // namespace Lib3MF4EOS

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif // __LIBEOS_EOSRAWTASK
