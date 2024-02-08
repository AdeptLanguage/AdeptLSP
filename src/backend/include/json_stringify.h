
#ifndef _ISAAC_JSON_STRINGIFY_H
#define _ISAAC_JSON_STRINGIFY_H

#include "UTIL/ground.h"

// ---------------- json_stringify_string ----------------
// Turns the contents of a C-String into a JSON string
strong_cstr_t json_stringify_string(weak_cstr_t content);

#endif // _ISAAC_JSON_STRINGIFY_H
