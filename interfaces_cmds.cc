#include <config.h>

#include <cc/command_interpreter.h>
#include <dhcpsrv/cfgmgr.h>
#include <process/daemon.h>

#include <sstream>
#include <string>
#include <vector>

#include <interfaces_cmds.h>
#include <interfaces_cmds_log.h>

#include "cc/data.h"
#include "dhcp/iface_mgr.h"
#include "dhcpsrv/cfg_iface.h"
#include "dhcpsrv/srv_config.h"
#include "dhcpsrv/subnet.h"
#include "dhcpsrv/subnet_id.h"
#include "hooks/callout_handle.h"
#include "log/macros.h"

using namespace isc::config;
using namespace isc::dhcp;
using namespace isc::data;
using namespace isc::hooks;
using namespace isc::util;
using namespace isc::process;
using namespace std;

namespace {
    string dhcp4_config_path = "";
    string dhcp6_config_path = "";
}  // namespace

namespace isc {
    namespace interfaces_cmds {

        void
        storeConfiguration(string dhcp4_config_path_param, string dhcp6_config_path_param) {
            dhcp4_config_path = dhcp4_config_path_param;
            dhcp6_config_path = dhcp6_config_path_param;
        }

        void
        clearConfiguration() {
            dhcp4_config_path = "";
            dhcp6_config_path = "";
        }

        /// @brief Implementation of the @c InterfacesCmds class.
        ///
        /// It provides functions for class manipulations.
        class InterfacesCmdsImpl {
        public:
            /// @brief Constructor.
            InterfacesCmdsImpl() {
                family_ = CfgMgr::instance().getFamily();
            }

            /// @brief Destructor.
            ~InterfacesCmdsImpl() {
            }

        private:
            /// @brief Retrieves mandatory arguments from the command encapsulated
            /// within the callout handle.
            ///
            /// @param callout_handle Callout handle encapsulating the command.
            ///
            /// @return Pointer to the data structure encapsulating arguments being
            /// a map.
            /// @throw CtrlChannelError if the arguments are not present or aren't
            /// a map.
            ConstElementPtr getMandatoryArguments(CalloutHandle& callout_handle) const {
                ConstElementPtr command;
                callout_handle.getArgument("command", command);
                ConstElementPtr arguments;
                static_cast<void>(parseCommandWithArgs(arguments, command));
                return (arguments);
            }

            /// @brief Checks all arguments received from the user via the API for validity.
            ///
            /// @param command_name Command name, used for error reporting.
            /// @param arguments Command arguments to be parsed and validated.
            void validateArguments(const string& command_name, const ConstElementPtr& arguments) {
                // If no parameter sections are specified, no command API should be available.
                ConstElementPtr section = arguments->get("section");
                if (!section) {
                    isc_throw(BadValue, "missing 'section' argument for the '"
                    << command_name
                    << "' command. (Type argument - enum \"global\", \"subnet\")");
                }

                if (section->getType() != Element::string) {
                    isc_throw(BadValue, "'section' argument specified for the '"
                    << command_name << "' command is not a string");
                }

                // If an incorrect value is specified in the section, access to the API is blocked.
                string section_value = section->stringValue();
                if (section_value != "global" && section_value != "subnet") {
                    isc_throw(BadValue, "invalid value in param 'section' for the '"
                    << command_name
                    << "' command. Expected enum \"global\", \"subnet\"");
                }

                // For APIs that provide the ability to modify the configuration, the "interfaces" parameter
                // must be present.
                if (command_name == "append-interfaces" || command_name == "delete-interfaces") {
                    ConstElementPtr interfaces = arguments->get("interfaces");
                    if (!interfaces) {
                        isc_throw(BadValue, "missing 'interfaces' argument for the '"
                        << command_name
                        << "' command. (Type argument - list of strings)");
                    }

                    if (interfaces->getType() != Element::list) {
                        isc_throw(BadValue, "'interfaces' argument specified for the '"
                        << command_name << "' command is not a list of string");
                    }
                }

                // If the subnet configuration is affected, its ID must also be obtained.
                if ((command_name == "get-interfaces" || command_name == "append-interfaces" ||
                    command_name == "delete-interfaces") &&
                    section_value == "subnet") {
                    ConstElementPtr subnet_id = arguments->get("subnet_id");
                if (!subnet_id) {
                    isc_throw(BadValue, "missing 'subnet_id' argument for the '"
                    << command_name
                    << "' command. (Type argument - unsigned integer)");
                }

                if (subnet_id->getType() != Element::integer) {
                    isc_throw(BadValue, "'subnet_id' argument specified for the '"
                    << command_name << "' command is not a unsigned integer");
                }
                    }
            }

