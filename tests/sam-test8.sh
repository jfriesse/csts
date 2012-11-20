#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test SAM - cmap integration with quit policy"

. common.sh

if run "$nodes_ip" 'grep CONFDB /usr/include/corosync/sam.h' &>/dev/null;then
    configure_corosync "$nodes_ip"
    start_corosync "$nodes_ip"

    compile_app "$nodes_ip" "sam-test8" "-lsam -lcmap"
    run_app "$nodes_ip" 'sam-test8'
fi

exit 0
