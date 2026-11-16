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


Abstract: The commands a raw task writer sends to the EOS SDK, the recorder that keeps them in recording mode and
checks the order in which the SDK accepts them, and the fingerprint of a command stream.

*/


#ifndef __LIBEOS_TOOLPATHCOMMANDS
#define __LIBEOS_TOOLPATHCOMMANDS

#include <array>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

namespace Lib3MF4EOS {
	namespace Impl {

		enum class eToolpathCommand : uint32_t {
			OpenFile = 1,
			BeginLayer = 2,           // Z in micrometers
			BeginPart = 3,            // part ID
			SelectLaserScanner = 4,   // scanner index
			SetLaserPower = 5,        // power in W, delay in microseconds
			SetScanSpeed = 6,         // speed in mm/s
			SetFocus = 7,             // focus
			SetSkywriting = 8,        // 0 or 1
			SetExposureType = 9,      // exposure type
			SetExposureStep = 10,     // exposure set index, exposure step index
			SetLaserPulse = 11,       // period and power-on time in microseconds
			SetDepth = 12,            // exposed depth in micrometers
			SetBeamShape = 13,        // beam shape, profile
			Delay = 14,               // delay in microseconds
			Hatch = 15,               // start X, start Y, end X, end Y in mm
			PolyStart = 16,           // X, Y
			PolyCoord = 17,           // X, Y, tracking error compensation in microseconds
			PolyEnd = 18,             // X, Y
			CloseFile = 19
		};

		struct SToolpathCommand {
			eToolpathCommand m_eCommand;
			uint32_t m_nArgumentCount;
			std::array<double, 4> m_Arguments;

			SToolpathCommand(eToolpathCommand eCommand, std::initializer_list<double> arguments);
		};

		std::string toolpathCommandName(eToolpathCommand eCommand);

		// Numbers are written with at most 6 decimals, without trailing zeros.
		std::string toolpathNumberToString(double dValue);

		// For example "Hatch 1.5 2 11.5 2".
		std::string toolpathCommandToText(const SToolpathCommand& command);


		// Keeps the commands of all toolpath files written in recording mode, and checks each command against
		// the order the SDK accepts. A command that breaks a rule is not kept and fails with TOOLPATHORDERVIOLATION.
		class CToolpathRecorder {
		private:
			std::mutex m_Mutex;
			std::vector<SToolpathCommand> m_Commands;

			// State of the file being written.
			bool m_bFileOpen;
			bool m_bInLayer;
			bool m_bLayerHasExposure;
			bool m_bScannerSelected;
			bool m_bPartBegun;
			bool m_bPolylineOpen;
			double m_dLayerZ;
			uint64_t m_nLayerCount;
			// Laser scanners of the simulated machine; 0 leaves scanner indices and idle scanners unchecked.
			uint32_t m_nLaserScannerCount;
			uint32_t m_nSelectedScanner;
			std::set<uint32_t> m_ScannersWithPart;

			void resetFileState();
			void checkRules(const SToolpathCommand& command);
			void violate(const SToolpathCommand& command, const std::string& sRule, const std::string& sExplanation);

		public:
			CToolpathRecorder();

			void setLaserScannerCount(uint32_t nCount);

			void record(const SToolpathCommand& command);

			void clear();
			uint64_t getCommandCount();
			SToolpathCommand getCommand(uint64_t nIndex);
			// One command per line.
			std::string toText();
			// Layers begun since the recording started or was cleared.
			uint64_t getLayerCount();
		};

		typedef std::shared_ptr<CToolpathRecorder> PToolpathRecorder;


		// A hash of the commands of every layer. Values are rounded to 1e-6 of their unit, so that the hash does
		// not depend on the last bits of a floating point computation.
		class CToolpathFingerprint {
		private:
			struct SLayer {
				int32_t m_nLayerIndex;
				double m_dZ;
				uint64_t m_nCommandCount;
				uint64_t m_nHash;
			};

			std::vector<SLayer> m_Layers;
			uint64_t m_nTotalHash;

			static void hashValue(uint64_t& nHash, uint64_t nValue);

		public:
			CToolpathFingerprint();

			void add(const SToolpathCommand& command, int32_t nLayerIndex);

			bool isEmpty() const;
			uint64_t getTotalHash() const;
			size_t getLayerCount() const;
			uint64_t getLayerHash(size_t nIndex) const;

			static std::string hashToString(uint64_t nHash);

			// The "fingerprint" section of the conversion log.
			std::string toJSON() const;
		};

	} // namespace Impl
} // namespace Lib3MF4EOS

#endif // __LIBEOS_TOOLPATHCOMMANDS
