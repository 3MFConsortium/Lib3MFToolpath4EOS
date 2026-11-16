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


Abstract: This is a stub class definition of CFuture

*/

#include "libeos_future.hpp"
#include "lib3mftoolpath4eos_interfaceexception.hpp"

// Include custom headers here.
#include <chrono>
#include <thread>

using namespace Lib3MF4EOS::Impl;

/*************************************************************************************************************************
 Class definition of CFuture 
**************************************************************************************************************************/

bool CFuture::WaitFor(const Lib3MF4EOS_uint32 nTime)
{

    uint32_t nCheckResolutionIntervalInMs = 1;
    auto startClock = std::chrono::steady_clock::now();

    while (IsRunning ()) {
        auto currentClock = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> elapsedMilliseconds = currentClock - startClock;
        if ((int64_t)elapsedMilliseconds.count() > (int64_t)nTime)
            return false;

        std::this_thread::sleep_for(std::chrono::milliseconds(nCheckResolutionIntervalInMs));
    }

    return HasFinished();
}

std::string CFuture::GetErrorMessage()
{
    std::lock_guard<std::mutex> lockGuard(m_ErrorMessageMutex);
    return m_sErrorMessage;
}

void CFuture::setErrorMessage(const std::string& sErrorMessage)
{
    std::lock_guard<std::mutex> lockGuard(m_ErrorMessageMutex);
    m_sErrorMessage = sErrorMessage;
}

void CFuture::setErrorMessageFromException(std::exception_ptr pException)
{
    try {
        if (pException)
            std::rethrow_exception(pException);
        setErrorMessage("unknown error");
    }
    catch (const std::exception& e) {
        setErrorMessage(e.what());
    }
    catch (...) {
        setErrorMessage("unknown error");
    }
}

