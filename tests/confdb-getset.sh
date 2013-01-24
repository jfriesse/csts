#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test confdb - basic get/set/replace and delete operations"

. inc/common.sh

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

if run "$nodes_ip" 'cat /usr/include/corosync/confdb.h' &>/dev/null;then
    compile_app "$nodes_ip" "confdb-getset" "-lconfdb"
    run_app "$nodes_ip" 'confdb-getset'
else
    compile_app "$nodes_ip" "cmap-getset" "-lcmap"
    run_app "$nodes_ip" 'cmap-getset'
fi

exit 0
