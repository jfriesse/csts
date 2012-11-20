#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test SAM - event driven healtchecking"

. common.sh

if run "$nodes_ip" 'grep CONFDB /usr/include/corosync/sam.h' &>/dev/null;then
    compile_app "$nodes_ip" "sam-test5" "-lsam"
    run_app "$nodes_ip" 'sam-test5'
fi

exit 0
