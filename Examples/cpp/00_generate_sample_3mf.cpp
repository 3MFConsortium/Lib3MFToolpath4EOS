/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Example 00 - creates a sample mesh 3MF for the other examples.

  The model contains three parts that exercise the part placement:
  a cube moved by its build item, a bar rotated about Z by its build item,
  and a cube placed through a components object.

*/

#include "example_common.hpp"

#include <cmath>

using namespace LibEOSExamples;

static Lib3MF::PMeshObject addBox(Lib3MF::PModel pModel, const std::string& sName, float fSizeX, float fSizeY, float fSizeZ)
{
	std::vector<Lib3MF::sPosition> vertices = {
		{ 0.0f, 0.0f, 0.0f }, { fSizeX, 0.0f, 0.0f }, { fSizeX, fSizeY, 0.0f }, { 0.0f, fSizeY, 0.0f },
		{ 0.0f, 0.0f, fSizeZ }, { fSizeX, 0.0f, fSizeZ }, { fSizeX, fSizeY, fSizeZ }, { 0.0f, fSizeY, fSizeZ }
	};
	std::vector<Lib3MF::sTriangle> triangles = {
		{ 2, 1, 0 }, { 0, 3, 2 }, { 4, 5, 6 }, { 6, 7, 4 },
		{ 0, 1, 5 }, { 5, 4, 0 }, { 1, 2, 6 }, { 6, 5, 1 },
		{ 2, 3, 7 }, { 7, 6, 2 }, { 3, 0, 4 }, { 4, 7, 3 }
	};

	auto pMesh = pModel->AddMeshObject();
	pMesh->SetName(sName);
	pMesh->SetGeometry(vertices, triangles);
	return pMesh;
}

static Lib3MF::sTransform makeTransform(float fRotationZInDegrees, float fX, float fY, float fZ)
{
	float fAngle = fRotationZInDegrees * 3.14159265358979f / 180.0f;
	float fCos = cosf(fAngle);
	float fSin = sinf(fAngle);

	// 3MF uses row vectors (p' = p * M), so the rotation appears transposed.
	Lib3MF::sTransform transform = {};
	transform.m_Fields[0][0] = fCos;  transform.m_Fields[0][1] = fSin;
	transform.m_Fields[1][0] = -fSin; transform.m_Fields[1][1] = fCos;
	transform.m_Fields[2][2] = 1.0f;
	transform.m_Fields[3][0] = fX;
	transform.m_Fields[3][1] = fY;
	transform.m_Fields[3][2] = fZ;
	return transform;
}

int main(int argc, char** argv)
{
	CExampleArguments arguments(
		"Creates a sample mesh 3MF with three parts for the other examples.\n"
		"With --material-set, the material set is embedded and its exposure sets are listed (loads the EOS SDK);\n"
		"--part-parameter must then name one of them.",
		"<output.3mf>");
	arguments.addSDKOptions();
	arguments.addOption("material-set", "LIBEOS_MATERIAL_SET", false, "EOS material set (.eospar) to embed into the 3MF.");
	arguments.addOption("part-parameter", "", false, "Exposure set name to store as eos3mf:partparameter on every part (required with --material-set).");

	return runExample(arguments, argc, argv, [&]() {
		std::string sOutputFileName = arguments.positional(0, "output.3mf");

		auto libraries = loadLibraries();
		auto pModel = libraries.m_pLib3MF->CreateModel();
		pModel->SetUnit(Lib3MF::eModelUnit::MilliMeter);

		auto pCube = addBox(pModel, "Cube", 20.0f, 20.0f, 20.0f);
		auto pBar = addBox(pModel, "Rotated bar", 40.0f, 10.0f, 10.0f);
		auto pSmallCube = addBox(pModel, "Component cube", 10.0f, 10.0f, 10.0f);

		auto pAssembly = pModel->AddComponentsObject();
		pAssembly->SetName("Assembly");
		pAssembly->AddComponent(pSmallCube.get(), makeTransform(0.0f, 5.0f, 5.0f, 0.0f));

		pModel->AddBuildItem(pCube.get(), makeTransform(0.0f, 50.0f, 50.0f, 0.0f));
		pModel->AddBuildItem(pBar.get(), makeTransform(30.0f, 100.0f, 60.0f, 0.0f));
		pModel->AddBuildItem(pAssembly.get(), makeTransform(0.0f, 50.0f, 120.0f, 0.0f));

		if (arguments.has("part-parameter")) {
			for (auto pMesh : { pCube, pBar, pSmallCube })
				pMesh->GetMetaDataGroup()->AddMetaData(EOS3MF_NAMESPACE, "partparameter", arguments.get("part-parameter"), "xs:string", true);
		}

		if (arguments.has("material-set")) {
			std::string sMaterialSet = arguments.get("material-set");
			auto pContext = libraries.m_pLibEOS->CreateContext(arguments.get("sdk"), arguments.get("log"));
			printSDKVersion(pContext);

			auto pMaterialSet = pContext->ImportMaterialSet(sMaterialSet);
			std::cout << "Material set \"" << pMaterialSet->GetName() << "\" for " << pMaterialSet->GetModelType()
				<< ", material " << pMaterialSet->GetMaterial() << ", " << pMaterialSet->GetBaseLayerthickness() << "um base layer" << std::endl;
			std::cout << "Exposure sets (usable with --part-parameter):" << std::endl;
			bool bFoundPartParameter = false;
			for (uint32_t nIndex = 0; nIndex < pMaterialSet->GetExposureSetCount(); nIndex++) {
				std::string sExposureSetName = pMaterialSet->GetExposureSetName(nIndex);
				std::cout << "  " << sExposureSetName << " (" << pMaterialSet->GetExposureSetLayerThickness(nIndex) << "um)" << std::endl;
				if (arguments.has("part-parameter") && (arguments.get("part-parameter") == sExposureSetName))
					bFoundPartParameter = true;
			}

			if (!arguments.has("part-parameter"))
				throw EExampleUsage("no --part-parameter given; choose one of the exposure sets listed above");
			if (!bFoundPartParameter)
				throw EExampleUsage("exposure set \"" + arguments.get("part-parameter") + "\" is not part of material set \"" + pMaterialSet->GetName() + "\"");

			pContext->AttachMaterialSetTo3MF(pModel.get(), sMaterialSet);
		}
		else {
			std::cout << "No --material-set given; the other examples need one embedded, or pass it to them." << std::endl;
		}

		auto pWriter = pModel->QueryWriter("3mf");
		pWriter->RegisterCustomNamespace("eos3mf", EOS3MF_NAMESPACE);
		std::cout << "Writing " << sOutputFileName << std::endl;
		pWriter->WriteToFile(sOutputFileName);
	});
}
