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


Abstract: Toolpath commands, the recorder and the fingerprint.

*/

#include "libeos_toolpathcommands.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"
#include "libeos_jsonwriter.hpp"

#include <cmath>
#include <cstdio>

namespace Lib3MF4EOS {
namespace Impl {

SToolpathCommand::SToolpathCommand(eToolpathCommand eCommand, std::initializer_list<double> arguments)
	: m_eCommand(eCommand), m_nArgumentCount(0), m_Arguments({ 0.0, 0.0, 0.0, 0.0 })
{
	if (arguments.size() > m_Arguments.size())
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "too many toolpath command arguments");
	for (double dArgument : arguments)
		m_Arguments[m_nArgumentCount++] = dArgument;
}

std::string toolpathCommandName(eToolpathCommand eCommand)
{
	switch (eCommand) {
	case eToolpathCommand::OpenFile: return "OpenFile";
	case eToolpathCommand::BeginLayer: return "BeginLayer";
	case eToolpathCommand::BeginPart: return "BeginPart";
	case eToolpathCommand::SelectLaserScanner: return "SelectLaserScanner";
	case eToolpathCommand::SetLaserPower: return "SetLaserPower";
	case eToolpathCommand::SetScanSpeed: return "SetScanSpeed";
	case eToolpathCommand::SetFocus: return "SetFocus";
	case eToolpathCommand::SetSkywriting: return "SetSkywriting";
	case eToolpathCommand::SetExposureType: return "SetExposureType";
	case eToolpathCommand::SetExposureStep: return "SetExposureStep";
	case eToolpathCommand::SetLaserPulse: return "SetLaserPulse";
	case eToolpathCommand::SetDepth: return "SetDepth";
	case eToolpathCommand::SetBeamShape: return "SetBeamShape";
	case eToolpathCommand::Delay: return "Delay";
	case eToolpathCommand::Hatch: return "Hatch";
	case eToolpathCommand::PolyStart: return "PolyStart";
	case eToolpathCommand::PolyCoord: return "PolyCoord";
	case eToolpathCommand::PolyEnd: return "PolyEnd";
	case eToolpathCommand::CloseFile: return "CloseFile";
	default: return "Unknown";
	}
}

std::string toolpathNumberToString(double dValue)
{
	if (!std::isfinite(dValue))
		return std::isnan(dValue) ? "nan" : ((dValue > 0.0) ? "inf" : "-inf");

	char buffer[64];
	std::snprintf(buffer, sizeof(buffer), "%.6f", dValue);
	std::string sValue = buffer;
	size_t nDot = sValue.find('.');
	if (nDot != std::string::npos) {
		size_t nLast = sValue.find_last_not_of('0');
		sValue.erase((nLast == nDot) ? nDot : nLast + 1);
	}
	if (sValue == "-0")
		sValue = "0";
	return sValue;
}

std::string toolpathCommandToText(const SToolpathCommand& command)
{
	std::string sText = toolpathCommandName(command.m_eCommand);
	for (uint32_t nIndex = 0; nIndex < command.m_nArgumentCount; nIndex++)
		sText += " " + toolpathNumberToString(command.m_Arguments[nIndex]);
	return sText;
}


/*************************************************************************************************************************
 CToolpathRecorder
**************************************************************************************************************************/

CToolpathRecorder::CToolpathRecorder()
	: m_nLayerCount(0), m_nLaserScannerCount(0)
{
	resetFileState();
	m_bFileOpen = false;
}

void CToolpathRecorder::setLaserScannerCount(uint32_t nCount)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	m_nLaserScannerCount = nCount;
}

void CToolpathRecorder::resetFileState()
{
	m_bFileOpen = true;
	m_bInLayer = false;
	m_bLayerHasExposure = false;
	m_bScannerSelected = false;
	m_bPartBegun = false;
	m_bPolylineOpen = false;
	m_dLayerZ = 0.0;
	m_nSelectedScanner = 0;
	m_ScannersWithPart.clear();
}

void CToolpathRecorder::violate(const SToolpathCommand& command, const std::string& sRule, const std::string& sExplanation)
{
	throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_TOOLPATHORDERVIOLATION, "recorded toolpath command " + std::to_string(m_Commands.size()) + " (" +
		toolpathCommandToText(command) + ") breaks the rule " + sRule + ": " + sExplanation);
}

