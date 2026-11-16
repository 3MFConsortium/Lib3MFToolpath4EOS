# EOSPRINT API headers

Public headers of the EOSPRINT API and the EOS Toolpath API, one folder per SDK version. They are
copyright EOS GmbH and are redistributed here with the permission of EOS GmbH.

| Folder | Source |
|---|---|
| `2.18/include/EosprintApi` | EOSPRINT 2 SDK 2.18 (`Constants.h`, `EosprintApi.h`, `EventIds.h`, `Types.h`) and the Toolpath API package for 2.18 (`ToolpathApi.h`, `ToolpathTypes.h`) |
| `2.26/include/EosprintApi` | EOSPRINT 2 SDK 2.26, draft release of 8 October 2026 |

LibEOS loads `EosprintApi.dll` at runtime and keeps its own declarations per SDK version in
`Implementation/SDK`, generated from these headers without their comments. The headers are used by:

- `Scripts/eossdk_headers.py generate <version>`, which writes `libeos_eossdk_decl_<version>.hpp/.cpp`
  (declarations and entry point loading) and `libeos_eossdk_layout_<version>.cpp`;
- `libeos_eossdk_layout_<version>.cpp`, which includes the headers next to the generated declarations
  and checks at compile time that sizes, member offsets and enum values are the same;
- `Scripts/eossdk_headers.py compare <old> <new>`, which writes `Documentation/eossdk-versions.html`.

To add a version, create `<version>/include/EosprintApi` with the headers of that release, run
`generate` for it and `compare` against the previous version, and add an adapter in `Implementation/SDK`.
