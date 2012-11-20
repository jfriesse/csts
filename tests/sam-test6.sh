#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test SAM - warn signal set"

. common.sh

if run "$nodes_ip" 'grep CONFDB /usr/include/corosync/sam.h' &>/dev/null;then
    compile_app "$nodes_ip" "sam-test6" "-lsam"
    run_app "$nodes_ip" 'sam-test6'
fi

exit 0
