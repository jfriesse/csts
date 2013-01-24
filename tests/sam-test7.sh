#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test SAM - quorum integration"
test_corover_flatiron_enabled=false
test_corover_needle_enabled=true

. inc/common.sh

generate_corosync_conf_cb() {
    sed 's/^[ \t]*#provider: .*$/\tprovider: corosync_votequorum\n\texpected_votes: 1/'
}

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

compile_app "$nodes_ip" "sam-test7" "-lsam -lcmap -pthread"
run_app "$nodes_ip" 'sam-test7'

exit 0
