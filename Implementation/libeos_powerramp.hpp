/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Ramps of laser power, scan speed and focus. EOS sets these values per vector,
          so a ramp along a vector is split into sub-vectors of constant values.

*/

#ifndef __LIBEOS_POWERRAMP
#define __LIBEOS_POWERRAMP

#include "lib3mftoolpath4eos_types.hpp"
#include "libeos_eostoolpathconversionsettings.hpp"

#include <vector>
#include <cstdint>

namespace Lib3MF4EOS {
namespace Impl {

// EOS focus values go from 0.0 (fully defocused) to 1.0 (fully focused). A scanner is fully
// focused until a focus is written, and machines without focus control refuse every focus command.
constexpr double EOS_FOCUS_NOMINAL = 1.0;

// The values EOS can change between two vectors of a polyline.
struct SVectorValues {
	double m_dLaserPower;
	double m_dScanSpeed;
	double m_dLaserFocus;
};

struct SPowerRampSubVector {
	double m_dEndX;
	double m_dEndY;
	SVectorValues m_Values;
};

class CPowerRamp {
public:
	static bool isRamp(double dPower1, double dPower2);

	static bool isRamp(const SVectorValues& values1, const SVectorValues& values2);

	static bool equalValues(const SVectorValues& values1, const SVectorValues& values2);

	static SVectorValues interpolate(const SVectorValues& values1, const SVectorValues& values2, double dT);

	// The number of sub-vectors is set by the value that needs the most steps, limited by the shortest sub-vector length.
	static uint32_t subVectorCount(const SConversionSettings& settings, const SVectorValues& values1, const SVectorValues& values2, double dLengthInMM);

	// Appends the sub-vectors of a linear ramp from (dX1, dY1) to (dX2, dY2). Every sub-vector gets the mean values
	// over its interval, so the deposited energy is preserved. Zero-length vectors append nothing.
	static void appendSubVectors(const SConversionSettings& settings, double dX1, double dY1, const SVectorValues& values1,
		double dX2, double dY2, const SVectorValues& values2, std::vector<SPowerRampSubVector>& subVectors);

	static double modifiedValue(double dMinValue, double dMaxValue, double dFactor);
};

} // namespace Impl
} // namespace Lib3MF4EOS

#endif // __LIBEOS_POWERRAMP
