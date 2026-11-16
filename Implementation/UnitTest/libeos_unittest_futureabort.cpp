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


Abstract: Tests of Abort() on the machine session, machine configuration, task creation and task upload
          futures, and of releasing them while their background thread runs. Uses the simulated SDK, where
          connecting and downloading the configuration each take two seconds, a task creation ten seconds
          and an upload ten seconds.

*/

#include "libeos_unittests.hpp"
#include "libeos_eossdk.hpp"
#include "libeos_machinesessionfuture.hpp"
#include "libeos_machineconfigfuture.hpp"
#include "libeos_taskcreationfuture.hpp"
#include "libeos_taskuploadfuture.hpp"

#include <chrono>
#include <memory>
#include <thread>

using namespace Lib3MF4EOS::Impl;

namespace LibEOSUnitTest {

	struct SDecRefCountDeleter {
		void operator()(Lib3MF4EOS::Impl::IBase* pInstance) const
		{
			if (pInstance != nullptr)
				pInstance->DecRefCount();
		}
	};

	typedef std::unique_ptr<IMachineSessionFuture, SDecRefCountDeleter> PSessionInstance;
	typedef std::unique_ptr<IMachineConfigFuture, SDecRefCountDeleter> PConfigInstance;
	typedef std::unique_ptr<ITaskCreationFuture, SDecRefCountDeleter> PTaskCreationInstance;
	typedef std::unique_ptr<ITaskUploadFuture, SDecRefCountDeleter> PTaskUploadInstance;

	// Long enough for a background thread to notice an abort, which it checks every 100 ms.
	static const std::chrono::milliseconds THREAD_SETTLE_TIME(400);

	class CFutureAbortTests : public CUnitTestGroup {
	public:
		std::string getName() override
		{
			return "FutureAbort";
		}

