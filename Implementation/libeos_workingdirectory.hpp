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


Abstract: This is the class declaration of CEOSContext

*/


#ifndef __LIBEOS_WORKINGDIRECTORY
#define __LIBEOS_WORKINGDIRECTORY

#include <string>

namespace Lib3MF4EOS {
    namespace Impl {

		// The EOS SDK needs its installation directory as current directory and DLL directory while it loads
		// its components. Both settings are process-wide, so all overrides share one state: the first active
		// override sets the directories and the last one restores them, on whatever threads they run.
		// Overrides that are active at the same time must use the same directory.
		class CCurrentDirectoryOverride {

			public:

				CCurrentDirectoryOverride (const std::string & newWorkingDirectory);
				virtual ~CCurrentDirectoryOverride ();

				CCurrentDirectoryOverride (const CCurrentDirectoryOverride &) = delete;
				CCurrentDirectoryOverride & operator= (const CCurrentDirectoryOverride &) = delete;

		};


    } // namespace Impl
} // namespace Lib3MF4EOS

#endif // __LIBEOS_WORKINGDIRECTORY



