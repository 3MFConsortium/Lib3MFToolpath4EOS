/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Ramps of laser power, scan speed and focus. EOS sets these values per vector,
          so a ramp along a vector is split into sub-vectors of constant values.

*/

#include "libeos_powerramp.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"

#include <algorithm>
#include <cmath>

using namespace Lib3MF4EOS::Impl;

#define LIBEOS_POWERRAMP_EPSILON 1.0e-6
#define LIBEOS_POWERRAMP_MAXSUBVECTORS 100000

bool CPowerRamp::isRamp(double dPower1, double dPower2)
{
	return std::fabs(dPower2 - dPower1) > LIBEOS_POWERRAMP_EPSILON;
}

bool CPowerRamp::isRamp(const SVectorValues& values1, const SVectorValues& values2)
{
	return isRamp(values1.m_dLaserPower, values2.m_dLaserPower) ||
		(std::fabs(values2.m_dScanSpeed - values1.m_dScanSpeed) > LIBEOS_POWERRAMP_EPSILON) ||
		(std::fabs(values2.m_dLaserFocus - values1.m_dLaserFocus) > LIBEOS_POWERRAMP_EPSILON);
}

bool CPowerRamp::equalValues(const SVectorValues& values1, const SVectorValues& values2)
{
	return !isRamp(values1, values2);
}

SVectorValues CPowerRamp::interpolate(const SVectorValues& values1, const SVectorValues& values2, double dT)
{
	SVectorValues values;
	values.m_dLaserPower = values1.m_dLaserPower + dT * (values2.m_dLaserPower - values1.m_dLaserPower);
	values.m_dScanSpeed = values1.m_dScanSpeed + dT * (values2.m_dScanSpeed - values1.m_dScanSpeed);
	values.m_dLaserFocus = values1.m_dLaserFocus + dT * (values2.m_dLaserFocus - values1.m_dLaserFocus);
	return values;
}

uint32_t CPowerRamp::subVectorCount(const SConversionSettings& settings, const SVectorValues& values1, const SVectorValues& values2, double dLengthInMM)
{
	if (!isRamp(values1, values2))
		return 1;

	double dCountByPower = std::ceil(std::fabs(values2.m_dLaserPower - values1.m_dLaserPower) / settings.m_dMaxPowerStep - LIBEOS_POWERRAMP_EPSILON);
	double dCountBySpeed = std::ceil(std::fabs(values2.m_dScanSpeed - values1.m_dScanSpeed) / settings.m_dMaxSpeedStep - LIBEOS_POWERRAMP_EPSILON);
	double dCountByFocus = std::ceil(std::fabs(values2.m_dLaserFocus - values1.m_dLaserFocus) / settings.m_dMaxFocusStep - LIBEOS_POWERRAMP_EPSILON);
	double dCountByValues = (std::max)((std::max)(dCountByPower, dCountBySpeed), dCountByFocus);
	double dCountByLength = std::floor(dLengthInMM / settings.m_dMinSubVectorLength + LIBEOS_POWERRAMP_EPSILON);
	double dCount = (std::min)((std::min)(dCountByValues, dCountByLength), (double)LIBEOS_POWERRAMP_MAXSUBVECTORS);

	if (dCount < 1.0)
		return 1;
	return (uint32_t)dCount;
}

void CPowerRamp::appendSubVectors(const SConversionSettings& settings, double dX1, double dY1, const SVectorValues& values1,
	double dX2, double dY2, const SVectorValues& values2, std::vector<SPowerRampSubVector>& subVectors)
{
	double dDeltaX = dX2 - dX1;
	double dDeltaY = dY2 - dY1;
	double dLength = std::sqrt(dDeltaX * dDeltaX + dDeltaY * dDeltaY);
	if (dLength <= 0.0)
		return;

	uint32_t nCount = subVectorCount(settings, values1, values2, dLength);
	for (uint32_t nIndex = 0; nIndex < nCount; nIndex++) {
		double dEndT = (double)(nIndex + 1) / (double)nCount;
		double dMeanT = ((double)nIndex + 0.5) / (double)nCount;

		SPowerRampSubVector subVector;
		if (nIndex + 1 == nCount) {
			subVector.m_dEndX = dX2;
			subVector.m_dEndY = dY2;
		}
		else {
			subVector.m_dEndX = dX1 + dEndT * dDeltaX;
			subVector.m_dEndY = dY1 + dEndT * dDeltaY;
		}
		subVector.m_Values = interpolate(values1, values2, dMeanT);
		subVectors.push_back(subVector);
	}
}

double CPowerRamp::modifiedValue(double dMinValue, double dMaxValue, double dFactor)
{
	double dClampedFactor = (std::min)((std::max)(dFactor, 0.0), 1.0);
	return dMinValue + dClampedFactor * (dMaxValue - dMinValue);
}
