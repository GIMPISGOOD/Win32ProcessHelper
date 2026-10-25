#include "common/Json.h"

#include <cstdio>

namespace w32ph::common {

std::string EscapeJson(const std::string& value) {
    std::string out;
    out.reserve(value.size() + 16);

    for (char c : value) {
        const unsigned char ch = static_cast<unsigned char>(c);

        switch (ch) {
        case '"':
            out.append("\\\"");
            break;
        case '\\':
            out.append("\\\\");
            break;
        case '\b':
            out.append("\\b");
            break;
        case '\f':
            out.append("\\f");
            break;
        case '\n':
            out.append("\\n");
            break;
        case '\r':
            out.append("\\r");
            break;
        case '\t':
            out.append("\\t");
            break;
        default:
            if (ch < 0x20) {
                char buffer[8];
                const int written = std::snprintf(buffer, sizeof(buffer), "\\u%04x", ch);
                if (written > 0) {
                    out.append(buffer, static_cast<size_t>(written));
                }
            } else {
                out.push_back(static_cast<char>(ch));
            }
            break;
        }
    }

    return out;
}

} // namespace w32ph::common