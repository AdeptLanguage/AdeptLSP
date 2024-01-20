
#include "UTIL/util.h"
#include "UTIL/string.h"
#include "json_stringify.h"

strong_cstr_t json_stringify_string(weak_cstr_t content){
    // Determine characters needed
    return string_to_escaped_string(content, strlen(content), '"', true);
}
