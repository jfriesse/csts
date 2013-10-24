#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test cpg init in loop for maximum 5 minutes"

. inc/common.sh


configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

compile_app "$nodes_ip" "cpg-init-load" "-lcpg"
run_app "$nodes_ip" "cpg-init-load"

exit 0
