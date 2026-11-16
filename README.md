# Lib3MFToolpath4EOS

Lib3MFToolpath4EOS is a C++ library with a stable C interface that connects the [3MF](https://3mf.io) world to EOS
laser powder bed fusion machines. It drives an installed EOSPRINT API ("EOS SDK") at runtime and adds
3MF support on top of it:

- convert mesh 3MF files into EOS OpenJZ jobs and back,
- compute EOS toolpaths through the EOS Toolpath API and store them in a 3MF with the 3MF Toolpath extension,
- turn a toolpath 3MF back into an EOS task,
- create EOS tasks from jobs, estimate build times, and upload tasks to machines,
- read and write raw EOS tasks layer by layer.

The library file and the binding headers carry the name: `lib3mftoolpath4eos.dll`,
`lib3mftoolpath4eos_dynamic.hpp` and so on. The API is defined in `ACT/Lib3MFToolpath4EOS.xml` and uses the
namespace `Lib3MF4EOS` (C++ namespace `Lib3MF4EOS`, exported symbols `lib3mf4eos_*`, `Lib3MF4EOS.py`).

Lib3MFToolpath4EOS does not ship any EOS binaries. The EOS SDK binding loads `EosprintApi.dll` from an
existing EOSPRINT 2 or EOSPRINT API installation when it is needed. The repository contains the headers of
the EOSPRINT API and the EOS Toolpath API in `Libraries/EOSSDK`. They are copyright EOS GmbH, are shipped
with the permission of EOS GmbH, and are only used at build time to generate and check the SDK declarations
(see [Libraries/EOSSDK/README.md](Libraries/EOSSDK/README.md)).

## EOS Toolpath API: access, contractual restrictions, and confidentiality

- **An agreement with EOS and authorization are required.** Computing toolpaths (`ComputeToolpathFrom3MF`, `ComputeToolpathFromOpenJZ`) and creating tasks from toolpaths (`CreateTaskFrom3MFToolpath`) use the EOS Toolpath API. Access requires an agreement with EOS GmbH, under which EOS may assign a client ID and client-side encryption key. EDN membership is mandatory, and access is reserved for selected partners; there is no entitlement to access. Lib3MFToolpath4EOS does not grant any rights to the Toolpath API.

- **EOS process data must not be made accessible to third parties.** The EOS agreement prohibits making EOS process data - including parameters, vector data, and energy input data - accessible to third parties in human- or machine-readable form. This includes EOS toolpaths and 3MF files containing EOS process data, such as output from `WriteResultTo3MFFile`. Do not distribute or otherwise make such data or files accessible to other, unauthorized parties or the public. These restrictions apply to EOS processes (original or customized), not to exposure strategies developed entirely by Toolpath API users themselves. The agreement defines the binding terms and scope of use.

- **The client-side key is confidential.** The client-side encryption key assigned by EOS is a confidential credential, even though RSA libraries may refer to it as a "public key" and it is provided in PEM format. Do not ship it as a file alongside an application or check it into a repository. Protect it from unauthorized access. The examples store it in the Windows Credential Manager; see [Examples/README.md](Examples/README.md#toolpath-api-client-key).

Detailed documentation: [Documentation/index.html](Documentation/index.html) (library) and
[Documentation/examples.html](Documentation/examples.html) (examples). How laser power ramps on hatches and
polylines are written to EOS is described in [Documentation/power-ramps.html](Documentation/power-ramps.html), the
conversion settings and the conversion log in [Documentation/conversion.html](Documentation/conversion.html).

## Requirements

- Windows x64.
- An installed **EOSPRINT 2** or **EOSPRINT API (SDK)**. Without an explicit directory, the library searches
  `C:\Program Files\EOS\EOSPRINT 2\2_xx` first and then `C:\Program Files\EOS\EOSPRINT 2 SDK\2_xx\bin\x64\release`,
  newest version first. Pass the directory explicitly to pick a specific installation.
- For toolpath computation: an **agreement with EOS GmbH** for the Toolpath API, the **client ID and
  client-side encryption key** assigned under it, and an SDK version that includes the Toolpath API.
- An EOS **material set** (`.eospar`) and a **machine configuration** (a machine package file or a
  configuration directory) for everything that goes beyond file conversion.
- To build from source: CMake 3.10 or newer and Visual Studio 2022 or newer (C++17).

## Repository layout

| Folder | Content |
|---|---|
| `ACT/` | API definition `Lib3MFToolpath4EOS.xml` and the Automatic Component Toolkit used to generate bindings and interfaces |
| `Bindings/` | Generated Lib3MFToolpath4EOS bindings: C, C++ (implicit and dynamic), C#, Go, Java, Node.js, Pascal, Python |
| `Interfaces/` | Generated C++ interfaces implemented by the library |
| `Implementation/` | The library implementation |
| `Common/` | Shared utilities (streams, ZIP writer, working directories, string and file helpers) |
| `Libraries/` | Third-party code: lib3mf (binary and bindings), libzip, zlib, PugiXML, crossguid, cppcodec |
| `Examples/` | Command line examples in C++, see [Examples/README.md](Examples/README.md) |
| `Documentation/` | HTML documentation |
| `Scripts/` | Release packaging script |

## Building

```
cmake -S . -B build
cmake --build build --config Release
```

This builds `build/Release/lib3mftoolpath4eos.dll` and the examples in `build/Examples/Release`, next to copies of
`lib3mftoolpath4eos.dll` and `lib3mf.dll`. Turn the examples off with `-DLIBEOS_BUILD_EXAMPLES=OFF`.

The source list is collected with a glob, so re-run the configure step after adding or removing files.

## Using Lib3MFToolpath4EOS

The library works on Lib3MF objects, so an application loads both libraries and hands its Lib3MF instance to
Lib3MFToolpath4EOS. With the dynamic C++ bindings (`Bindings/CppDynamic` and `Libraries/lib3mf/bin/Bindings/CppDynamic`):

```cpp
#include "lib3mftoolpath4eos_dynamic.hpp"
#include "lib3mf_dynamic.hpp"

auto pLib3MF = Lib3MF::CWrapper::loadLibrary("lib3mf.dll");
auto pLibEOS = Lib3MF4EOS::CWrapper::loadLibrary("lib3mftoolpath4eos.dll");
pLibEOS->InjectComponent("Lib3MF", pLib3MF->GetSymbolLookupMethod());

// Empty SDK directory: auto-detect. Empty log file: a temporary file is used.
auto pContext = pLibEOS->CreateContext("", "");

// Read a mesh 3MF. The material set attachment must be registered before reading.
auto pModel = pLib3MF->CreateModel();
auto pReader = pModel->QueryReader("3mf");
pReader->AddRelationToRead(pContext->GetMaterialSetRelationshipType());
auto pSource = pModel->CreatePersistentSourceFromFile("part.3mf");
pReader->ReadFromPersistentSource(pSource.get());

// Embed a material set if the file has none, then convert to OpenJZ.
pContext->AttachMaterialSetTo3MF(pModel, "316L_040_FlexM291.eospar");
pContext->Convert3MFToOpenJZ(pModel, "part.openjz");
```

Long running operations return futures. Poll `IsRunning()` (or call `WaitFor(milliseconds)`), then check
`HasFinished()`, `HasFailed()` and `GetErrorMessage()`:

```cpp
auto pConfig = pContext->LoadMachinePackageFromFile("M290.eosmachineinformation");
while (pConfig->IsRunning())
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
if (pConfig->HasFailed())
	throw std::runtime_error(pConfig->GetErrorMessage());

// The client key is stored as a generic Windows credential (see toolpath_credential in the examples).
pContext->AuthenticateToolpathAPIFromCredentialStore(nClientID, "LibEOS-TPAPI");
// Or pass the PEM text yourself: pContext->AuthenticateToolpathAPI(nClientID, sClientKeyPEM);
auto pComputation = pContext->ComputeToolpathFrom3MF(pModel, pConfig);
// ... wait as above ...
pComputation->WriteResultTo3MFFile("part.toolpath.3mf");
```

### Workflows

| From | To | API |
|---|---|---|
| mesh 3MF | OpenJZ file | `Convert3MFToOpenJZ` |
| OpenJZ file | mesh 3MF | `ConvertOpenJZTo3MF` |
| mesh 3MF | EOS job | `LoadJobFrom3MF`, then `CreateTask` / `EstimateBuildTime` |
| OpenJZ file | EOS job | `LoadOpenJZFromFile`, then `CreateTask` / `EstimateBuildTime` |
| mesh 3MF | toolpath 3MF | `ComputeToolpathFrom3MF`, then `WriteResultTo3MFFile` |
| OpenJZ file | toolpath 3MF | `ComputeToolpathFromOpenJZ`, then `WriteResultTo3MFFile` |
| toolpath 3MF | EOS task | `CreateTaskFrom3MFToolpath` |
| EOS task | machine | `ConnectToMachine`, then `UploadTask` or `UploadTaskFile` |

### Things to know

- **Material set.** The library takes the machine target, parameter set and layer thickness from the material
  set embedded in the 3MF (`/Attachments/materialset.eospar`). Without one, the model must carry all
  `eos3mf` job metadata listed below.
- **Reading toolpath 3MFs.** Toolpath layers are read lazily. Load such files with
  `ReadFromPersistentSource` and keep the persistent source alive while the library works on the model.
  `ReadFromFile` closes the file and later layer reads fail with "Data source is invalidated".
- **Machine connection simulation.** `EnableSDKSimulation(SimulateConnectivity)` must be called before
  the SDK is loaded. A simulated machine cannot deliver its configuration, so pass one explicitly.
- **Older SDKs.** Functions missing from an older `EosprintApi.dll` are detected. Reading skips optional
  toolpath callbacks; writing a value that needs a missing function fails with `TOOLPATHSDKNOTSUPPORTED`.

## 3MF conventions

Job metadata uses the namespace `http://schemas.3mf.io/eos/job/2026/11` (prefix `eos3mf`):

| Where | Name | Meaning |
|---|---|---|
| model | `machinetarget`, `paramsetname`, `paramsetversion`, `materialname`, `materialthickness` | Job settings; defaults come from the embedded material set |
| object | `partparameter` | Exposure set name of the part (one of the material set's exposure sets) |
| object | `issupport` | `true` for support structures |

The material set attachment uses the relationship type `http://schemas.3mf.io/eos/materialset/2026/11`.

Toolpath profiles store `laserpower` (W), `laserspeed` (mm/s) and `laserindex` without a
namespace, and the EOS specific values in `http://schemas.3mf.io/eos/toolpath/2026/11`: `exposuretype`, `skywriting`,
`focus` (the EOS focus value, 0.0 fully defocused to 1.0 fully focused; left out when fully focused)
and, when they differ from 0, `exposuresetindex`, `exposurestepindex`, `pulsedwaveperiodtime`,
`pulsedwavepowerontime` and `exposeddepth`. The 3MF `laserfocus` is a focus offset in mm with no EOS
equivalent: a non-zero value is refused in strict mode and ignored with a warning otherwise, and a
`laserfocus` modifier is rejected. Focus commands are checked against the scanner capabilities of the
machine configuration.

Machine packages use the namespace `http://schemas.3mf.io/eos/machinepackage/2026/11`.

Lib3MFToolpath4EOS always writes these current `schemas.3mf.io/eos` namespaces. When reading, it also accepts any
legacy namespace registered with `EOSContext::RegisterLegacyNamespace(Kind, NamespaceURI)`.
Registrations apply to the context and to the futures and loaders it creates afterwards; the current
namespace always wins when a value is present under both. Reading a 3MF with a legacy material set
relationship type additionally requires the host to add that type to its lib3mf reader with
`AddRelationToRead`, because lib3mf drops attachments whose relationship type is not registered before
reading. `GetNamespace(Kind)` returns the current namespace and `ClearLegacyNamespaces()` removes all
registered ones.

Parts are numbered 1..N in build item order, depth first through component objects. OpenJZ files store
each part's mesh relative to its bounding box minimum. Parts keep their Z position, so they stay aligned with
the toolpath layers; a part below Z = 0 is refused by the EOS SDK (`EOS_ERR_STL_BELOW_PLATFORM`).

## Regenerating the bindings

The API is defined in `ACT/Lib3MFToolpath4EOS.xml`. After changing it, run `ACT/generate.bat`, which calls ACT 1.8.1:

```
act.exe Lib3MFToolpath4EOS.xml -bindings ..\Bindings -interfaces ..\Interfaces -suppresssubcomponents -suppresslicense -suppressstub -suppressexamples
```

## Binary release

```
powershell -ExecutionPolicy Bypass -File Scripts\create_release.ps1
```

configures, builds and packages `build-release/LibEOS-<version>-win64.zip` with:

| Folder | Content |
|---|---|
| `bin/` | `lib3mftoolpath4eos.dll`, `lib3mf.dll` and the example executables |
| `lib/` | `lib3mftoolpath4eos.lib` import library |
| `include/LibEOS`, `include/Lib3MF` | C++ headers (`Cpp` and `CppDynamic`) |
| `bindings/LibEOS`, `bindings/Lib3MF` | All language bindings |
| `examples/` | Example sources with a standalone CMake project |
| `doc/` | HTML documentation |
| `licenses/` | Third-party license texts |

The same package can be built by hand with `cmake --build <build dir> --config Release --target package`.

## License

Lib3MFToolpath4EOS is licensed under the BSD license in [LICENSE.txt](LICENSE.txt). Third-party components and their
licenses are listed in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). EOS, EOSPRINT and the EOSPRINT API
are products of EOS GmbH and are not part of this repository. The BSD license does not cover the use of the
EOS Toolpath API, which requires an agreement with EOS GmbH (see above).