            /// @brief Writes the current server configuration to a JSON configuration file.
            ConstElementPtr writeConfiguration() {
                try {
                    ConstElementPtr answer;

                    Daemon daemon = Daemon{};

                    string filename;
                    if (family_ == AF_INET) {
                        filename = dhcp4_config_path;
                    } else {
                        filename = dhcp6_config_path;
                    }

                    ConstElementPtr cfg = CfgMgr::instance().getCurrentCfg()->toElement();

                    daemon.writeConfigFile(filename, cfg);
                    return (createAnswer(CONTROL_RESULT_SUCCESS, "Configuration updated successfully."));
                } catch (const isc::Exception& ex) {
                    return (createAnswer(CONTROL_RESULT_ERROR,
                                         string("Error saving config to file: ") + ex.what()));
                }
            }

        public:
            /// @brief Processes and returns a response to 'available-interfaces' command.
            ///
            /// @param callout_handle Reference to the callout handle holding command
            /// to be processed and where result should be stored.
            void getAvailableInterfaces(CalloutHandle& handle) {
                ConstElementPtr response;

                try {
                    // Two variables are necessary because KEA understands a network address both by name
                    // and by name/address.
                    ElementPtr ifaces = Element::createList();
                    ElementPtr ifaces_with_ip = Element::createList();

                    for (auto& el : IfaceMgr::instance().getIfaces()) {
                        string iface = el->getName();
                        ifaces->add(Element::create(iface));

                        for (auto& el_ip : el->getAddresses()) {
                            // Determine which addresses belong to a specific network interface.
                            if (el->hasAddress(el_ip)) {
                                // If the user requested network interfaces for v4 or v6, we display only
                                // them.
                                if (family_ == AF_INET && el_ip.get().isV4()) {
                                    ifaces_with_ip->add(
                                        Element::create(iface + "/" + el_ip.get().toText()));
                                } else if (family_ == AF_INET6 && el_ip.get().isV6()) {
                                    ifaces_with_ip->add(
                                        Element::create(iface + "/" + el_ip.get().toText()));
                                }
                            }
                        };
                    };

                    // We assemble the final object that the client will receive.
                    ElementPtr map = Element::createMap();
                    map->set("interfaces", ifaces);
                    map->set("interfaces-with-ip", ifaces_with_ip);

                    ostringstream text;
                    text << ifaces->size() << " interfaces found.";

                    // We deliver the result to the client
                    response = createAnswer(CONTROL_RESULT_SUCCESS, text.str(), map);
                    LOG_INFO(interfaces_cmds_logger, INTERFACES_CMDS_INTERFACES_AVAILABLE);

                } catch (const exception& ex) {
                    LOG_ERROR(interfaces_cmds_logger, INTERFACES_CMDS_INTERFACES_AVAILABLE_HANDLER_FAILED)
                    .arg(ex.what());

                    response = createAnswer(CONTROL_RESULT_ERROR, ex.what());
                }

                handle.setArgument("response", response);
            }

