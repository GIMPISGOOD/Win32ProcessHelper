#include "common/Memory.h"

#include <windows.h>
#include <cstring>

namespace w32ph::common {

char* DuplicateStringToHeap(const char* src) {
    if (src == nullptr) {
        return nullptr;
    }

    const size_t length = strlen(src) + 1;
    char* dst = static_cast<char*>(HeapAlloc(GetProcessHeap(), 0, length));
    if (dst != nullptr) {
        memcpy(dst, src, length);
    }

    return dst;
}

} // namespace w32ph::common