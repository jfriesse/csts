#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test SAM - quorum integration"

. common.sh

generate_corosync_conf_cb() {
    sed 's/^[ \t]*#provider: .*$/\tprovider: corosync_votequorum\n\texpected_votes: 1/'
}

if run "$nodes_ip" 'grep CONFDB /usr/include/corosync/sam.h' &>/dev/null;then
    configure_corosync "$nodes_ip"
    start_corosync "$nodes_ip"

    compile_app "$nodes_ip" "sam-test7" "-lsam -lcmap"
    run_app "$nodes_ip" 'sam-test7'
fi

exit 0