void CToolpathRecorder::checkRules(const SToolpathCommand& command)
{
	auto eCommand = command.m_eCommand;

	if (eCommand == eToolpathCommand::OpenFile) {
		resetFileState();
		return;
	}
	if (!m_bFileOpen)
		violate(command, "NO_FILE", "commands need an open toolpath file");

	bool bValueChange = (eCommand == eToolpathCommand::SetLaserPower) || (eCommand == eToolpathCommand::SetScanSpeed) || (eCommand == eToolpathCommand::SetFocus);
	bool bPolylinePoint = (eCommand == eToolpathCommand::PolyCoord) || (eCommand == eToolpathCommand::PolyEnd);
	if (m_bPolylineOpen && !bValueChange && !bPolylinePoint)
		violate(command, "POLYLINE_OPEN", "only polyline points and changes of laser power, scan speed and focus may follow a polyline start until its end");

	if ((eCommand == eToolpathCommand::BeginLayer) || (eCommand == eToolpathCommand::CloseFile)) {
		// A layer without exposure still needs a begun part, as the layers that fill the job's layer stack have.
		if (m_bInLayer && !m_bLayerHasExposure && m_ScannersWithPart.empty())
			violate(command, "EMPTY_LAYER", "the layer at Z " + toolpathNumberToString(m_dLayerZ) + " micrometers has neither exposure nor a begun part");
		// The SDK also refuses a layer in which a laser scanner has no begun part (status 87, LayerDataUnavailable).
		if (m_bInLayer) {
			for (uint32_t nScanner = 0; nScanner < m_nLaserScannerCount; nScanner++)
				if (m_ScannersWithPart.count(nScanner) == 0)
					violate(command, "SCANNER_WITHOUT_PART", "laser scanner " + std::to_string(nScanner) + " has no begun part in the layer at Z " +
						toolpathNumberToString(m_dLayerZ) + " micrometers");
		}
		if (eCommand == eToolpathCommand::CloseFile) {
			m_bFileOpen = false;
			return;
		}
		if (m_bInLayer && (command.m_Arguments[0] <= m_dLayerZ))
			violate(command, "LAYER_ORDER", "the layer Z must be greater than the Z of the previous layer (" + toolpathNumberToString(m_dLayerZ) + " micrometers)");
		m_bInLayer = true;
		m_bLayerHasExposure = false;
		m_bScannerSelected = false;
		m_bPartBegun = false;
		m_ScannersWithPart.clear();
		m_dLayerZ = command.m_Arguments[0];
		m_nLayerCount++;
		return;
	}

	if (!m_bInLayer)
		violate(command, "NO_LAYER", "commands other than BeginLayer need a layer");

	switch (eCommand) {
	case eToolpathCommand::SelectLaserScanner:
		// The SDK refuses a scanner the machine does not have (status 95).
		if ((m_nLaserScannerCount > 0) && !(command.m_Arguments[0] < (double)m_nLaserScannerCount))
			violate(command, "SCANNER_RANGE", "the simulated machine has " + std::to_string(m_nLaserScannerCount) + " laser scanners");
		// The part must be begun again on the new scanner.
		m_bScannerSelected = true;
		m_bPartBegun = false;
		m_nSelectedScanner = (uint32_t)command.m_Arguments[0];
		return;
	case eToolpathCommand::BeginPart:
		if (!m_bScannerSelected)
			violate(command, "NO_SCANNER", "a part needs a selected laser scanner");
		m_bPartBegun = true;
		m_ScannersWithPart.insert(m_nSelectedScanner);
		return;
	case eToolpathCommand::Hatch:
	case eToolpathCommand::PolyStart:
		// The SDK refuses exposure without a part on the selected scanner (status 92).
		if (!m_bPartBegun)
			violate(command, "NO_PART", "an exposure needs a part that was begun after the laser scanner was selected");
		m_bLayerHasExposure = true;
		if (eCommand == eToolpathCommand::PolyStart)
			m_bPolylineOpen = true;
		return;
	case eToolpathCommand::PolyCoord:
	case eToolpathCommand::PolyEnd:
		if (!m_bPolylineOpen)
			violate(command, "NO_POLYLINE", "polyline points need a polyline start");
		if (eCommand == eToolpathCommand::PolyEnd)
			m_bPolylineOpen = false;
		return;
	case eToolpathCommand::SetFocus:
		// The SDK refuses focus values outside 0 to 1, and a focus outside a part.
		if (!((command.m_Arguments[0] >= 0.0) && (command.m_Arguments[0] <= 1.0)))
			violate(command, "FOCUS_RANGE", "the focus must be between 0.0 (fully defocused) and 1.0 (fully focused)");
		if (!m_bPartBegun)
			violate(command, "NO_PART", "a focus needs a part that was begun after the laser scanner was selected");
		return;
	default:
		// Settings apply to the selected scanner, so they need one.
		if (!m_bScannerSelected)
			violate(command, "NO_SCANNER", "settings and delays need a selected laser scanner");
		return;
	}
}