            /// @brief Processes and returns a response to 'get-interfaces' command.
            ///
            /// @param callout_handle Reference to the callout handle holding command
            /// to be processed and where result should be stored.
            void getInterfaces(CalloutHandle& handle) {
                ConstElementPtr response;
                ElementPtr map = Element::createMap();

                try {
                    // Validate arguments before starting to manipulate the configuration.
                    ConstElementPtr arguments = getMandatoryArguments(handle);
                    validateArguments("get-interfaces", arguments);

                    string section = arguments->get("section")->stringValue();

                    if (section == "global") {
                        // In the global section, we get a list of configuration addresses. Since the
                        // methods are called on CfgMgr, we don't need to specify them for v4 or v6.
                        ConstElementPtr interfaces_config =
                        CfgMgr::instance().getCurrentCfg()->getCfgIface()->toElement();
                        ConstElementPtr interfaces = interfaces_config->get("interfaces");

                        map->set("interfaces", interfaces);

                        ostringstream text;
                        text << "in section " << section << " " << interfaces->size()
                        << " interfaces found.";

                        // We determine how many subnets we received, and then build a response based on
                        // this.
                        if (interfaces->size() > 0) {
                            response = createAnswer(CONTROL_RESULT_SUCCESS, text.str(), map);
                            LOG_INFO(interfaces_cmds_logger, INTERFACES_CMDS_INTERFACES_GET);
                        } else {
                            response = createAnswer(CONTROL_RESULT_EMPTY, text.str(), map);
                            LOG_INFO(interfaces_cmds_logger, INTERFACES_CMDS_INTERFACES_GET_EMPTY);
                        }

                    } else if (section == "subnet") {
                        // If the user requested an interface from a subnet, we first get its ID.
                        int subnet_id = arguments->get("subnet_id")->intValue();

                        // We will predefine a pointer for the network interface, and then search through
                        // the subnet configuration to find the required element.
                        ConstElementPtr interface = nullptr;
                        if (family_ == AF_INET) {
                            Subnet4Ptr subnet4_config =
                            CfgMgr::instance().getCurrentCfg()->getCfgSubnets4()->getSubnet(
                                SubnetID(subnet_id));
                            if (subnet4_config) {
                                interface = subnet4_config->toElement()->get("interface");
                            }
                        } else {
                            Subnet6Ptr subnet6_config =
                            CfgMgr::instance().getCurrentCfg()->getCfgSubnets6()->getSubnet(
                                SubnetID(subnet_id));
                            if (subnet6_config) {
                                interface = subnet6_config->toElement()->get("interface");
                            }
                        }

                        // If the network interface is not found, we return the result to the user with a
                        // status corresponding to an empty value.
                        if (!interface) {
                            ostringstream text;
                            text << "in section " << section << " with id " << subnet_id
                            << " not setted interface.";
                            response = createAnswer(CONTROL_RESULT_EMPTY, text.str(), map);
                            map->set("interfaces", Element::createList());
                            LOG_INFO(interfaces_cmds_logger, INTERFACES_CMDS_INTERFACES_GET_EMPTY)
                            .arg(subnet_id);

                            handle.setArgument("response", response);
                            return;
                        }

                        // If a network interface is found, we create a single-element list, placing the
                        // interface name in it. To maintain compatibility with the global section, a list
                        // is always returned to the user.
                        ElementPtr list = Element::createList();
                        list->add(Element::create(interface->stringValue()));
                        map->set("interfaces", list);
                        ostringstream text;
                        text << "in section " << section << " with id " << subnet_id << " interface found.";
                        response = createAnswer(CONTROL_RESULT_SUCCESS, text.str(), map);
                    }

                } catch (const exception& ex) {
                    LOG_ERROR(interfaces_cmds_logger, INTERFACES_CMDS_INTERFACES_GET_HANDLER_FAILED)
                    .arg(ex.what());

                    response = createAnswer(CONTROL_RESULT_ERROR, ex.what());
                }

                handle.setArgument("response", response);
            }

