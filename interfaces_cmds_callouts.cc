#include <config.h>

#include <dhcpsrv/cfgmgr.h>
#include <hooks/hooks.h>
#include <process/daemon.h>

#include <fstream>
#include <string>

#include <interfaces_cmds_log.h>

#include "cc/data.h"
#include "exceptions/exceptions.h"
#include "interfaces_cmds.h"
#include "log/macros.h"
#include "process/daemon.h"

namespace isc {
namespace interfaces_cmds {

using namespace dhcp;
using namespace isc::process;
using namespace isc::hooks;
using namespace isc::log;
using namespace isc::data;
using namespace isc::config;

/// @brief This is a command callout for 'available-interfaces' command.
///
/// @param handle Callout handle used to retrieve a command and
/// provide a response.
/// @return 0 if this callout has been invoked successfully,
/// 1 otherwise.
int
available_interfaces(CalloutHandle& handle) {
    try {
        InterfacesCmds instance;
        instance.getAvailableInterfaces(handle);
    } catch (const std::exception& ex) {
        LOG_ERROR(interfaces_cmds_logger, INTERFACES_CMDS_INTERFACES_AVAILABLE_HANDLER_FAILED)
            .arg(ex.what());
        return (1);
    }

    return (0);
}

/// @brief This is a command callout for 'get-interfaces' command.
///
/// @param handle Callout handle used to retrieve a command and
/// provide a response.
/// @return 0 if this callout has been invoked successfully,
/// 1 otherwise.
int
get_interfaces(CalloutHandle& handle) {
    try {
        InterfacesCmds instance;
        instance.getInterfaces(handle);
    } catch (const std::exception& ex) {
        LOG_ERROR(interfaces_cmds_logger, INTERFACES_CMDS_INTERFACES_GET_HANDLER_FAILED)
            .arg(ex.what());
        return (1);
    }

    return (0);
}

/// @brief This is a command callout for 'append-interfaces' command.
///
/// @param handle Callout handle used to retrieve a command and
/// provide a response.
/// @return 0 if this callout has been invoked successfully,
/// 1 otherwise.
int
append_interfaces(CalloutHandle& handle) {
    try {
        InterfacesCmds instance;
        instance.appendInterfaces(handle);
    } catch (const std::exception& ex) {
        LOG_ERROR(interfaces_cmds_logger, INTERFACES_CMDS_INTERFACES_SET_HANDLER_FAILED)
            .arg(ex.what());
        return (1);
    }

    return (0);
}

/// @brief This is a command callout for 'delete-interfaces' command.
///
/// @param handle Callout handle used to retrieve a command and
/// provide a response.
/// @return 0 if this callout has been invoked successfully,
/// 1 otherwise.
int
delete_interfaces(CalloutHandle& handle) {
    try {
        InterfacesCmds instance;
        instance.deleteInterfaces(handle);
    } catch (const std::exception& ex) {
        LOG_ERROR(interfaces_cmds_logger, INTERFACES_CMDS_INTERFACES_DELETE_HANDLER_FAILED)
            .arg(ex.what());
        return (1);
    }

    return (0);
}

// A small helper for checking if a file exists in the file system.
bool
is_file_exists(const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

extern "C" {
int
load(hooks::LibraryHandle& handle) {
    try {
        // Make the hook library not loadable by d2 or ca.
        uint16_t family = CfgMgr::instance().getFamily();
        const std::string& proc_name = Daemon::getProcName();
        if (family == AF_INET) {
            if (proc_name != "kea-dhcp4") {
                isc_throw(isc::Unexpected,
                          "Bad process name: " << proc_name << ", expected kea-dhcp4");
            }
        } else {
            if (proc_name != "kea-dhcp6") {
                isc_throw(isc::Unexpected,
                          "Bad process name: " << proc_name << ", expected kea-dhcp6");
            }
        }

        // Obtaining pointers to user configuration parameters.
        ConstElementPtr dhcp4_config_path = handle.getParameter("dhcp4-config-path");
        ConstElementPtr dhcp6_config_path = handle.getParameter("dhcp6-config-path");

        if (dhcp4_config_path == nullptr || dhcp6_config_path == nullptr) {
            LOG_ERROR(interfaces_cmds_logger, INTERFACES_CMDS_MISSING_PARAMS);
            return (1);
        }

        // Validating user configuration parameters.
        if (dhcp4_config_path->getType() != Element::string ||
            dhcp6_config_path->getType() != Element::string) {
            LOG_ERROR(interfaces_cmds_logger, INTERFACES_CMDS_PARAM_BAD_TYPE);
            return (1);
        }

        std::string dhcp4_config_path_value = dhcp4_config_path->stringValue();
        std::string dhcp6_config_path_value = dhcp6_config_path->stringValue();

        storeConfiguration(dhcp4_config_path_value, dhcp6_config_path_value);

        if (!is_file_exists(dhcp4_config_path_value) || !is_file_exists(dhcp6_config_path_value)) {
            LOG_ERROR(interfaces_cmds_logger, INTERFACES_CMDS_FILE_NOT_FOUND);
            return (1);
        }

        // Registering API event handlers.
        handle.registerCommandCallout("available-interfaces", available_interfaces);
        handle.registerCommandCallout("get-interfaces", get_interfaces);
        handle.registerCommandCallout("append-interfaces", append_interfaces);
        handle.registerCommandCallout("delete-interfaces", delete_interfaces);
    } catch (const std::exception& ex) {
        LOG_ERROR(interfaces_cmds_logger, INTERFACES_CMDS_INIT_FAILED).arg(ex.what());
        return (1);
    }

    LOG_INFO(interfaces_cmds_logger, INTERFACES_CMDS_INIT_OK);
    return (0);
}

int
unload() {
    LOG_INFO(interfaces_cmds_logger, INTERFACES_CMDS_DEINIT_OK);
    return (0);
}

int
multi_threading_compatible() {
    return (1);
}
}

}  // namespace interfaces_cmds
}  // namespace isc
