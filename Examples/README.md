# Lib3MFToolpath4EOS examples

Five small command line programs that cover the Lib3MFToolpath4EOS workflows, and a helper that stores the
Toolpath API client key. Each one is a single C++ file in `cpp/`, sharing the helpers in
`cpp/example_common.hpp` (argument parsing, library loading, waiting for futures). They use the dynamic C++
bindings, so they only need `lib3mftoolpath4eos.dll` and `lib3mf.dll` next to the executable.

| Example | What it does | Needs |
|---|---|---|
| `00_generate_sample_3mf` | Creates a mesh 3MF with three parts; optionally embeds a material set | material set (optional) |
| `01_convert_3mf_openjz` | Converts a mesh 3MF into an OpenJZ job, or an OpenJZ job into a mesh 3MF | material set |
| `02_openjz_to_machine` | Loads an OpenJZ job, creates an EOS task and uploads it | machine configuration |
| `03_mesh3mf_to_machine` | Loads a mesh 3MF as job, creates an EOS task and uploads it | material set, machine configuration |
| `06_toolpath3mf_to_machine` | Creates an EOS task from your own toolpath 3MF and uploads it | machine configuration, Toolpath API |
| `toolpath_credential` | Stores, checks and deletes the Toolpath API client key in the Windows Credential Manager | client key |

