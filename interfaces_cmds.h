#ifndef INTERFACES_CMDS_H
#define INTERFACES_CMDS_H

#include <cc/data.h>
#include <exceptions/exceptions.h>
#include <hooks/hooks.h>

#include <boost/shared_ptr.hpp>

namespace isc {
namespace interfaces_cmds {

// Functions for storing and clearing user-defined parameters when the hook is activated
void
storeConfiguration(std::string dhcp4_config_path, std::string dhcp6_config_path);

void
clearConfiguration();

/// @brief Forward declaration of implementation class.
class InterfacesCmdsImpl;

class InterfacesCmds {
public:
    /// @brief Constructor.
    ///
    /// It creates an instance of the @c InterfacesCmdsImpl.
    InterfacesCmds();

    void getAvailableInterfaces(hooks::CalloutHandle& handle) const;

    void getInterfaces(hooks::CalloutHandle& handle) const;

    void appendInterfaces(hooks::CalloutHandle& handle);

    void deleteInterfaces(hooks::CalloutHandle& handle);

private:
    /// Pointer to the actual implementation
    boost::shared_ptr<InterfacesCmdsImpl> impl_;
};

}  // namespace interfaces_cmds
}  // end of namespace isc

#endif  // INTERFACES_CMDS_H
