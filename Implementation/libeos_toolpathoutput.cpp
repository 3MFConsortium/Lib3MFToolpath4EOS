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


Abstract: The SDK output and the recording output of a raw task writer.

*/

#include "libeos_toolpathoutput.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"
#include "../Common/common_utils.hpp"

#include <cmath>

namespace Lib3MF4EOS {
namespace Impl {

template<typename TFunction>
static TFunction requireEntryPoint(TFunction pFunction, const char* pszName)
{
	if (pFunction == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHSDKNOTSUPPORTED, std::string("the loaded EOS SDK does not export ") + pszName);
	return pFunction;
}

CToolpathOutputSDK::CToolpathOutputSDK(PEOSSDK pSDK, const std::string& sFileName)
	: m_pSDK(pSDK), m_hFile(0)
{
	if (pSDK.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM);

	requireEntryPoint(m_pSDK->EosToolpath_OpenOutputFile, "EosToolpath_OpenOutputFile");
	requireEntryPoint(m_pSDK->EosToolpath_CloseFile, "EosToolpath_CloseFile");
	requireEntryPoint(m_pSDK->EosToolpath_WriteBeginLayer, "EosToolpath_WriteBeginLayer");
	requireEntryPoint(m_pSDK->EosToolpath_WriteBeginPart, "EosToolpath_WriteBeginPart");
	requireEntryPoint(m_pSDK->EosToolpath_WriteHatch, "EosToolpath_WriteHatch");
	requireEntryPoint(m_pSDK->EosToolpath_WritePolyStart, "EosToolpath_WritePolyStart");
	requireEntryPoint(m_pSDK->EosToolpath_WritePolyCoord, "EosToolpath_WritePolyCoord");
	requireEntryPoint(m_pSDK->EosToolpath_WritePolyEnd, "EosToolpath_WritePolyEnd");

	std::wstring sFileNameW = AMCCommon::CUtils::UTF8toUTF16(sFileName);
	m_pSDK->checkEOSError(m_pSDK->EosToolpath_OpenOutputFile(sFileNameW.c_str(), &m_hFile));
	if (m_hFile == 0)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_COULDNOTOPENTASKFILE, "could not open output task file: " + sFileName);
}

CToolpathOutputSDK::~CToolpathOutputSDK()
{
	if (m_hFile != 0) {
		m_pSDK->EosToolpath_CloseFile(m_hFile);
		m_hFile = 0;
	}
}

void CToolpathOutputSDK::write(const SToolpathCommand& command)
{
	if (m_hFile == 0)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TASKHASBEENRELEASED);

	auto& sdk = *m_pSDK;
	auto& a = command.m_Arguments;
	EOS_ERROR error = EOS_ERR_NO_ERROR;

