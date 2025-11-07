interfaces_cmds_lib = shared_library(
    'dhcp_interfaces_cmds',
    'interfaces_cmds.cc',
    'interfaces_cmds_log.cc',
    'interfaces_cmds_messages.cc',
    'interfaces_cmds_callouts.cc',
    'version.cc',
    include_directories: [include_directories('.')] + INCLUDES,
    install: true,
    install_dir: HOOKS_PATH,
    install_rpath: INSTALL_RPATH,
    build_rpath: BUILD_RPATH,
    link_with: LIBS_BUILT_SO_FAR,
    name_suffix: 'so',
)
interfaces_cmds_archive = static_library(
    'interfaces_cmds',
    objects: interfaces_cmds_lib.extract_all_objects(recursive: false),
)
