#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test confdb - iteration operations"

. inc/common.sh

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

if run "$nodes_ip" 'cat /usr/include/corosync/confdb.h' &>/dev/null;then
    compile_app "$nodes_ip" "confdb-iter" "-lconfdb"
    run_app "$nodes_ip" 'confdb-iter'
else
    compile_app "$nodes_ip" "cmap-iter" "-lcmap"
    run_app "$nodes_ip" 'cmap-iter'
fi

exit 0