	// Scanner selection, exposure type, skywriting, speed and focus are left out if the SDK cannot write them.
	switch (command.m_eCommand) {
	case eToolpathCommand::BeginLayer:
		error = sdk.EosToolpath_WriteBeginLayer(m_hFile, (EOS_MICROMETER)std::llround(a[0]));
		break;
	case eToolpathCommand::BeginPart:
		error = sdk.EosToolpath_WriteBeginPart(m_hFile, (EOS_INT)std::llround(a[0]));
		break;
	case eToolpathCommand::SelectLaserScanner:
		if (sdk.EosToolpath_WriteSelectLaserScanner != nullptr)
			error = sdk.EosToolpath_WriteSelectLaserScanner(m_hFile, (EOS_BYTE)std::llround(a[0]));
		break;
	case eToolpathCommand::SetLaserPower:
		error = requireEntryPoint(sdk.EosToolpath_WriteSetLaserPower, "EosToolpath_WriteSetLaserPower")(m_hFile, a[0], a[1]);
		break;
	case eToolpathCommand::SetScanSpeed:
		if (sdk.EosToolpath_WriteSetScanSpeed != nullptr)
			error = sdk.EosToolpath_WriteSetScanSpeed(m_hFile, a[0]);
		break;
	case eToolpathCommand::SetFocus:
		if (sdk.EosToolpath_WriteSetFocus != nullptr)
			error = sdk.EosToolpath_WriteSetFocus(m_hFile, a[0]);
		break;
	case eToolpathCommand::SetSkywriting:
		if (sdk.EosToolpath_WriteSetSkywriting != nullptr)
			error = sdk.EosToolpath_WriteSetSkywriting(m_hFile, (a[0] != 0.0) ? EOS_BOOL::EOS_TRUE : EOS_BOOL::EOS_FALSE);
		break;
	case eToolpathCommand::SetExposureType:
		if (sdk.EosToolpath_WriteSetExposureType != nullptr)
			error = sdk.EosToolpath_WriteSetExposureType(m_hFile, (EOS_EXPOSURE_TYPE)std::llround(a[0]));
		break;
	case eToolpathCommand::SetExposureStep:
		error = requireEntryPoint(sdk.EosToolpath_WriteSetExposureStep, "EosToolpath_WriteSetExposureStep")(m_hFile, (EOS_UINT)std::llround(a[0]), (EOS_UINT)std::llround(a[1]));
		break;
	case eToolpathCommand::SetLaserPulse:
		error = requireEntryPoint(sdk.EosToolpath_WriteSetLaserPulse, "EosToolpath_WriteSetLaserPulse")(m_hFile, a[0], a[1]);
		break;
	case eToolpathCommand::SetDepth:
		error = requireEntryPoint(sdk.EosToolpath_WriteSetDepth, "EosToolpath_WriteSetDepth")(m_hFile, (EOS_MICROMETER)std::llround(a[0]));
		break;
	case eToolpathCommand::SetBeamShape:
		error = sdk.getAdapter().writeSetBeamShape(m_hFile, (EOS_BEAMSHAPE)std::llround(a[0]), (EOS_INT)std::llround(a[1]));
		break;
	case eToolpathCommand::Delay:
		error = requireEntryPoint(sdk.EosToolpath_WriteDelay, "EosToolpath_WriteDelay")(m_hFile, a[0]);
		break;
	case eToolpathCommand::Hatch:
		error = sdk.EosToolpath_WriteHatch(m_hFile, { a[0], a[1] }, { a[2], a[3] });
		break;
	case eToolpathCommand::PolyStart:
		error = sdk.EosToolpath_WritePolyStart(m_hFile, { a[0], a[1] });
		break;
	case eToolpathCommand::PolyCoord:
		error = sdk.EosToolpath_WritePolyCoord(m_hFile, { a[0], a[1] }, a[2]);
		break;
	case eToolpathCommand::PolyEnd:
		error = sdk.EosToolpath_WritePolyEnd(m_hFile, { a[0], a[1] });
		break;
	default:
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "the toolpath command " + toolpathCommandName(command.m_eCommand) + " cannot be written to a file");
	}

	m_pSDK->checkEOSError(error);
}

void CToolpathOutputSDK::close()
{
	if (m_hFile == 0)
		return;
	EOS_FILE_HANDLE hFile = m_hFile;
	m_hFile = 0;
	m_pSDK->checkEOSError(m_pSDK->EosToolpath_CloseFile(hFile));
}


CToolpathOutputRecording::CToolpathOutputRecording(PToolpathRecorder pRecorder)
	: m_pRecorder(pRecorder), m_bClosed(false)
{
	if (pRecorder.get() == nullptr)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM);
	m_pRecorder->record(SToolpathCommand(eToolpathCommand::OpenFile, {}));
}

void CToolpathOutputRecording::write(const SToolpathCommand& command)
{
	if (m_bClosed)
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TASKHASBEENRELEASED);
	m_pRecorder->record(command);
}

void CToolpathOutputRecording::close()
{
	if (m_bClosed)
		return;
	m_bClosed = true;
	m_pRecorder->record(SToolpathCommand(eToolpathCommand::CloseFile, {}));
}

} // namespace Impl
} // namespace Lib3MF4EOS