            /// @brief Processes and returns a response to 'append-interfaces' command.
            ///
            /// @param callout_handle Reference to the callout handle holding command
            /// to be processed and where result should be stored.
            void appendInterfaces(CalloutHandle& handle) {
                ConstElementPtr response;
                ElementPtr map = Element::createMap();

                try {
                    // Validate arguments before starting to manipulate the configuration
                    ConstElementPtr arguments = getMandatoryArguments(handle);
                    validateArguments("append-interfaces", arguments);

                    string section = arguments->get("section")->stringValue();
                    vector<ElementPtr> new_interfaces = arguments->get("interfaces")->listValue();

                    CfgIfacePtr config_interfaces = CfgMgr::instance().getCurrentCfg()->getCfgIface();
                    // If the user wants to add addresses in the global section, we will set them up first
                    // through a loop, and then write the configuration to disk.
                    if (section == "global") {
                        for (auto& el : new_interfaces) {
                            string iface = el->stringValue();
                            config_interfaces->use(family_, iface);
                        }

                        response = this->writeConfiguration();
                    } else if (section == "subnet") {
                        // If the user wishes to set a subnet address, we obtain the subnet ID and the first
                        // element of the array of addresses that he passed for setting.
                        int subnet_id = arguments->get("subnet_id")->intValue();

                        bool is_found_subnet = false;
                        if (family_ == AF_INET) {
                            Subnet4Ptr subnet =
                            CfgMgr::instance().getCurrentCfg()->getCfgSubnets4()->getSubnet(
                                SubnetID(subnet_id));
                            if (subnet) {
                                is_found_subnet = true;
                                subnet->setIface(new_interfaces[0]->stringValue());
                            }
                        } else {
                            Subnet6Ptr subnet =
                            CfgMgr::instance().getCurrentCfg()->getCfgSubnets6()->getSubnet(
                                SubnetID(subnet_id));
                            if (subnet) {
                                is_found_subnet = true;
                                subnet->setIface(new_interfaces[0]->stringValue());
                            }
                        }

                        // If the subnet is not found, we report this with an error status and do not
                        // continue working.
                        if (!is_found_subnet) {
                            ostringstream text;
                            text << "subnet with id " << subnet_id << " not found.";
                            response = createAnswer(CONTROL_RESULT_ERROR, text.str(), map);
                        } else {
                            response = this->writeConfiguration();
                        }
                    }

                } catch (const exception& ex) {
                    LOG_ERROR(interfaces_cmds_logger, INTERFACES_CMDS_INTERFACES_SET_HANDLER_FAILED)
                    .arg(ex.what());

                    response = createAnswer(CONTROL_RESULT_ERROR, ex.what());
                }

                handle.setArgument("response", response);
            }