> **EOS Toolpath API.** Example 06 uses the EOS Toolpath API, which requires an agreement with EOS GmbH and
> authorization by EOS. EOS process data, including EOS toolpaths and 3MF files containing EOS process data, must
> not be made accessible to third parties in human- or machine-readable form. See
> [EOS Toolpath API](../README.md#eos-toolpath-api-access-contractual-restrictions-and-confidentiality) in the main README.

## Building

The examples are part of the main build:

```
cmake -S . -B build
cmake --build build --config Release
```

The executables end up in `build/Examples/Release` together with `lib3mftoolpath4eos.dll` and `lib3mf.dll`.

`Examples/CMakeLists.txt` also works as a standalone project, both in this repository (pass the built
library with `-DLIBEOS_DLL=<path>/lib3mftoolpath4eos.dll`) and in the `examples/` folder of a binary release, where it
picks up the headers and DLLs of the release automatically:

```
cmake -S examples -B build-examples
cmake --build build-examples --config Release
```

## Common options

Every example prints its usage with `--help`. Options can also be set through environment variables,
which is convenient for values that never change:

| Option | Environment variable | Meaning |
|---|---|---|
| `--sdk <dir>` | `LIBEOS_SDK_DIR` | EOSPRINT API directory; auto-detected if omitted |
| `--log <file>` | | EOS SDK log file; a temporary file is used if omitted |
| `--material-set <file>` | `LIBEOS_MATERIAL_SET` | Material set (`.eospar`) to embed if the 3MF has none |
| `--machine-config <path>` | `LIBEOS_MACHINE_CONFIG` | Machine configuration: a configuration directory or a machine package file |
| `--client-id <id>` | `LIBEOS_TPAPI_CLIENT_ID` | Toolpath API client ID |
| `--credential <name>` | `LIBEOS_TPAPI_CREDENTIAL` | Name of the Windows Credential Manager entry with the Toolpath API client key |
| `--machine <address>` | `LIBEOS_MACHINE_ADDRESS` | Network address of the machine; without it, the connection is simulated |
| `--store-task <dir>` | | Also copy the created task into this directory |

The exit code is 0 on success, 2 for usage errors and 1 for all other errors.

If both EOSPRINT 2 and the EOSPRINT API SDK are installed, auto-detection picks the EOSPRINT 2 installation.
Use `--sdk` to select the SDK, for example
`--sdk "C:\Program Files\EOS\EOSPRINT 2 SDK\2_18\bin\x64\release"`.

## Walkthrough

Start by generating a sample part with an embedded material set. The first run without `--part-parameter`
lists the exposure sets of the material set and stops:

```
00_generate_sample_3mf.exe box.3mf --material-set 316L_040_FlexM291.eospar
00_generate_sample_3mf.exe box.3mf --material-set 316L_040_FlexM291.eospar --part-parameter _Default_DirectPart
```

Convert it to OpenJZ and back:

```
01_convert_3mf_openjz.exe box.3mf box.openjz
01_convert_3mf_openjz.exe box.openjz box_roundtrip.3mf
```

Create a task and upload it. Without `--machine`, the connection is simulated, which needs an explicit
machine configuration:

```
02_openjz_to_machine.exe --machine-config M290.eosmachineinformation --store-task tasks box.openjz
03_mesh3mf_to_machine.exe --machine-config M290.eosmachineinformation --machine 192.168.0.10 box.3mf
```

To expose your own toolpaths, write them into a 3MF with the 3MF Toolpath extension, using the profile
parameters of the 3MF conventions in the [main README](../README.md#3mf-conventions). Store the Toolpath API
client key once (see [below](#toolpath-api-client-key)) and create a task from the toolpath 3MF:

```
toolpath_credential.exe store LibEOS-TPAPI client_key.pem
06_toolpath3mf_to_machine.exe --machine-config M290.eosmachineinformation --client-id 1 --credential LibEOS-TPAPI my.toolpath.3mf
```

## Toolpath API client key

The client key (the client-side encryption key that EOS assigns under the Toolpath API agreement) is a PEM file.
It is a confidential credential, even though RSA libraries may refer to it as a "public key", so the examples do not read it from a file. Its PEM text is stored once as a generic credential in the
Windows Credential Manager, which encrypts it for the current Windows user, and the examples get the
credential's name with `--credential` (or `LIBEOS_TPAPI_CREDENTIAL`):

```
toolpath_credential.exe store LibEOS-TPAPI client_key.pem   # import the file issued by EOS
toolpath_credential.exe store LibEOS-TPAPI                  # or paste the PEM text into the console
toolpath_credential.exe check LibEOS-TPAPI                  # shows the PEM type and size, never the key
toolpath_credential.exe delete LibEOS-TPAPI
```

After importing, remove the PEM file from the computer and keep the original in a secure place. The credential
is visible under *Control Panel > Credential Manager > Windows Credentials > Generic Credentials*. It is bound to
the Windows user that stored it, so a service that runs under another account needs its own entry.

`authenticateToolpathAPI` in `example_common.hpp` passes only the credential name to
`AuthenticateToolpathAPIFromCredentialStore`. The library reads the key itself and erases its copies after
authenticating, so the key never passes through the example. `toolpath_credential check` uses
`readToolpathClientKey` (`CredReadW`) to inspect the stored key.

## The examples in detail

### 00_generate_sample_3mf

`00_generate_sample_3mf [options] <output.3mf>`

Writes three parts that exercise part placement: a 20 mm cube moved by its build item, a 40 × 10 × 10 mm
bar rotated by 30° about Z, and a 10 mm cube placed through a components object.

With `--material-set`, the material set is imported (this loads the EOS SDK), its exposure sets are listed
and it is embedded into the 3MF. `--part-parameter` must then name one of the listed exposure sets; it is
stored as `eos3mf:partparameter` on every part. Without `--material-set`, the 3MF contains only the meshes.

### 01_convert_3mf_openjz

`01_convert_3mf_openjz [options] <input.3mf|input.openjz> <output>`

The direction follows the input extension. For 3MF input, the material set comes from the file or from
`--material-set`. For OpenJZ input, the geometry, part metadata and the OpenJZ's material set are written
into a mesh 3MF.

### 02_openjz_to_machine and 03_mesh3mf_to_machine

`02_openjz_to_machine [options] <job.openjz>`, `03_mesh3mf_to_machine [options] <model.3mf>`

Load the job, activate the machine configuration, create an EOS task and upload it. The machine
configuration is `--machine-config` if given, otherwise it is downloaded from the connected machine.

### 06_toolpath3mf_to_machine

`06_toolpath3mf_to_machine [options] <toolpath.3mf>`

Reads a toolpath 3MF from your own toolpath generation (through a persistent source, see below), writes its
layers into an EOS task and uploads it. Besides the toolpath, the 3MF needs the part meshes and an embedded
material set, or else all `eos3mf` job metadata (see the main README).

## Notes for your own code

- Load `lib3mf.dll` and `lib3mftoolpath4eos.dll` and call `InjectComponent("Lib3MF", ...)`, so both libraries share
  one Lib3MF instance (`loadLibraries` in `example_common.hpp`).
- Register the material set relationship with `AddRelationToRead` before reading a 3MF, otherwise Lib3MF
  drops the attachment.
- Read toolpath 3MFs with `ReadFromPersistentSource` and keep the source alive (`read3MF`).
- Call `EnableSDKSimulation` before anything loads the SDK (`createContext`).
