#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that running corosync-cfgtool doesn't cause stop of logging and doesn't crash corosync"
test_corover_flatiron_enabled=false
test_corover_needle_enabled=true

. inc/common.sh

generate_corosync_conf_cb() {
    cat ; echo "nodelist {"; echo "  node {"; echo "    ring0_addr: $nodes_ip"; echo "    nodeid: 1"; echo "  }"; echo "}"
}

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

run "$nodes_ip" "for ((i=0; i<1024; i++));do corosync-cfgtool -R;done"

stop_corosync "$nodes_ip"

exit 0
