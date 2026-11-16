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


Abstract: This is the class declaration of CEOSSDKSimulator

*/


#ifndef __LIBEOS_EOSSDKSIMULATOR
#define __LIBEOS_EOSSDKSIMULATOR

#include "lib3mftoolpath4eos_interfaces.hpp"
#include "libeos_eossdk.hpp"

// Parent classes
#include "libeos_base.hpp"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4250)
#endif

// Include custom headers here.


namespace Lib3MF4EOS {
namespace Impl {


/*************************************************************************************************************************
 Class declaration of CEOSSDKSimulator 
**************************************************************************************************************************/

class CEOSSDKSimulator : public virtual IEOSSDKSimulator, public virtual CBase {
private:

    PEOSSDK m_pEOSSDK;

    PToolpathRecorder getRecorder();

protected:

public:

    CEOSSDKSimulator(PEOSSDK pEOSSDK);

    virtual ~CEOSSDKSimulator();

	std::string GetAPIVersion() override;

	bool IsRecording() override;

	Lib3MF4EOS_uint64 GetRecordedCommandCount() override;

	std::string GetRecordedCommandName(const Lib3MF4EOS_uint64 nIndex) override;

	void GetRecordedCommandArguments(const Lib3MF4EOS_uint64 nIndex, Lib3MF4EOS_uint64 nArgumentsBufferSize, Lib3MF4EOS_uint64* pArgumentsNeededCount, Lib3MF4EOS_double* pArgumentsBuffer) override;

	std::string GetRecordedCommandsAsText() override;

	Lib3MF4EOS_uint64 GetRecordedLayerCount() override;

	void ClearRecording() override;

	void SetNumberOfLaserScanners(const Lib3MF4EOS_uint32 nCount) override;

	Lib3MF4EOS_uint32 GetNumberOfLaserScanners() override;

};

} // namespace Impl
} // namespace Lib3MF4EOS

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif // __LIBEOS_EOSSDKSIMULATOR
