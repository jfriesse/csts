#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that parsing of file with service without name fails"
test_corover_flatiron_enabled=true
test_corover_needle_enabled=false

. inc/common.sh

generate_corosync_conf_cb() {
    cat ; echo "service {"; echo "    ver: 1"; echo "}"
}

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip" && exit 1 || true
cat_corosync_log "$nodes_ip" | grep 'without name key'

exit 0
