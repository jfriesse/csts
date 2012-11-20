#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test SAM - cmap integration with restart policy"

. common.sh

if run "$nodes_ip" 'grep CONFDB /usr/include/corosync/sam.h' &>/dev/null;then
    configure_corosync "$nodes_ip"
    start_corosync "$nodes_ip"

    compile_app "$nodes_ip" "sam-test9" "-lsam"
    run_app "$nodes_ip" 'sam-test9'
fi

exit 0
