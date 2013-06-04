#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that parsing of file with extra closing brace fails"

. inc/common.sh

generate_corosync_conf_cb() {
    cat ; echo "}"
}

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip" && exit 1
start_corosync "$nodes_ip" 2>&1 | grep -i 'closing brace'

exit 0
