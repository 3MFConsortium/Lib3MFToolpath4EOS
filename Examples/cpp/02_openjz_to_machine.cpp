/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Example 02 - builds a task from an OpenJZ job and uploads it to a machine.

  Without --machine, the machine connection is simulated; the task is still created by the EOS SDK.

*/

#include "example_common.hpp"

using namespace LibEOSExamples;

int main(int argc, char** argv)
{
	CExampleArguments arguments(
		"Loads an OpenJZ job, creates an EOS task from it and uploads it to a machine.",
		"<job.openjz>");
	arguments.addSDKOptions();
	arguments.addMachineConfigOption();
	arguments.addMachineOptions();

	return runExample(arguments, argc, argv, [&]() {
		std::string sOpenJZFileName = arguments.positional(0, "job.openjz");

		auto libraries = loadLibraries();
		auto pContext = createContext(libraries, arguments, !arguments.has("machine"));

		auto pSession = connectToMachine(pContext, arguments);
		auto pMachineConfig = getMachineConfigForUpload(pContext, pSession, arguments);

		std::cout << "Loading OpenJZ " << sOpenJZFileName << std::endl;
		pContext->LoadOpenJZFromFile(sOpenJZFileName);
		printMachineInfo(pContext->ActivateMachineConfiguration(pMachineConfig));

		auto pTask = pContext->CreateTask("LibEOS example 02", "Created from " + sOpenJZFileName, GENERATOR_NAME, GENERATOR_VERSION);
		waitForTaskCreation(pTask, "Creating task");
		storeTaskIfRequested(pTask, arguments);

		uploadTask(pSession, pTask);
		pSession->Disconnect();
	});
}
