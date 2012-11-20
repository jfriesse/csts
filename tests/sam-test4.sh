#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test SAM - sam_data_store, sam_data_restore and sam_data_getsize"

. common.sh

if run "$nodes_ip" 'grep CONFDB /usr/include/corosync/sam.h' &>/dev/null;then
    compile_app "$nodes_ip" "sam-test4" "-lsam"
    run_app "$nodes_ip" 'sam-test4'
fi

exit 0
