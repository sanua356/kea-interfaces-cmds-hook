#include <config.h>

#include <hooks/hooks.h>

namespace isc {
namespace interfaces_cmds {

extern "C" {

int
version() {
    return (KEA_HOOKS_VERSION);
}

}  // extern "C"

}  // namespace interfaces_cmds
}  // namespace isc
