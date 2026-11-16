/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Layer analysis of 3MF toolpaths: the parts of every layer, the layer thickness of every part,
the EOS layer Z values, and the comparison with the layer stack of the EOS SDK.

*/

#ifndef __LIBEOS_TOOLPATHLAYERS
#define __LIBEOS_TOOLPATHLAYERS

#include "lib3mftoolpath4eos_interfaces.hpp"
#include "libeos_eosconversionlog.hpp"
#include "libeos_eossdk_types.hpp"
#include "libeos_3mfjobloader.hpp"

#include <map>
#include <string>
#include <vector>

namespace Lib3MF4EOS {
namespace Impl {

// Layer Z values and distances that differ by at most this many micrometers are taken as equal.
constexpr int64_t TOOLPATHLAYERS_TOLERANCE_UM = 1;

struct SToolpathLayerScan {
	// Layer Z values as written in the toolpath, in micrometers.
	std::vector<int64_t> m_LayerZ;
	// Sorted job part IDs that have segments in each layer.
	std::vector<std::vector<uint32_t>> m_LayerParts;
};

// Reads every layer of the toolpath once.
SToolpathLayerScan scanToolpathLayers(Lib3MF::PToolpath pToolpath, double dToolpathUnitInMM, const std::map<std::string, uint32_t>& buildItemPartIDs);

enum class eLayerThicknessSource {
	// The smallest distance between the part's layers.
	PartLayers,
	// The part has fewer than two layers; the smallest layer distance of the toolpath is used.
	ToolpathStep
};

struct SPartLayers {
	uint32_t m_nLayerCount = 0;
	// EOS Z of the first and last layer of the part, in micrometers.
	int64_t m_nFirstZ = 0;
	int64_t m_nLastZ = 0;
	// 0 if unknown.
	uint32_t m_nLayerThickness = 0;
	eLayerThicknessSource m_eThicknessSource = eLayerThicknessSource::ToolpathStep;
};

struct SToolpathZLayout {
	Lib3MF4EOS::eEOSLayerZConvention m_eConvention = Lib3MF4EOS::eEOSLayerZConvention::Top;
	// Smallest distance between two layers of the toolpath, in micrometers. For a single layer, the fallback thickness.
	int64_t m_nStep = 0;
	// Added to every toolpath layer Z to get the EOS layer Z, which is the top of the layer.
	int64_t m_nOffset = 0;
	std::vector<int64_t> m_EOSLayerZ;
	std::map<uint32_t, SPartLayers> m_Parts;

	// 0 if the part is unknown or its thickness is unknown.
	uint32_t getPartLayerThickness(uint32_t nPartID) const;
};

// nFallbackThickness is used as layer distance if the toolpath has fewer than two layers; 0 if unknown.
// Fails with LAYERZINVALID or LAYERTHICKNESSINCONSISTENT, after logging the reason.
SToolpathZLayout computeToolpathZLayout(const SToolpathLayerScan& scan, Lib3MF4EOS::eEOSLayerZConvention eConvention, uint32_t nFallbackThickness,
	const std::vector<uint32_t>& partIDs, PConversionLogData pLog);

std::string layerThicknessSourceToString(eLayerThicknessSource eSource);

struct SEOSLayerStack {
	std::vector<sEOSLayerInfo> m_Layers;
	sEOSProcessHeights m_ProcessHeights;
	bool m_bSimulated = false;
};

// The layer stack of the full SDK simulation: every part has layers at the multiples of its layer thickness
// that overlap its mesh, and the Z travel is the height of the highest layer.
SEOSLayerStack simulateLayerStack(const std::vector<SEOS3MFPart>& parts, const SToolpathZLayout& layout, double dMaxZTravelInMM);

// Compares the toolpath layers with the layer stack and logs the result. Toolpath layers without a layer in the
// stack are errors in strict mode and warnings otherwise; a Z travel above the maximum is an error. A Z shrinkage
// compensation of the job cannot be applied to the toolpath, so it is an error in strict mode and a warning otherwise.
// If bThrowOnError is set, the first kind of error found is thrown after everything is logged.
// Returns the number of toolpath layers with vectors that have no layer in the stack.
uint64_t checkLayerCoverage(const SToolpathLayerScan& scan, const SToolpathZLayout& layout, const SEOSLayerStack& stack, bool bStrictMode, bool bThrowOnError, PConversionLogData pLog);

// Walks up the layer stack of the job along the written toolpath layers, to find the exposed stack layers that get no
// toolpath layer. Z values that differ by at most TOOLPATHLAYERS_TOLERANCE_UM are equal.
class CStackLayerCursor {
private:
	const SEOSLayerStack& m_Stack;
	size_t m_nNextLayer;

	bool needsFilling(const sEOSLayerInfo& layer) const;

public:
	CStackLayerCursor(const SEOSLayerStack& stack);

	// The exposed stack layers below nZ that were not passed yet. Afterwards, every stack layer up to nZ is passed.
	std::vector<sEOSLayerInfo> passLayersUpTo(int64_t nZ);

	// The exposed stack layers that were not passed yet. Afterwards, every stack layer is passed.
	std::vector<sEOSLayerInfo> passRemainingLayers();
};

} // namespace Impl
} // namespace Lib3MF4EOS

#endif // __LIBEOS_TOOLPATHLAYERS
