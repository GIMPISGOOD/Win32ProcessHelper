#pragma once

#include <stdint.h>

namespace w32ph::cli::commands {

int RunBasicProcessEnumeration(bool json);
int RunDetailedProcessEnumeration(bool json);
int RunProcessDetail(uint32_t processId);

} // namespace w32ph::cli::commands