#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test SAM - cmap integration with quit policy"
test_corover_flatiron_enabled=false
test_corover_needle_enabled=true

. inc/common.sh

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

compile_app "$nodes_ip" "sam-test8" "-lsam -lcmap"
run_app "$nodes_ip" 'sam-test8'

exit 0
