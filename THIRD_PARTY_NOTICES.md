# Third-party notices

LibEOS includes or links the following third-party components. Their license texts are in the
corresponding folders under `Libraries/` and, in a binary release, under `licenses/`.

| Component | Used for | License | Copyright |
|---|---|---|---|
| [lib3mf](https://github.com/3MFConsortium/lib3mf) | 3MF reading and writing (shipped as `lib3mf.dll` with bindings) | BSD 2-Clause | 3MF Consortium |
| [libzip](https://libzip.org) | Reading OpenJZ files and machine packages | BSD 3-Clause | Dieter Baron and Thomas Klausner |
| [zlib](https://zlib.net) | Compression for OpenJZ files and libzip | zlib License | Jean-loup Gailly and Mark Adler |
| [pugixml](https://pugixml.org) | XML parsing | MIT | Arseny Kapoulkine |
| [crossguid](https://github.com/graeme-hill/crossguid) | UUID generation | MIT | Graeme Hill |
| [cppcodec](https://github.com/tplgy/cppcodec) | Base64 encoding and decoding | MIT | Topology Inc., Jakob Petsovits and contributors |

The `LICENSE` files of lib3mf, libzip and zlib reproduce the license headers of their sources
(`Libraries/lib3mf/bin/Bindings`, `Libraries/libzip/zip.h` and `Libraries/zlib/zlib.h`).

EOS, EOSPRINT and the EOSPRINT API are products of EOS GmbH. The EOSPRINT API itself is not part of
LibEOS and is not redistributed with it; LibEOS loads an existing installation at runtime.

`Libraries/EOSSDK/<version>/include` contains the public headers of the EOSPRINT API and the Toolpath
API, redistributed with the permission of EOS GmbH. They are copyright EOS GmbH and are used as the
reference for LibEOS's own SDK declarations and for the layout tests. See `Libraries/EOSSDK/README.md`.
