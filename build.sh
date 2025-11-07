g++ -I. -I /usr/include/kea -L /usr/lib/kea/lib -fpic -shared -o interfaces_cmds.so \
  interfaces_cmds.cc interfaces_cmds_callouts.cc version.cc interfaces_cmds_messages.cc interfaces_cmds_log.cc  \
  -lkea-dhcpsrv -lkea-dhcp -lkea-hooks -lkea-log -lkea-util -lkea-exceptions -lcurl
