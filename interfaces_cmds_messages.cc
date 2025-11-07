#include <log/message_types.h>
#include <log/message_initializer.h>

extern const isc::log::MessageID INTERFACES_CMDS_INIT_OK = "INTERFACES_CMDS_INIT_OK";
extern const isc::log::MessageID INTERFACES_CMDS_MISSING_PARAMS = "INTERFACES_CMDS_MISSING_PARAMS";
extern const isc::log::MessageID INTERFACES_CMDS_PARAM_BAD_TYPE = "INTERFACES_CMDS_PARAM_BAD_TYPE";
extern const isc::log::MessageID INTERFACES_CMDS_FILE_NOT_FOUND = "INTERFACES_CMDS_FILE_NOT_FOUND";
extern const isc::log::MessageID INTERFACES_CMDS_INIT_FAILED = "INTERFACES_CMDS_INIT_FAILED";
extern const isc::log::MessageID INTERFACES_CMDS_DEINIT_OK = "INTERFACES_CMDS_DEINIT_OK";
extern const isc::log::MessageID INTERFACES_CMDS_INTERFACES_AVAILABLE = "INTERFACES_CMDS_INTERFACES_AVAILABLE";
extern const isc::log::MessageID INTERFACES_CMDS_INTERFACES_AVAILABLE_HANDLER_FAILED = "INTERFACES_CMDS_INTERFACES_AVAILABLE_HANDLER_FAILED";
extern const isc::log::MessageID INTERFACES_CMDS_INTERFACES_GET = "INTERFACES_CMDS_INTERFACES_GET";
extern const isc::log::MessageID INTERFACES_CMDS_INTERFACES_GET_EMPTY = "INTERFACES_CMDS_INTERFACES_GET_EMPTY";
extern const isc::log::MessageID INTERFACES_CMDS_INTERFACES_GET_HANDLER_FAILED = "INTERFACES_CMDS_INTERFACES_GET_HANDLER_FAILED";
extern const isc::log::MessageID INTERFACES_CMDS_INTERFACES_SET = "INTERFACES_CMDS_INTERFACES_SET";
extern const isc::log::MessageID INTERFACES_CMDS_INTERFACES_SET_HANDLER_FAILED = "INTERFACES_CMDS_INTERFACES_SET_HANDLER_FAILED";
extern const isc::log::MessageID INTERFACES_CMDS_INTERFACES_DELETE = "INTERFACES_CMDS_INTERFACES_DELETE";
extern const isc::log::MessageID INTERFACES_CMDS_INTERFACES_DELETE_HANDLER_FAILED = "INTERFACES_CMDS_INTERFACES_DELETE_HANDLER_FAILED";

namespace {

const char* values[] = {
    "INTERFACES_CMDS_INIT_OK", "Hook 'interface commands' initialized successfully.",
    "INTERFACES_CMDS_MISSING_PARAMS", "Hook 'interface commands' expected required params: \"dhcp4-config-path\" and \"dhcp6-config-path\"",
    "INTERFACES_CMDS_PARAM_BAD_TYPE", "Hook 'interface commands' params type should be strings.",
    "INTERFACES_CMDS_FILE_NOT_FOUND", "Hook 'interface commands' file by path params not found.",
    "INTERFACES_CMDS_INIT_FAILED", "Failed to initialize hook 'interface commands'.",
    "INTERFACES_CMDS_DEINIT_OK", "Hook 'interface commands' uninitialized successfully.",
    "INTERFACES_CMDS_INTERFACES_AVAILABLE", "Available interfaces returned successfully.",
    "INTERFACES_CMDS_INTERFACES_AVAILABLE_HANDLER_FAILED", "An error occurred while retrieving a list of available network interfaces.",
    "INTERFACES_CMDS_INTERFACES_GET", "Configuration interfaces returned successfully.",
    "INTERFACES_CMDS_INTERFACES_GET_EMPTY", "Configuration not included interfaces. Subnet ID: %1.",
    "INTERFACES_CMDS_INTERFACES_GET_HANDLER_FAILED", "An error occurred while retrieving a list of interfaces in configuration.",
    "INTERFACES_CMDS_INTERFACES_SET", "New interfaces installed successfully.",
    "INTERFACES_CMDS_INTERFACES_SET_HANDLER_FAILED", "An error occurred set interfaces in configuration.",
    "INTERFACES_CMDS_INTERFACES_DELETE", "Interfaced deleted successfully.",
    "INTERFACES_CMDS_INTERFACES_DELETE_HANDLER_FAILED", "An error occurred delete interfaces in configuration.",
};

const isc::log::MessageInitializer initializer(values);

} // Anonymous namespace
