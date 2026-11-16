/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Example 01 - converts a mesh 3MF into an OpenJZ job, or an OpenJZ job into a mesh 3MF.

  The direction is chosen by the extension of the input file.
  3MF to OpenJZ: job settings are taken from the eos3mf metadata; missing values come from the
  embedded material set, which requires the EOS SDK.
  OpenJZ to 3MF: no EOS SDK is needed.

*/

#include "example_common.hpp"

using namespace LibEOSExamples;

int main(int argc, char** argv)
{
	CExampleArguments arguments(
		"Converts a mesh 3MF into an OpenJZ job (input *.3mf) or an OpenJZ job into a mesh 3MF (input *.openjz).",
		"<input.3mf|input.openjz> <output>");
	arguments.addSDKOptions();
	arguments.addOption("material-set", "LIBEOS_MATERIAL_SET", false, "Material set (.eospar) to embed if the input 3MF has none.");

	return runExample(arguments, argc, argv, [&]() {
		std::string sInputFileName = arguments.positional(0, "input");
		std::string sOutputFileName = arguments.positional(1, "output");
		std::string sExtension = fileExtension(sInputFileName);

		auto libraries = loadLibraries();
		auto pContext = libraries.m_pLibEOS->CreateContext(arguments.get("sdk"), arguments.get("log"));

		if (sExtension == "3mf") {
			auto pModel = read3MF(libraries, sInputFileName);

			printSDKVersion(pContext);
			ensureMaterialSet(pContext, pModel, arguments);

			std::cout << "Converting to OpenJZ " << sOutputFileName << std::endl;
			pContext->Convert3MFToOpenJZ(pModel, sOutputFileName);
		}
		else if (sExtension == "openjz") {
			auto pModel = libraries.m_pLib3MF->CreateModel();

			std::cout << "Converting OpenJZ " << sInputFileName << std::endl;
			pContext->ConvertOpenJZTo3MF(sInputFileName, pModel);

			std::cout << "Model has " << pModel->GetMeshObjects()->Count() << " mesh object(s)" << std::endl;

			auto pWriter = pModel->QueryWriter("3mf");
			pWriter->RegisterCustomNamespace("eos3mf", EOS3MF_NAMESPACE);
			std::cout << "Writing " << sOutputFileName << std::endl;
			pWriter->WriteToFile(sOutputFileName);
		}
		else {
			throw EExampleUsage("input must be a .3mf or .openjz file: " + sInputFileName);
		}
	});
}