		void registerTests() override
		{
			registerTest("SessionAbortWhileConnecting", "aborting a running connect reports Aborted, and the late connection is not used", [this]() {
				PSessionInstance pSession(new CMachineSessionFuture(createSimulatedSDK(), "192.0.2.1"));
				assertTrue(pSession->IsRunning(), "connecting");

				pSession->Abort();
				assertTrue(pSession->HasBeenAborted(), "aborted");
				assertFalse(pSession->IsRunning(), "not running");
				assertFalse(pSession->HasFailed(), "not failed");
				assertFalse(pSession->IsConnected(), "not connected");

				std::this_thread::sleep_for(THREAD_SETTLE_TIME);
				assertTrue(pSession->HasBeenAborted(), "still aborted after the thread ended");
				assertFalse(pSession->IsConnected(), "still not connected");

				pSession->Abort();
				assertTrue(pSession->HasBeenAborted(), "a second Abort does nothing");
			});

			registerTest("SessionReleasedWhileConnecting", "releasing a session future during the connect is safe", [this]() {
				{
					PSessionInstance pSession(new CMachineSessionFuture(createSimulatedSDK(), "192.0.2.1"));
					assertTrue(pSession->IsRunning(), "connecting");
				}
				std::this_thread::sleep_for(THREAD_SETTLE_TIME);
			});

			registerTest("ConfigAbortAndRelease", "Abort and release during the configuration download; Abort after connect and after the download does nothing", [this]() {
				PSessionInstance pSession(new CMachineSessionFuture(createSimulatedSDK(), "192.0.2.1"));
				assertTrue(waitUntilFinished(pSession.get()), "connected");

				pSession->Abort();
				assertTrue(pSession->IsConnected(), "Abort after connecting keeps the session");
				assertFalse(pSession->HasBeenAborted(), "Abort after connecting does not report Aborted");

				PConfigInstance pAborted(pSession->DownloadMachineConfig());
				assertTrue(pAborted->IsRunning(), "download running");
				pAborted->Abort();
				assertTrue(pAborted->HasBeenAborted(), "download aborted");
				assertFalse(pAborted->IsRunning(), "download not running");
				assertFalse(pAborted->HasFinished(), "download not finished");
				std::this_thread::sleep_for(THREAD_SETTLE_TIME);
				assertTrue(pAborted->HasBeenAborted(), "still aborted after the thread ended");
				Lib3MF4EOS_uint64 nNeededCount = 1;
				pAborted->RetrievePackageData(0, &nNeededCount, nullptr);
				assertTrue(nNeededCount == 0, "no package data after Abort");

				{
					PConfigInstance pReleased(pSession->DownloadMachineConfig());
					assertTrue(pReleased->IsRunning(), "second download running");
				}

				PConfigInstance pCompleted(pSession->DownloadMachineConfig());
				assertTrue(waitUntilFinished(pCompleted.get()), "third download finished");
				pCompleted->RetrievePackageData(0, &nNeededCount, nullptr);
				assertTrue(nNeededCount > 0, "package data after the download");

				pCompleted->Abort();
				assertTrue(pCompleted->HasFinished(), "Abort after the download keeps the result");
				assertFalse(pCompleted->HasBeenAborted(), "Abort after the download does not report Aborted");
			});

			registerTest("TaskCreationAbortAndRelease", "aborting a running task creation stops its thread, and releasing one while it runs is safe", [this]() {
				auto pSDK = createSimulatedSDK();
				PTaskCreationInstance pAborted(new CTaskCreationFuture(pSDK, "abort", "", "unittest", "1.0", false));
				assertTrue(pAborted->IsRunning(), "task creation running");

				pAborted->Abort();
				assertTrue(pAborted->HasBeenAborted(), "aborted");
				assertFalse(pAborted->IsRunning(), "not running");
				assertFalse(pAborted->HasFailed(), "not failed");

				Lib3MF4EOS_uint32 nLayer1, nLayers, nZ, nTargetZ;
				std::this_thread::sleep_for(THREAD_SETTLE_TIME);
				pAborted->GetProgress(nLayer1, nLayers, nZ, nTargetZ);
				std::this_thread::sleep_for(THREAD_SETTLE_TIME);
				Lib3MF4EOS_uint32 nLayer2;
				pAborted->GetProgress(nLayer2, nLayers, nZ, nTargetZ);
				assertEqual(nLayer2, nLayer1, "the thread stopped reporting progress");
				assertTrue(nLayer2 < nLayers, "the thread stopped before the last layer");
				assertTrue(pAborted->HasBeenAborted(), "still aborted after the thread ended");

				{
					PTaskCreationInstance pReleased(new CTaskCreationFuture(pSDK, "release", "", "unittest", "1.0", false));
					assertTrue(pReleased->IsRunning(), "second task creation running");
				}
				std::this_thread::sleep_for(THREAD_SETTLE_TIME);
			});

			registerTest("TaskUploadAbortAndRelease", "aborting a running upload stops its thread, and releasing one while it runs is safe", [this]() {
				auto pSDK = createSimulatedSDK();
				PSessionInstance pSession(new CMachineSessionFuture(pSDK, "192.0.2.1"));
				assertTrue(waitUntilFinished(pSession.get()), "connected");
				PTaskCreationInstance pTask(new CTaskCreationFuture(pSDK, "upload", std::make_shared<CConversionLogData>()));

				PTaskUploadInstance pAborted(pSession->UploadTask(pTask.get()));
				assertTrue(pAborted->IsRunning(), "upload running");
				pAborted->Abort();
				assertTrue(pAborted->HasBeenAborted(), "aborted");
				assertFalse(pAborted->IsRunning(), "not running");

				std::this_thread::sleep_for(THREAD_SETTLE_TIME);
				double dProgress1 = pAborted->GetProgress();
				std::this_thread::sleep_for(THREAD_SETTLE_TIME);
				assertNear(pAborted->GetProgress(), dProgress1, 0.0, "the thread stopped reporting progress");
				assertTrue(dProgress1 < 0.5, "the thread stopped early");
				assertTrue(pAborted->HasBeenAborted(), "still aborted after the thread ended");

				{
					PTaskUploadInstance pReleased(pSession->UploadTask(pTask.get()));
					assertTrue(pReleased->IsRunning(), "second upload running");
				}
				std::this_thread::sleep_for(THREAD_SETTLE_TIME);
			});
		}

	private:
		PEOSSDK createSimulatedSDK()
		{
			auto pSDK = std::make_shared<CEOSSDK>("", "", 2, 26);
			pSDK->setToSimulationMode(eSimulationMode::smSimulateFullSDK);
			return pSDK;
		}

		bool waitUntilFinished(IFuture* pFuture)
		{
			for (uint32_t nStep = 0; nStep < 100; nStep++) {
				if (!pFuture->IsRunning())
					return pFuture->HasFinished();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			return false;
		}
	};

	PUnitTestGroup createFutureAbortTests()
	{
		return std::make_shared<CFutureAbortTests>();
	}

}