void CToolpathRecorder::record(const SToolpathCommand& command)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	checkRules(command);
	m_Commands.push_back(command);
}

void CToolpathRecorder::clear()
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	m_Commands.clear();
	m_nLayerCount = 0;
}

uint64_t CToolpathRecorder::getCommandCount()
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	return m_Commands.size();
}

SToolpathCommand CToolpathRecorder::getCommand(uint64_t nIndex)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	if (nIndex >= m_Commands.size())
		throw ELib3MF4EOSInterfaceException(LIB3MF4EOS_ERROR_INVALIDPARAM, "recorded command index " + std::to_string(nIndex) + " is out of range (" + std::to_string(m_Commands.size()) + " commands)");
	return m_Commands[nIndex];
}

std::string CToolpathRecorder::toText()
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	std::string sText;
	for (auto& command : m_Commands)
		sText += toolpathCommandToText(command) + "\n";
	return sText;
}

uint64_t CToolpathRecorder::getLayerCount()
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	return m_nLayerCount;
}


/*************************************************************************************************************************
 CToolpathFingerprint
**************************************************************************************************************************/

static const uint64_t FINGERPRINT_OFFSET_BASIS = 14695981039346656037ULL;
static const uint64_t FINGERPRINT_PRIME = 1099511628211ULL;

CToolpathFingerprint::CToolpathFingerprint()
	: m_nTotalHash(FINGERPRINT_OFFSET_BASIS)
{
}

void CToolpathFingerprint::hashValue(uint64_t& nHash, uint64_t nValue)
{
	for (int nByte = 0; nByte < 8; nByte++) {
		nHash ^= (nValue >> (nByte * 8)) & 0xff;
		nHash *= FINGERPRINT_PRIME;
	}
}

void CToolpathFingerprint::add(const SToolpathCommand& command, int32_t nLayerIndex)
{
	if ((command.m_eCommand == eToolpathCommand::OpenFile) || (command.m_eCommand == eToolpathCommand::CloseFile))
		return;

	if (command.m_eCommand == eToolpathCommand::BeginLayer)
		m_Layers.push_back(SLayer{ nLayerIndex, command.m_Arguments[0] / 1000.0, 0, FINGERPRINT_OFFSET_BASIS });

	uint64_t values[5] = { (uint64_t)command.m_eCommand, 0, 0, 0, 0 };
	for (uint32_t nIndex = 0; nIndex < command.m_nArgumentCount; nIndex++)
		values[nIndex + 1] = (uint64_t)std::llround(command.m_Arguments[nIndex] * 1.0e6);

	for (uint32_t nIndex = 0; nIndex <= command.m_nArgumentCount; nIndex++) {
		hashValue(m_nTotalHash, values[nIndex]);
		if (!m_Layers.empty())
			hashValue(m_Layers.back().m_nHash, values[nIndex]);
	}
	if (!m_Layers.empty())
		m_Layers.back().m_nCommandCount++;
}

bool CToolpathFingerprint::isEmpty() const
{
	return m_Layers.empty();
}

uint64_t CToolpathFingerprint::getTotalHash() const
{
	return m_nTotalHash;
}

size_t CToolpathFingerprint::getLayerCount() const
{
	return m_Layers.size();
}

uint64_t CToolpathFingerprint::getLayerHash(size_t nIndex) const
{
	return m_Layers.at(nIndex).m_nHash;
}

std::string CToolpathFingerprint::hashToString(uint64_t nHash)
{
	char buffer[32];
	std::snprintf(buffer, sizeof(buffer), "%016llx", (unsigned long long)nHash);
	return buffer;
}

std::string CToolpathFingerprint::toJSON() const
{
	CJSONArray layers;
	for (auto& layer : m_Layers) {
		CJSONObject layerJSON;
		layerJSON.addInteger("layerIndex", layer.m_nLayerIndex);
		layerJSON.addNumber("z", layer.m_dZ);
		layerJSON.addUnsigned("commands", layer.m_nCommandCount);
		layerJSON.addString("hash", hashToString(layer.m_nHash));
		layers.addRaw(layerJSON.toString());
	}

	CJSONObject fingerprint;
	fingerprint.addString("algorithm", "FNV-1a 64");
	fingerprint.addNumber("resolution", 1.0e-6);
	fingerprint.addUnsigned("layers", m_Layers.size());
	fingerprint.addString("hash", hashToString(m_nTotalHash));
	fingerprint.addRaw("layerHashes", layers.toString());
	return fingerprint.toString();
}

} // namespace Impl
} // namespace Lib3MF4EOS
