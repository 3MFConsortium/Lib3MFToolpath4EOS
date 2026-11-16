/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Example 03 - builds a task from a mesh 3MF and uploads it to a machine.

  The 3MF is converted into an OpenJZ job internally. It needs an embedded material set
  (see example 00 or --material-set). Without --machine, the machine connection is simulated.

*/

#include "example_common.hpp"

using namespace LibEOSExamples;

int main(int argc, char** argv)
{
	CExampleArguments arguments(
		"Loads a mesh 3MF as job, creates an EOS task from it and uploads it to a machine.",
		"<model.3mf>");
	arguments.addSDKOptions();
	arguments.addMachineConfigOption();
	arguments.addMachineOptions();
	arguments.addOption("material-set", "LIBEOS_MATERIAL_SET", false, "Material set (.eospar) to embed if the 3MF has none.");

	return runExample(arguments, argc, argv, [&]() {
		std::string s3MFFileName = arguments.positional(0, "model.3mf");

		auto libraries = loadLibraries();
		auto pModel = read3MF(libraries, s3MFFileName);

		auto pContext = createContext(libraries, arguments, !arguments.has("machine"));
		ensureMaterialSet(pContext, pModel, arguments);

		auto pSession = connectToMachine(pContext, arguments);
		auto pMachineConfig = getMachineConfigForUpload(pContext, pSession, arguments);

		std::cout << "Loading job from 3MF" << std::endl;
		pContext->LoadJobFrom3MF(pModel);
		printMachineInfo(pContext->ActivateMachineConfiguration(pMachineConfig));

		auto pTask = pContext->CreateTask("LibEOS example 03", "Created from " + s3MFFileName, GENERATOR_NAME, GENERATOR_VERSION);
		waitForTaskCreation(pTask, "Creating task");
		storeTaskIfRequested(pTask, arguments);

		uploadTask(pSession, pTask);
		pSession->Disconnect();
	});
}
