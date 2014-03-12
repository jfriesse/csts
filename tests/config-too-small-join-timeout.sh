#!/bin/bash
#
# Author: Jan Friesse <jfriesse@redhat.com>
#

test_description="Test that parsing of file with too small join timeout value fails"

. inc/common.sh

generate_corosync_conf_cb() {
    sed '/^[ \t]*crypto_hash:/a \
        join: 1'
}

configure_corosync "$nodes_ip"
start_corosync "$nodes_ip" && exit 1
cat_corosync_log "$nodes_ip" | grep -i 'may not be less then'

exit 0
