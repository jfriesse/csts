#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test confdb - inc/dec operations"

. inc/common.sh

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

if run "$nodes_ip" 'cat /usr/include/corosync/confdb.h' &>/dev/null;then
    compile_app "$nodes_ip" "confdb-incdec" "-lconfdb"
    run_app "$nodes_ip" 'confdb-incdec'
else
    compile_app "$nodes_ip" "cmap-incdec" "-lcmap"
    run_app "$nodes_ip" 'cmap-incdec'
fi

exit 0
