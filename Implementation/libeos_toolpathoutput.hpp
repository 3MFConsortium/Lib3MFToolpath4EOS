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


Abstract: Where the commands of a raw task writer go: to the SDK's toolpath file, or to a recorder.

*/


#ifndef __LIBEOS_TOOLPATHOUTPUT
#define __LIBEOS_TOOLPATHOUTPUT

#include "libeos_eossdk.hpp"
#include "libeos_toolpathcommands.hpp"

#include <memory>
#include <string>

namespace Lib3MF4EOS {
	namespace Impl {

		class CToolpathOutput {
		public:
			virtual ~CToolpathOutput() = default;

			virtual void write(const SToolpathCommand& command) = 0;

			// Closing twice does nothing.
			virtual void close() = 0;
		};

		typedef std::unique_ptr<CToolpathOutput> PToolpathOutput;


		// Writes to a toolpath file of the EOS SDK.
		class CToolpathOutputSDK : public CToolpathOutput {
		private:
			PEOSSDK m_pSDK;
			EOS_FILE_HANDLE m_hFile;

		public:
			CToolpathOutputSDK(PEOSSDK pSDK, const std::string& sFileName);
			~CToolpathOutputSDK();

			void write(const SToolpathCommand& command) override;
			void close() override;
		};


		class CToolpathOutputRecording : public CToolpathOutput {
		private:
			PToolpathRecorder m_pRecorder;
			bool m_bClosed;

		public:
			CToolpathOutputRecording(PToolpathRecorder pRecorder);

			void write(const SToolpathCommand& command) override;
			void close() override;
		};

	} // namespace Impl
} // namespace Lib3MF4EOS

#endif // __LIBEOS_TOOLPATHOUTPUT
