#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test SAM - cmap integration with restart policy"
test_corover_flatiron_enabled=false
test_corover_needle_enabled=true

. inc/common.sh

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

compile_app "$nodes_ip" "sam-test9" "-lsam -lcmap"
run_app "$nodes_ip" 'sam-test9'

exit 0
