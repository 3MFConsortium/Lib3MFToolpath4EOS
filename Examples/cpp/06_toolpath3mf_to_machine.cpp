/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Example 06 - turns the toolpath of a toolpath 3MF into an EOS task and uploads it to a machine.

  The toolpath 3MF comes from your own toolpath generation. The job is loaded from the meshes of the
  3MF, and the 3MF toolpath is written as the exposure data. Profiles are read with the parameter
  names of the 3MF conventions in README.md (laserpower, laserspeed, laserindex, and the EOS exposure
  type and focus). Requires Toolpath API credentials, and using
  the Toolpath API requires a contract with EOS GmbH. Without --machine, the machine connection is
  simulated.

*/

#include "example_common.hpp"

using namespace LibEOSExamples;

int main(int argc, char** argv)
{
	CExampleArguments arguments(
		"Creates an EOS task from the toolpath in a toolpath 3MF and uploads it to a machine.\n\n" + TOOLPATH_API_NOTICE,
		"<toolpath.3mf>");
	arguments.addSDKOptions();
	arguments.addMachineConfigOption();
	arguments.addToolpathAPIOptions();
	arguments.addMachineOptions();
	arguments.addConversionOptions();

	return runExample(arguments, argc, argv, [&]() {
		std::string s3MFFileName = arguments.positional(0, "toolpath.3mf");

		auto libraries = loadLibraries();
		auto pModel = read3MF(libraries, s3MFFileName);

		auto pContext = createContext(libraries, arguments, !arguments.has("machine"));
		authenticateToolpathAPI(pContext, arguments);

		auto pSession = connectToMachine(pContext, arguments);
		auto pMachineConfig = getMachineConfigForUpload(pContext, pSession, arguments);

		auto pSettings = createConversionSettings(pContext, arguments);
		auto pTask = pContext->CreateTaskFrom3MFToolpath(pModel, pMachineConfig, pSettings,
			"LibEOS example 06", "Created from " + s3MFFileName, GENERATOR_NAME, GENERATOR_VERSION);
		waitForTaskCreationWithLog(pTask, "Creating task", arguments);
		storeTaskIfRequested(pTask, arguments);

		uploadTask(pSession, pTask);
		pSession->Disconnect();
	});
}
