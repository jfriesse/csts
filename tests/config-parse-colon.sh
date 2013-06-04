#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that parsing of file with ending colons works and colons are not removed"

. inc/common.sh

generate_corosync_conf_cb() {
    cat ; echo "testkey: testvalue::"
}

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip"

[ "`cmap_get \"$nodes_ip\" \"testkey\"`" == "testvalue::" ]

exit 0
