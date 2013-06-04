#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that ipc doesn't overflow and doesn't overwrite it's own data"
test_max_runtime=240

. inc/common.sh
. inc/cpg-load.sh

configure_corosync "$nodes_ip"
cpg_load_prepare "$nodes_ip"
start_corosync "$nodes_ip"
cpg_load_start "$nodes_ip" "500"
sleep 180
cpg_load_stop "$nodes_ip"
cpg_load_verify "$nodes_ip"
stop_corosync "$nodes_ip"

exit 0