            /// @brief Processes and returns a response to 'delete-interfaces' command.
            ///
            /// @param callout_handle Reference to the callout handle holding command
            /// to be processed and where result should be stored.
            void deleteInterfaces(CalloutHandle& handle) {
                ConstElementPtr response;
                ElementPtr map = Element::createMap();

                try {
                    // Validate arguments before starting to manipulate the configuration
                    ConstElementPtr arguments = getMandatoryArguments(handle);
                    validateArguments("delete-interfaces", arguments);

                    // First, let's define all the parameters that need to be worked with. The section from
                    // which interfaces are removed, the list of current interfaces in the global
                    // configuration, and the user-specified interfaces to remove.
                    string section = arguments->get("section")->stringValue();
                    ConstElementPtr interfaces_map =
                    CfgMgr::instance().getCurrentCfg()->getCfgIface()->toElement();
                    vector<ElementPtr> current_interfaces = interfaces_map->get("interfaces")->listValue();
                    vector<ElementPtr> delete_interfaces = arguments->get("interfaces")->listValue();

                    CfgMgr::instance().getCurrentCfg()->getCfgIface()->reset();
                    CfgIfacePtr interfaces_config = CfgMgr::instance().getCurrentCfg()->getCfgIface();

                    if (section == "global") {
                        // Since there is no function to delete a specific interface in the configuration
                        // manager API, we clear all interfaces and add only those that were previously
                        // created and not specified by the user for deletion.
                        for (auto& el : current_interfaces) {
                            string iface = el->stringValue();

                            bool is_found = false;
                            for (auto& delete_el : delete_interfaces) {
                                string delete_iface = delete_el->stringValue();
                                if (iface == delete_iface) {
                                    is_found = true;
                                }
                            }

                            if (is_found) {
                                continue;
                            }

                            // If the interface is not specified by the user for deletion, we return it to
                            // the configuration.
                            interfaces_config->use(family_, iface);
                        }

                        response = this->writeConfiguration();

                    } else if (section == "subnet") {
                        // In subnets, we first obtain the subnet ID to be deleted and the interface itself
                        // that the user wants to delete.
                        int subnet_id = arguments->get("subnet_id")->intValue();
                        string delete_interface =
                        arguments->get("interfaces")->listValue()[0]->stringValue();

                        // Two flags determine whether the user-specified subnet exists and whether the
                        // address they specify to delete exists on the subnet.
                        bool is_found_subnet = false;
                        bool is_found_interface = false;
                        if (family_ == AF_INET) {
                            Subnet4Ptr subnet =
                            CfgMgr::instance().getCurrentCfg()->getCfgSubnets4()->getSubnet(
                                SubnetID(subnet_id));
                            if (subnet) {
                                is_found_subnet = true;
                                if (delete_interface == subnet->getIface().valueOr("")) {
                                    is_found_interface = true;
                                    subnet->setIface("");
                                }
                            }
                        } else {
                            Subnet6Ptr subnet =
                            CfgMgr::instance().getCurrentCfg()->getCfgSubnets6()->getSubnet(
                                SubnetID(subnet_id));
                            if (subnet) {
                                is_found_subnet = true;
                                if (delete_interface == subnet->getIface().valueOr("")) {
                                    is_found_interface = true;
                                    subnet->setIface("");
                                }
                            }
                        }

                        ostringstream text;

                        // If the subnet or interface is not found, we inform the user about this and take
                        // no action.
                        if (!is_found_subnet) {
                            text << "subnet with id " << subnet_id << " not found.";
                            response = createAnswer(CONTROL_RESULT_ERROR, text.str(), map);
                        } else if (!is_found_interface) {
                            text << "interface " << delete_interface << " in " << "subnet with id "
                            << subnet_id << " not found.";
                            response = createAnswer(CONTROL_RESULT_ERROR, text.str(), map);
                        } else {
                            response = this->writeConfiguration();
                        }
                    }

                } catch (const exception& ex) {
                    LOG_ERROR(interfaces_cmds_logger, INTERFACES_CMDS_INTERFACES_DELETE_HANDLER_FAILED)
                    .arg(ex.what());

                    response = createAnswer(CONTROL_RESULT_ERROR, ex.what());
                }

                handle.setArgument("response", response);
            }

        private:
            /// @brief Protocol family (IPv4 or IPv6)
            uint16_t family_;
        };

        InterfacesCmds::InterfacesCmds() : impl_(new InterfacesCmdsImpl()) {
        }

        void
        InterfacesCmds::getAvailableInterfaces(CalloutHandle& handle) const {
            impl_->getAvailableInterfaces(handle);
        }

        void
        InterfacesCmds::getInterfaces(CalloutHandle& handle) const {
            impl_->getInterfaces(handle);
        }

        void
        InterfacesCmds::appendInterfaces(CalloutHandle& handle) {
            impl_->appendInterfaces(handle);
        }

        void
        InterfacesCmds::deleteInterfaces(CalloutHandle& handle) {
            impl_->deleteInterfaces(handle);
        }

    }  // namespace interfaces_cmds
}  // namespace isc
