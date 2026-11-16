/*++

Copyright (C) 2026 3MF Consortium.

All rights reserved.

Abstract: Converts an OpenJZ job archive into a Lib3MF Model.

*/

#ifndef __LIBEOS_OPENJZREADER
#define __LIBEOS_OPENJZREADER

#include "lib3mftoolpath4eos_interfaces.hpp"

#include <string>

namespace Lib3MF4EOS {
namespace Impl {

// Adds all parts of the OpenJZ as mesh objects with build items to the model, placed in world
// coordinates, together with the eos3mf job metadata and the embedded material set (if any).
void convertOpenJZToModel(const std::string& sOpenJZFileName, Lib3MF::PModel pModel);

} // namespace Impl
} // namespace Lib3MF4EOS

#endif // __LIBEOS_OPENJZREADER
